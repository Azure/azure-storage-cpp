// -----------------------------------------------------------------------------------------
// <copyright file="cloud_blob_container.cpp" company="Microsoft">
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
#include "was/error_code_strings.h"
#include "wascore/protocol.h"
#include "wascore/protocol_xml.h"
#include "wascore/util.h"
#include "wascore/constants.h"

namespace azure { namespace storage {

    cloud_blob_container::cloud_blob_container(storage_uri uri)
        : m_uri(std::move(uri)), m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_blob_container_properties>())
    {
        init(storage_credentials());
    }

    cloud_blob_container::cloud_blob_container(storage_uri uri, storage_credentials credentials)
        : m_uri(std::move(uri)), m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_blob_container_properties>())
    {
        init(std::move(credentials));
    }

    cloud_blob_container::cloud_blob_container(utility::string_t name, cloud_blob_client client)
        : m_name(std::move(name)), m_client(std::move(client)), m_uri(core::append_path_to_uri(m_client.base_uri(), m_name)),
        m_metadata(std::make_shared<cloud_metadata>()), m_properties(std::make_shared<cloud_blob_container_properties>())
    {
    }

    cloud_blob_container::cloud_blob_container(utility::string_t name, cloud_blob_client client, cloud_blob_container_properties properties, cloud_metadata metadata)
        : m_name(std::move(name)), m_client(std::move(client)), m_uri(core::append_path_to_uri(m_client.base_uri(), m_name)),
        m_metadata(std::make_shared<cloud_metadata>(std::move(metadata))), m_properties(std::make_shared<cloud_blob_container_properties>(std::move(properties)))
    {
    }

    void cloud_blob_container::init(storage_credentials credentials)
    {
        utility::string_t snapshot;
        m_uri = core::verify_blob_uri(m_uri, credentials, snapshot);

        if (!core::parse_container_uri(m_uri, m_name))
        {
            throw std::invalid_argument("uri");
        }

        m_client = cloud_blob_client(core::get_service_client_uri(m_uri), std::move(credentials));
    }

    utility::string_t cloud_blob_container::get_shared_access_signature(const blob_shared_access_policy& policy, const utility::string_t& stored_policy_identifier) const
    {
        if (!service_client().credentials().is_shared_key())
        {
            throw std::logic_error(protocol::error_sas_missing_credentials);
        }

        // since 2015-02-21, canonicalized resource is changed from "/account/name" to "/blob/account/name"
        utility::string_t resource_str;
        resource_str.reserve(service_client().credentials().account_name().size() + name().size() + 7);
        resource_str.append(_XPLATSTR("/"));
        resource_str.append(protocol::service_blob);
        resource_str.append(_XPLATSTR("/"));
        resource_str.append(service_client().credentials().account_name());
        resource_str.append(_XPLATSTR("/"));
        resource_str.append(name());

        // Future resource type changes from "c" => "container"
        return protocol::get_blob_sas_token(stored_policy_identifier, policy, cloud_blob_shared_access_headers(), _XPLATSTR("c"), resource_str, service_client().credentials());
    }

    cloud_blob cloud_blob_container::get_blob_reference(utility::string_t blob_name) const
    {
        return get_blob_reference(std::move(blob_name), utility::string_t());
    }

    cloud_blob cloud_blob_container::get_blob_reference(utility::string_t blob_name, utility::string_t snapshot_time) const
    {
        return cloud_blob(std::move(blob_name), std::move(snapshot_time), *this);
    }

    cloud_page_blob cloud_blob_container::get_page_blob_reference(utility::string_t blob_name) const
    {
        return get_page_blob_reference(std::move(blob_name), utility::string_t());
    }

    cloud_page_blob cloud_blob_container::get_page_blob_reference(utility::string_t blob_name, utility::string_t snapshot_time) const
    {
        return cloud_page_blob(std::move(blob_name), std::move(snapshot_time), *this);
    }

    cloud_block_blob cloud_blob_container::get_block_blob_reference(utility::string_t blob_name) const
    {
        return get_block_blob_reference(std::move(blob_name), utility::string_t());
    }

    cloud_block_blob cloud_blob_container::get_block_blob_reference(utility::string_t blob_name, utility::string_t snapshot_time) const
    {
        return cloud_block_blob(std::move(blob_name), std::move(snapshot_time), *this);
    }

    cloud_append_blob cloud_blob_container::get_append_blob_reference(utility::string_t blob_name) const
    {
        return get_append_blob_reference(std::move(blob_name), utility::string_t());
    }

    cloud_append_blob cloud_blob_container::get_append_blob_reference(utility::string_t blob_name, utility::string_t snapshot_time) const
    {
        return cloud_append_blob(std::move(blob_name), std::move(snapshot_time), *this);
    }

    cloud_blob_directory cloud_blob_container::get_directory_reference(utility::string_t directory_name) const
    {
        return cloud_blob_directory(std::move(directory_name), *this);
    }

    pplx::task<void> cloud_blob_container::download_attributes_async(const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;
        auto metadata = m_metadata;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::get_blob_container_properties, condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary);
        command->set_preprocess_response([properties, metadata] (const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            *properties = protocol::blob_response_parsers::parse_blob_container_properties(response);
            *metadata = protocol::parse_metadata(response);
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_blob_container::upload_metadata_async(const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::set_blob_container_metadata, metadata(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_container_properties(response));
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<utility::string_t> cloud_blob_container::acquire_lease_async(const lease_time& duration, const utility::string_t& proposed_lease_id, const access_condition& condition, const blob_request_options& options, operation_context context) const
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<utility::string_t>>(uri());
        command->set_build_request(std::bind(protocol::lease_blob_container, protocol::header_value_lease_acquire, proposed_lease_id, duration, lease_break_period(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context) -> utility::string_t
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_container_properties(response));
            return protocol::parse_lease_id(response);
        });
        return core::executor<utility::string_t>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_blob_container::renew_lease_async(const access_condition& condition, const blob_request_options& options, operation_context context) const
    {
        if (condition.lease_id().empty())
        {
            throw std::invalid_argument("condition");
        }

        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::lease_blob_container, protocol::header_value_lease_renew, utility::string_t(), lease_time(), lease_break_period(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_container_properties(response));
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<utility::string_t> cloud_blob_container::change_lease_async(const utility::string_t& proposed_lease_id, const access_condition& condition, const blob_request_options& options, operation_context context) const
    {
        if (condition.lease_id().empty())
        {
            throw std::invalid_argument("condition");
        }

        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<utility::string_t>>(uri());
        command->set_build_request(std::bind(protocol::lease_blob_container, protocol::header_value_lease_change, proposed_lease_id, lease_time(), lease_break_period(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context) -> utility::string_t
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_container_properties(response));
            return protocol::parse_lease_id(response);
        });
        return core::executor<utility::string_t>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_blob_container::release_lease_async(const access_condition& condition, const blob_request_options& options, operation_context context) const
    {
        if (condition.lease_id().empty())
        {
            throw std::invalid_argument("condition");
        }

        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::lease_blob_container, protocol::header_value_lease_release, utility::string_t(), lease_time(), lease_break_period(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_container_properties(response));
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<std::chrono::seconds> cloud_blob_container::break_lease_async(const lease_break_period& break_period, const access_condition& condition, const blob_request_options& options, operation_context context) const
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<std::chrono::seconds>>(uri());
        command->set_build_request(std::bind(protocol::lease_blob_container, protocol::header_value_lease_break, utility::string_t(), lease_time(), break_period, condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context) -> std::chrono::seconds
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_container_properties(response));
            return protocol::parse_lease_time(response);
        });
        return core::executor<std::chrono::seconds>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_blob_container::create_async(blob_container_public_access_type public_access, const blob_request_options& options, operation_context context)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::create_blob_container, public_access, metadata(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties, public_access] (const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->m_public_access = public_access;
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_container_properties(response));
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<bool> cloud_blob_container::create_if_not_exists_async(blob_container_public_access_type public_access, const blob_request_options& options, operation_context context)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto instance = std::make_shared<cloud_blob_container>(*this);
        return exists_async(true, modified_options, context).then([instance, public_access, modified_options, context] (bool exists_result) -> pplx::task<bool>
        {
            if (!exists_result)
            {
                return instance->create_async(public_access, modified_options, context).then([] (pplx::task<void> create_task) -> bool
                {
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
                            (result.extended_error().code() == protocol::error_code_container_already_exists))
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

    pplx::task<void> cloud_blob_container::delete_container_async(const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::delete_blob_container, condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());

        auto properties = m_properties;
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->initialization();
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<bool> cloud_blob_container::delete_container_if_exists_async(const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto instance = std::make_shared<cloud_blob_container>(*this);
        return exists_async(true, modified_options, context).then([instance, condition, modified_options, context] (bool exists_result) -> pplx::task<bool>
        {
            if (exists_result)
            {
                return instance->delete_container_async(condition, modified_options, context).then([] (pplx::task<void> delete_task) -> bool
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
                            (result.extended_error().code() == protocol::error_code_container_not_found))
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

    list_blob_item_iterator cloud_blob_container::list_blobs(const utility::string_t& prefix, bool use_flat_blob_listing, blob_listing_details::values includes, int max_results, const blob_request_options& options, operation_context context) const
    {
        auto instance = std::make_shared<cloud_blob_container>(*this);
        return list_blob_item_iterator(
            [instance, prefix, use_flat_blob_listing, includes, options, context](const continuation_token& token, size_t max_results_per_segment)
        {
            return instance->list_blobs_segmented(prefix, use_flat_blob_listing, includes, (int)max_results_per_segment, token, options, context);
        },
            max_results, 0);
    }

    pplx::task<list_blob_item_segment> cloud_blob_container::list_blobs_segmented_async(const utility::string_t& prefix, bool use_flat_blob_listing, blob_listing_details::values includes, int max_results, const continuation_token& token, const blob_request_options& options, operation_context context) const
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto container = *this;
        utility::string_t delimiter;

        if (!use_flat_blob_listing)
        {
            if ((includes & blob_listing_details::snapshots) != 0)
            {
                throw std::invalid_argument("includes");
            }

            delimiter = service_client().directory_delimiter();
        }

        auto command = std::make_shared<core::storage_command<list_blob_item_segment>>(uri());
        command->set_build_request(std::bind(protocol::list_blobs, prefix, delimiter, includes, max_results, token, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary, token.target_location());
        command->set_preprocess_response(std::bind(protocol::preprocess_response<list_blob_item_segment>, list_blob_item_segment(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_postprocess_response([container, delimiter] (const web::http::http_response& response, const request_result& result, const core::ostream_descriptor&, operation_context context) -> pplx::task<list_blob_item_segment>
        {
            protocol::list_blobs_reader reader(response.body());

            std::vector<protocol::cloud_blob_list_item> blob_items(reader.move_blob_items());
            std::vector<protocol::cloud_blob_prefix_list_item> blob_prefix_items(reader.move_blob_prefix_items());

            std::vector<list_blob_item> list_blob_items;
            list_blob_items.reserve(blob_items.size() + blob_prefix_items.size());

            for (auto iter = blob_items.begin(); iter != blob_items.end(); ++iter)
            {
                list_blob_items.push_back(list_blob_item(iter->move_name(), iter->move_snapshot_time(), container, iter->move_properties(), iter->move_metadata(), iter->move_copy_state()));
            }

            for (auto iter = blob_prefix_items.begin(); iter != blob_prefix_items.end(); ++iter)
            {
                list_blob_items.push_back(list_blob_item(iter->move_name(), container));
            }

            continuation_token next_token(reader.move_next_marker());
            next_token.set_target_location(result.target_location());

            return pplx::task_from_result(list_blob_item_segment(std::move(list_blob_items), std::move(next_token)));
        });
        return core::executor<list_blob_item_segment>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_blob_container::upload_permissions_async(const blob_container_permissions& permissions, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        protocol::access_policy_writer<blob_shared_access_policy> writer;
        concurrency::streams::istream stream(concurrency::streams::bytestream::open_istream(writer.write(permissions.policies())));

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::set_blob_container_acl, permissions.public_access(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_container_properties(response));
        });
        return core::istream_descriptor::create(stream).then([command, context, modified_options] (core::istream_descriptor request_body) -> pplx::task<void>
        {
            command->set_request_body(request_body);
            return core::executor<void>::execute_async(command, modified_options, context);
        });
    }

    pplx::task<blob_container_permissions> cloud_blob_container::download_permissions_async(const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<blob_container_permissions>>(uri());
        command->set_build_request(std::bind(protocol::get_blob_container_acl, condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary);
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context) -> blob_container_permissions
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_container_properties(response));
            return blob_container_permissions();
        });
        command->set_postprocess_response([properties](const web::http::http_response& response, const request_result&, const core::ostream_descriptor&, operation_context context) -> pplx::task<blob_container_permissions>
        {
            blob_container_permissions permissions;
            protocol::access_policy_reader<blob_shared_access_policy> reader(response.body());
            permissions.set_policies(reader.move_policies());

            auto public_access_type = protocol::parse_public_access_type(response);
            permissions.set_public_access(public_access_type);
            properties->m_public_access = public_access_type;

            return pplx::task_from_result<blob_container_permissions>(permissions);
        });
        return core::executor<blob_container_permissions>::execute_async(command, modified_options, context);
    }

    pplx::task<bool> cloud_blob_container::exists_async(bool primary_only, const blob_request_options& options, operation_context context)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), blob_type::unspecified);

        auto properties = m_properties;
        auto metadata = m_metadata;

        auto command = std::make_shared<core::storage_command<bool>>(uri());
        command->set_build_request(std::bind(protocol::get_blob_container_properties, access_condition(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(primary_only ? core::command_location_mode::primary_only : core::command_location_mode::primary_or_secondary);
        command->set_preprocess_response([properties, metadata] (const web::http::http_response& response, const request_result& result, operation_context context) -> bool
        {
            if (response.status_code() == web::http::status_codes::NotFound)
            {
                return false;
            }

            protocol::preprocess_response_void(response, result, context);
            *properties = protocol::blob_response_parsers::parse_blob_container_properties(response);
            *metadata = protocol::parse_metadata(response);
            return true;
        });
        return core::executor<bool>::execute_async(command, modified_options, context);
    }

}} // namespace azure::storage
