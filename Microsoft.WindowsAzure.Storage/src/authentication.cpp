// -----------------------------------------------------------------------------------------
// <copyright file="authentication.cpp" company="Microsoft">
//    Copyright 2013 Microsoft Corporation
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//      http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
// </copyright>
// -----------------------------------------------------------------------------------------

#include "stdafx.h"
#include "was/auth.h"
#include "wascore/util.h"
#include "wascore/constants.h"
#include "wascore/logging.h"
#include "wascore/streams.h"

namespace azure { namespace storage { namespace protocol {

    utility::string_t calculate_hmac_sha256_hash(const utility::string_t& string_to_hash, const storage_credentials& credentials)
    {
        std::string utf8_string_to_hash = utility::conversions::to_utf8string(string_to_hash);
        core::hash_provider provider = core::hash_provider::create_hmac_sha256_hash_provider(credentials.account_key());
        provider.write(reinterpret_cast<const uint8_t*>(utf8_string_to_hash.data()), utf8_string_to_hash.size());
        provider.close();
        return provider.hash();
    }

    void sas_authentication_handler::sign_request(web::http::http_request& request, operation_context context) const
    {
        web::http::uri request_uri = request.request_uri();
        request_uri = m_credentials.transform_uri(request_uri);
        request.set_request_uri(request_uri);
    }

    void shared_key_authentication_handler::sign_request(web::http::http_request& request, operation_context context) const
    {
        web::http::http_headers& headers = request.headers();
        headers.add(ms_header_date, utility::datetime::utc_now().to_string());

        if (m_credentials.is_shared_key())
        {
            utility::string_t string_to_sign = m_canonicalizer->canonicalize(request, context);
            
            if (core::logger::instance().should_log(context, client_log_level::log_level_verbose))
            {
                utility::string_t with_dots(string_to_sign);
                std::replace(with_dots.begin(), with_dots.end(), _XPLATSTR('\n'), _XPLATSTR('.'));
                core::logger::instance().log(context, client_log_level::log_level_verbose, _XPLATSTR("StringToSign: ") + with_dots);
            }

            utility::string_t header_value;
            header_value.reserve(256);
            header_value.append(m_canonicalizer->authentication_scheme());
            header_value.append(_XPLATSTR(" "));
            header_value.append(m_credentials.account_name());
            header_value.append(_XPLATSTR(":"));
            header_value.append(calculate_hmac_sha256_hash(string_to_sign, m_credentials));

            headers.add(web::http::header_names::authorization, header_value);
        }
    }

    void canonicalizer_helper::append_resource(bool query_only_comp)
    {
        m_result.append(_XPLATSTR("/"));
        m_result.append(m_account_name);

        web::http::uri uri = m_request.request_uri();
        const utility::string_t& resource = uri.path();
        if (resource.front() != _XPLATSTR('/'))
        {
            m_result.append(_XPLATSTR("/"));
        }

        m_result.append(resource);

        std::map<utility::string_t, utility::string_t> query_map = web::http::uri::split_query(uri.query());
        if (query_only_comp)
        {
            std::map<utility::string_t, utility::string_t>::iterator it = query_map.find(_XPLATSTR("comp"));
            if (it != query_map.end())
            {
                m_result.append(_XPLATSTR("?comp="));
                m_result.append(web::http::uri::decode(it->second));
            }
        }
        else
        {
            // std::map keys are already sorted
            for (std::map<utility::string_t, utility::string_t>::const_iterator it = query_map.cbegin(); it != query_map.cend(); ++it)
            {
                utility::string_t parameter_name = it->first;
                std::transform(parameter_name.begin(), parameter_name.end(), parameter_name.begin(), core::utility_char_tolower);

                m_result.append(_XPLATSTR("\n"));
                m_result.append(parameter_name);
                m_result.append(_XPLATSTR(":"));
                m_result.append(web::http::uri::decode(it->second));
            }
        }
    }

