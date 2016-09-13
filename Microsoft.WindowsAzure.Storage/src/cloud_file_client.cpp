// -----------------------------------------------------------------------------------------
// <copyright file="cloud_file_client.cpp" company="Microsoft">
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
#include "was/file.h"
#include "wascore/protocol.h"
#include "wascore/protocol_xml.h"

namespace azure { namespace storage {

    share_result_iterator cloud_file_client::list_shares(const utility::string_t& prefix, bool get_metadata, int max_results, const file_request_options& options, operation_context context)
    {
        auto instance = std::make_shared<cloud_file_client>(*this);
        return share_result_iterator(
            [instance, prefix, get_metadata, options, context](const continuation_token& token, size_t max_results_per_segment)
        {
            return instance->list_shares_segmented(prefix, get_metadata, static_cast<int>(max_results_per_segment), token, options, context);
        },
            max_results, 0);
    }

    pplx::task<share_result_segment> cloud_file_client::list_shares_segmented_async(const utility::string_t& prefix, bool get_metadata, int max_results, const continuation_token& token, const file_request_options& options, operation_context context)
    {
        file_request_options modified_options(options);
        modified_options.apply_defaults(default_request_options());

        auto client = *this;

        auto command = std::make_shared<core::storage_command<share_result_segment>>(base_uri());
        command->set_build_request(std::bind(protocol::list_shares, prefix, get_metadata, max_results, token, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary, token.target_location());
        command->set_preprocess_response(std::bind(protocol::preprocess_response<share_result_segment>, share_result_segment(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_postprocess_response([this](const web::http::http_response& response, const request_result& result, const core::ostream_descriptor&, operation_context context) -> pplx::task<share_result_segment>
        {
            protocol::list_shares_reader reader(response.body());

            std::vector<protocol::cloud_file_share_list_item> items(reader.move_items());
            std::vector<cloud_file_share> results;
            results.reserve(items.size());

            for (std::vector<protocol::cloud_file_share_list_item>::iterator iter = items.begin(); iter != items.end(); ++iter)
            {
                results.push_back(cloud_file_share(iter->move_name(), *this, iter->move_properties(), iter->move_metadata()));
            }

            continuation_token next_token(reader.move_next_marker());
            next_token.set_target_location(result.target_location());
            return pplx::task_from_result(share_result_segment(std::move(results), next_token));
        });
        return core::executor<share_result_segment>::execute_async(command, modified_options, context);
    }

    void cloud_file_client::set_authentication_scheme(azure::storage::authentication_scheme value)
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

    pplx::task<service_properties> cloud_file_client::download_service_properties_async(const file_request_options& options, operation_context context) const
    {
        file_request_options modified_options(options);
        modified_options.apply_defaults(default_request_options());

        return download_service_properties_base_async(options, context);
    }

    pplx::task<void> cloud_file_client::upload_service_properties_async(const service_properties& properties, const service_properties_includes& includes, const file_request_options& options, operation_context context) const
    {
        file_request_options modified_options(options);
        modified_options.apply_defaults(default_request_options());

        return upload_service_properties_base_async(properties, includes, options, context);
    }

    WASTORAGE_API cloud_file_share cloud_file_client::get_share_reference(utility::string_t share_name) const
    {
        return cloud_file_share(std::move(share_name), *this);
    }

}}
