// -----------------------------------------------------------------------------------------
// <copyright file="shared_access_signature.cpp" company="Microsoft">
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
#include "wascore/streams.h"
#include "was/blob.h"
#include "was/queue.h"
#include "was/table.h"
#include "wascore/logging.h"
#include "wascore/util.h"
#include "wascore/resources.h"

namespace azure { namespace storage { namespace protocol {

#pragma region Common Helpers

    void add_query_if_not_empty(web::http::uri_builder& builder, const utility::string_t& name, const utility::string_t& value, bool do_encoding)
    {
        if (!value.empty())
        {
            builder.append_query(core::make_query_parameter(name, value, do_encoding));
        }
    }

    utility::string_t convert_datetime_if_initialized(utility::datetime value)
    {
        return value.is_initialized() ? core::truncate_fractional_seconds(value).to_string(utility::datetime::ISO_8601) : utility::string_t();
    }

    void log_sas_string_to_sign(const utility::string_t& string_to_sign)
    {
        operation_context context;
        if (core::logger::instance().should_log(context, client_log_level::log_level_verbose))
        {
            utility::string_t with_dots(string_to_sign);
            std::replace(with_dots.begin(), with_dots.end(), U('\n'), U('.'));
            core::logger::instance().log(context, client_log_level::log_level_verbose, U("StringToSign: ") + with_dots);
        }
    }

    web::http::uri_builder get_sas_token_builder(const utility::string_t& identifier, const shared_access_policy& policy, const utility::string_t& signature)
    {
        web::http::uri_builder builder;

        add_query_if_not_empty(builder, uri_query_sas_version, header_value_storage_version, /* do_encoding */ true);
        add_query_if_not_empty(builder, uri_query_sas_identifier, identifier, /* do_encoding */ true);
        add_query_if_not_empty(builder, uri_query_sas_signature, signature, /* do_encoding */ true);

        if (policy.is_valid())
        {
            add_query_if_not_empty(builder, uri_query_sas_start, convert_datetime_if_initialized(policy.start()), /* do_encoding */ true);
            add_query_if_not_empty(builder, uri_query_sas_expiry, convert_datetime_if_initialized(policy.expiry()), /* do_encoding */ true);
            add_query_if_not_empty(builder, uri_query_sas_permissions, policy.permissions_to_string(), /* do_encoding */ true);
        }

        return builder;
    }

    void get_sas_string_to_sign(utility::ostringstream_t& str, const utility::string_t& identifier, const shared_access_policy& policy, const utility::string_t& resource)
    {
        str << policy.permissions_to_string() << U('\n');
        str << convert_datetime_if_initialized(policy.start()) << U('\n');
        str << convert_datetime_if_initialized(policy.expiry()) << U('\n');
        str << resource << U('\n');
        str << identifier << U('\n');
        str << header_value_storage_version;
    }

    storage_credentials parse_query(const web::http::uri& uri, bool require_signed_resource)
    {
        bool sas_parameter_found = false;
        auto splitted_query = web::http::uri::split_query(uri.query());
        std::vector<utility::string_t> remove_list;
        for (auto iter = splitted_query.cbegin(); iter != splitted_query.cend(); ++iter)
        {
            utility::string_t query_key = iter->first;
            std::transform(query_key.begin(), query_key.end(), query_key.begin(), core::utility_char_tolower);

            if (query_key == protocol::uri_query_sas_signature)
            {
                sas_parameter_found = true;
            }
            else if (query_key == protocol::uri_query_resource_type ||
                query_key == protocol::uri_query_component ||
                query_key == protocol::uri_query_snapshot ||
                query_key == protocol::uri_query_sas_api_version)
            {
                remove_list.push_back(iter->first);
            }
        }

        if (!sas_parameter_found)
        {
            return storage_credentials();
        }

        for (auto remove_param : remove_list)
        {
            splitted_query.erase(remove_param);
        }

        auto signed_resource = splitted_query.find(protocol::uri_query_sas_resource);
        if (require_signed_resource && signed_resource == splitted_query.end())
        {
            throw std::invalid_argument(protocol::error_missing_params_for_sas);
        }

        web::http::uri_builder builder;
        for (auto iter = splitted_query.cbegin(); iter != splitted_query.cend(); ++iter)
        {
            add_query_if_not_empty(builder, iter->first, iter->second, /* do_encoding */ false);
        }

        return storage_credentials(builder.query());
    }

#pragma endregion

#pragma region Blob SAS Helpers

    utility::string_t get_blob_sas_string_to_sign(const utility::string_t& identifier, const shared_access_policy& policy, const cloud_blob_shared_access_headers& headers, const utility::string_t& resource, const storage_credentials& credentials)
    {
        //// StringToSign =      signedpermissions + "\n" +
        ////                     signedstart + "\n" +
        ////                     signedexpiry + "\n" +
        ////                     canonicalizedresource + "\n" +
        ////                     signedidentifier + "\n" +
        ////                     signedversion + "\n" +
        ////                     cachecontrol + "\n" +
        ////                     contentdisposition + "\n" +
        ////                     contentencoding + "\n" +
        ////                     contentlanguage + "\n" +
        ////                     contenttype
        ////
        //// HMAC-SHA256(UTF8.Encode(StringToSign))
        ////
        //// Note that the final five headers are invalid for the 2012-02-12 version.

        utility::ostringstream_t str;
        get_sas_string_to_sign(str, identifier, policy, resource);
        str << U('\n') << headers.cache_control();
        str << U('\n') << headers.content_disposition();
        str << U('\n') << headers.content_encoding();
        str << U('\n') << headers.content_language();
        str << U('\n') << headers.content_type();

        auto string_to_sign = str.str();
        log_sas_string_to_sign(string_to_sign);

        return calculate_hmac_sha256_hash(string_to_sign, credentials);
    }

