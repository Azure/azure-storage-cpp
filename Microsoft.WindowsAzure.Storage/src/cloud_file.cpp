// -----------------------------------------------------------------------------------------
// <copyright file="cloud_file.cpp" company="Microsoft">
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

#include <condition_variable>

#include "was/file.h"
#include "was/error_code_strings.h"
#include "wascore/protocol.h"
#include "wascore/protocol_xml.h"
#include "wascore/util.h"
#include "wascore/constants.h"
#include "wascore/filestream.h"

namespace azure { namespace storage {


    void cloud_file_properties::update_etag_and_last_modified(const cloud_file_properties& other)
    {
        m_etag = other.etag();
        m_last_modified = other.last_modified();
    }

    void cloud_file_properties::update_etag(const cloud_file_properties& other)
    {
        m_etag = other.etag();
    }

    cloud_file::cloud_file(storage_uri uri)
        : m_uri(std::move(uri)), m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_file_properties>()),
        m_copy_state(std::make_shared<azure::storage::copy_state>())
    {
        init(std::move(storage_credentials()));
    }
    
    cloud_file::cloud_file(storage_uri uri, storage_credentials credentials)
        : m_uri(std::move(uri)), m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_file_properties>()),
        m_copy_state(std::make_shared<azure::storage::copy_state>())
    {
        init(std::move(credentials));
    }

    cloud_file::cloud_file(utility::string_t name, cloud_file_directory directory)
        : m_name(std::move(name)), m_directory(std::move(directory)), m_uri(core::append_path_to_uri(m_directory.uri(), m_name)),
        m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_file_properties>()),
        m_copy_state(std::make_shared<azure::storage::copy_state>())
    {
    }

    cloud_file::cloud_file(utility::string_t name, cloud_file_directory directory, cloud_file_properties properties, cloud_metadata metadata, azure::storage::copy_state copy_state)
        : m_name(std::move(name)), m_directory(std::move(directory)), m_uri(core::append_path_to_uri(m_directory.uri(), m_name)),
        m_metadata(std::make_shared<cloud_metadata>(metadata)), m_properties(std::make_shared<cloud_file_properties>(properties)),
        m_copy_state(std::make_shared<azure::storage::copy_state>(copy_state))
    {
    }

    void cloud_file::init(storage_credentials credentials)
    {
        core::parse_query_and_verify(m_uri, credentials, true);
        m_uri = core::create_stripped_uri(m_uri);

        utility::string_t share_name;
        utility::string_t directory_name;
        if (!core::parse_file_uri(m_uri, share_name, directory_name, m_name))
        {
            throw std::invalid_argument("uri");
        }

        m_directory = cloud_file_client(core::get_service_client_uri(m_uri), std::move(credentials)).get_share_reference(share_name).get_directory_reference(directory_name);
    }

    pplx::task<void> cloud_file::create_async(int64_t length, const file_access_condition& access_condition, const file_request_options& options, operation_context context)
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::create_file, length, metadata(), this->properties(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties, length](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::file_response_parsers::parse_file_properties(response));
            properties->m_length = length;
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<bool> cloud_file::create_if_not_exists_async(int64_t length, const file_access_condition& access_condition, const file_request_options& options, operation_context context)
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto instance = std::make_shared<cloud_file>(*this);

        return exists_async(access_condition, modified_options, context).then([instance, length, access_condition, modified_options, context] (bool exists) -> pplx::task<bool>
        {
            if (!exists)
            {
                return instance->create_async(length, access_condition, modified_options, context).then([](pplx::task<void> create_task)
                {
                    create_task.wait();
                    return true;
                });
            }
            else
            {
                return pplx::task_from_result<bool>(false);
            }
        });
    }

    pplx::task<void> cloud_file::delete_file_async(const file_access_condition& access_condition, const file_request_options& options, operation_context context)
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::delete_file, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<bool> cloud_file::delete_file_if_exists_async(const file_access_condition& access_condition, const file_request_options& options, operation_context context)
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto instance = std::make_shared<cloud_file>(*this);

        return exists_async(false, access_condition, modified_options, context).then([instance, access_condition, modified_options, context](bool exists) -> pplx::task<bool>
        {
            if (exists)
            {
                return instance->delete_file_async(access_condition, modified_options, context).then([](pplx::task<void> delete_task)
                {
                    delete_task.wait();
                    return true;
                });
            }
            else
            {
                return pplx::task_from_result<bool>(false);
            }
        });
    }

    pplx::task<void> cloud_file::download_attributes_async(const file_access_condition& access_condition, const file_request_options& options, operation_context context)
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto metadata = m_metadata;
        auto properties = m_properties;
        auto copy_state = m_copy_state;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::get_file_properties, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([metadata, properties, copy_state](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            *properties = protocol::file_response_parsers::parse_file_properties(response);
            *metadata = protocol::parse_metadata(response);
            *copy_state = protocol::response_parsers::parse_copy_state(response);
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_file::upload_properties_async(const file_access_condition& access_condition, const file_request_options& options, operation_context context) const
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::set_file_properties, this->properties(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::file_response_parsers::parse_file_properties(response));
        });

        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_file::upload_metadata_async(const file_access_condition& access_condition, const file_request_options& options, operation_context context) const
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::set_file_metadata, this->metadata(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::file_response_parsers::parse_file_properties(response));
        });

        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<utility::string_t> cloud_file::start_copy_async(const web::http::uri& source, const file_access_condition& source_condition, const file_access_condition& dest_condition, const file_request_options& options, operation_context context) const
    {
        UNREFERENCED_PARAMETER(source_condition);
        UNREFERENCED_PARAMETER(dest_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;
        auto copy_state = m_copy_state;

        auto command = std::make_shared<core::storage_command<utility::string_t>>(uri());
        command->set_build_request(std::bind(protocol::copy_file, source, this->metadata(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties, copy_state](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::file_response_parsers::parse_file_properties(response));
            *copy_state = protocol::response_parsers::parse_copy_state(response);
            return copy_state->copy_id();
        });

        return core::executor<utility::string_t>::execute_async(command, modified_options, context);
    }

    pplx::task<utility::string_t> cloud_file::start_copy_async(const cloud_file& source, const file_access_condition& source_condition, const file_access_condition& dest_condition, const file_request_options& options, operation_context context) const
    {
        web::http::uri raw_source_uri = source.uri().primary_uri();
        web::http::uri source_uri = source.service_client().credentials().transform_uri(raw_source_uri);

        return start_copy_async(source_uri, source_condition, dest_condition, options, context);
    }

    pplx::task<utility::string_t> cloud_file::start_copy_async(const web::http::uri& source, const access_condition& source_condition, const file_access_condition& dest_condition, const file_request_options& options, operation_context context) const
    {
        UNREFERENCED_PARAMETER(dest_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;
        auto copy_state = m_copy_state;

        auto command = std::make_shared<core::storage_command<utility::string_t>>(uri());
        command->set_build_request(std::bind(protocol::copy_file_from_blob, source, source_condition, this->metadata(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties, copy_state](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::file_response_parsers::parse_file_properties(response));
            *copy_state = protocol::response_parsers::parse_copy_state(response);
            return copy_state->copy_id();
        });

        return core::executor<utility::string_t>::execute_async(command, modified_options, context);
    }


    pplx::task<utility::string_t> cloud_file::start_copy_async(const cloud_blob& source) const
    {
        return start_copy_async(source, access_condition(), file_access_condition(), file_request_options(), operation_context());
    }

    pplx::task<utility::string_t> cloud_file::start_copy_async(const cloud_blob& source, const access_condition& source_condition, const file_access_condition& dest_condition, const file_request_options& options, operation_context context) const
    {
        web::http::uri raw_source_uri = source.snapshot_qualified_uri().primary_uri();
        web::http::uri source_uri = source.service_client().credentials().transform_uri(raw_source_uri);

        return start_copy_async(source_uri, source_condition, dest_condition, options, context);
    }

    pplx::task<void> cloud_file::abort_copy_async(const utility::string_t& copy_id, const file_access_condition& access_condition, const file_request_options& options, operation_context context) const
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::abort_copy_file, copy_id, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response(std::bind(protocol::preprocess_response_void, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        return core::executor<void>::execute_async(command, modified_options, context);
    }
    
    pplx::task<std::vector<file_range>> cloud_file::list_ranges_async(utility::size64_t start_offset, utility::size64_t length, const file_access_condition& access_condition, const file_request_options& options, operation_context context) const
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<std::vector<file_range>>>(uri());
        command->set_build_request(std::bind(protocol::list_file_ranges, start_offset, length, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context) -> std::vector<file_range>
        {
            protocol::preprocess_response_void(response, result, context);
            auto modified_properties = protocol::file_response_parsers::parse_file_properties(response);
            properties->update_etag_and_last_modified(modified_properties);
            properties->m_length = modified_properties.length();
            return std::vector<file_range>();
        });

        command->set_postprocess_response([] (const web::http::http_response& response, const request_result&, const core::ostream_descriptor&, operation_context context) -> pplx::task<std::vector<file_range>>
        {
            UNREFERENCED_PARAMETER(context);
            UNREFERENCED_PARAMETER(response);
            protocol::list_file_ranges_reader reader(response.body());
            return pplx::task_from_result(reader.move_result());
        });
        return core::executor<std::vector<file_range>>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_file::clear_range_async(utility::size64_t start_offset, utility::size64_t length, const file_access_condition& access_condition, const file_request_options& options, operation_context context) const
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;
        auto end_offset = start_offset + length - 1;
        file_range range(start_offset, end_offset);

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::put_file_range, range, file_range_write::clear, utility::string_t(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            auto modified_properties = protocol::file_response_parsers::parse_file_properties(response);
            properties->update_etag_and_last_modified(modified_properties);
            properties->m_content_md5 = modified_properties.content_md5();
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }
    
    pplx::task<void> cloud_file::write_range_async(Concurrency::streams::istream stream, int64_t start_offset, const utility::string_t& content_md5, const file_access_condition& access_condition, const file_request_options& options, operation_context context) const
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;
        bool needs_md5 = content_md5.empty() && modified_options.use_transactional_md5();

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            auto modified_properties = protocol::file_response_parsers::parse_file_properties(response);
            properties->update_etag_and_last_modified(modified_properties);
            properties->m_content_md5 = modified_properties.content_md5();
        });
        return core::istream_descriptor::create(stream, needs_md5, std::numeric_limits<utility::size64_t>::max(), protocol::max_range_size).then([command, context, start_offset, content_md5, modified_options](core::istream_descriptor request_body)->pplx::task<void>
        {
            const utility::string_t& md5 = content_md5.empty() ? request_body.content_md5() : content_md5;
            auto end_offset = start_offset + request_body.length() - 1;
            file_range range(start_offset, end_offset);
            command->set_build_request(std::bind(protocol::put_file_range, range, file_range_write::update, md5, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            command->set_request_body(request_body);
            return core::executor<void>::execute_async(command, modified_options, context);
        });
    }

    struct file_download_info
    {
        bool m_are_properties_populated;
        utility::size64_t m_total_written_to_destination_stream;
        utility::size64_t m_response_length;
        utility::string_t m_response_md5;
        utility::string_t m_locked_etag;
        bool m_reset_target;
        concurrency::streams::ostream::pos_type m_target_offset;
    };

    pplx::task<void> cloud_file::download_single_range_to_stream_async(concurrency::streams::ostream target, utility::size64_t offset, utility::size64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context, bool update_properties, bool validate_last_modify) const
    {
        UNREFERENCED_PARAMETER(condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;
        auto metadata = m_metadata;
        auto copy_state = m_copy_state;

        std::shared_ptr<file_download_info> download_info = std::make_shared<file_download_info>();
        download_info->m_are_properties_populated = false;
        download_info->m_total_written_to_destination_stream = 0;
        download_info->m_response_length = std::numeric_limits<utility::size64_t>::max();
        download_info->m_reset_target = false;
        download_info->m_target_offset = target.can_seek() ? target.tell() : (Concurrency::streams::basic_ostream<unsigned char>::pos_type)0;

        std::shared_ptr<core::storage_command<void>> command = std::make_shared<core::storage_command<void>>(uri());
        std::weak_ptr<core::storage_command<void>> weak_command(command);
        command->set_build_request([offset, length, modified_options, download_info](web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context) -> web::http::http_request
        {
            utility::size64_t current_offset = offset;
            utility::size64_t current_length = length;
            if (download_info->m_total_written_to_destination_stream > 0)
            {
                if (offset == std::numeric_limits<utility::size64_t>::max())
                {
                    current_offset = 0;
                }

                current_offset += download_info->m_total_written_to_destination_stream;

                if (length > 0)
                {
                    current_length -= download_info->m_total_written_to_destination_stream;

                    if (current_length <= 0)
                    {
                        // The entire file has already been downloaded
                        throw std::invalid_argument("offset");
                    }
                }
            }

            return protocol::get_file(current_offset, current_length, modified_options.use_transactional_md5() && !download_info->m_are_properties_populated, uri_builder, timeout, context);
        });
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary);
        command->set_destination_stream(target);
        command->set_calculate_response_body_md5(!modified_options.disable_content_md5_validation());
        command->set_recover_request([target, download_info](utility::size64_t total_written_to_destination_stream, operation_context context) -> bool
        {
            if (download_info->m_reset_target)
            {
                download_info->m_total_written_to_destination_stream = 0;

                if (total_written_to_destination_stream > 0)
                {
                    if (!target.can_seek())
                    {
                        return false;
                    }

                    target.seek(download_info->m_target_offset);
                }

                download_info->m_reset_target = false;
            }
            else
            {
                download_info->m_total_written_to_destination_stream = total_written_to_destination_stream;
            }

            return target.is_open();
        });
        command->set_preprocess_response([weak_command, offset, modified_options, properties, metadata, copy_state, download_info, update_properties, validate_last_modify](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            std::shared_ptr<core::storage_command<void>> command(weak_command);

            try
            {
                protocol::preprocess_response_void(response, result, context);
            }
            catch (...)
            {
                // In case any error happens, error information contained in response body might
                // have been written into the destination stream. So need to reset target to make
                // sure the destination stream doesn't contain unexpected data since a retry might
                // be needed.
                download_info->m_reset_target = true;
                download_info->m_are_properties_populated = false;
                command->set_location_mode(core::command_location_mode::primary_or_secondary);

                throw;
            }

            if (!download_info->m_are_properties_populated)
            {
                if (validate_last_modify == true
                    && properties->last_modified() != protocol::file_response_parsers::parse_file_properties(response).last_modified())
                {
                    throw std::runtime_error("File is modified during downloading.");
                }

                if (update_properties == true)
                {
                    *properties = protocol::file_response_parsers::parse_file_properties(response);
                    *metadata = protocol::parse_metadata(response);
                    *copy_state = protocol::response_parsers::parse_copy_state(response);
                }

                download_info->m_response_length = result.content_length();
                download_info->m_response_md5 = result.content_md5();

                if (modified_options.use_transactional_md5() && !modified_options.disable_content_md5_validation() && download_info->m_response_md5.empty()
                    // If range is not set and the file has no MD5 hash, no content md5 will not be returned.
                    // Consider the file has no MD5 hash in default.
                    && offset < std::numeric_limits<utility::size64_t>::max())
                {
                    throw storage_exception(protocol::error_missing_md5);
                }

                // Lock to the current storage location when resuming a failed download. This is locked 
                // early before the retry policy has the opportunity to change the storage location.
                command->set_location_mode(core::command_location_mode::primary_or_secondary, result.target_location());

                download_info->m_locked_etag = properties->etag();
                download_info->m_are_properties_populated = true;
            }
        });
        command->set_postprocess_response([weak_command, download_info](const web::http::http_response&, const request_result&, const core::ostream_descriptor& descriptor, operation_context context) -> pplx::task<void>
        {
            std::shared_ptr<core::storage_command<void>> command(weak_command);

            // Start the download over from the beginning if a retry is needed again because the last
            // response was successfully downloaded and the MD5 hash has already been calculated
            download_info->m_reset_target = true;
            download_info->m_are_properties_populated = false;

            command->set_location_mode(core::command_location_mode::primary_or_secondary);

            if (!download_info->m_response_md5.empty() && !descriptor.content_md5().empty() && download_info->m_response_md5 != descriptor.content_md5())
            {
                throw storage_exception(protocol::error_md5_mismatch);
            }

            return pplx::task_from_result();
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_file::download_range_to_stream_async(concurrency::streams::ostream target, utility::size64_t offset, utility::size64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context) const
    {
        if (options.parallelism_factor() > 1)
        {
            auto instance = std::make_shared<cloud_file>(*this);
            // if download a whole blob, enable download strategy(download 32MB first).
            utility::size64_t single_file_download_threshold(protocol::default_single_blob_download_threshold);
            // If tranactional md5 validation is set, first range should be 4MB.
            if (options.use_transactional_md5())
            {
                single_file_download_threshold = protocol::default_single_block_download_threshold;
            }

            if (offset >= std::numeric_limits<utility::size64_t>::max())
            {
                if (length == 0)
                {
                    offset = 0;
                    length = std::numeric_limits<utility::size64_t>::max();
                }
                else
                {
                    throw std::invalid_argument("length");
                }
            }

            // download first range.
            // if 416 thrown, it's an empty blob. need to download attributes.
            // otherwise, properties must be updated for further parallel download.
            return instance->download_single_range_to_stream_async(target, offset, length < single_file_download_threshold ? length : single_file_download_threshold, condition, options, context, true).then([=](pplx::task<void> download_task)
            {
                try
                {
                    download_task.wait();
                }
                catch (storage_exception &e)
                {
                    // For empty blob, swallow the exception and update the attributes.
                    if (e.result().http_status_code() == web::http::status_codes::RangeNotSatisfiable
                        && offset == 0)
                    {
                        return instance->download_attributes_async(condition, options, context);
                    }
                    else
                    {
                        throw;
                    }
                }

                // download the rest data in parallel.
                utility::size64_t target_offset = offset;
                utility::size64_t target_length = length;
                if (target_length >= std::numeric_limits<utility::size64_t>::max()
                    || target_length > instance->properties().size() - offset)
                {
                    target_length = instance->properties().size() - offset;
                }

                // Download completes in first range download.
                if (target_length <= single_file_download_threshold)
                {
                    return pplx::task_from_result();
                }
                target_offset += single_file_download_threshold;
                target_length -= single_file_download_threshold;

                return pplx::task_from_result().then([instance, offset, target, target_offset, target_length, single_file_download_threshold, condition, options, context]()
                {
                    auto semaphore = std::make_shared<core::async_semaphore>(options.parallelism_factor());
                    // lock to the target ostream
                    pplx::extensibility::reader_writer_lock_t mutex;

                    // limit the number of parallel writer(maximum number is options.parallelism_factor()) to write to target stream. prevent OOM.
                    pplx::details::atomic_long writer(0);

                    auto smallest_offset = std::make_shared<utility::size64_t>(target_offset);
                    auto condition_variable = std::make_shared<std::condition_variable>();
                    std::mutex  condition_variable_mutex;
                    for (utility::size64_t current_offset = target_offset; current_offset < target_offset + target_length; current_offset += protocol::transactional_md5_block_size)
                    {
                        utility::size64_t current_length = protocol::transactional_md5_block_size;
                        if (current_offset + current_length > target_offset + target_length)
                        {
                            current_length = target_offset + target_length - current_offset;
                        }
                        semaphore->lock_async().then([instance, &mutex, semaphore, condition_variable, &condition_variable_mutex, &writer, offset, target, smallest_offset, current_offset, current_length, condition, options, context]()
                        {
                            concurrency::streams::container_buffer<std::vector<uint8_t>> buffer;
                            auto segment_ostream = buffer.create_ostream();
                            // if trasaction MD5 is enabled, it will be checked inside each download_single_range_to_stream_async.
                            instance->download_single_range_to_stream_async(segment_ostream, current_offset, current_length, condition, options, context)
                                .then([buffer, segment_ostream, semaphore, condition_variable, &condition_variable_mutex, smallest_offset, offset, current_offset, current_length, &mutex, target, &writer, options](pplx::task<void> download_task)
                            {
                                segment_ostream.close().then([download_task](pplx::task<void> close_task)
                                {
                                    download_task.wait();
                                    close_task.wait();
                                }).wait();

                                // status of current semaphore.
                                bool released = false;
                                // target stream is seekable, could write to target stream once the download finished.
                                if (target.can_seek())
                                {
                                    pplx::extensibility::scoped_rw_lock_t guard(mutex);
                                    target.streambuf().seekpos(current_offset - offset, std::ios_base::out);
                                    target.streambuf().putn_nocopy(buffer.collection().data(), buffer.collection().size()).wait();
                                    *smallest_offset += protocol::transactional_md5_block_size;
                                    released = true;
                                    semaphore->unlock();
                                }
                                else
                                {
                                    {
                                        pplx::extensibility::scoped_rw_lock_t guard(mutex);
                                        if (*smallest_offset == current_offset)
                                        {
                                            target.streambuf().putn_nocopy(buffer.collection().data(), buffer.collection().size()).wait();
                                            *smallest_offset += protocol::transactional_md5_block_size;
                                            condition_variable->notify_all();
                                            released = true;
                                            semaphore->unlock();
                                        }
                                    }
                                    if (!released)
                                    {
                                        pplx::details::atomic_increment(writer);
                                        if (writer < options.parallelism_factor())
                                        {
                                            released = true;
                                            semaphore->unlock();
                                        }
                                        std::unique_lock<std::mutex> locker(condition_variable_mutex);
                                        condition_variable->wait(locker, [smallest_offset, current_offset, &mutex]()
                                        {
                                            pplx::extensibility::scoped_rw_lock_t guard(mutex);
                                            return *smallest_offset == current_offset;
                                        });
                                        {
                                            pplx::extensibility::scoped_rw_lock_t guard(mutex);

                                            if (*smallest_offset == current_offset)
                                            {
                                                target.streambuf().putn_nocopy(buffer.collection().data(), buffer.collection().size()).wait();
                                                *smallest_offset += protocol::transactional_md5_block_size;
                                            }
                                            else if (*smallest_offset > current_offset)
                                            {
                                                throw std::runtime_error("Out of order in parallel downloading blob.");
                                            }
                                        }
                                        condition_variable->notify_all();
                                        pplx::details::atomic_decrement(writer);
                                        if (!released)
                                        {
                                            semaphore->unlock();
                                        }
                                    }
                                }
                            });
                        });
                    }
                    semaphore->wait_all_async().wait();
                    std::unique_lock<std::mutex> locker(condition_variable_mutex);
                    condition_variable->wait(locker, [smallest_offset, &mutex, target_offset, target_length]()
                    {
                        pplx::extensibility::scoped_rw_lock_t guard(mutex);
                        return *smallest_offset >= target_offset + target_length;
                    });
                });
            });
        }
        else
        {
            return download_single_range_to_stream_async(target, offset, length, condition, options, context, true);
        }
    }

    pplx::task<void> cloud_file::download_to_file_async(const utility::string_t &path, const file_access_condition& access_condition, const file_request_options& options, operation_context context) const
    {
        auto instance = std::make_shared<cloud_file>(*this);
        return concurrency::streams::file_stream<uint8_t>::open_ostream(path).then([instance, access_condition, options, context](concurrency::streams::ostream stream) -> pplx::task<void>
        {
            return instance->download_to_stream_async(stream, access_condition, options, context).then([stream](pplx::task<void> upload_task) -> pplx::task<void>
            {
                return stream.close().then([upload_task]()
                {
                    upload_task.wait();
                });
            });
        });
    }

    pplx::task<utility::string_t> cloud_file::download_text_async(const file_access_condition& access_condition, const file_request_options& options, operation_context context) const
    {
        auto properties = m_properties;

        concurrency::streams::container_buffer<std::vector<uint8_t>> buffer;
        return this->download_to_stream_async(buffer.create_ostream(), access_condition, options, context).then([buffer, properties]() mutable -> utility::string_t
        {
            if (properties->content_type() != protocol::header_value_content_type_utf8)
            {
                throw std::logic_error(protocol::error_unsupported_text);
            }

            std::string utf8_body(reinterpret_cast<char*>(buffer.collection().data()), static_cast<unsigned int>(buffer.size()));
            return utility::conversions::to_string_t(utf8_body);
        });
    }

    pplx::task<concurrency::streams::ostream> cloud_file::open_write_async(const file_access_condition& access_condition, const file_request_options& options, operation_context context) const
    {
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto instance = std::make_shared<cloud_file>(*this);
        return instance->download_attributes_async(access_condition, modified_options, context).then([instance, access_condition, modified_options, context]() -> concurrency::streams::ostream
        {
            return core::cloud_file_ostreambuf(instance, instance->properties().length(), access_condition, modified_options, context).create_ostream();
        });
    }
    
    pplx::task<concurrency::streams::ostream> cloud_file::open_write_async(utility::size64_t length, const file_access_condition& access_condition, const file_request_options& options, operation_context context) const
    {
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto instance = std::make_shared<cloud_file>(*this);
        return instance->create_async(length, access_condition, modified_options, context).then([instance, length, access_condition, modified_options, context]() -> concurrency::streams::ostream
        {
            return core::cloud_file_ostreambuf(instance, length, access_condition, modified_options, context).create_ostream();
        });
    }
    
    pplx::task<void> cloud_file::upload_from_stream_async(concurrency::streams::istream source, utility::size64_t length, const file_access_condition& access_condition, const file_request_options& options, operation_context context) const
    {
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        if (length == std::numeric_limits<utility::size64_t>::max())
        {
            length = core::get_remaining_stream_length(source);
            if (length == std::numeric_limits<utility::size64_t>::max())
            {
                throw std::logic_error(protocol::error_file_size_unknown);
            }
        }

        return open_write_async(length, access_condition, modified_options, context).then([source, length](concurrency::streams::ostream file_stream) -> pplx::task<void>
        {
            return core::stream_copy_async(source, file_stream, length).then([file_stream](utility::size64_t) -> pplx::task<void>
            {
                return file_stream.close();
            });
        });
    }
    
    pplx::task<void> cloud_file::upload_from_file_async(const utility::string_t& path, const file_access_condition& access_condition, const file_request_options& options, operation_context context) const
    {
        auto instance = std::make_shared<cloud_file>(*this);
        return concurrency::streams::file_stream<uint8_t>::open_istream(path).then([instance, access_condition, options, context](concurrency::streams::istream stream) -> pplx::task<void>
        {
            return instance->upload_from_stream_async(stream, access_condition, options, context).then([stream](pplx::task<void> upload_task) -> pplx::task<void>
            {
                return stream.close().then([upload_task]()
                {
                    upload_task.wait();
                });
            });
        });
    }
    
    pplx::task<void> cloud_file::upload_text_async(const utility::string_t& text, const file_access_condition& condition, const file_request_options& options, operation_context context) const
    {
        auto utf8_body = utility::conversions::to_utf8string(text);
        auto length = utf8_body.size();
        auto stream = concurrency::streams::bytestream::open_istream(std::move(utf8_body));
        m_properties->set_content_type(protocol::header_value_content_type_utf8);
        return upload_from_stream_async(stream, length, condition, options, context);
    }

    pplx::task<void> cloud_file::resize_async(int64_t length, const file_access_condition& access_condition, const file_request_options& options, operation_context context) const
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;
        properties->m_length = length;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::resize_with_properties, this->properties(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::file_response_parsers::parse_file_properties(response));
        });

        return core::executor<void>::execute_async(command, modified_options, context);
    }

    const utility::string_t cloud_file::path() const
    {
        auto start = azure::storage::core::find_path_start(m_uri.primary_uri());
        return m_uri.primary_uri().path().substr(start);
    }

    utility::string_t cloud_file::get_shared_access_signature(const file_shared_access_policy& policy, const utility::string_t& stored_policy_identifier, const cloud_file_shared_access_headers& headers) const
    {
        if (!service_client().credentials().is_shared_key())
        {
            throw std::logic_error(protocol::error_sas_missing_credentials);
        }

        // since 2015-02-21, canonicalized resource is changed from "/account/container/name" to "/blob/account/container/name"
        utility::string_t resource_str;
        auto path_name = this->path();
        resource_str.reserve(service_client().credentials().account_name().size() + path_name.size() + 8);
        resource_str.append(_XPLATSTR("/"));
        resource_str.append(protocol::service_file);
        resource_str.append(_XPLATSTR("/"));
        resource_str.append(service_client().credentials().account_name());
        if (path_name[0] != _XPLATSTR('/'))
        {
            resource_str.append(_XPLATSTR("/"));
        }
        resource_str.append(path_name);

        return protocol::get_file_sas_token(stored_policy_identifier, policy, headers, _XPLATSTR("f"), resource_str, service_client().credentials());
    }

    pplx::task<bool> cloud_file::exists_async(bool primary_only, const file_access_condition& access_condition, const file_request_options& options, operation_context context) const
    {
        UNREFERENCED_PARAMETER(access_condition);
        file_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options());

        auto properties = m_properties;
        auto metadata = m_metadata;

        auto command = std::make_shared<core::storage_command<bool>>(uri());
        command->set_build_request(std::bind(protocol::get_file_properties, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(primary_only ? core::command_location_mode::primary_only : core::command_location_mode::primary_or_secondary);
        command->set_preprocess_response([properties, metadata](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            if (response.status_code() == web::http::status_codes::NotFound)
            {
                return false;
            }
            protocol::preprocess_response_void(response, result, context);
            *properties = protocol::file_response_parsers::parse_file_properties(response);
            *metadata = protocol::parse_metadata(response);
            return true;
        });
        return core::executor<bool>::execute_async(command, modified_options, context);
    }

}}