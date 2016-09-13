// -----------------------------------------------------------------------------------------
// <copyright file="cloud_table.cpp" company="Microsoft">
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
#include "wascore/protocol_json.h"
#include "wascore/resources.h"
#include "wascore/util.h"
#include "was/table.h"

namespace azure { namespace storage {

    const utility::string_t query_comparison_operator::equal = _XPLATSTR("eq");
    const utility::string_t query_comparison_operator::not_equal = _XPLATSTR("ne");
    const utility::string_t query_comparison_operator::greater_than = _XPLATSTR("gt");
    const utility::string_t query_comparison_operator::greater_than_or_equal = _XPLATSTR("ge");
    const utility::string_t query_comparison_operator::less_than = _XPLATSTR("lt");
    const utility::string_t query_comparison_operator::less_than_or_equal = _XPLATSTR("le");

    const utility::string_t query_logical_operator::op_and = _XPLATSTR("and");
    const utility::string_t query_logical_operator::op_not = _XPLATSTR("not");
    const utility::string_t query_logical_operator::op_or = _XPLATSTR("or");

    cloud_table::cloud_table(const storage_uri& uri)
        : m_client(create_service_client(uri, storage_credentials())), m_name(read_table_name(uri)), m_uri(create_uri(uri))
    {
    }

    cloud_table::cloud_table(const storage_uri& uri, storage_credentials credentials)
        : m_client(create_service_client(uri, std::move(credentials))), m_name(read_table_name(uri)), m_uri(create_uri(uri))
    {
    }

    cloud_table::cloud_table(cloud_table_client client, utility::string_t name)
        : m_client(std::move(client)), m_name(std::move(name)), m_uri(core::append_path_to_uri(m_client.base_uri(), m_name))
    {
    }

    pplx::task<void> cloud_table::create_async(const table_request_options& options, operation_context context)
    {
        return create_async_impl(options, context, /* allow_conflict */ false).then([] (bool)
        {
        });
    }

    pplx::task<bool> cloud_table::create_if_not_exists_async(const table_request_options& options, operation_context context)
    {
        auto instance = std::make_shared<cloud_table>(*this);
        return exists_async_impl(options, context, /* allow_secondary */ false).then([instance, options, context] (bool exists) -> pplx::task<bool>
        {
            if (exists)
            {
                return pplx::task_from_result(false);
            }

            return instance->create_async_impl(options, context, /* allow_conflict */ true);
        });
    }

    pplx::task<void> cloud_table::delete_table_async(const table_request_options& options, operation_context context)
    {
        return delete_async_impl(options, context, /* allow_not_found */ false).then([] (bool)
        {
        });
    }

    pplx::task<bool> cloud_table::delete_table_if_exists_async(const table_request_options& options, operation_context context)
    {
        auto instance = std::make_shared<cloud_table>(*this);
        return exists_async_impl(options, context, /* allow_secondary */ false).then([instance, options, context] (bool exists) -> pplx::task<bool>
        {
            if (!exists)
            {
                return pplx::task_from_result(false);
            }

            return instance->delete_async_impl(options, context, /* allow_not_found */ true);
        });
    }

    pplx::task<bool> cloud_table::exists_async(const table_request_options& options, operation_context context) const
    {
        return exists_async_impl(options, context, /* allow_secondary */ true);
    }

