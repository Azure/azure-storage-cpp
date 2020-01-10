// -----------------------------------------------------------------------------------------
// <copyright file="cloud_blob_client.cpp" company="Microsoft">
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
#include "was/blob.h"
#include "wascore/protocol.h"
#include "wascore/protocol_xml.h"

namespace azure { namespace storage {

    container_result_iterator cloud_blob_client::list_containers(const utility::string_t& prefix, container_listing_details::values includes, int max_results, const blob_request_options& options, operation_context context) const
    {
        auto instance = std::make_shared<cloud_blob_client>(*this);
        return container_result_iterator(
            [instance, prefix, includes, options, context](const continuation_token& token, size_t max_results_per_segment)
        {
            return instance->list_containers_segmented(prefix, includes, (int)max_results_per_segment, token, options, context);
        },
            max_results, 0);
    }

    pplx::task<container_result_segment> cloud_blob_client::list_containers_segmented_async(const utility::string_t& prefix, container_listing_details::values includes, int max_results, const continuation_token& token, const blob_request_options& options, operation_context context, const pplx::cancellation_token& cancellation_token) const
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(default_request_options(), blob_type::unspecified);

        auto client = *this;

        auto command = std::make_shared<core::storage_command<container_result_segment>>(base_uri(), cancellation_token, modified_options.is_maximum_execution_time_customized());
        command->set_build_request(std::bind(protocol::list_containers, prefix, includes, max_results, token, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary, token.target_location());
        command->set_preprocess_response(std::bind(protocol::preprocess_response<container_result_segment>, container_result_segment(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_postprocess_response([client] (const web::http::http_response& response, const request_result& result, const core::ostream_descriptor&, operation_context context) -> pplx::task<container_result_segment>
        {
            protocol::list_containers_reader reader(response.body());

            std::vector<protocol::cloud_blob_container_list_item> items(reader.move_items());
            std::vector<cloud_blob_container> results;
            results.reserve(items.size());

            for (std::vector<protocol::cloud_blob_container_list_item>::iterator iter = items.begin(); iter != items.end(); ++iter)
            {
                results.push_back(cloud_blob_container(iter->move_name(), client, iter->move_properties(), iter->move_metadata()));
            }

            continuation_token next_token(reader.move_next_marker());
            next_token.set_target_location(result.target_location());
            return pplx::task_from_result(container_result_segment(std::move(results), next_token));
        });
        return core::executor<container_result_segment>::execute_async(command, modified_options, context);
    }

    list_blob_item_iterator cloud_blob_client::list_blobs(const utility::string_t& prefix, bool use_flat_blob_listing, blob_listing_details::values includes, int max_results, const blob_request_options& options, operation_context context) const
    {
        utility::string_t container_name;
        utility::string_t actual_prefix;
        parse_blob_name_prefix(prefix, container_name, actual_prefix);

        auto container = container_name.empty() ? get_root_container_reference() : get_container_reference(container_name);
        return container.list_blobs(actual_prefix, use_flat_blob_listing, includes, max_results, options, context);
    }

    pplx::task<list_blob_item_segment> cloud_blob_client::list_blobs_segmented_async(const utility::string_t& prefix, bool use_flat_blob_listing, blob_listing_details::values includes, int max_results, const continuation_token& token, const blob_request_options& options, operation_context context, const pplx::cancellation_token& cancellation_token) const
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(default_request_options(), blob_type::unspecified);

        utility::string_t container_name;
        utility::string_t actual_prefix;
        parse_blob_name_prefix(prefix, container_name, actual_prefix);

        auto container = container_name.empty() ? get_root_container_reference() : get_container_reference(container_name);
        return container.list_blobs_segmented_async(actual_prefix, use_flat_blob_listing, includes, max_results, token, modified_options, context, cancellation_token);
    }

    pplx::task<service_properties> cloud_blob_client::download_service_properties_async(const blob_request_options& options, operation_context context, const pplx::cancellation_token& cancellation_token) const
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(default_request_options(), blob_type::unspecified);

        return download_service_properties_base_async(modified_options, context, cancellation_token);
    }

    pplx::task<void> cloud_blob_client::upload_service_properties_async(const service_properties& properties, const service_properties_includes& includes, const blob_request_options& options, operation_context context, const pplx::cancellation_token& cancellation_token) const
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(default_request_options(), blob_type::unspecified);

        return upload_service_properties_base_async(properties, includes, modified_options, context, cancellation_token);
    }

