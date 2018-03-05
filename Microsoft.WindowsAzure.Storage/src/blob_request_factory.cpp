// -----------------------------------------------------------------------------------------
// <copyright file="blob_request_factory.cpp" company="Microsoft">
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
#include "wascore/protocol.h"
#include "wascore/constants.h"
#include "wascore/resources.h"

namespace azure { namespace storage { namespace protocol {

    void add_blob_container_public_access_type(web::http::http_headers& headers, blob_container_public_access_type access_type)
    {
        switch (access_type)
        {
        case blob_container_public_access_type::blob:
            headers.add(ms_header_blob_public_access, resource_blob);
            break;

        case blob_container_public_access_type::container:
            headers.add(ms_header_blob_public_access, resource_container);
            break;
        }
    }

    void add_snapshot_time(web::http::uri_builder& uri_builder, const utility::string_t& snapshot_time)
    {
        if (!snapshot_time.empty())
        {
            uri_builder.append_query(core::make_query_parameter(uri_query_snapshot, snapshot_time));
        }
    }

    void add_previous_snapshot_time(web::http::uri_builder& uri_builder, const utility::string_t& snapshot_time)
    {
        if (!snapshot_time.empty())
        {
            uri_builder.append_query(core::make_query_parameter(uri_query_prevsnapshot, snapshot_time));
        }
    }

    web::http::http_request create_blob_container(blob_container_public_access_type access_type, const cloud_metadata& metadata, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_container, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        add_blob_container_public_access_type(request.headers(), access_type);
        add_metadata(request, metadata);
        return request;
    }