    void canonicalizer_helper::append_header(const utility::string_t& header_name)
    {
        utility::string_t value;
        m_request.headers().match(header_name, value);
        append(value);
    }

    void canonicalizer_helper::append_content_length_header()
    {
        utility::string_t value;
        m_request.headers().match(web::http::header_names::content_length, value);
        if (value == _XPLATSTR("0"))
        {
            value.clear();
        }
        append(value);
    }

    void canonicalizer_helper::append_date_header(bool allow_x_ms_date)
    {
        utility::string_t value;
        if (!m_request.headers().match(ms_header_date, value))
        {
            append_header(web::http::header_names::date);
        }
        else if (allow_x_ms_date)
        {
            append(value);
        }
        else
        {
            append(utility::string_t());
        }
    }

    void canonicalizer_helper::append_x_ms_headers()
    {
        const web::http::http_headers& headers = m_request.headers();
        for (web::http::http_headers::const_iterator it = headers.begin(); it != headers.end(); ++it)
        {
            const utility::char_t *key = it->first.c_str();
            size_t key_size = it->first.size();
            // disables warning 4996 to bypass the usage of std::equal;
            // a more secure usage of std::equal with 5 parameters is supported by c++14.
            // to be compatible with c++11, warning 4996 is disabled.
            if ((key_size > ms_header_prefix_size) &&
                std::equal(ms_header_prefix, ms_header_prefix + ms_header_prefix_size, key, [](const utility::char_t &c1, const utility::char_t &c2) {return c1 == c2;}))
            {
                utility::string_t transformed_key(key);
                std::transform(transformed_key.begin(), transformed_key.end(), transformed_key.begin(), core::utility_char_tolower);
                m_result.append(transformed_key);
                m_result.append(_XPLATSTR(":"));
                append(it->second);
            }
        }
    }

    utility::string_t shared_key_blob_queue_canonicalizer::canonicalize(const web::http::http_request& request, operation_context context) const
    {
        canonicalizer_helper helper(request, m_account_name);
        helper.append(request.method());
        helper.append_header(web::http::header_names::content_encoding);
        helper.append_header(web::http::header_names::content_language);
        helper.append_content_length_header();
        helper.append_header(web::http::header_names::content_md5);
        helper.append_header(web::http::header_names::content_type);
        helper.append_date_header(false);
        helper.append_header(web::http::header_names::if_modified_since);
        helper.append_header(web::http::header_names::if_match);
        helper.append_header(web::http::header_names::if_none_match);
        helper.append_header(web::http::header_names::if_unmodified_since);
        helper.append_header(web::http::header_names::range);
        helper.append_x_ms_headers();
        helper.append_resource(false);
        return helper.str();
    }

    utility::string_t shared_key_lite_blob_queue_canonicalizer::canonicalize(const web::http::http_request& request, operation_context context) const
    {
        canonicalizer_helper helper(request, m_account_name);
        helper.append(request.method());
        helper.append_header(web::http::header_names::content_md5);
        helper.append_header(web::http::header_names::content_type);
        helper.append_date_header(false);
        helper.append_x_ms_headers();
        helper.append_resource(true);
        return helper.str();
    }

    utility::string_t shared_key_table_canonicalizer::canonicalize(const web::http::http_request& request, operation_context context) const
    {
        canonicalizer_helper helper(request, m_account_name);
        helper.append(request.method());
        helper.append_header(web::http::header_names::content_md5);
        helper.append_header(web::http::header_names::content_type);
        helper.append_date_header(true);
        helper.append_resource(true);
        return helper.str();
    }

    utility::string_t shared_key_lite_table_canonicalizer::canonicalize(const web::http::http_request& request, operation_context context) const
    {
        canonicalizer_helper helper(request, m_account_name);
        helper.append_date_header(true);
        helper.append_resource(true);
        return helper.str();
    }

}}} // namespace azure::storage::protocol