    pplx::task<table_result> cloud_table::execute_async(const table_operation& operation, const table_request_options& options, operation_context context) const
    {
        table_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_table_uri(service_client(), *this, operation);

        // Do not throw an exception when the retrieve fails because the entity does not exist
        bool allow_not_found = operation.operation_type() == table_operation_type::retrieve_operation;

        std::shared_ptr<core::storage_command<table_result>> command = std::make_shared<core::storage_command<table_result>>(uri);
        command->set_build_request(std::bind(protocol::execute_operation, operation, modified_options.payload_format(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(operation.operation_type() == azure::storage::table_operation_type::retrieve_operation ? core::command_location_mode::primary_or_secondary : core::command_location_mode::primary_only);
        command->set_preprocess_response([allow_not_found] (const web::http::http_response& response, const request_result& result, operation_context context) -> table_result
        {
            if (!allow_not_found || response.status_code() != web::http::status_codes::NotFound)
            {
                protocol::preprocess_response_void(response, result, context);
            }
            return table_result();
        });
        command->set_postprocess_response([] (const web::http::http_response& response, const request_result&, const core::ostream_descriptor&, operation_context context) -> pplx::task<table_result>
        {
            UNREFERENCED_PARAMETER(context);
            int status_code = response.status_code();
            utility::string_t etag = protocol::table_response_parsers::parse_etag(response);

            if (status_code == web::http::status_codes::NoContent)
            {
                table_result result;
                result.set_http_status_code(status_code);
                result.set_etag(std::move(etag));
                return pplx::task_from_result(result);
            }
            else
            {
                return response.extract_json().then([status_code, etag] (const web::json::value& obj) -> table_result
                {
                    table_entity entity = protocol::parse_table_entity(obj);
                    if (entity.etag().empty())
                    {
                        entity.set_etag(etag);
                    }

                    table_result result;
                    result.set_http_status_code(status_code);
                    result.set_etag(std::move(etag));
                    result.set_entity(std::move(entity));
                    return result;
                });
            }
        });
        return core::executor<table_result>::execute_async(command, modified_options, context);
    }

    pplx::task<std::vector<table_result>> cloud_table::execute_batch_async(const table_batch_operation& operation, const table_request_options& options, operation_context context) const
    {
        table_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_table_uri(service_client(), *this, operation);

        std::vector<table_operation> operations = operation.operations();
        if (operations.size() == 0U)
        {
            throw std::invalid_argument(protocol::error_empty_batch_operation);
        }

        utility::string_t partition_key = operations.front().entity().partition_key();
        for (std::vector<table_operation>::const_iterator itr = operations.cbegin(); itr != operations.cend(); ++itr)
        {
            if (partition_key.compare(itr->entity().partition_key()) != 0)
            {
                throw std::invalid_argument(protocol::error_batch_operation_partition_key_mismatch);
            }
        }

        bool is_query = false;
        for (std::vector<table_operation>::const_iterator itr = operations.cbegin(); itr != operations.cend(); ++itr)
        {
            if (itr->operation_type() == table_operation_type::retrieve_operation)
            {
                if (is_query)
                {
                    throw std::invalid_argument(protocol::error_batch_operation_retrieve_count);
                }

                is_query = true;
            }
        }

        if (is_query && operations.size() != 1)
        {
            throw std::invalid_argument(protocol::error_batch_operation_retrieve_mix);
        }

        // TODO: Pre-create a stream for the response to pass to response handler in other functions too so the response doesn't need to be copied
        Concurrency::streams::stringstreambuf response_buffer;

        std::shared_ptr<core::storage_command<std::vector<table_result>>> command = std::make_shared<core::storage_command<std::vector<table_result>>>(uri);
        command->set_build_request(std::bind(protocol::execute_batch_operation, response_buffer, *this, operation, options.payload_format(), is_query, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(is_query ? core::command_location_mode::primary_or_secondary : core::command_location_mode::primary_only);
        command->set_preprocess_response(std::bind(protocol::preprocess_response<std::vector<table_result>>, std::vector<table_result>(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_postprocess_response([response_buffer, operations, is_query] (const web::http::http_response& response, const request_result&, const core::ostream_descriptor&, operation_context context) mutable -> pplx::task<std::vector<table_result>>
        {
            UNREFERENCED_PARAMETER(context);
            return response.content_ready().then([response_buffer, operations, is_query](const web::http::http_response& response) mutable -> pplx::task<std::vector<table_result>>
            {
                std::vector<table_result> batch_result = protocol::table_response_parsers::parse_batch_results(response, response_buffer, is_query, operations.size());
                return pplx::task_from_result(batch_result);
            });
        });
        return core::executor<std::vector<table_result>>::execute_async(command, modified_options, context);
    }

    table_query_iterator cloud_table::execute_query(const table_query& query, const table_request_options& options, operation_context context) const
    {
        auto instance = std::make_shared<cloud_table>(*this);
        return table_query_iterator(
            [instance, &query, options, context](const continuation_token& token, size_t)
        {
            return instance->execute_query_segmented(query, token, options, context);
        },
            query.take_count() <= 0 ? 0 : query.take_count(), 0);
    }

    pplx::task<table_query_segment> cloud_table::execute_query_segmented_async(const table_query& query, const continuation_token& token, const table_request_options& options, operation_context context) const
    {
        table_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_table_uri(service_client(), *this, query, token);

        std::shared_ptr<core::storage_command<table_query_segment>> command = std::make_shared<core::storage_command<table_query_segment>>(uri);
        command->set_build_request(std::bind(protocol::execute_query, modified_options.payload_format(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary, token.target_location());
        command->set_preprocess_response(std::bind(protocol::preprocess_response<table_query_segment>, table_query_segment(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_postprocess_response([] (const web::http::http_response& response, const request_result& result, const core::ostream_descriptor&, operation_context context) -> pplx::task<table_query_segment>
        {
            UNREFERENCED_PARAMETER(context);
            continuation_token next_token = protocol::table_response_parsers::parse_continuation_token(response, result);

            return response.extract_json().then([next_token] (const web::json::value& obj) -> table_query_segment
            {
                table_query_segment query_segment(protocol::table_response_parsers::parse_query_results(obj), std::move(next_token));
                return query_segment;
            });
        });
        return core::executor<table_query_segment>::execute_async(command, modified_options, context);
    }

    utility::string_t cloud_table::get_shared_access_signature(const table_shared_access_policy& policy, const utility::string_t& stored_policy_identifier, const utility::string_t& start_partition_key, const utility::string_t& start_row_key, const utility::string_t& end_partition_key, const utility::string_t& end_row_key) const
    {
        if (!service_client().credentials().is_shared_key())
        {
            throw std::logic_error(protocol::error_sas_missing_credentials);
        }

        utility::string_t table_name = name();
        std::transform(table_name.begin(), table_name.end(), table_name.begin(), core::utility_char_tolower);

        // since 2015-02-21, canonicalized resource is changed from "/account/name" to "/table/account/name"
        utility::string_t resource_str;
        resource_str.reserve(service_client().credentials().account_name().size() + table_name.size() + 8);
        resource_str.append(_XPLATSTR("/"));
        resource_str.append(protocol::service_table);
        resource_str.append(_XPLATSTR("/"));
        resource_str.append(service_client().credentials().account_name());
        resource_str.append(_XPLATSTR("/"));
        resource_str.append(table_name);

        return protocol::get_table_sas_token(stored_policy_identifier, policy, name(), start_partition_key, start_row_key, end_partition_key, end_row_key, resource_str, service_client().credentials());
    }

    cloud_table_client cloud_table::create_service_client(const storage_uri& uri, storage_credentials credentials)
    {
        storage_uri base_uri = core::get_service_client_uri(uri);
        core::parse_query_and_verify(uri, credentials, false);
        return cloud_table_client(std::move(base_uri), std::move(credentials));
    }

    utility::string_t cloud_table::read_table_name(const storage_uri& uri)
    {
        utility::string_t table_name;
        bool is_valid_table_name = core::parse_object_uri(uri, table_name);
        if (!is_valid_table_name)
        {
            throw std::invalid_argument("uri");
        }

        return table_name;
    }

    storage_uri cloud_table::create_uri(const storage_uri& uri)
    {
        return core::create_stripped_uri(uri);
    }

    table_request_options cloud_table::get_modified_options(const table_request_options& options) const
    {
        table_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());
        return modified_options;
    }

    pplx::task<bool> cloud_table::create_async_impl(const table_request_options& options, operation_context context, bool allow_conflict)
    {
        table_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_table_uri(service_client(), *this, /* create_table */ true);

        std::shared_ptr<core::storage_command<bool>> command = std::make_shared<core::storage_command<bool>>(uri);
        command->set_build_request(std::bind(protocol::execute_table_operation, *this, table_operation_type::insert_operation, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([allow_conflict] (const web::http::http_response& response, const request_result& result, operation_context context) -> bool
        {
            if (allow_conflict && response.status_code() == web::http::status_codes::Conflict)
            {
                return false;
            }

            protocol::preprocess_response_void(response, result, context);
            return true;
        });
        return core::executor<bool>::execute_async(command, modified_options, context);
    }

    pplx::task<bool> cloud_table::delete_async_impl(const table_request_options& options, operation_context context, bool allow_not_found)
    {
        table_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_table_uri(service_client(), *this, false);

        std::shared_ptr<core::storage_command<bool>> command = std::make_shared<core::storage_command<bool>>(uri);
        command->set_build_request(std::bind(protocol::execute_table_operation, *this, table_operation_type::delete_operation, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([allow_not_found] (const web::http::http_response& response, const request_result& result, operation_context context) -> bool
        {
            if (allow_not_found && response.status_code() == web::http::status_codes::NotFound)
            {
                return false;
            }

            protocol::preprocess_response_void(response, result, context);
            return true;
        });
        return core::executor<bool>::execute_async(command, modified_options, context);
    }

    pplx::task<bool> cloud_table::exists_async_impl(const table_request_options& options, operation_context context, bool allow_secondary) const
    {
        table_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_table_uri(service_client(), *this, false);

        std::shared_ptr<core::storage_command<bool>> command = std::make_shared<core::storage_command<bool>>(uri);
        command->set_build_request(std::bind(protocol::execute_table_operation, *this, table_operation_type::retrieve_operation, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(allow_secondary ? core::command_location_mode::primary_or_secondary : core::command_location_mode::primary_only);
        command->set_preprocess_response([] (const web::http::http_response& response, const request_result& result, operation_context context) -> bool
        {
            if (response.status_code() != web::http::status_codes::NotFound)
            {
                protocol::preprocess_response_void(response, result, context);
                return true;
            }

            return false;
        });
        return core::executor<bool>::execute_async(command, modified_options, context);
    }

    pplx::task<table_permissions> cloud_table::download_permissions_async(const table_request_options& options, operation_context context) const
    {
        table_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_table_uri(service_client(), *this);

        std::shared_ptr<core::storage_command<table_permissions>> command = std::make_shared<core::storage_command<table_permissions>>(uri);
        command->set_build_request(std::bind(protocol::get_table_acl, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary);
        command->set_preprocess_response(std::bind(protocol::preprocess_response<table_permissions>, table_permissions(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_postprocess_response([] (const web::http::http_response& response, const request_result&, const core::ostream_descriptor&, operation_context context) -> pplx::task<table_permissions>
        {
            UNREFERENCED_PARAMETER(context);
            table_permissions permissions;
            protocol::access_policy_reader<table_shared_access_policy> reader(response.body());
            permissions.set_policies(reader.move_policies());
            return pplx::task_from_result<table_permissions>(permissions);
        });
        return core::executor<table_permissions>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_table::upload_permissions_async(const table_permissions& permissions, const table_request_options& options, operation_context context)
    {
        table_request_options modified_options = get_modified_options(options);
        storage_uri uri = protocol::generate_table_uri(service_client(), *this);

        protocol::access_policy_writer<table_shared_access_policy> writer;
        concurrency::streams::istream stream(concurrency::streams::bytestream::open_istream(writer.write(permissions.policies())));

        std::shared_ptr<core::storage_command<void>> command = std::make_shared<core::storage_command<void>>(uri);
        command->set_build_request(std::bind(protocol::set_table_acl, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response(std::bind(protocol::preprocess_response_void, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        return core::istream_descriptor::create(stream).then([command, context, modified_options] (core::istream_descriptor request_body) -> pplx::task<void>
        {
            command->set_request_body(request_body);
            return core::executor<void>::execute_async(command, modified_options, context);
        });
    }

}} // namespace azure::storage
