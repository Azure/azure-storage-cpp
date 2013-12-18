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
#include "wascore/constants.h"
#include "wascore/logging.h"
#include "wascore/streams.h"

namespace wa { namespace storage { namespace protocol {

    utility::string_t calculate_hmac_sha256_hash(const utility::string_t& string_to_hash, const storage_credentials& credentials)
    {
        auto utf8_string_to_hash = utility::conversions::to_utf8string(string_to_hash);
        auto hash_streambuf = core::hash_hmac_sha256_streambuf(credentials.account_key());
        hash_streambuf.putn(reinterpret_cast<const uint8_t*>(utf8_string_to_hash.data()), utf8_string_to_hash.size()).wait();
        hash_streambuf.close().wait();
        return utility::conversions::to_base64(hash_streambuf.hash());
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
                std::replace(with_dots.begin(), with_dots.end(), U('\n'), U('.'));
                core::logger::instance().log(context, client_log_level::log_level_verbose, U("StringToSign: ") + with_dots);
            }

            utility::ostringstream_t header_value;
            header_value << m_canonicalizer->authentication_scheme() << U(" ") << m_credentials.account_name() << U(":") << calculate_hmac_sha256_hash(string_to_sign, m_credentials);

            headers.add(web::http::header_names::authorization, header_value.str());
        }
    }

    void canonicalizer_helper::append_resource(bool query_only_comp)
    {
        m_result << U("/") << m_account_name;

        auto uri = m_request.request_uri();
        auto& resource = uri.path();
        if (resource.front() != U('/'))
        {
            m_result << U("/");
        }

        m_result << resource;

        auto query_map = web::http::uri::split_query(web::http::uri::decode(uri.query()));
        if (query_only_comp)
        {
            auto it = query_map.find(U("comp"));
            if (it != query_map.end())
            {
                m_result << U("?comp=") << it->second;
            }
        }
        else
        {
            // std::map keys are already sorted
            for (auto iter = query_map.cbegin(); iter != query_map.cend(); ++iter)
            {
                auto name = iter->first;
                std::transform(name.begin(), name.end(), name.begin(), tolower);

                m_result << U("\n") << name << U(":") << iter->second;
            }
        }
    }

    void canonicalizer_helper::append_header(const utility::string_t& header_name)
    {
        utility::string_t value;
        m_request.headers().match(header_name, value);
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
        auto& headers = m_request.headers();
        for (auto iter = headers.begin(); iter != headers.end(); ++iter)
        {
            auto key = iter->first;
            if ((key.size() > ms_header_prefix.size()) &&
                std::equal(ms_header_prefix.cbegin(), ms_header_prefix.cend(), key.cbegin()))
            {
                std::transform(key.begin(), key.end(), key.begin(), tolower);
                m_result << key << U(":");
                append(iter->second);
            }
        }
    }

    utility::string_t shared_key_blob_queue_canonicalizer::canonicalize(const web::http::http_request& request, operation_context context) const
    {
        canonicalizer_helper helper(request, m_account_name);
        helper.append(request.method());
        helper.append_header(web::http::header_names::content_encoding);
        helper.append_header(web::http::header_names::content_language);
        helper.append_header(web::http::header_names::content_length);
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

}}} // namespace wa::storage::protocol
