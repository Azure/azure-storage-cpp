// -----------------------------------------------------------------------------------------
// <copyright file="cloud_file_share.cpp" company="Microsoft">
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
#include "was/error_code_strings.h"
#include "wascore/protocol.h"
#include "wascore/protocol_xml.h"
#include "wascore/util.h"
#include "wascore/constants.h"

namespace azure { namespace storage {

    void cloud_file_share_properties::update_etag_and_last_modified(const cloud_file_share_properties& other)
    {
        m_etag = other.etag();
        m_last_modified = other.last_modified();
    }

#pragma region cloud file share

    cloud_file_share::cloud_file_share(storage_uri uri)
        : m_uri(create_uri(uri)), m_name(read_share_name(uri)), m_client(create_service_client(uri, storage_credentials())), m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_file_share_properties>())
    {
    }

    cloud_file_share::cloud_file_share(storage_uri uri, storage_credentials credentials)
        : m_uri(create_uri(uri)), m_name(read_share_name(uri)), m_client(create_service_client(uri, credentials)), m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_file_share_properties>())
    {
    }

    cloud_file_share::cloud_file_share(utility::string_t name, cloud_file_client client)
        : m_name(std::move(name)), m_client(std::move(client)), m_uri(core::append_path_to_uri(m_client.base_uri(), m_name)),
        m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_file_share_properties>())
    {
    }

    cloud_file_share::cloud_file_share(utility::string_t name, cloud_file_client client, cloud_file_share_properties properties, cloud_metadata metadata)
        : m_name(std::move(name)), m_client(std::move(client)), m_uri(core::append_path_to_uri(m_client.base_uri(), m_name)),
        m_metadata(std::make_shared<cloud_metadata>(std::move(metadata))), m_properties(std::make_shared<cloud_file_share_properties>(std::move(properties)))
    {
    }

    cloud_file_client cloud_file_share::create_service_client(const storage_uri& uri, storage_credentials credentials)
    {
        storage_uri base_uri = core::get_service_client_uri(uri);
        core::parse_query_and_verify(uri, credentials, false);
        return cloud_file_client(std::move(base_uri), std::move(credentials));
    }

    utility::string_t cloud_file_share::read_share_name(const storage_uri& uri)
    {
        utility::string_t share_name;
        bool is_valid_share_name = core::parse_object_uri(uri, share_name);
        if (!is_valid_share_name)
        {
            throw std::invalid_argument("uri");
        }

        return share_name;
    }

    storage_uri cloud_file_share::create_uri(const storage_uri& uri)
    {
        return core::create_stripped_uri(uri);
    }