    utility::string_t get_blob_sas_token(const utility::string_t& identifier, const shared_access_policy& policy, const cloud_blob_shared_access_headers& headers, const utility::string_t& resource_type, const utility::string_t& resource, const storage_credentials& credentials)
    {
        auto signature = get_blob_sas_string_to_sign(identifier, policy, headers, resource, credentials);
        auto builder = get_sas_token_builder(identifier, policy, signature);

        add_query_if_not_empty(builder, uri_query_sas_resource, resource_type, /* do_encoding */ true);
        add_query_if_not_empty(builder, uri_query_sas_cache_control, headers.cache_control(), /* do_encoding */ true);
        add_query_if_not_empty(builder, uri_query_sas_content_type, headers.content_type(), /* do_encoding */ true);
        add_query_if_not_empty(builder, uri_query_sas_content_encoding, headers.content_encoding(), /* do_encoding */ true);
        add_query_if_not_empty(builder, uri_query_sas_content_language, headers.content_language(), /* do_encoding */ true);
        add_query_if_not_empty(builder, uri_query_sas_content_disposition, headers.content_disposition(), /* do_encoding */ true);

        return builder.query();
    }

#pragma endregion

#pragma region Queue SAS Helpers

    utility::string_t get_queue_sas_string_to_sign(const utility::string_t& identifier, const shared_access_policy& policy, const utility::string_t& resource, const storage_credentials& credentials)
    {
        //// StringToSign =      signedpermissions + "\n" +
        ////                     signedstart + "\n" +
        ////                     signedexpiry + "\n" +
        ////                     canonicalizedresource + "\n" +
        ////                     signedidentifier + "\n" +
        ////                     signedversion
        ////
        //// HMAC-SHA256(UTF8.Encode(StringToSign))

        utility::ostringstream_t str;
        get_sas_string_to_sign(str, identifier, policy, resource);

        auto string_to_sign = str.str();
        log_sas_string_to_sign(string_to_sign);

        return calculate_hmac_sha256_hash(string_to_sign, credentials);
    }

    utility::string_t get_queue_sas_token(const utility::string_t& identifier, const shared_access_policy& policy, const utility::string_t& resource, const storage_credentials& credentials)
    {
        auto signature = get_queue_sas_string_to_sign(identifier, policy, resource, credentials);
        auto builder = get_sas_token_builder(identifier, policy, signature);

        return builder.query();
    }

#pragma endregion

#pragma region Table SAS Helpers

    utility::string_t get_table_sas_string_to_sign(const utility::string_t& identifier, const shared_access_policy& policy, const utility::string_t& start_partition_key, const utility::string_t& start_row_key, const utility::string_t& end_partition_key, const utility::string_t& end_row_key, const utility::string_t& resource, const storage_credentials& credentials)
    {
        //// StringToSign =      signedpermissions + "\n" +
        ////                     signedstart + "\n" +
        ////                     signedexpiry + "\n" +
        ////                     canonicalizedresource + "\n" +
        ////                     signedidentifier + "\n" +
        ////                     signedversion + "\n" +
        ////                     startpk + "\n" +
        ////                     startrk + "\n" +
        ////                     endpk + "\n" +
        ////                     endrk
        ////
        //// HMAC-SHA256(UTF8.Encode(StringToSign))

        utility::ostringstream_t str;
        get_sas_string_to_sign(str, identifier, policy, resource);
        str << U('\n') << start_partition_key;
        str << U('\n') << start_row_key;
        str << U('\n') << end_partition_key;
        str << U('\n') << end_row_key;

        auto string_to_sign = str.str();
        log_sas_string_to_sign(string_to_sign);

        return calculate_hmac_sha256_hash(string_to_sign, credentials);
    }

    utility::string_t get_table_sas_token(const utility::string_t& identifier, const shared_access_policy& policy, const utility::string_t& table_name, const utility::string_t& start_partition_key, const utility::string_t& start_row_key, const utility::string_t& end_partition_key, const utility::string_t& end_row_key, const utility::string_t& resource, const storage_credentials& credentials)
    {
        auto signature = get_table_sas_string_to_sign(identifier, policy, start_partition_key, start_row_key, end_partition_key, end_row_key, resource, credentials);
        auto builder = get_sas_token_builder(identifier, policy, signature);

        add_query_if_not_empty(builder, uri_query_sas_table_name, table_name, /* do_encoding */ true);
        add_query_if_not_empty(builder, uri_query_sas_start_partition_key, start_partition_key, /* do_encoding */ true);
        add_query_if_not_empty(builder, uri_query_sas_start_row_key, start_row_key, /* do_encoding */ true);
        add_query_if_not_empty(builder, uri_query_sas_end_partition_key, end_partition_key, /* do_encoding */ true);
        add_query_if_not_empty(builder, uri_query_sas_end_row_key, end_row_key, /* do_encoding */ true);

        return builder.query();
    }

#pragma endregion

}}} // namespace azure::storage::protocol
