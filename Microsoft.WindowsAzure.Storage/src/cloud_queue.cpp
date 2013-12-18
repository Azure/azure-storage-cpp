// -----------------------------------------------------------------------------------------
// <copyright file="cloud_queue.cpp" company="Microsoft">
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
#include "wascore/executor.h"
#include "wascore/protocol.h"
#include "wascore/protocol_xml.h"
#include "wascore/resources.h"
#include "was/queue.h"

namespace wa { namespace storage {

    cloud_queue::cloud_queue(const storage_uri& uri)
        : m_client(create_client(uri, storage_credentials())), m_name(read_queue_name(uri)), m_uri(uri), m_approximate_message_count(-1)
    {
    }

    cloud_queue::cloud_queue(const storage_uri& uri, const storage_credentials& credentials)
        : m_client(create_client(uri, credentials)), m_name(read_queue_name(uri)), m_uri(uri), m_approximate_message_count(-1)
    {
    }

    cloud_queue::cloud_queue(const cloud_queue_client& client, const utility::string_t& name)
        : m_client(client), m_name(name), m_uri(create_uri(client, name)), m_approximate_message_count(-1)
    {
    }

    pplx::task<void> cloud_queue::create_async(const queue_request_options& options, operation_context context)
    {
        return create_async_impl(options, context, /* allow_conflict */ false).then([] (bool created)
        {
        });
    }

    pplx::task<bool> cloud_queue::create_if_not_exists_async(const queue_request_options& options, operation_context context)
    {
        return exists_async_impl(options, context, /* allow_secondary */ false).then([this, options, context] (bool exists) -> pplx::task<bool>
        {
            if (exists)
            {
                return pplx::task_from_result(false);
            }

            return create_async_impl(options, context, /* allow_conflict */ true);
        });
    }

    pplx::task<void> cloud_queue::delete_queue_async(const queue_request_options& options, operation_context context)
    {
        return delete_async_impl(options, context, /* allow_not_found */ false).then([] (bool created)
        {
        });
    }

    pplx::task<bool> cloud_queue::delete_queue_if_exists_async(const queue_request_options& options, operation_context context)
    {
        return exists_async_impl(options, context, /* allow_secondary */ false).then([this, options, context] (bool exists) -> pplx::task<bool>
        {
            if (!exists)
            {
                return pplx::task_from_result(false);
            }

            return delete_async_impl(options, context, /* allow_not_found */ true);
        });
    }

    pplx::task<bool> cloud_queue::exists_async(const queue_request_options& options, operation_context context) const
    {
        return exists_async_impl(options, context, /* allow_secondary */ true);
    }

