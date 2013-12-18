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

namespace wa { namespace storage {

    queue_request_options cloud_queue_client::get_modified_options(const queue_request_options& options) const
    {
        queue_request_options modified_options(options);
        modified_options.apply_defaults(default_request_options());
        return modified_options;
    }

    pplx::task<std::vector<cloud_queue>> cloud_queue_client::list_queues_async(const utility::string_t& prefix, bool get_metadata, const queue_request_options& options, operation_context context) const
    {
        std::shared_ptr<std::vector<cloud_queue>> results = std::make_shared<std::vector<cloud_queue>>();
        std::shared_ptr<continuation_token> continuation_token = std::make_shared<wa::storage::continuation_token>();

        return pplx::details::do_while([this, results, prefix, get_metadata, continuation_token, options, context] () mutable -> pplx::task<bool>
        {
            return list_queues_segmented_async(prefix, get_metadata, -1, *continuation_token, options, context).then([results, continuation_token] (queue_result_segment result_segment) mutable -> bool
            {
                std::vector<wa::storage::cloud_queue> partial_results = result_segment.results();
                results->insert(results->end(), partial_results.begin(), partial_results.end());
                *continuation_token = result_segment.continuation_token();
                return !continuation_token->empty();
            });
        }).then([results] (bool guard) -> pplx::task<std::vector<cloud_queue>>
        {
            return pplx::task_from_result(*results);
        });
    }

    pplx::task<queue_result_segment> cloud_queue_client::list_queues_segmented_async(const utility::string_t& prefix, bool get_metadata, int max_results, const continuation_token& continuation_token, const queue_request_options& options, operation_context context) const
    {
        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_uri(*this, prefix, get_metadata, max_results, continuation_token);

        std::shared_ptr<core::storage_command<queue_result_segment>> command = std::make_shared<core::storage_command<queue_result_segment>>(uri);
        command->set_build_request(std::bind(protocol::list_queues, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary, continuation_token.target_location());
        command->set_preprocess_response(std::bind(protocol::preprocess_response<queue_result_segment>, queue_result_segment(), std::placeholders::_1, std::placeholders::_2));
        command->set_postprocess_response([this, get_metadata] (const web::http::http_response& response, const request_result& result, const core::ostream_descriptor&, operation_context context) -> pplx::task<queue_result_segment>
        {
            protocol::list_queues_reader reader(response.body());
            std::vector<protocol::cloud_queue_list_item> queue_items = reader.extract_items();

            std::vector<cloud_queue> results;
            results.reserve(queue_items.size());

            for (std::vector<protocol::cloud_queue_list_item>::const_iterator itr = queue_items.cbegin(); itr != queue_items.cend(); ++itr)
            {
                cloud_queue queue(*this, itr->name());
                if (get_metadata)
                {
                    queue.set_metadata(itr->metadata());
                }

                results.push_back(queue);
            }

            wa::storage::continuation_token next_token(reader.extract_next_marker());
            next_token.set_target_location(result.target_location());

            queue_result_segment result_segment;
            result_segment.set_results(results);
            result_segment.set_continuation_token(next_token);

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

    cloud_queue cloud_queue_client::get_queue_reference(const utility::string_t& queue_name) const
    {
        cloud_queue queue(*this, queue_name);
        return queue;
    }

    void cloud_queue_client::set_authentication_scheme(wa::storage::authentication_scheme value)
    {
        cloud_client::set_authentication_scheme(value);

        storage_credentials creds = credentials();
        if (creds.is_shared_key())
        {
            switch (authentication_scheme())
            {
            case wa::storage::authentication_scheme::shared_key_lite:
                set_authentication_handler(std::make_shared<protocol::shared_key_authentication_handler>(std::make_shared<protocol::shared_key_lite_blob_queue_canonicalizer>(creds.account_name()), creds));
                break;

            default: // wa::storage::authentication_scheme::shared_key
                set_authentication_handler(std::make_shared<protocol::shared_key_authentication_handler>(std::make_shared<protocol::shared_key_blob_queue_canonicalizer>(creds.account_name()), creds));
                break;
            }
        }
        else if (creds.is_sas())
        {
            set_authentication_handler(std::make_shared<protocol::sas_authentication_handler>(creds));
        }
        else
        {
            set_authentication_handler(std::make_shared<protocol::authentication_handler>());
        }
    }

}} // namespace wa::storage
