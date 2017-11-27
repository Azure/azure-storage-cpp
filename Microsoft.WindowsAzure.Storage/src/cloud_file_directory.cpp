// -----------------------------------------------------------------------------------------
// <copyright file="cloud_file_directory.cpp" company="Microsoft">
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

    void cloud_file_directory_properties::update_etag_and_last_modified(const cloud_file_directory_properties& other)
    {
        m_etag = other.etag();
        m_last_modified = other.last_modified();
    }

    void cloud_file_directory_properties::update_etag(const cloud_file_directory_properties& other)
    {
        m_etag = other.etag();
    }

    cloud_file_directory::cloud_file_directory(storage_uri uri)
        : m_uri(std::move(uri)), m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_file_directory_properties>())
    {
        init(std::move(storage_credentials()));
    }

    cloud_file_directory::cloud_file_directory(storage_uri uri, storage_credentials credentials)
        :m_uri(std::move(uri)),  m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_file_directory_properties>())
    {
        init(std::move(credentials));
    }
    
    cloud_file_directory::cloud_file_directory(utility::string_t name, cloud_file_share share)
        : m_name(std::move(name)), m_share(std::move(share)), m_uri(core::append_path_to_uri(m_share.uri(), m_name)),
        m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_file_directory_properties>())
    {
    }

    cloud_file_directory::cloud_file_directory(utility::string_t name, cloud_file_share share, cloud_file_directory_properties properties, cloud_metadata metadata)
        : m_name(std::move(name)), m_share(std::move(share)), m_uri(core::append_path_to_uri(m_share.uri(), m_name)),
        m_metadata(std::make_shared<cloud_metadata>(std::move(metadata))), m_properties(std::make_shared<cloud_file_directory_properties>(std::move(properties)))
    {
    }

    cloud_file_directory::cloud_file_directory(utility::string_t name, cloud_file_directory directory)
        : m_name(std::move(name)), m_share(std::move(directory.get_parent_share_reference())), m_uri(core::append_path_to_uri(directory.uri(), m_name)),
        m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_file_directory_properties>())
    {
    }

    cloud_file_directory::cloud_file_directory(utility::string_t name, cloud_file_directory directory, cloud_file_directory_properties properties, cloud_metadata metadata)
        : m_name(std::move(name)), m_share(std::move(directory.get_parent_share_reference())), m_uri(core::append_path_to_uri(directory.uri(), m_name)),
        m_metadata(std::make_shared<cloud_metadata>(std::move(metadata))), m_properties(std::make_shared<cloud_file_directory_properties>(std::move(properties)))
    {
    }

    void cloud_file_directory::init(storage_credentials credentials)
    {
        core::parse_query_and_verify(m_uri, credentials, true);
        m_uri = core::create_stripped_uri(m_uri);

        utility::string_t share_name;
        if (!core::parse_file_directory_uri(m_uri, share_name, m_name))
        {
            throw std::invalid_argument("uri");
        }

        m_share = cloud_file_share(std::move(share_name), cloud_file_client(core::get_service_client_uri(m_uri), std::move(credentials)));
    }

    list_file_and_diretory_result_iterator cloud_file_directory::list_files_and_directories(const utility::string_t& prefix, int64_t max_results, const file_request_options& options, operation_context context) const
    {
        auto instance = std::make_shared<cloud_file_directory>(*this);
        return list_file_and_diretory_result_iterator(
            [instance, prefix, options, context](const continuation_token& token, size_t max_results_per_segment)
        {
            return instance->list_files_and_directories_segmented(prefix, max_results_per_segment, token, options, context);
        },
            max_results, 0);
    }

    pplx::task<list_file_and_directory_result_segment> cloud_file_directory::list_files_and_directories_segmented_async(const utility::string_t& prefix, int64_t max_results, const continuation_token& token, const file_request_options& options, operation_context context) const
    {
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto command = std::make_shared<core::storage_command<list_file_and_directory_result_segment>>(uri());
        command->set_build_request(std::bind(protocol::list_files_and_directories, prefix, max_results, token, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary, token.target_location());
        command->set_preprocess_response(std::bind(protocol::preprocess_response<list_file_and_directory_result_segment>, list_file_and_directory_result_segment(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_postprocess_response([this](const web::http::http_response& response, const request_result& result, const core::ostream_descriptor&, operation_context context) -> pplx::task<list_file_and_directory_result_segment>
        {
            protocol::list_files_and_directories_reader reader(response.body());

            std::vector<list_file_and_directory_item> results(reader.move_items());

            for (std::vector<list_file_and_directory_item>::iterator iter = results.begin(); iter != results.end(); ++iter)
            {
                iter->set_directory(*this);
            }

            continuation_token next_token(reader.move_next_marker());
            next_token.set_target_location(result.target_location());
            return pplx::task_from_result(list_file_and_directory_result_segment(std::move(results), next_token));
        });
        return core::executor<list_file_and_directory_result_segment>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_file_directory::create_async(const file_access_condition& access_condition, const file_request_options& options, operation_context context)
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::create_file_directory, metadata(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            *properties = protocol::file_response_parsers::parse_file_directory_properties(response);
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }
    
    pplx::task<bool> cloud_file_directory::create_if_not_exists_async(const file_access_condition& access_condition, const file_request_options& options, operation_context context)
    {
        auto instance = std::make_shared<cloud_file_directory>(*this);

        return exists_async(true, access_condition, options, context).then([instance, access_condition, options, context] (bool exists) -> pplx::task<bool> {
            if (!exists)
            {
                return instance->create_async(access_condition, options, context).then([](pplx::task<void> create_task) {
                    try
                    {
                        create_task.wait();
                        return true;
                    }
                    catch (const storage_exception&)
                    {
                        throw;
                    }
                });
            }
            else
            {
                return pplx::task_from_result(false);
            }
        });
    }

    pplx::task<void> cloud_file_directory::delete_directory_async(const file_access_condition& access_condition, const file_request_options& options, operation_context context)
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::delete_file_directory, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<bool> cloud_file_directory::delete_directory_if_exists_async(const file_access_condition& access_condition, const file_request_options& options, operation_context context)
    {
        auto instance = std::make_shared<cloud_file_directory>(*this);

        return exists_async(true, access_condition, options, context).then([instance, access_condition, options, context](bool exists) -> pplx::task<bool> {
            if (exists)
            {
                return instance->delete_directory_async(access_condition, options, context).then([](pplx::task<void> delete_task) -> bool
                {
                    try
                    {
                        delete_task.wait();
                        return true;
                    }
                    catch (const storage_exception&)
                    {
                        throw;
                    }
                });
            }
            else
            {
                return pplx::task_from_result(false);
            }
        });
    }

    pplx::task<void> cloud_file_directory::download_attributes_async(const file_access_condition& access_condition, const file_request_options& options, operation_context context)
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;
        auto metadata = m_metadata;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::get_file_directory_properties, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties, metadata](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            *properties = protocol::file_response_parsers::parse_file_directory_properties(response);
            *metadata = protocol::parse_metadata(response);
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_file_directory::upload_metadata_async(const file_access_condition& access_condition, const file_request_options& options, operation_context context) const
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::set_file_directory_metadata, metadata(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::file_response_parsers::parse_file_directory_properties(response));
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    cloud_file cloud_file_directory::get_file_reference(const utility::string_t& name) const
    {
        return cloud_file(name, *this);
    }

    cloud_file_directory cloud_file_directory::get_parent_directory_reference() const
    {
        utility::string_t parent_name(core::get_parent_name(m_uri.path(), _XPLATSTR("/")));
        
        if (parent_name.empty() || parent_name == m_share.uri().path())
        {
            return m_share.get_root_directory_reference();
        }
        else
        {
            web::http::uri_builder primary_builder(m_uri.primary_uri());
            primary_builder.set_path(parent_name);
            web::http::uri_builder secondary_builder(m_uri.secondary_uri());
            secondary_builder.set_path(parent_name);

            return cloud_file_directory(storage_uri(primary_builder.to_uri(), secondary_builder.to_uri()), m_share.service_client().credentials());
        }
    }

    pplx::task<bool> cloud_file_directory::exists_async(bool primary_only, const file_access_condition& access_condition, const file_request_options& options, operation_context context)
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;
        auto metadata = m_metadata;

        auto command = std::make_shared<core::storage_command<bool>>(uri());
        command->set_build_request(std::bind(protocol::get_file_directory_properties, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(primary_only ? core::command_location_mode::primary_only : core::command_location_mode::primary_or_secondary);
        command->set_preprocess_response([properties, metadata](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            if (response.status_code() == web::http::status_codes::NotFound)
            {
                return false;
            }
            protocol::preprocess_response_void(response, result, context);
            *properties = protocol::file_response_parsers::parse_file_directory_properties(response);
            *metadata = protocol::parse_metadata(response);
            return true;
        });
        return core::executor<bool>::execute_async(command, modified_options, context);
    }
}} // namespace azure::storeage