    web::http::http_request delete_blob_container(const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_container, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::DEL, uri_builder, timeout, context));
        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request get_blob_container_properties(const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_container, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::HEAD, uri_builder, timeout, context));
        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request set_blob_container_metadata(const cloud_metadata& metadata, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_container, /* do_encoding */ false));
        return set_blob_metadata(metadata, condition, uri_builder, timeout, context);
    }

    web::http::http_request get_blob_container_acl(const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_container, /* do_encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_acl, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        add_lease_id(request, condition);
        return request;
    }

    web::http::http_request set_blob_container_acl(blob_container_public_access_type access_type, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_container, /* do_encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_acl, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        add_blob_container_public_access_type(request.headers(), access_type);
        add_lease_id(request, condition);
        return request;
    }

    web::http::http_request list_containers(const utility::string_t& prefix, container_listing_details::values includes, int max_results, const continuation_token& token, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_list, /* do_encoding */ false));

        if (!prefix.empty())
        {
            uri_builder.append_query(core::make_query_parameter(uri_query_prefix, prefix));
        }

        if (!token.empty())
        {
            uri_builder.append_query(core::make_query_parameter(uri_query_marker, token.next_marker()));
        }

        if (max_results > 0)
        {
            uri_builder.append_query(core::make_query_parameter(uri_query_max_results, max_results, /* do_encoding */ false));
        }

        if ((includes & container_listing_details::metadata) != 0)
        {
            uri_builder.append_query(core::make_query_parameter(uri_query_include, component_metadata, /* do_encoding */ false));
        }

        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        return request;
    }

    web::http::http_request list_blobs(const utility::string_t& prefix, const utility::string_t& delimiter, blob_listing_details::values includes, int max_results, const continuation_token& token, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_container, /* do_encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_list, /* do_encoding */ false));
        utility::string_t include;

        if (!prefix.empty())
        {
            uri_builder.append_query(core::make_query_parameter(uri_query_prefix, prefix));
        }

        if (!delimiter.empty())
        {
            uri_builder.append_query(core::make_query_parameter(uri_query_delimiter, delimiter));
        }

        if (!token.empty())
        {
            uri_builder.append_query(core::make_query_parameter(uri_query_marker, token.next_marker()));
        }

        if (max_results > 0)
        {
            uri_builder.append_query(core::make_query_parameter(uri_query_max_results, max_results, /* do_encoding */ false));
        }

        if ((includes & blob_listing_details::snapshots) != 0)
        {
            include.append(component_snapshots);
            include.append(_XPLATSTR(","));
        }

        if ((includes & blob_listing_details::metadata) != 0)
        {
            include.append(component_metadata);
            include.append(_XPLATSTR(","));
        }

        if ((includes & blob_listing_details::uncommitted_blobs) != 0)
        {
            include.append(component_uncommitted_blobs);
            include.append(_XPLATSTR(","));
        }

        if ((includes & blob_listing_details::copy) != 0)
        {
            include.append(component_copy);
            include.append(_XPLATSTR(","));
        }

        if (!include.empty())
        {
            include.pop_back();
            uri_builder.append_query(core::make_query_parameter(uri_query_include, include, /* do_encoding */ false));
        }

        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        return request;
    }

    web::http::http_request lease(const utility::string_t& lease_action, const utility::string_t& proposed_lease_id, const lease_time& duration, const lease_break_period& break_period, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_lease, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        web::http::http_headers& headers = request.headers();
        headers.add(ms_header_lease_action, lease_action);

        if (lease_action == header_value_lease_acquire)
        {
            headers.add(ms_header_lease_duration, duration.seconds().count());
        }
        else if ((lease_action == header_value_lease_break) && break_period.is_valid())
        {
            headers.add(ms_header_lease_break_period, break_period.seconds().count());
        }

        if (!proposed_lease_id.empty())
        {
            headers.add(ms_header_lease_proposed_id, proposed_lease_id);
        }

        return request;
    }

    web::http::http_request lease_blob_container(const utility::string_t& lease_action, const utility::string_t& proposed_lease_id, const lease_time& duration, const lease_break_period& break_period, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_container, /* do_encoding */ false));
        web::http::http_request request(lease(lease_action, proposed_lease_id, duration, break_period, uri_builder, timeout, context));
        add_lease_id(request, condition);
        return request;
    }

    web::http::http_request lease_blob(const utility::string_t& lease_action, const utility::string_t& proposed_lease_id, const lease_time& duration, const lease_break_period& break_period, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request(lease(lease_action, proposed_lease_id, duration, break_period, uri_builder, timeout, context));
        add_access_condition(request, condition);
        return request;
    }

    void add_properties(web::http::http_request& request, const cloud_blob_properties& properties)
    {
        web::http::http_headers& headers = request.headers();
        add_optional_header(headers, ms_header_blob_cache_control, properties.cache_control());
        add_optional_header(headers, ms_header_blob_content_disposition, properties.content_disposition());
        add_optional_header(headers, ms_header_blob_content_encoding, properties.content_encoding());
        add_optional_header(headers, ms_header_blob_content_language, properties.content_language());
        add_optional_header(headers, ms_header_blob_content_md5, properties.content_md5());
        add_optional_header(headers, ms_header_blob_content_type, properties.content_type());
    }

    void add_range(web::http::http_request& request, utility::size64_t offset, utility::size64_t length)
    {
        if (offset < std::numeric_limits<utility::size64_t>::max())
        {
            utility::ostringstream_t value;
            value << header_value_range_prefix << offset << _XPLATSTR('-');
            if (length > 0)
            {
                length += offset - 1;
                value << length;
            }

            request.headers().add(ms_header_range, value.str());
        }
        else if (length > 0)
        {
            throw std::invalid_argument("length");
        }
    }

    web::http::http_request put_block(const utility::string_t& block_id, const utility::string_t& content_md5, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_block, /* do_encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_block_id, block_id));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        request.headers().add(web::http::header_names::content_md5, content_md5);
        add_lease_id(request, condition);
        return request;
    }

    web::http::http_request put_block_list(const cloud_blob_properties& properties, const cloud_metadata& metadata, const utility::string_t& content_md5, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_block_list, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        request.headers().add(web::http::header_names::content_md5, content_md5);
        add_properties(request, properties);
        add_metadata(request, metadata);
        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request get_block_list(block_listing_filter listing_filter, const utility::string_t& snapshot_time, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        add_snapshot_time(uri_builder, snapshot_time);

        switch (listing_filter)
        {
        case block_listing_filter::all:
            uri_builder.append_query(core::make_query_parameter(uri_query_block_list_type, resource_block_list_all, /* do_encoding */ false));
            break;

        case block_listing_filter::committed:
            uri_builder.append_query(core::make_query_parameter(uri_query_block_list_type, resource_block_list_committed, /* do_encoding */ false));
            break;

        case block_listing_filter::uncommitted:
            uri_builder.append_query(core::make_query_parameter(uri_query_block_list_type, resource_block_list_uncommitted, /* do_encoding */ false));
            break;
        }

        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_block_list, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request get_page_ranges(utility::size64_t offset, utility::size64_t length, const utility::string_t& snapshot_time, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        add_snapshot_time(uri_builder, snapshot_time);
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_page_list, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        add_range(request, offset, length);
        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request get_page_ranges_diff(utility::string_t previous_snapshot_time, utility::size64_t offset, utility::size64_t length, const utility::string_t& snapshot_time, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        add_snapshot_time(uri_builder, snapshot_time);
        add_previous_snapshot_time(uri_builder, previous_snapshot_time);
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_page_list, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        add_range(request, offset, length);
        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request put_page(page_range range, page_write write, const utility::string_t& content_md5, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_page, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));

        web::http::http_headers& headers = request.headers();
        headers.add(ms_header_range, range.to_string());

        switch (write)
        {
        case page_write::update:
            headers.add(ms_header_page_write, header_value_page_write_update);
            add_optional_header(headers, web::http::header_names::content_md5, content_md5);
            break;

        case page_write::clear:
            headers.add(ms_header_page_write, header_value_page_write_clear);
            break;
        }

        add_sequence_number_condition(request, condition);
        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request append_block(const utility::string_t& content_md5, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_append_block, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        request.headers().add(web::http::header_names::content_md5, content_md5);
        add_append_condition(request, condition);
        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request put_block_blob(const cloud_blob_properties& properties, const cloud_metadata& metadata, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        request.headers().add(ms_header_blob_type, header_value_blob_type_block);
        add_properties(request, properties);
        add_metadata(request, metadata);
        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request put_page_blob(utility::size64_t size, const utility::string_t& tier, int64_t sequence_number, const cloud_blob_properties& properties, const cloud_metadata& metadata, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        web::http::http_headers& headers = request.headers();
        headers.add(ms_header_blob_type, header_value_blob_type_page);
        headers.add(ms_header_blob_content_length, size);
        headers.add(ms_header_blob_sequence_number, sequence_number);
        if (tier != header_value_access_tier_unknown)
        {
            headers.add(ms_header_access_tier, tier);
        }
        add_properties(request, properties);
        add_metadata(request, metadata);
        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request put_append_blob(const cloud_blob_properties& properties, const cloud_metadata& metadata, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        request.headers().add(ms_header_blob_type, header_value_blob_type_append);
        add_properties(request, properties);
        add_metadata(request, metadata);
        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request get_blob(utility::size64_t offset, utility::size64_t length, bool get_range_content_md5, const utility::string_t& snapshot_time, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        add_snapshot_time(uri_builder, snapshot_time);
        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        add_range(request, offset, length);

        if ((offset < std::numeric_limits<utility::size64_t>::max()) && get_range_content_md5)
        {
            request.headers().add(ms_header_range_get_content_md5, header_value_true);
        }

        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request get_blob_properties(const utility::string_t& snapshot_time, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        add_snapshot_time(uri_builder, snapshot_time);
        web::http::http_request request(base_request(web::http::methods::HEAD, uri_builder, timeout, context));
        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request set_blob_properties(const cloud_blob_properties& properties, const cloud_metadata& metadata, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_properties, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        add_properties(request, properties);
        add_metadata(request, metadata);
        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request resize_page_blob(utility::size64_t size, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_properties, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        request.headers().add(ms_header_blob_content_length, size);
        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request set_page_blob_sequence_number(const azure::storage::sequence_number& sequence_number, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_properties, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        web::http::http_headers& headers = request.headers();

        switch (sequence_number.action())
        {
        case sequence_number::sequence_number_action::increment:
            headers.add(ms_header_sequence_number_action, header_value_sequence_increment);
            break;

        case sequence_number::sequence_number_action::maximum:
            headers.add(ms_header_sequence_number_action, header_value_sequence_max);
            headers.add(ms_header_blob_sequence_number, sequence_number.value());
            break;

        case sequence_number::sequence_number_action::update:
            headers.add(ms_header_sequence_number_action, header_value_sequence_update);
            headers.add(ms_header_blob_sequence_number, sequence_number.value());
            break;
        }

        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request snapshot_blob(const cloud_metadata& metadata, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_snapshot, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        add_metadata(request, metadata);
        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request set_blob_metadata(const cloud_metadata& metadata, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_metadata, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        add_metadata(request, metadata);
        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request delete_blob(delete_snapshots_option snapshots_option, const utility::string_t& snapshot_time, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        if (!snapshot_time.empty() && (snapshots_option != delete_snapshots_option::none))
        {
            throw std::invalid_argument("snapshots_option");
        }

        add_snapshot_time(uri_builder, snapshot_time);
        web::http::http_request request(base_request(web::http::methods::DEL, uri_builder, timeout, context));

        if (snapshots_option == delete_snapshots_option::include_snapshots)
        {
            request.headers().add(ms_header_delete_snapshots, header_value_snapshots_include);
        }
        else if (snapshots_option == delete_snapshots_option::delete_snapshots_only)
        {
            request.headers().add(ms_header_delete_snapshots, header_value_snapshots_only);
        }

        add_access_condition(request, condition);
        return request;
    }

    web::http::http_request copy_blob(const web::http::uri& source, const utility::string_t& tier, const access_condition& source_condition, const cloud_metadata& metadata, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        request.headers().add(ms_header_copy_source, source.to_string());
        if (tier != header_value_access_tier_unknown)
        {
            request.headers().add(ms_header_access_tier, tier);
        }
        add_source_access_condition(request, source_condition);
        add_access_condition(request, condition);
        add_metadata(request, metadata);
        return request;
    }

    web::http::http_request abort_copy_blob(const utility::string_t& copy_id, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_copy, /* do_encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_copy_id, copy_id));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        request.headers().add(ms_header_copy_action, header_value_copy_abort);
        add_lease_id(request, condition);
        return request;
    }

    web::http::http_request incremental_copy_blob(const web::http::uri& source, const access_condition& condition, const cloud_metadata& metadata, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_incrementalcopy, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        request.headers().add(ms_header_copy_source, source.to_string());
        add_access_condition(request, condition);
        add_metadata(request, metadata);
        return request;
    }

    web::http::http_request set_blob_tier(const utility::string_t& tier, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_tier, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        
        request.headers().add(ms_header_access_tier, tier);
        add_access_condition(request, condition);
        return request;
    }

    void add_lease_id(web::http::http_request& request, const access_condition& condition)
    {
        add_optional_header(request.headers(), ms_header_lease_id, condition.lease_id());
    }

    void add_sequence_number_condition(web::http::http_request& request, const access_condition& condition)
    {
        switch (condition.sequence_number_operator())
        {
        case access_condition::sequence_number_operators::eq:
            request.headers().add(ms_header_if_sequence_number_eq, condition.sequence_number());
            break;

        case access_condition::sequence_number_operators::le:
            request.headers().add(ms_header_if_sequence_number_le, condition.sequence_number());
            break;

        case access_condition::sequence_number_operators::lt:
            request.headers().add(ms_header_if_sequence_number_lt, condition.sequence_number());
            break;
        }
    }

    void add_access_condition(web::http::http_request& request, const access_condition& condition)
    {
        web::http::http_headers& headers = request.headers();

        add_optional_header(headers, web::http::header_names::if_match, condition.if_match_etag());
        add_optional_header(headers, web::http::header_names::if_none_match, condition.if_none_match_etag());

        if (condition.if_modified_since_time().is_initialized())
        {
            headers.add(web::http::header_names::if_modified_since, condition.if_modified_since_time().to_string(utility::datetime::date_format::RFC_1123));
        }

        if (condition.if_not_modified_since_time().is_initialized())
        {
            headers.add(web::http::header_names::if_unmodified_since, condition.if_not_modified_since_time().to_string(utility::datetime::date_format::RFC_1123));
        }

        add_lease_id(request, condition);
    }

    void add_source_access_condition(web::http::http_request& request, const access_condition& condition)
    {
        web::http::http_headers& headers = request.headers();

        add_optional_header(headers, ms_header_source_if_match, condition.if_match_etag());
        add_optional_header(headers, ms_header_source_if_none_match, condition.if_none_match_etag());

        if (condition.if_modified_since_time().is_initialized())
        {
            headers.add(ms_header_source_if_modified_since, condition.if_modified_since_time().to_string(utility::datetime::date_format::RFC_1123));
        }

        if (condition.if_not_modified_since_time().is_initialized())
        {
            headers.add(ms_header_source_if_unmodified_since, condition.if_not_modified_since_time().to_string(utility::datetime::date_format::RFC_1123));
        }

        if (!condition.lease_id().empty())
        {
            throw storage_exception(protocol::error_lease_id_on_source, false);
        }
    }

    void add_append_condition(web::http::http_request& request, const access_condition& condition)
    {
        if (condition.max_size() != -1)
        {
            request.headers().add(ms_header_blob_condition_maxsize, condition.max_size());
        }

        if (condition.append_position() != -1)
        {
            request.headers().add(ms_header_blob_condition_appendpos, condition.append_position());
        }
    }

}}} // namespace azure::storage::protocol
