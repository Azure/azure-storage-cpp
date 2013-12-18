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
#include "wascore/util.h"

namespace wa { namespace storage { namespace protocol {

#pragma region Common Helpers

    void add_query_if_not_empty(web::http::uri_builder& builder, const utility::string_t& name, const utility::string_t& value)
    {
        if (!value.empty())
        {
            builder.append_query(name + U("=") + web::http::uri::encode_data_string(value), false);
        }
    }

    utility::string_t convert_datetime_if_initialized(const utility::datetime& value)
    {
        return value.is_initialized() ? core::convert_to_string(core::truncate_fractional_seconds(value)) : utility::string_t();
    }

    web::http::uri_builder get_sas_token_builder(const utility::string_t& identifier, const shared_access_policy& policy, const utility::string_t& signature)
    {
        web::http::uri_builder builder;

        add_query_if_not_empty(builder, uri_query_sas_version, header_value_storage_version);
        add_query_if_not_empty(builder, uri_query_sas_identifier, identifier);
        add_query_if_not_empty(builder, uri_query_sas_signature, signature);

        if (policy.is_valid())
        {
            add_query_if_not_empty(builder, uri_query_sas_start, convert_datetime_if_initialized(policy.start()));
            add_query_if_not_empty(builder, uri_query_sas_expiry, convert_datetime_if_initialized(policy.expiry()));
            add_query_if_not_empty(builder, uri_query_sas_permissions, policy.permissions_to_string());
        }

        return builder;
    }

    utility::ostringstream_t get_sas_string_to_sign(const utility::string_t& identifier, const shared_access_policy& policy, const utility::string_t& resource)
    {
        utility::ostringstream_t str;
        str << policy.permissions_to_string() << U('\n');
        str << convert_datetime_if_initialized(policy.start()) << U('\n');
        str << convert_datetime_if_initialized(policy.expiry()) << U('\n');
        str << resource << U('\n');
        str << identifier << U('\n');
        str << header_value_storage_version;
        return str;
    }

    storage_credentials parse_query(const web::http::uri& uri, bool require_signed_resource)
    {
        // Order of the strings in the sas_parameters vector is per MSDN's
        // definition of Shared Access Signatures.
        const utility::string_t sas_parameters[] = {
            protocol::uri_query_sas_version,
            protocol::uri_query_sas_resource,
            protocol::uri_query_sas_table_name,
            protocol::uri_query_sas_start,
            protocol::uri_query_sas_expiry,
            protocol::uri_query_sas_permissions,
            protocol::uri_query_sas_start_partition_key,
            protocol::uri_query_sas_start_row_key,
            protocol::uri_query_sas_end_partition_key,
            protocol::uri_query_sas_end_row_key,
            protocol::uri_query_sas_identifier,
            protocol::uri_query_sas_cache_control,
            protocol::uri_query_sas_content_disposition,
            protocol::uri_query_sas_content_encoding,
            protocol::uri_query_sas_content_language,
            protocol::uri_query_sas_content_type,
            protocol::uri_query_sas_signature,
        };

        const int sas_parameters_size = sizeof(sas_parameters) / sizeof(sas_parameters[0]);

        auto splitted_query = web::http::uri::split_query(uri.query());

        bool params_found = false;
        web::http::uri_builder builder;
        for (int i = 0; i < sas_parameters_size; ++i)
        {
            auto param = splitted_query.find(sas_parameters[i]);
            if (param != splitted_query.end())
            {
                params_found = true;
                add_query_if_not_empty(builder, param->first, param->second);
            }
        }

        if (!params_found)
        {
            return storage_credentials();
        }

        auto signature = splitted_query.find(protocol::uri_query_sas_signature);
        auto signed_resource = splitted_query.find(protocol::uri_query_sas_resource);
        if ((signature == splitted_query.end()) || (require_signed_resource && (signed_resource == splitted_query.end())))
        {
            throw std::invalid_argument(utility::conversions::to_utf8string(protocol::error_missing_params_for_sas));
        }

        return storage_credentials(builder.query());
    }

#pragma endregion

#pragma region Blob SAS Helpers

