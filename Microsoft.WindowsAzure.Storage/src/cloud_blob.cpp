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
#include "was/blob.h"
#include "wascore/protocol.h"
#include "wascore/resources.h"
#include "wascore/blobstreams.h"
#include "wascore/util.h"

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

    utility::string_t cloud_blob::get_shared_access_signature(const blob_shared_access_policy& policy, const utility::string_t& stored_policy_identifier, const cloud_blob_shared_access_headers& headers) const
    {
        if (!service_client().credentials().is_shared_key())
        {
            throw std::logic_error(protocol::error_sas_missing_credentials);
        }

        utility::ostringstream_t resource_str;
        resource_str << U('/') << service_client().credentials().account_name() << U('/') << container().name() << U('/') << name();

        return protocol::get_blob_sas_token(stored_policy_identifier, policy, headers, U("b"), resource_str.str(), service_client().credentials());
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
        command->set_preprocess_response([properties, metadata, copy_state] (const web::http::http_response& response, operation_context context)
        {
            protocol::preprocess_response(response, context);
            properties->update_all(protocol::blob_response_parsers::parse_blob_properties(response), false);
            *metadata = protocol::parse_metadata(response);
            *copy_state = protocol::blob_response_parsers::parse_copy_state(response);
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
        command->set_preprocess_response([properties] (const web::http::http_response& response, operation_context context)
        {
            protocol::preprocess_response(response, context);
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
        command->set_preprocess_response([properties] (const web::http::http_response& response, operation_context context)
        {
            protocol::preprocess_response(response, context);
            
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
        command->set_preprocess_response(std::bind(protocol::preprocess_response, std::placeholders::_1, std::placeholders::_2));
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
        command->set_preprocess_response([properties] (const web::http::http_response& response, operation_context context) -> utility::string_t
        {
            protocol::preprocess_response(response, context);
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
        command->set_preprocess_response([properties] (const web::http::http_response& response, operation_context context)
        {
            protocol::preprocess_response(response, context);
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
        command->set_preprocess_response([properties] (const web::http::http_response& response, operation_context context) -> utility::string_t
        {
            protocol::preprocess_response(response, context);
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
        command->set_preprocess_response([properties] (const web::http::http_response& response, operation_context context)
        {
            protocol::preprocess_response(response, context);
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
        command->set_preprocess_response([properties] (const web::http::http_response& response, operation_context context) -> std::chrono::seconds
        {
            protocol::preprocess_response(response, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_properties(response));
            return protocol::parse_lease_time(response);
        });
        return core::executor<std::chrono::seconds>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_blob::download_range_to_stream_async(concurrency::streams::ostream target, utility::size64_t offset, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;
        auto metadata = m_metadata;
        auto copy_state = m_copy_state;
        auto target_offset = target.can_seek() ? target.tell() : 0;
        auto response_md5 = std::make_shared<utility::string_t>();
        auto response_length = std::make_shared<utility::size64_t>(std::numeric_limits<utility::size64_t>::max());

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::get_blob, offset, length, modified_options.use_transactional_md5(), snapshot_time(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary);
        command->set_destination_stream(target);
        command->set_calculate_response_body_md5(!modified_options.disable_content_md5_validation());
        command->set_recover_request([target, target_offset] (operation_context context) -> bool
        {
            // TODO support resume
            if (target.can_seek())
            {
                target.seek(target_offset);
                return true;
            }
            else
            {
                return false;
            }
        });
        command->set_preprocess_response([modified_options, properties, metadata, copy_state, offset, response_md5, response_length] (const web::http::http_response& response, operation_context context)
        {
            protocol::preprocess_response(response, context);
            properties->update_all(protocol::blob_response_parsers::parse_blob_properties(response), offset < std::numeric_limits<utility::size64_t>::max());
            *metadata = protocol::parse_metadata(response);
            *copy_state = protocol::blob_response_parsers::parse_copy_state(response);
            
            *response_md5 = protocol::get_header_value(response, web::http::header_names::content_md5);
            
            if (modified_options.use_transactional_md5() && !modified_options.disable_content_md5_validation() && response_md5->empty())
            {
                throw storage_exception(protocol::error_missing_md5);
            }

            *response_length = response.headers().content_length();
        });
        command->set_postprocess_response([response_md5, response_length] (const web::http::http_response&, const request_result&, const core::ostream_descriptor& descriptor, operation_context context) -> pplx::task<void>
        {
            protocol::check_stream_length_and_md5(*response_length, *response_md5, descriptor);
            return pplx::task_from_result();
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_blob::download_to_file_async(const utility::string_t &path, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        auto instance = std::make_shared<cloud_blob>(*this);
        return concurrency::streams::file_stream<uint8_t>::open_ostream(path).then([instance, condition, options, context] (concurrency::streams::ostream stream) -> pplx::task<void>
        {
            return instance->download_to_stream_async(stream, condition, options, context).then([stream] (pplx::task<void> download_task) -> pplx::task<void>
            {
                return stream.close().then([download_task] ()
                {
                    download_task.wait();
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
        command->set_preprocess_response([properties, metadata, copy_state] (const web::http::http_response& response, operation_context context) -> bool
        {
            if (response.status_code() == web::http::status_codes::NotFound)
            {
                return false;
            }

            protocol::preprocess_response(response, context);
            properties->update_all(protocol::blob_response_parsers::parse_blob_properties(response), false);
            *metadata = protocol::parse_metadata(response);
            *copy_state = protocol::blob_response_parsers::parse_copy_state(response);
            return true;
        });
        return core::executor<bool>::execute_async(command, modified_options, context);
    }

    pplx::task<utility::string_t> cloud_blob::start_copy_from_blob_async(const web::http::uri& source, const access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context)
    {
        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;
        auto copy_state = m_copy_state;

        auto command = std::make_shared<core::storage_command<utility::string_t>>(uri());
        command->set_build_request(std::bind(protocol::copy_blob, source, source_condition, metadata(), destination_condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties, copy_state] (const web::http::http_response& response, operation_context context) -> utility::string_t
        {
            protocol::preprocess_response(response, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_properties(response));
            auto new_state = protocol::blob_response_parsers::parse_copy_state(response);
            *copy_state = new_state;
            return new_state.copy_id();
        });
        return core::executor<utility::string_t>::execute_async(command, modified_options, context);
    }

    pplx::task<utility::string_t> cloud_blob::start_copy_from_blob_async(const cloud_blob& source, const access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context)
    {
        const web::http::uri& raw_source_uri = source.snapshot_qualified_uri().primary_uri();
        web::http::uri source_uri = service_client().credentials().transform_uri(raw_source_uri);

        return start_copy_from_blob_async(source_uri, source_condition, destination_condition, options, context);
    }

    pplx::task<void> cloud_blob::abort_copy_async(const utility::string_t& copy_id, const access_condition& condition, const blob_request_options& options, operation_context context) const
    {
        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::abort_copy_blob, copy_id, condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response(std::bind(protocol::preprocess_response, std::placeholders::_1, std::placeholders::_2));
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
        command->set_preprocess_response([snapshot_name, snapshot_container, resulting_metadata, properties] (const web::http::http_response& response, operation_context context) -> cloud_blob
        {
            protocol::preprocess_response(response, context);
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