#pragma endregion

    pplx::task<void> cloud_file_share::create_async(const file_request_options& options, operation_context context)
    {
        return create_async(protocol::maximum_share_quota, options, context);
    }

    pplx::task<void> cloud_file_share::create_async(utility::size64_t max_size, const file_request_options& options, operation_context context)
    {
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::create_file_share, max_size, metadata(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            *properties = protocol::file_response_parsers::parse_file_share_properties(response);
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<bool> cloud_file_share::create_if_not_exists_async(const file_request_options& options, operation_context context)
    {
        return create_if_not_exists_async(protocol::maximum_share_quota, options, context);
    }

    pplx::task<bool> cloud_file_share::create_if_not_exists_async(utility::size64_t max_size, const file_request_options& options, operation_context context)
    {
        auto instance = std::make_shared<cloud_file_share>(*this);

        return exists_async(false, options, context).then([instance, max_size, options, context](bool exists) -> pplx::task<bool> {
            if (!exists)
            {
                return instance->create_async(max_size, options, context).then([](pplx::task<void> create_task) {
                    try
                    {
                        create_task.wait();
                        return true;
                    }
                    catch (const storage_exception& e)
                    {
                        const azure::storage::request_result& result = e.result();
                        if (result.is_response_available() &&
                            (result.http_status_code() == web::http::status_codes::Conflict) &&
                            (result.extended_error().code() == _XPLATSTR("ShareAlreadyExists")))
                        {
                            return false;
                        }
                        else
                        {
                            throw;
                        }
                    }
                });
            }
            else
            {
                return pplx::task_from_result<bool>(false);
            }
        });
    }

    pplx::task<void> cloud_file_share::delete_share_async(const file_access_condition& condition, const file_request_options& options, operation_context context)
    {
        UNREFERENCED_PARAMETER(condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::delete_file_share, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<bool> cloud_file_share::delete_share_if_exists_async(const file_access_condition& condition, const file_request_options& options, operation_context context)
    {
        auto instance = std::make_shared<cloud_file_share>(*this);
        return exists_async(false, options, context).then([instance, condition, options, context](bool exists) -> pplx::task<bool> {
            if (exists)
            {
                return instance->delete_share_async(condition, options, context).then([](pplx::task<void> delete_task) -> bool
                {
                    try
                    {
                        delete_task.wait();
                        return true;
                    }
                    catch (const storage_exception& e)
                    {
                        const azure::storage::request_result& result = e.result();
                        if (result.is_response_available() &&
                            (result.http_status_code() == web::http::status_codes::NotFound) &&
                            (result.extended_error().code() == _XPLATSTR("ShareNotFound")))
                        {
                            return false;
                        }
                        else
                        {
                            throw;
                        }
                    }
                });
            }
            else
            {
                return pplx::task_from_result<bool>(false);
            }
        });
    }

    pplx::task<void> cloud_file_share::download_attributes_async(const file_access_condition& condition, const file_request_options& options, operation_context context)
    {
        UNREFERENCED_PARAMETER(condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;
        auto metadata = m_metadata;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::get_file_share_properties, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties, metadata](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            *properties = protocol::file_response_parsers::parse_file_share_properties(response);
            *metadata = protocol::parse_metadata(response);
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_file_share::upload_metadata_async(const file_access_condition& condition, const file_request_options& options, operation_context context) const
    {
        UNREFERENCED_PARAMETER(condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::set_file_share_metadata, metadata(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::file_response_parsers::parse_file_share_properties(response));
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<bool> cloud_file_share::exists_async(bool primary_only, const file_request_options& options, operation_context context)
    {
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;
        auto metadata = m_metadata;

        auto command = std::make_shared<core::storage_command<bool>>(uri());
        command->set_build_request(std::bind(protocol::get_file_share_properties, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(primary_only ? core::command_location_mode::primary_only : core::command_location_mode::primary_or_secondary);
        command->set_preprocess_response([properties, metadata](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            if (response.status_code() == web::http::status_codes::NotFound)
            {
                return false;
            }
            protocol::preprocess_response_void(response, result, context);
            *properties = protocol::file_response_parsers::parse_file_share_properties(response);
            *metadata = protocol::parse_metadata(response);
            return true;
        });
        return core::executor<bool>::execute_async(command, modified_options, context);
    }

    pplx::task<int32_t> cloud_file_share::download_share_usage_aysnc(const file_access_condition& condition, const file_request_options& options, operation_context context) const
    {
        UNREFERENCED_PARAMETER(condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto command = std::make_shared<core::storage_command<int32_t>>(uri());
        command->set_build_request(std::bind(protocol::get_file_share_stats, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            protocol::get_share_stats_reader reader(response.body());
            return reader.get();
        });
        return core::executor<int32_t>::execute_async(command, modified_options, context);
    }

    utility::string_t cloud_file_share::get_shared_access_signature(const file_shared_access_policy& policy, const utility::string_t& stored_policy_identifier, const cloud_file_shared_access_headers& headers) const
    {
        if (!service_client().credentials().is_shared_key())
        {
            throw std::logic_error(protocol::error_sas_missing_credentials);
        }

        utility::string_t resource_str;
        resource_str.reserve(service_client().credentials().account_name().size() + name().size() + 7);
        resource_str.append(_XPLATSTR("/"));
        resource_str.append(protocol::service_file);
        resource_str.append(_XPLATSTR("/"));
        resource_str.append(service_client().credentials().account_name());
        resource_str.append(_XPLATSTR("/"));
        resource_str.append(name());
        
        return protocol::get_file_sas_token(stored_policy_identifier, policy, headers, _XPLATSTR("s"), resource_str, service_client().credentials());
    }

    cloud_file_directory cloud_file_share::get_root_directory_reference() const
    {
        return get_directory_reference(utility::string_t());
    }

    cloud_file_directory cloud_file_share::get_directory_reference(utility::string_t name) const
    {
        return cloud_file_directory(name, *this);
    }

    pplx::task<void> cloud_file_share::resize_async(utility::size64_t quota, const file_access_condition& condition, const file_request_options& options, operation_context context)
    {
        UNREFERENCED_PARAMETER(condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto update_properties = m_properties;
        auto properties = cloud_file_share_properties(*m_properties);
        properties.m_quota = quota;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::set_file_share_properties, properties, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([update_properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            update_properties->update_etag_and_last_modified(protocol::file_response_parsers::parse_file_share_properties(response));
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<file_share_permissions> cloud_file_share::download_permissions_async(const file_access_condition& condition, const file_request_options& options, operation_context context) const
    {
        UNREFERENCED_PARAMETER(condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<file_share_permissions>>(uri());
        command->set_build_request(std::bind(protocol::get_file_share_acl, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::file_response_parsers::parse_file_share_properties(response));
            return file_share_permissions();
        });
        command->set_postprocess_response([](const web::http::http_response& response, const request_result&, const core::ostream_descriptor&, operation_context context) -> pplx::task<file_share_permissions>
        {
            file_share_permissions permissions;
            protocol::access_policy_reader<file_shared_access_policy> reader(response.body());
            permissions.set_policies(reader.move_policies());
            return pplx::task_from_result<file_share_permissions>(permissions);
        });
        return core::executor<file_share_permissions>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_file_share::upload_permissions_async(const file_share_permissions& permissions, const file_access_condition& condition, const file_request_options& options, operation_context context) const
    {
        UNREFERENCED_PARAMETER(condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        protocol::access_policy_writer<file_shared_access_policy> writer;
        concurrency::streams::istream stream(concurrency::streams::bytestream::open_istream(writer.write(permissions.policies())));

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::set_file_share_acl, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::file_response_parsers::parse_file_share_properties(response));
        });
        return core::istream_descriptor::create(stream).then([command, context, modified_options](core::istream_descriptor request_body) -> pplx::task<void>
        {
            command->set_request_body(request_body);
            return core::executor<void>::execute_async(command, modified_options, context);
        });
    }

}} // namespace azure::storage