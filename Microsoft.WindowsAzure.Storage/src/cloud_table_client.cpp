// -----------------------------------------------------------------------------------------
// <copyright file="cloud_table_client.cpp" company="Microsoft">
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
#include "was/table.h"
#include "wascore/util.h"

namespace azure { namespace storage {

    table_request_options cloud_table_client::get_modified_options(const table_request_options& options) const
    {
        table_request_options modified_options(options);
        modified_options.apply_defaults(default_request_options());
        return modified_options;
    }

    table_result_iterator cloud_table_client::list_tables(const utility::string_t& prefix, utility::size64_t max_results, const table_request_options& options, operation_context context) const
    {
        auto instance = std::make_shared<cloud_table_client>(*this);
        return table_result_iterator(
            [instance, prefix, options, context](const continuation_token& token, size_t max_results_per_segment)
        {
            return instance->list_tables_segmented(prefix, (int)max_results_per_segment, token, options, context);
        },
            max_results, 0);
    }

    pplx::task<table_result_segment> cloud_table_client::list_tables_segmented_async(const utility::string_t& prefix, int max_results, const continuation_token& token, const table_request_options& options, operation_context context) const
    {
        table_request_options modified_options = get_modified_options(options);

        // This operation is processed internally and it does not need metadata because all property types are known
        modified_options.set_payload_format(table_payload_format::json_no_metadata);

        cloud_table table = get_table_reference(_XPLATSTR("Tables"));
        table_query query;

        if (max_results > 0)
        {
            query.set_take_count(max_results);
        }

        if (!prefix.empty())
        {
            utility::string_t filter_string = table_query::combine_filter_conditions(
                table_query::generate_filter_condition(_XPLATSTR("TableName"), query_comparison_operator::greater_than_or_equal, prefix), 
                query_logical_operator::op_and, 
                table_query::generate_filter_condition(_XPLATSTR("TableName"), query_comparison_operator::less_than, prefix + _XPLATSTR('{')));
            query.set_filter_string(filter_string);
        }

        auto instance = std::make_shared<cloud_table_client>(*this);
        return table.execute_query_segmented_async(query, token, modified_options, context).then([instance] (table_query_segment query_segment) -> table_result_segment
        {
            std::vector<table_entity> query_results = query_segment.results();

            std::vector<cloud_table> table_results;
            table_results.reserve(query_results.size());

            // TODO: Consider making this an independent operation instead of using execute_query_segmented to avoid creating intermediary table_entity objects
            for (std::vector<table_entity>::const_iterator itr = query_results.cbegin(); itr != query_results.cend(); ++itr)
            {
                table_entity entity = *itr;

                utility::string_t table_name = entity.properties()[_XPLATSTR("TableName")].string_value();
                cloud_table current_table = instance->get_table_reference(std::move(table_name));
                table_results.push_back(std::move(current_table));
            }

            table_result_segment result_segment(std::move(table_results), query_segment.continuation_token());
            return result_segment;
        });
    }

    pplx::task<service_properties> cloud_table_client::download_service_properties_async(const table_request_options& options, operation_context context) const
    {
        table_request_options modified_options = get_modified_options(options);

        return download_service_properties_base_async(modified_options, context);
    }

    pplx::task<void> cloud_table_client::upload_service_properties_async(const service_properties& properties, const service_properties_includes& includes, const table_request_options& options, operation_context context) const
    {
        table_request_options modified_options = get_modified_options(options);

        return upload_service_properties_base_async(properties, includes, modified_options, context);
    }

    pplx::task<service_stats> cloud_table_client::download_service_stats_async(const table_request_options& options, operation_context context) const
    {
        table_request_options modified_options = get_modified_options(options);

        return download_service_stats_base_async(modified_options, context);
    }

    cloud_table cloud_table_client::get_table_reference(utility::string_t table_name) const
    {
        cloud_table table(*this, std::move(table_name));
        return table;
    }

    void cloud_table_client::set_authentication_scheme(azure::storage::authentication_scheme value)
    {
        cloud_client::set_authentication_scheme(value);

        storage_credentials creds = credentials();
        if (creds.is_shared_key())
        {
            utility::string_t account_name = creds.account_name();
            switch (authentication_scheme())
            {
            case azure::storage::authentication_scheme::shared_key_lite:
                set_authentication_handler(std::make_shared<protocol::shared_key_authentication_handler>(std::make_shared<protocol::shared_key_lite_table_canonicalizer>(std::move(account_name)), std::move(creds)));
                break;

            default: // azure::storage::authentication_scheme::shared_key
                set_authentication_handler(std::make_shared<protocol::shared_key_authentication_handler>(std::make_shared<protocol::shared_key_table_canonicalizer>(std::move(account_name)), std::move(creds)));
                break;
            }
        }
        else if (creds.is_sas())
        {
            set_authentication_handler(std::make_shared<protocol::sas_authentication_handler>(std::move(creds)));
        }
        else
        {
            set_authentication_handler(std::make_shared<protocol::authentication_handler>());
        }
    }

}} // namespace azure::storage