    utility::string_t get_blob_sas_string_to_sign(const utility::string_t& identifier, const shared_access_policy& policy, const cloud_blob_shared_access_headers& headers, const utility::string_t& resource, const storage_credentials& credentials)
    {
        auto str = get_sas_string_to_sign(identifier, policy, resource);

        str << U('\n') << headers.cache_control();
        str << U('\n') << headers.content_disposition();
        str << U('\n') << headers.content_encoding();
        str << U('\n') << headers.content_language();
        str << U('\n') << headers.content_type();

        return calculate_hmac_sha256_hash(str.str(), credentials);
    }

    utility::string_t get_blob_sas_token(const utility::string_t& identifier, const shared_access_policy& policy, const cloud_blob_shared_access_headers& headers, const utility::string_t& resource_type, const utility::string_t& resource, const storage_credentials& credentials)
    {
        auto signature = get_blob_sas_string_to_sign(identifier, policy, headers, resource, credentials);
        auto builder = get_sas_token_builder(identifier, policy, signature);

        add_query_if_not_empty(builder, uri_query_sas_resource, resource_type);
        add_query_if_not_empty(builder, uri_query_sas_cache_control, headers.cache_control());
        add_query_if_not_empty(builder, uri_query_sas_content_type, headers.content_type());
        add_query_if_not_empty(builder, uri_query_sas_content_encoding, headers.content_encoding());
        add_query_if_not_empty(builder, uri_query_sas_content_language, headers.content_language());
        add_query_if_not_empty(builder, uri_query_sas_content_disposition, headers.content_disposition());

        return U("?") + builder.query();
    }

#pragma endregion

#pragma region Queue SAS Helpers

    utility::string_t get_queue_sas_string_to_sign(const utility::string_t& identifier, const shared_access_policy& policy, const utility::string_t& resource, const storage_credentials& credentials)
    {
        auto str = get_sas_string_to_sign(identifier, policy, resource);

        return calculate_hmac_sha256_hash(str.str(), credentials);
    }

    utility::string_t get_queue_sas_token(const utility::string_t& identifier, const shared_access_policy& policy, const utility::string_t& resource, const storage_credentials& credentials)
    {
        auto signature = get_queue_sas_string_to_sign(identifier, policy, resource, credentials);
        auto builder = get_sas_token_builder(identifier, policy, signature);

        return U("?") + builder.query();
    }

#pragma endregion

#pragma region Table SAS Helpers

    utility::string_t get_table_sas_string_to_sign(const utility::string_t& identifier, const shared_access_policy& policy, const utility::string_t& start_partition_key, const utility::string_t& start_row_key, const utility::string_t& end_partition_key, const utility::string_t& end_row_key, const utility::string_t& resource, const storage_credentials& credentials)
    {
        auto str = get_sas_string_to_sign(identifier, policy, resource);

        str << U('\n') << start_partition_key;
        str << U('\n') << start_row_key;
        str << U('\n') << end_partition_key;
        str << U('\n') << end_row_key;

        return calculate_hmac_sha256_hash(str.str(), credentials);
    }

    utility::string_t get_table_sas_token(const utility::string_t& identifier, const shared_access_policy& policy, const utility::string_t& table_name, const utility::string_t& start_partition_key, const utility::string_t& start_row_key, const utility::string_t& end_partition_key, const utility::string_t& end_row_key, const utility::string_t& resource, const storage_credentials& credentials)
    {
        auto signature = get_table_sas_string_to_sign(identifier, policy, start_partition_key, start_row_key, end_partition_key, end_row_key, resource, credentials);
        auto builder = get_sas_token_builder(identifier, policy, signature);

        add_query_if_not_empty(builder, uri_query_sas_table_name, table_name);
        add_query_if_not_empty(builder, uri_query_sas_start_partition_key, start_partition_key);
        add_query_if_not_empty(builder, uri_query_sas_start_row_key, start_row_key);
        add_query_if_not_empty(builder, uri_query_sas_end_partition_key, end_partition_key);
        add_query_if_not_empty(builder, uri_query_sas_end_row_key, end_row_key);

        return U("?") + builder.query();
    }

#pragma endregion

}}} // namespace wa::storage::protocol
