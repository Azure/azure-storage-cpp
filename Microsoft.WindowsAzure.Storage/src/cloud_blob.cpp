// -----------------------------------------------------------------------------------------
// <copyright file="cloud_blob.cpp" company="Microsoft">
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

#include "was/blob.h"
#include "was/error_code_strings.h"
#include "wascore/protocol.h"
#include "wascore/resources.h"
#include "wascore/blobstreams.h"
#include "wascore/util.h"
#include "wascore/async_semaphore.h"

namespace azure { namespace storage {

    cloud_blob::cloud_blob(storage_uri uri)
        : m_uri(std::move(uri)), m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_blob_properties>()),
        m_copy_state(std::make_shared<azure::storage::copy_state>())
    {
        init(utility::string_t(), storage_credentials());
    }

    cloud_blob::cloud_blob(storage_uri uri, storage_credentials credentials)
        : m_uri(std::move(uri)), m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_blob_properties>()),
        m_copy_state(std::make_shared<azure::storage::copy_state>())
    {
        init(utility::string_t(), std::move(credentials));
    }

    cloud_blob::cloud_blob(storage_uri uri, utility::string_t snapshot_time, storage_credentials credentials)
        : m_uri(std::move(uri)), m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_blob_properties>()),
        m_copy_state(std::make_shared<azure::storage::copy_state>())
    {
        init(std::move(snapshot_time), std::move(credentials));
    }

    cloud_blob::cloud_blob(utility::string_t name, utility::string_t snapshot_time, cloud_blob_container container)
        : m_name(std::move(name)), m_snapshot_time(std::move(snapshot_time)), m_container(std::move(container)), m_uri(core::append_path_to_uri(m_container.uri(), m_name)),
        m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_blob_properties>()),
        m_copy_state(std::make_shared<azure::storage::copy_state>())
    {
    }

    cloud_blob::cloud_blob(utility::string_t name, utility::string_t snapshot_time, cloud_blob_container container, cloud_blob_properties properties, cloud_metadata metadata, azure::storage::copy_state copy_state)
        : m_name(std::move(name)), m_snapshot_time(std::move(snapshot_time)), m_container(std::move(container)), m_uri(core::append_path_to_uri(m_container.uri(), m_name)),
        m_metadata(std::make_shared<cloud_metadata>(std::move(metadata))), m_properties(std::make_shared<cloud_blob_properties>(std::move(properties))),
        m_copy_state(std::make_shared<azure::storage::copy_state>(std::move(copy_state)))
    {
    }

    void cloud_blob::init(utility::string_t snapshot_time, storage_credentials credentials)
    {
        m_snapshot_time = std::move(snapshot_time);
        m_uri = core::verify_blob_uri(m_uri, credentials, m_snapshot_time);

        utility::string_t container_name;
        if (!core::parse_blob_uri(m_uri, container_name, m_name))
        {
            throw std::invalid_argument("uri");
        }

        m_container = cloud_blob_container(std::move(container_name), cloud_blob_client(core::get_service_client_uri(m_uri), std::move(credentials)));
    }

    web::http::uri add_snapshot_to_uri(const web::http::uri& uri, const utility::string_t& snapshot_time)
    {
        if (uri.is_empty() || snapshot_time.empty())
        {
            return uri;
        }

        web::http::uri_builder builder(uri);
        builder.append_query(core::make_query_parameter(protocol::uri_query_snapshot, snapshot_time));
        return builder.to_uri();
    }

    storage_uri cloud_blob::snapshot_qualified_uri() const
    {
        return storage_uri(add_snapshot_to_uri(m_uri.primary_uri(), m_snapshot_time), add_snapshot_to_uri(m_uri.secondary_uri(), m_snapshot_time));
    }

    cloud_blob_directory cloud_blob::get_parent_reference() const
    {
        utility::string_t parent_name(core::get_parent_name(m_name, m_container.service_client().directory_delimiter()));
        if (parent_name.empty())
        {
            return cloud_blob_directory();
        }
        else
        {
            return cloud_blob_directory(parent_name, m_container);
        }
    }

    utility::string_t cloud_blob::get_premium_access_tier_string(const premium_blob_tier tier)
    {
        switch (tier)
        {
        case premium_blob_tier::p4:
            return protocol::header_value_access_tier_p4;

        case premium_blob_tier::p6:
            return protocol::header_value_access_tier_p6;

        case premium_blob_tier::p10:
            return protocol::header_value_access_tier_p10;

        case premium_blob_tier::p20:
            return protocol::header_value_access_tier_p20;

        case premium_blob_tier::p30:
            return protocol::header_value_access_tier_p30;

        case premium_blob_tier::p40:
            return protocol::header_value_access_tier_p40;

        case premium_blob_tier::p50:
            return protocol::header_value_access_tier_p50;

        case premium_blob_tier::p60:
            return protocol::header_value_access_tier_p60;

        default:
            return protocol::header_value_access_tier_unknown;
        }
    }

    utility::string_t cloud_blob::get_shared_access_signature(const blob_shared_access_policy& policy, const utility::string_t& stored_policy_identifier, const cloud_blob_shared_access_headers& headers) const
    {
        if (!service_client().credentials().is_shared_key())
        {
            throw std::logic_error(protocol::error_sas_missing_credentials);
        }

        // since 2015-02-21, canonicalized resource is changed from "/account/container/name" to "/blob/account/container/name"
        utility::string_t resource_str;
        resource_str.reserve(service_client().credentials().account_name().size() + container().name().size() + name().size() + 8);
        resource_str.append(_XPLATSTR("/"));
        resource_str.append(protocol::service_blob);
        resource_str.append(_XPLATSTR("/"));
        resource_str.append(service_client().credentials().account_name());
        resource_str.append(_XPLATSTR("/"));
        resource_str.append(container().name());
        resource_str.append(_XPLATSTR("/"));
        resource_str.append(name());

        return protocol::get_blob_sas_token(stored_policy_identifier, policy, headers, _XPLATSTR("b"), resource_str, service_client().credentials());
    }

    pplx::task<concurrency::streams::istream> cloud_blob::open_read_async(const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), type(), false);

        auto instance = std::make_shared<cloud_blob>(*this);
        return instance->download_attributes_async(condition, modified_options, context).then([instance, condition, modified_options, context] () -> concurrency::streams::istream
        {
            auto modified_condition = azure::storage::access_condition::generate_if_match_condition(instance->properties().etag());
            modified_condition.set_lease_id(condition.lease_id());
            return core::cloud_blob_istreambuf(instance, modified_condition, modified_options, context).create_istream();
        });
    }

    pplx::task<void> cloud_blob::download_attributes_async(const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), type());

        auto properties = m_properties;
        auto metadata = m_metadata;
        auto copy_state = m_copy_state;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::get_blob_properties, snapshot_time(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary);
        command->set_preprocess_response([properties, metadata, copy_state] (const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_all(protocol::blob_response_parsers::parse_blob_properties(response));
            *metadata = protocol::parse_metadata(response);
            *copy_state = protocol::response_parsers::parse_copy_state(response);
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_blob::upload_metadata_async(const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), type());

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::set_blob_metadata, metadata(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_properties(response));
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_blob::upload_properties_async(const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::set_blob_properties, *properties, metadata(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            
            auto parsed_properties = protocol::blob_response_parsers::parse_blob_properties(response);
            properties->update_etag_and_last_modified(parsed_properties);
            properties->update_page_blob_sequence_number(parsed_properties);
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_blob::delete_blob_async(delete_snapshots_option snapshots_option, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::delete_blob, snapshots_option, snapshot_time(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());

        auto properties = m_properties;
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->initialization();
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<bool> cloud_blob::delete_blob_if_exists_async(delete_snapshots_option snapshots_option, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto instance = std::make_shared<cloud_blob>(*this);
        return exists_async(true, modified_options, context).then([instance, snapshots_option, condition, modified_options, context] (bool exists_result) -> pplx::task<bool>
        {
            if (exists_result)
            {
                return instance->delete_blob_async(snapshots_option, condition, modified_options, context).then([] (pplx::task<void> delete_task) -> bool
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
                            (result.extended_error().code() == protocol::error_code_blob_not_found))
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
                return pplx::task_from_result(false);
            }
        });
    }

    pplx::task<utility::string_t> cloud_blob::acquire_lease_async(const lease_time& duration, const utility::string_t& proposed_lease_id, const access_condition& condition, const blob_request_options& options, operation_context context) const
    {
        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;
        
        auto command = std::make_shared<core::storage_command<utility::string_t>>(uri());
        command->set_build_request(std::bind(protocol::lease_blob, protocol::header_value_lease_acquire, proposed_lease_id, duration, lease_break_period(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context) -> utility::string_t
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_properties(response));
            return protocol::parse_lease_id(response);
        });
        return core::executor<utility::string_t>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_blob::renew_lease_async(const access_condition& condition, const blob_request_options& options, operation_context context) const
    {
        if (condition.lease_id().empty())
        {
            throw std::invalid_argument("condition");
        }

        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::lease_blob, protocol::header_value_lease_renew, utility::string_t(), lease_time(), lease_break_period(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_properties(response));
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<utility::string_t> cloud_blob::change_lease_async(const utility::string_t& proposed_lease_id, const access_condition& condition, const blob_request_options& options, operation_context context) const
    {
        if (condition.lease_id().empty())
        {
            throw std::invalid_argument("condition");
        }

        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<utility::string_t>>(uri());
        command->set_build_request(std::bind(protocol::lease_blob, protocol::header_value_lease_change, proposed_lease_id, lease_time(), lease_break_period(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context) -> utility::string_t
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_properties(response));
            return protocol::parse_lease_id(response);
        });
        return core::executor<utility::string_t>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_blob::release_lease_async(const access_condition& condition, const blob_request_options& options, operation_context context) const
    {
        if (condition.lease_id().empty())
        {
            throw std::invalid_argument("condition");
        }

        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::lease_blob, protocol::header_value_lease_release, utility::string_t(), lease_time(), lease_break_period(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_properties(response));
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<std::chrono::seconds> cloud_blob::break_lease_async(const lease_break_period& break_period, const access_condition& condition, const blob_request_options& options, operation_context context) const
    {
        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<std::chrono::seconds>>(uri());
        command->set_build_request(std::bind(protocol::lease_blob, protocol::header_value_lease_break, utility::string_t(), lease_time(), break_period, condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context) -> std::chrono::seconds
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_properties(response));
            return protocol::parse_lease_time(response);
        });
        return core::executor<std::chrono::seconds>::execute_async(command, modified_options, context);
    }

    struct blob_download_info
    {
        bool m_are_properties_populated;
        utility::size64_t m_total_written_to_destination_stream;
        utility::size64_t m_response_length;
        utility::string_t m_response_md5;
        utility::string_t m_locked_etag;
        bool m_reset_target;
        concurrency::streams::ostream::pos_type m_target_offset;
    };

    pplx::task<void> cloud_blob::download_single_range_to_stream_async(concurrency::streams::ostream target, utility::size64_t offset, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context, bool update_properties)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;
        auto metadata = m_metadata;
        auto copy_state = m_copy_state;
        const utility::string_t& current_snapshot_time = snapshot_time();

        std::shared_ptr<blob_download_info> download_info = std::make_shared<blob_download_info>();
        download_info->m_are_properties_populated = false;
        download_info->m_total_written_to_destination_stream = 0;
        download_info->m_response_length = std::numeric_limits<utility::size64_t>::max();
        download_info->m_reset_target = false;
        download_info->m_target_offset = target.can_seek() ? target.tell() : static_cast<Concurrency::streams::basic_ostream<unsigned char>::pos_type>(0);

        std::shared_ptr<core::storage_command<void>> command = std::make_shared<core::storage_command<void>>(uri());
        std::weak_ptr<core::storage_command<void>> weak_command(command);
        command->set_build_request([offset, length, modified_options, condition, current_snapshot_time, download_info](web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context) -> web::http::http_request
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
                        // The entire blob has already been downloaded
                        throw std::invalid_argument("offset");
                    }
                }
            }

            access_condition current_condition;
            if (download_info->m_are_properties_populated && !download_info->m_locked_etag.empty())
            {
                current_condition.set_if_match_etag(download_info->m_locked_etag);

                if (!condition.lease_id().empty())
                {
                    current_condition.set_lease_id(condition.lease_id());
                }
            }
            else
            {
                current_condition = condition;
            }

            return protocol::get_blob(current_offset, current_length, modified_options.use_transactional_md5() && !download_info->m_are_properties_populated, current_snapshot_time, current_condition, uri_builder, timeout, context);
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
        command->set_preprocess_response([weak_command, offset, modified_options, properties, metadata, copy_state, download_info, update_properties](const web::http::http_response& response, const request_result& result, operation_context context)
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
                if (update_properties == true)
                {
                    properties->update_all(protocol::blob_response_parsers::parse_blob_properties(response));
                    *metadata = protocol::parse_metadata(response);
                    *copy_state = protocol::response_parsers::parse_copy_state(response);
                }

                download_info->m_response_length = result.content_length();
                download_info->m_response_md5 = result.content_md5();

                if (modified_options.use_transactional_md5() && !modified_options.disable_content_md5_validation() && download_info->m_response_md5.empty())
                {
                    throw storage_exception(protocol::error_missing_md5);
                }

                // Lock to the current storage location when resuming a failed download. This is locked 
                // early before the retry policy has the opportunity to change the storage location.
                command->set_location_mode(core::command_location_mode::primary_or_secondary, result.target_location());

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

    pplx::task<void> cloud_blob::download_range_to_stream_async(concurrency::streams::ostream target, utility::size64_t offset, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        if (options.parallelism_factor() > 1)
        {
            auto instance = std::make_shared<cloud_blob>(*this);
            // if download a whole blob, enable download strategy(download 32MB first).
            utility::size64_t single_blob_download_threshold(protocol::default_single_blob_download_threshold);
            // If tranactional md5 validation is set, first range should be 4MB.
            if (options.use_transactional_md5())
            {
                single_blob_download_threshold = protocol::default_single_block_download_threshold;
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
            return instance->download_single_range_to_stream_async(target, offset, length < single_blob_download_threshold ? length : single_blob_download_threshold, condition, options, context, true).then([=](pplx::task<void> download_task)
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
                if (target_length <= single_blob_download_threshold)
                {
                    return pplx::task_from_result();
                }
                target_offset += single_blob_download_threshold;
                target_length -= single_blob_download_threshold;

                access_condition modified_condition(condition);
                if (condition.if_match_etag().empty())
                {
                    modified_condition.set_if_match_etag(instance->properties().etag());
                }

                return pplx::task_from_result().then([instance, offset, target, target_offset, target_length, single_blob_download_threshold, modified_condition, options, context]()
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
                        semaphore->lock_async().then([instance, &mutex, semaphore, condition_variable, &condition_variable_mutex, &writer, offset, target, smallest_offset, current_offset, current_length, modified_condition, options, context]()
                        {
                            concurrency::streams::container_buffer<std::vector<uint8_t>> buffer;
                            auto segment_ostream = buffer.create_ostream();
                            // if trasaction MD5 is enabled, it will be checked inside each download_single_range_to_stream_async.
                            instance->download_single_range_to_stream_async(segment_ostream, current_offset, current_length, modified_condition, options, context)
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

    pplx::task<void> cloud_blob::download_to_file_async(const utility::string_t &path, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        auto instance = std::make_shared<cloud_blob>(*this);
        return concurrency::streams::file_stream<uint8_t>::open_ostream(path).then([instance, condition, options, context] (concurrency::streams::ostream stream) -> pplx::task<void>
        {
            return instance->download_to_stream_async(stream, condition, options, context).then([stream] (pplx::task<void> upload_task) -> pplx::task<void>
            {
                return stream.close().then([upload_task]()
                {
                    upload_task.wait();
                });
            });
        });
    }

    pplx::task<bool> cloud_blob::exists_async(bool primary_only, const blob_request_options& options, operation_context context)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;
        auto metadata = m_metadata;
        auto copy_state = m_copy_state;

        auto command = std::make_shared<core::storage_command<bool>>(uri());
        command->set_build_request(std::bind(protocol::get_blob_properties, snapshot_time(), access_condition(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(primary_only ? core::command_location_mode::primary_only : core::command_location_mode::primary_or_secondary);
        command->set_preprocess_response([properties, metadata, copy_state] (const web::http::http_response& response, const request_result& result, operation_context context) -> bool
        {
            if (response.status_code() == web::http::status_codes::NotFound)
            {
                return false;
            }

            protocol::preprocess_response_void(response, result, context);
            properties->update_all(protocol::blob_response_parsers::parse_blob_properties(response));
            *metadata = protocol::parse_metadata(response);
            *copy_state = protocol::response_parsers::parse_copy_state(response);
            return true;
        });
        return core::executor<bool>::execute_async(command, modified_options, context);
    }

    pplx::task<utility::string_t> cloud_blob::start_copy_async_impl(const web::http::uri& source, const premium_blob_tier tier, const access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context)
    {
        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;
        auto copy_state = m_copy_state;

        auto command = std::make_shared<core::storage_command<utility::string_t>>(uri());
        command->set_build_request(std::bind(protocol::copy_blob, source, get_premium_access_tier_string(tier), source_condition, metadata(), destination_condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties, copy_state, tier] (const web::http::http_response& response, const request_result& result, operation_context context) -> utility::string_t
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_properties(response));
            auto new_state = protocol::response_parsers::parse_copy_state(response);
            properties->m_premium_blob_tier = tier;
            *copy_state = new_state;
            return new_state.copy_id();
        });
        return core::executor<utility::string_t>::execute_async(command, modified_options, context);
    }

    pplx::task<utility::string_t> cloud_blob::start_copy_async(const cloud_blob& source, const access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context)
    {
        web::http::uri raw_source_uri = source.snapshot_qualified_uri().primary_uri();
        web::http::uri source_uri = source.service_client().credentials().transform_uri(raw_source_uri);

        return start_copy_async(source_uri, source_condition, destination_condition, options, context);
    }

    pplx::task<utility::string_t> cloud_blob::start_copy_async(const cloud_file& source)
    {
        return start_copy_async(source, file_access_condition(), access_condition(), blob_request_options(), operation_context());
    }

    pplx::task<utility::string_t> cloud_blob::start_copy_async(const cloud_file& source, const file_access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context)
    {
        UNREFERENCED_PARAMETER(source_condition);
        web::http::uri raw_source_uri = source.uri().primary_uri();
        web::http::uri source_uri = source.service_client().credentials().transform_uri(raw_source_uri);

        return start_copy_async(source_uri, access_condition(), destination_condition, options, context);
    }

    pplx::task<void> cloud_blob::abort_copy_async(const utility::string_t& copy_id, const access_condition& condition, const blob_request_options& options, operation_context context) const
    {
        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::abort_copy_blob, copy_id, condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response(std::bind(protocol::preprocess_response_void, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<cloud_blob> cloud_blob::create_snapshot_async(cloud_metadata metadata, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;
        auto snapshot_name = name();
        auto snapshot_container = container();
        auto snapshot_metadata = std::make_shared<cloud_metadata>(std::move(metadata));
        auto resulting_metadata = snapshot_metadata->empty() ? m_metadata : snapshot_metadata;

        auto command = std::make_shared<core::storage_command<cloud_blob>>(uri());
        command->set_build_request(std::bind(protocol::snapshot_blob, *snapshot_metadata, condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([snapshot_name, snapshot_container, resulting_metadata, properties] (const web::http::http_response& response, const request_result& result, operation_context context) -> cloud_blob
        {
            protocol::preprocess_response_void(response, result, context);
            auto snapshot_time = protocol::get_header_value(response, protocol::ms_header_snapshot);
            cloud_blob snapshot(snapshot_name, snapshot_time, snapshot_container);
            *snapshot.m_metadata = *resulting_metadata;
            snapshot.m_properties->copy_from_root(*properties);
            snapshot.m_properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_properties(response));
            return snapshot;
        });
        return core::executor<cloud_blob>::execute_async(command, modified_options, context);
    }

    void cloud_blob::assert_no_snapshot() const
    {
        if (!m_snapshot_time.empty())
        {
            throw std::logic_error(protocol::error_cannot_modify_snapshot);
        }
    }

}} // namespace azure::storage