    pplx::task<service_stats> cloud_blob_client::download_service_stats_async(const blob_request_options& options, operation_context context, const pplx::cancellation_token& cancellation_token) const
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(default_request_options(), blob_type::unspecified);

        return download_service_stats_base_async(modified_options, context, cancellation_token);
    }

    pplx::task<account_properties> cloud_blob_client::download_account_properties_async(const blob_request_options& options, operation_context context, const pplx::cancellation_token& cancellation_token) const
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(default_request_options(), blob_type::unspecified);

        return download_account_properties_base_async(base_uri(), modified_options, context, cancellation_token);
    }

    cloud_blob_container cloud_blob_client::get_root_container_reference() const
    {
        return get_container_reference(protocol::root_container);
    }

    cloud_blob_container cloud_blob_client::get_container_reference(utility::string_t container_name) const
    {
        return cloud_blob_container(std::move(container_name), *this);
    }

    pplx::task<account_properties> cloud_blob_client::download_account_properties_base_async(const storage_uri& uri, const request_options& modified_options, operation_context context, const pplx::cancellation_token& cancellation_token) const
    {
        auto command = std::make_shared<core::storage_command<account_properties>>(uri, cancellation_token, modified_options.is_maximum_execution_time_customized());
        command->set_build_request(std::bind(protocol::get_account_properties, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary);
        command->set_preprocess_response(std::bind(protocol::preprocess_response<account_properties>, account_properties(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_postprocess_response([](const web::http::http_response& response, const request_result&, const core::ostream_descriptor&, operation_context context) -> pplx::task<account_properties>
        {
            return pplx::task_from_result<account_properties>(protocol::blob_response_parsers::parse_account_properties(response));
        });
        return core::executor<account_properties>::execute_async(command, modified_options, context);
    }

    void cloud_blob_client::set_authentication_scheme(azure::storage::authentication_scheme value)
    {
        cloud_client::set_authentication_scheme(value);

        // TODO: Refactor authentication code to share common functionality among all services
        // TODO: Make authentication handlers and canonicalizers singletons

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
        else if (creds.is_bearer_token())
        {
            set_authentication_handler(std::make_shared<protocol::bearer_token_authentication_handler>(std::move(creds)));
        }
        else
        {
            set_authentication_handler(std::make_shared<protocol::authentication_handler>());
        }
    }

    void cloud_blob_client::parse_blob_name_prefix(const utility::string_t& prefix, utility::string_t& container_name, utility::string_t& actual_prefix)
    {
        auto first_slash = prefix.find(_XPLATSTR('/'));
        if (first_slash == prefix.npos)
        {
            container_name = utility::string_t();
            actual_prefix = prefix;
        }
        else
        {
            container_name = prefix.substr(0, first_slash);
            actual_prefix = prefix.substr(first_slash + 1);
        }
    }

    pplx::task<user_delegation_key> cloud_blob_client::get_user_delegation_key_async(const utility::datetime& start, const utility::datetime& expiry, const request_options& modified_options, operation_context context, const pplx::cancellation_token& cancellation_token)
    {
        if (!credentials().is_bearer_token())
        {
            throw std::logic_error(protocol::error_uds_missing_credentials);
        }

        protocol::user_delegation_key_time_writer writer;
        concurrency::streams::istream stream(concurrency::streams::bytestream::open_istream(writer.write(start, expiry)));

        auto command = std::make_shared<core::storage_command<user_delegation_key>>(base_uri(), cancellation_token, modified_options.is_maximum_execution_time_customized());
        command->set_build_request(std::bind(protocol::get_user_delegation_key, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary);
        command->set_preprocess_response(std::bind(protocol::preprocess_response<user_delegation_key>, user_delegation_key(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_postprocess_response([](const web::http::http_response& response, const request_result&, const core::ostream_descriptor&, operation_context context) -> pplx::task<user_delegation_key>
        {
            protocol::user_delegation_key_reader reader(response.body());
            return pplx::task_from_result<user_delegation_key>(reader.move_key());
        });
        return core::istream_descriptor::create(stream, checksum_type::none, std::numeric_limits<utility::size64_t>::max(), std::numeric_limits<utility::size64_t>::max(), command->get_cancellation_token()).then([command, context, modified_options, cancellation_token](core::istream_descriptor request_body) -> pplx::task<user_delegation_key>
        {
            command->set_request_body(request_body);
            return core::executor<user_delegation_key>::execute_async(command, modified_options, context);
        });
    }

}} // namespace azure::storage
