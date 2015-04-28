// -----------------------------------------------------------------------------------------
// <copyright file="cloud_queue_client.cpp" company="Microsoft">
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
#include "wascore/protocol.h"
#include "wascore/protocol_xml.h"
#include "wascore/util.h"
#include "was/queue.h"

namespace azure { namespace storage {

    queue_request_options cloud_queue_client::get_modified_options(const queue_request_options& options) const
    {
        queue_request_options modified_options(options);
        modified_options.apply_defaults(default_request_options());
        return modified_options;
    }

    queue_result_iterator cloud_queue_client::list_queues(const utility::string_t& prefix, bool get_metadata, utility::size64_t max_results, const queue_request_options& options, operation_context context) const
    {
        auto instance = std::make_shared<cloud_queue_client>(*this);
        return queue_result_iterator(
            [instance, prefix, get_metadata, options, context](const continuation_token& token, size_t max_results_per_segment)
        {
            return instance->list_queues_segmented(prefix, get_metadata, (int)max_results_per_segment, token, options, context);
        },
            max_results, 0);
    }

    pplx::task<queue_result_segment> cloud_queue_client::list_queues_segmented_async(const utility::string_t& prefix, bool get_metadata, int max_results, const continuation_token& token, const queue_request_options& options, operation_context context) const
    {
        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_uri(*this, prefix, get_metadata, max_results, token);

        auto instance = std::make_shared<cloud_queue_client>(*this);
        std::shared_ptr<core::storage_command<queue_result_segment>> command = std::make_shared<core::storage_command<queue_result_segment>>(uri);
        command->set_build_request(std::bind(protocol::list_queues, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary, token.target_location());
        command->set_preprocess_response(std::bind(protocol::preprocess_response<queue_result_segment>, queue_result_segment(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_postprocess_response([instance, get_metadata] (const web::http::http_response& response, const request_result& result, const core::ostream_descriptor&, operation_context context) -> pplx::task<queue_result_segment>
        {
            UNREFERENCED_PARAMETER(context);
            protocol::list_queues_reader reader(response.body());
            std::vector<protocol::cloud_queue_list_item> queue_items = reader.move_items();

            std::vector<cloud_queue> results;
            results.reserve(queue_items.size());

            for (std::vector<protocol::cloud_queue_list_item>::iterator it = queue_items.begin(); it != queue_items.end(); ++it)
            {
                cloud_queue queue = instance->get_queue_reference(it->move_name());
                if (get_metadata)
                {
                    queue.set_metadata(it->move_metadata());
                }

                results.push_back(std::move(queue));
            }

            utility::string_t next_marker = reader.move_next_marker();
            if (!next_marker.empty())
            {
                next_marker = core::make_query_parameter(protocol::queue_query_next_marker, next_marker);
            }

            azure::storage::continuation_token next_token(std::move(next_marker));
            next_token.set_target_location(result.target_location());

            queue_result_segment result_segment(std::move(results), std::move(next_token));
            return pplx::task_from_result(result_segment);
        });
        return core::executor<queue_result_segment>::execute_async(command, modified_options, context);
    }

    pplx::task<service_properties> cloud_queue_client::download_service_properties_async(const queue_request_options& options, operation_context context) const
    {
        queue_request_options modified_options(options);
        modified_options.apply_defaults(default_request_options());

        return download_service_properties_base_async(modified_options, context);
    }

    pplx::task<void> cloud_queue_client::upload_service_properties_async(const service_properties& properties, const service_properties_includes& includes, const queue_request_options& options, operation_context context) const
    {
        queue_request_options modified_options(options);
        modified_options.apply_defaults(default_request_options());

        return upload_service_properties_base_async(properties, includes, modified_options, context);
    }

    pplx::task<service_stats> cloud_queue_client::download_service_stats_async(const queue_request_options& options, operation_context context) const
    {
        queue_request_options modified_options(options);
        modified_options.apply_defaults(default_request_options());

        return download_service_stats_base_async(modified_options, context);
    }

    cloud_queue cloud_queue_client::get_queue_reference(utility::string_t queue_name) const
    {
        cloud_queue queue(*this, std::move(queue_name));
        return queue;
    }

    void cloud_queue_client::set_authentication_scheme(azure::storage::authentication_scheme value)
    {
        cloud_client::set_authentication_scheme(value);

        storage_credentials creds = credentials();
        if (creds.is_shared_key())
        {
            utility::string_t account_name = creds.account_name();
            switch (authentication_scheme())
            {
            case azure::storage::authentication_scheme::shared_key_lite:
                set_authentication_handler(std::make_shared<protocol::shared_key_authentication_handler>(std::make_shared<protocol::shared_key_lite_blob_queue_canonicalizer>(std::move(account_name)), std::move(creds)));
                break;

            default: // azure::storage::authentication_scheme::shared_key
                set_authentication_handler(std::make_shared<protocol::shared_key_authentication_handler>(std::make_shared<protocol::shared_key_blob_queue_canonicalizer>(std::move(account_name)), std::move(creds)));
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