    pplx::task<void> cloud_queue::add_message_async(cloud_queue_message& message, std::chrono::seconds time_to_live, std::chrono::seconds initial_visibility_timeout, queue_request_options& options, operation_context context)
    {
        if (time_to_live.count() <= 0LL)
        {
            throw std::invalid_argument("The time to live cannot be zero or negative.");
        }

        if (time_to_live.count() > 604800LL)
        {
            throw std::invalid_argument("The time to live cannot be greater than 604800.");
        }

        if (initial_visibility_timeout.count() < 0LL)
        {
            throw std::invalid_argument("The initial visibility timeout cannot be negative.");
        }

        if (initial_visibility_timeout.count() > 604800LL)
        {
            throw std::invalid_argument("The initial visibility timeout cannot be greater than 604800.");
        }

        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_message_uri(service_client(), *this);

        std::shared_ptr<core::storage_command<void>> command = std::make_shared<core::storage_command<void>>(uri);
        command->set_build_request(std::bind(protocol::add_message, *this, message, time_to_live, initial_visibility_timeout, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([] (const web::http::http_response& response, operation_context context)
        {
            protocol::preprocess_response(response, context);
            //message.set_pop_receipt(protocol::parse_pop_receipt(response));
            //message.set_next_visible_time(protocol::parse_next_visible_time(response));
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<cloud_queue_message> cloud_queue::get_message_async(std::chrono::seconds visibility_timeout, queue_request_options& options, operation_context context)
    {
        if (visibility_timeout.count() < 0LL)
        {
            throw std::invalid_argument("The visibility timeout cannot be negative.");
        }

        if (visibility_timeout.count() > 604800LL)
        {
            throw std::invalid_argument("The visibility timeout cannot be greater than 604800.");
        }

        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_message_uri(service_client(), *this);

        std::shared_ptr<core::storage_command<cloud_queue_message>> command = std::make_shared<core::storage_command<cloud_queue_message>>(uri);
        command->set_build_request(std::bind(protocol::get_messages, *this, 1U, visibility_timeout, /* is_peek */ false, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response(std::bind(protocol::preprocess_response<cloud_queue_message>, cloud_queue_message(), std::placeholders::_1, std::placeholders::_2));
        command->set_postprocess_response([] (const web::http::http_response& response, const request_result& result, const core::ostream_descriptor&, operation_context context) -> pplx::task<cloud_queue_message>
        {
            protocol::message_reader reader(response.body());
            std::vector<protocol::cloud_message_list_item> queue_items = reader.extract_items();

            if (!queue_items.empty())
            {
                protocol::cloud_message_list_item item = queue_items[0];
                cloud_queue_message message(item.content(), item.id(), item.pop_receipt(), item.insertion_time(), item.expiration_time(), item.next_visible_time(), item.dequeue_count());
                return pplx::task_from_result(message);
            }

            return pplx::task_from_result(cloud_queue_message());
        });
        return core::executor<cloud_queue_message>::execute_async(command, modified_options, context);
    }

    pplx::task<std::vector<cloud_queue_message>> cloud_queue::get_messages_async(size_t message_count, std::chrono::seconds visibility_timeout, queue_request_options& options, operation_context context)
    {
        if (visibility_timeout.count() < 0LL)
        {
            throw std::invalid_argument("The visibility timeout cannot be negative.");
        }

        if (visibility_timeout.count() > 604800LL)
        {
            throw std::invalid_argument("The visibility timeout cannot be greater than 604800.");
        }

        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_message_uri(service_client(), *this);

        std::shared_ptr<core::storage_command<std::vector<cloud_queue_message>>> command = std::make_shared<core::storage_command<std::vector<cloud_queue_message>>>(uri);
        command->set_build_request(std::bind(protocol::get_messages, *this, message_count, visibility_timeout, /* is_peek */ false, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response(std::bind(protocol::preprocess_response<std::vector<cloud_queue_message>>, std::vector<cloud_queue_message>(), std::placeholders::_1, std::placeholders::_2));
        command->set_postprocess_response([] (const web::http::http_response& response, const request_result& result, const core::ostream_descriptor&, operation_context context) -> pplx::task<std::vector<cloud_queue_message>>
        {
            protocol::message_reader reader(response.body());
            std::vector<protocol::cloud_message_list_item> queue_items = reader.extract_items();

            std::vector<cloud_queue_message> results;
            results.reserve(queue_items.size());

            for (std::vector<protocol::cloud_message_list_item>::const_iterator itr = queue_items.cbegin(); itr != queue_items.cend(); ++itr)
            {
                cloud_queue_message message(itr->content(), itr->id(), itr->pop_receipt(), itr->insertion_time(), itr->expiration_time(), itr->next_visible_time(), itr->dequeue_count());
                results.push_back(message);
            }

            return pplx::task_from_result(results);
        });
        return core::executor<std::vector<cloud_queue_message>>::execute_async(command, modified_options, context);
    }

    pplx::task<cloud_queue_message> cloud_queue::peek_message_async(const queue_request_options& options, operation_context context) const
    {
        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_message_uri(service_client(), *this);

        std::shared_ptr<core::storage_command<cloud_queue_message>> command = std::make_shared<core::storage_command<cloud_queue_message>>(uri);
        command->set_build_request(std::bind(protocol::get_messages, *this, 1U, std::chrono::seconds(0LL), /* is_peek */ true, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response(std::bind(protocol::preprocess_response<cloud_queue_message>, cloud_queue_message(), std::placeholders::_1, std::placeholders::_2));
        command->set_postprocess_response([] (const web::http::http_response& response, const request_result& result, const core::ostream_descriptor&, operation_context context) -> pplx::task<cloud_queue_message>
        {
            protocol::message_reader reader(response.body());
            std::vector<protocol::cloud_message_list_item> queue_items = reader.extract_items();

            if (!queue_items.empty())
            {
                protocol::cloud_message_list_item item = queue_items[0];
                cloud_queue_message message(item.content(), item.id(), item.pop_receipt(), item.insertion_time(), item.expiration_time(), item.next_visible_time(), item.dequeue_count());
                return pplx::task_from_result(message);
            }

            return pplx::task_from_result(cloud_queue_message());
        });
        return core::executor<cloud_queue_message>::execute_async(command, modified_options, context);
    }

    pplx::task<std::vector<cloud_queue_message>> cloud_queue::peek_messages_async(size_t message_count, const queue_request_options& options, operation_context context) const
    {
        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_message_uri(service_client(), *this);

        std::shared_ptr<core::storage_command<std::vector<cloud_queue_message>>> command = std::make_shared<core::storage_command<std::vector<cloud_queue_message>>>(uri);
        command->set_build_request(std::bind(protocol::get_messages, *this, message_count, std::chrono::seconds(0LL), /* is_peek */ true, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response(std::bind(protocol::preprocess_response<std::vector<cloud_queue_message>>, std::vector<cloud_queue_message>(), std::placeholders::_1, std::placeholders::_2));
        command->set_postprocess_response([] (const web::http::http_response& response, const request_result& result, const core::ostream_descriptor&, operation_context context) -> pplx::task<std::vector<cloud_queue_message>>
        {
            protocol::message_reader reader(response.body());
            std::vector<protocol::cloud_message_list_item> queue_items = reader.extract_items();

            std::vector<cloud_queue_message> results;
            results.reserve(queue_items.size());

            for (std::vector<protocol::cloud_message_list_item>::const_iterator itr = queue_items.cbegin(); itr != queue_items.cend(); ++itr)
            {
                cloud_queue_message message(itr->content(), itr->id(), itr->pop_receipt(), itr->insertion_time(), itr->expiration_time(), itr->next_visible_time(), itr->dequeue_count());
                results.push_back(message);
            }

            return pplx::task_from_result(results);
        });
        return core::executor<std::vector<cloud_queue_message>>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_queue::update_message_async(cloud_queue_message& message, std::chrono::seconds visibility_timeout, bool update_content, queue_request_options& options, operation_context context)
    {
        if (visibility_timeout.count() < 0LL)
        {
            throw std::invalid_argument("The visibility timeout cannot be negative.");
        }

        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_message_uri(service_client(), *this, message);

        std::shared_ptr<core::storage_command<void>> command = std::make_shared<core::storage_command<void>>(uri);
        command->set_build_request(std::bind(protocol::update_message, *this, message, visibility_timeout, update_content, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([&message] (const web::http::http_response& response, operation_context context) mutable
        {
            protocol::preprocess_response(response, context);
            message.set_pop_receipt(protocol::parse_pop_receipt(response));
            message.set_next_visible_time(protocol::parse_next_visible_time(response));
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_queue::delete_message_async(cloud_queue_message& message, queue_request_options& options, operation_context context)
    {
        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_message_uri(service_client(), *this, message);

        std::shared_ptr<core::storage_command<void>> command = std::make_shared<core::storage_command<void>>(uri);
        command->set_build_request(std::bind(protocol::delete_message, *this, message, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response(std::bind(protocol::preprocess_response, std::placeholders::_1, std::placeholders::_2));
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_queue::clear_async(const queue_request_options& options, operation_context context)
    {
        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_message_uri(service_client(), *this);

        std::shared_ptr<core::storage_command<void>> command = std::make_shared<core::storage_command<void>>(uri);
        command->set_build_request(std::bind(protocol::clear_messages, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response(std::bind(protocol::preprocess_response, std::placeholders::_1, std::placeholders::_2));
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_queue::download_attributes_async(const queue_request_options& options, operation_context context)
    {
        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_uri(service_client(), *this);

        std::shared_ptr<core::storage_command<void>> command = std::make_shared<core::storage_command<void>>(uri);
        command->set_build_request(std::bind(protocol::download_queue_metadata, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([this] (const web::http::http_response& response, operation_context context)
        {
            protocol::preprocess_response(response, context);
            m_metadata = protocol::parse_metadata(response);
            m_approximate_message_count = protocol::parse_approximate_messages_count(response);
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_queue::upload_metadata_async(const queue_request_options& options, operation_context context)
    {
        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_uri(service_client(), *this);

        std::shared_ptr<core::storage_command<void>> command = std::make_shared<core::storage_command<void>>(uri);
        command->set_build_request(std::bind(protocol::upload_queue_metadata, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([] (const web::http::http_response& response, operation_context context)
        {
            protocol::preprocess_response(response, context);
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<queue_permissions> cloud_queue::download_permissions_async(const queue_request_options& options, operation_context context) const
    {
        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_uri(service_client(), *this);

        std::shared_ptr<core::storage_command<queue_permissions>> command = std::make_shared<core::storage_command<queue_permissions>>(uri);
        command->set_build_request(std::bind(protocol::get_queue_acl, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary);
        command->set_preprocess_response([] (const web::http::http_response& response, operation_context context) -> queue_permissions
        {
            protocol::preprocess_response(response, context);
            return queue_permissions();
        });
        command->set_postprocess_response([] (const web::http::http_response& response, const request_result&, const core::ostream_descriptor&, operation_context context) -> pplx::task<queue_permissions>
        {
            queue_permissions permissions;
            protocol::access_policy_reader<queue_shared_access_policy> reader(response.body());
            permissions.set_policies(std::move(reader.extract_policies()));
            return pplx::task_from_result<queue_permissions>(permissions);
        });
        return core::executor<queue_permissions>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_queue::upload_permissions_async(const queue_permissions& permissions, const queue_request_options& options, operation_context context)
    {
        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_uri(service_client(), *this);

        protocol::access_policy_writer<queue_shared_access_policy> writer;
        concurrency::streams::istream stream(concurrency::streams::bytestream::open_istream(std::move(writer.write(permissions.policies()))));

        std::shared_ptr<core::storage_command<void>> command = std::make_shared<core::storage_command<void>>(uri);
        command->set_build_request(std::bind(protocol::set_queue_acl, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([] (const web::http::http_response& response, operation_context context)
        {
            protocol::preprocess_response(response, context);
        });
        return core::istream_descriptor::create(stream).then([command, context, modified_options] (core::istream_descriptor request_body) -> pplx::task<void>
        {
            command->set_request_body(request_body);
            return core::executor<void>::execute_async(command, modified_options, context);
        });
    }

    utility::string_t cloud_queue::get_shared_access_signature(const queue_shared_access_policy& policy, const utility::string_t& stored_policy_identifier) const
    {
        if (!service_client().credentials().is_shared_key())
        {
            throw std::logic_error(utility::conversions::to_utf8string(protocol::error_sas_missing_credentials));
        }

        utility::ostringstream_t resource_str;
        resource_str << U('/') << service_client().credentials().account_name() << U('/') << name();

        return protocol::get_queue_sas_token(stored_policy_identifier, policy, resource_str.str(), service_client().credentials());
    }

    cloud_queue_client cloud_queue::create_client(const storage_uri& uri, const storage_credentials& credentials)
    {
        // TODO: Read SAS credentials from URI and use them if present
        storage_uri base_uri = core::get_service_client_uri(uri);
        return cloud_queue_client(base_uri, credentials);
    }

    utility::string_t cloud_queue::read_queue_name(const storage_uri& uri)
    {
        utility::string_t queue_name;
        // TODO: Change parsing to avoid a "$root" queue name
        bool is_valid_queue_name = core::parse_container_uri(uri, queue_name);
        // TODO: throw exception on invalid queue name
        return queue_name;
    }

    storage_uri cloud_queue::create_uri(const cloud_queue_client& client, const utility::string_t& queue_name)
    {
        /*
        web::http::uri primary_uri;
        if (!client.base_uri().primary_uri().is_empty())
        {
            web::http::uri_builder builder(client.base_uri().primary_uri());
            builder.append_path(queue_name);
            primary_uri = builder.to_uri();
        }

        web::http::uri secondary_uri;
        if (!client.base_uri().secondary_uri().is_empty())
        {
            web::http::uri_builder builder(client.base_uri().secondary_uri());
            builder.append_path(queue_name);
            secondary_uri = builder.to_uri();
        }

        storage_uri uri(primary_uri, secondary_uri);
        */

        storage_uri uri = core::append_path_to_uri(client.base_uri(), queue_name);

        return uri;
    }

    queue_request_options cloud_queue::get_modified_options(const queue_request_options& options) const
    {
        queue_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());
        return modified_options;
    }

    pplx::task<bool> cloud_queue::create_async_impl(const queue_request_options& options, operation_context context, bool allow_conflict)
    {
        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_uri(service_client(), *this);

        std::shared_ptr<core::storage_command<bool>> command = std::make_shared<core::storage_command<bool>>(uri);
        command->set_build_request(std::bind(protocol::create_queue, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([allow_conflict] (const web::http::http_response& response, operation_context context) -> bool
        {
            if (response.status_code() == web::http::status_codes::Conflict)
            {
                if (allow_conflict)
                {
                    return false;
                }
            }
            else if (response.status_code() == web::http::status_codes::NoContent)
            {
                if (allow_conflict)
                {
                    return false;
                }
                else
                {
                    throw storage_exception("");
                }
            }

            protocol::preprocess_response(response, context);
            return true;
        });
        return core::executor<bool>::execute_async(command, modified_options, context);
    }

    pplx::task<bool> cloud_queue::delete_async_impl(const queue_request_options& options, operation_context context, bool allow_not_found)
    {
        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_uri(service_client(), *this);

        std::shared_ptr<core::storage_command<bool>> command = std::make_shared<core::storage_command<bool>>(uri);
        command->set_build_request(std::bind(protocol::delete_queue, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([allow_not_found] (const web::http::http_response& response, operation_context context) -> bool
        {
            if (allow_not_found && response.status_code() == web::http::status_codes::NotFound)
            {
                return false;
            }

            protocol::preprocess_response(response, context);
            return true;
        });
        return core::executor<bool>::execute_async(command, modified_options, context);
    }

    pplx::task<bool> cloud_queue::exists_async_impl(const queue_request_options& options, operation_context context, bool allow_secondary) const
    {
        queue_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_queue_uri(service_client(), *this);

        std::shared_ptr<core::storage_command<bool>> command = std::make_shared<core::storage_command<bool>>(uri);
        command->set_build_request(std::bind(protocol::download_queue_metadata, *this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(allow_secondary ? core::command_location_mode::primary_or_secondary : core::command_location_mode::primary_only);
        command->set_preprocess_response([] (const web::http::http_response& response, operation_context context) -> bool
        {
            if (response.status_code() != web::http::status_codes::NotFound)
            {
                protocol::preprocess_response(response, context);
                return true;
            }

            return false;
        });
        return core::executor<bool>::execute_async(command, modified_options, context);
    }

}} // namespace wa::storage
