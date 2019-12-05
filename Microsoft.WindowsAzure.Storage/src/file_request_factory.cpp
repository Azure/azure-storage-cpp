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

    void add_file_properties(web::http::http_request& request, const cloud_file_properties& properties)
    {
        web::http::http_headers& headers = request.headers();

        if (!core::is_empty_or_whitespace(properties.content_type()))
        {
            headers.add(ms_header_content_type, properties.content_type());
        }
        if (!core::is_empty_or_whitespace(properties.content_encoding()))
        {
            headers.add(ms_header_content_encoding, properties.content_encoding());
        }
        if (!core::is_empty_or_whitespace(properties.content_language()))
        {
            headers.add(ms_header_content_language, properties.content_language());
        }
        if (!core::is_empty_or_whitespace(properties.cache_control()))
        {
            headers.add(ms_header_cache_control, properties.cache_control());
        }
        if (!core::is_empty_or_whitespace(properties.content_md5()))
        {
            headers.add(ms_header_content_md5, properties.content_md5());
        }
        if (!core::is_empty_or_whitespace(properties.content_disposition()))
        {
            headers.add(ms_header_content_disposition, properties.content_disposition());
        }
    }

    utility::string_t file_properties_to_string(cloud_file_attributes value)
    {
        if (value == cloud_file_attributes::preserve)
        {
            return protocol::header_value_file_property_preserve;
        }
        if (value & cloud_file_attributes::source)
        {
            return protocol::header_value_file_property_source;
        }
        if (value & cloud_file_attributes::none)
        {
            return protocol::header_value_file_attribute_none;
        }

        std::vector<utility::string_t> properties;
        if (value & cloud_file_attributes::readonly)
        {
            properties.emplace_back(header_value_file_attribute_readonly);
        }
        if (value & cloud_file_attributes::hidden)
        {
            properties.emplace_back(header_value_file_attribute_hidden);
        }
        if (value & cloud_file_attributes::system)
        {
            properties.emplace_back(header_value_file_attribute_system);
        }
        if (value & cloud_file_attributes::directory)
        {
            properties.emplace_back(header_value_file_attribute_directory);
        }
        if (value & cloud_file_attributes::archive)
        {
            properties.emplace_back(header_value_file_attribute_archive);
        }
        if (value & cloud_file_attributes::temporary)
        {
            properties.emplace_back(header_value_file_attribute_temporary);
        }
        if (value & cloud_file_attributes::offline)
        {
            properties.emplace_back(header_value_file_attribute_offline);
        }
        if (value & cloud_file_attributes::not_content_indexed)
        {
            properties.emplace_back(header_value_file_attribute_notcontentindexed);
        }
        if (value & cloud_file_attributes::no_scrub_data)
        {
            properties.emplace_back(header_value_file_attribute_noscrubdata);
        }
        return core::string_join(properties, header_value_file_attribute_delimiter);
    }

    enum class file_operation_type
    {
        create,
        update,
        copy,
    };

    template<class Properties>
    void add_additional_properties(web::http::http_request& request, const Properties& properties, file_operation_type op_type)
    {
        web::http::http_headers& headers = request.headers();

        bool permission_set = false;
        if (!core::is_empty_or_whitespace(properties.permission_key()))
        {
            headers.add(ms_header_file_permission_key, properties.permission_key());
            permission_set = true;
        }
        if (!core::is_empty_or_whitespace(properties.permission()))
        {
            headers.add(ms_header_file_permission, properties.permission());
            permission_set = true;
        }
        if (!permission_set && op_type == file_operation_type::create)
        {
            headers.add(ms_header_file_permission, header_value_file_permission_inherit);
        }
        else if (!permission_set && op_type == file_operation_type::update)
        {
            headers.add(ms_header_file_permission, header_value_file_property_preserve);
        }
        if (op_type == file_operation_type::copy)
        {
            if (properties.permission() == header_value_file_property_source)
            {
                headers.remove(ms_header_file_permission);
                headers.remove(ms_header_file_permission_key);
                headers.add(ms_header_file_permission_copy_mode, header_value_file_property_source);
            }
            else if (permission_set)
            {
                headers.add(ms_header_file_permission_copy_mode, header_value_file_permission_override);
            }
        }

        auto attributes = properties.attributes();
        if (op_type == file_operation_type::create && attributes == cloud_file_attributes::preserve)
        {
            headers.add(ms_header_file_attributes, file_properties_to_string(cloud_file_attributes::none));
        }
        else if (op_type == file_operation_type::copy && attributes == cloud_file_attributes::preserve)
        {
        }
        else
        {
            headers.add(ms_header_file_attributes, file_properties_to_string(attributes));
        }

        if (properties.creation_time().is_initialized())
        {
            headers.add(ms_header_file_creation_time, core::convert_to_iso8601_string(properties.creation_time(), 7));
        }
        else if (op_type == file_operation_type::create)
        {
            headers.add(ms_header_file_creation_time, header_value_file_time_now);
        }
        else if (op_type == file_operation_type::update)
        {
            headers.add(ms_header_file_creation_time, header_value_file_property_preserve);
        }
        else if (op_type == file_operation_type::copy)
        {
        }

        if (properties.last_write_time().is_initialized())
        {
            headers.add(ms_header_file_last_write_time, core::convert_to_iso8601_string(properties.last_write_time(), 7));
        }
        else if (op_type == file_operation_type::create)
        {
            headers.add(ms_header_file_last_write_time, header_value_file_time_now);
        }
        else if (op_type == file_operation_type::update)
        {
            headers.add(ms_header_file_last_write_time, header_value_file_property_preserve);
        }
        else if (op_type == file_operation_type::copy)
        {
        }
    }

    void add_file_range(web::http::http_request& request, utility::size64_t offset, utility::size64_t length)
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

    web::http::http_request list_shares(const utility::string_t& prefix, bool get_metadata, int max_results, const continuation_token& token, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
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

        if (get_metadata)
        {
            uri_builder.append_query(core::make_query_parameter(uri_query_include, component_metadata, /* do_encoding */ false));
        }

        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        return request;
    }

    web::http::http_request create_file_share(const utility::size64_t max_size, const cloud_metadata& metadata, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_share, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        web::http::http_headers& headers = request.headers();
        if (max_size != std::numeric_limits<unsigned long long>::max())
        {
            headers.add(protocol::ms_header_share_quota, max_size);
        }
        add_metadata(request, metadata);
        return request;
    }

    web::http::http_request delete_file_share(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_share, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::DEL, uri_builder, timeout, context));
        return request;
    }

    web::http::http_request get_file_share_properties(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_share, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        return request;
    }
    web::http::http_request set_file_share_properties(const cloud_file_share_properties& properties, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_share, /* do_encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_properties, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        request.headers().add(protocol::ms_header_share_quota, properties.quota());
        return request;
    }

    web::http::http_request set_file_share_metadata(const cloud_metadata& metadata, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_share, /* do_encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_metadata, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        add_metadata(request, metadata);
        return request;
    }

    web::http::http_request get_file_share_stats(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_share, /* do_encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_stats, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        return request;
    }

    web::http::http_request get_file_share_acl(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_share, /* do_encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_acl, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        return request;
    }

    web::http::http_request set_file_share_acl(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_share, /* do_encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_acl, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        return request;
    }

    web::http::http_request get_file_share_permission(const utility::string_t& permission_key, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_share, /* do encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_file_permission, /* do encoding */ false));
        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        request.headers().add(protocol::ms_header_file_permission_key, permission_key);
        return request;
    }

    web::http::http_request set_file_share_permission(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_share, /* do encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_file_permission, /* do encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        request.headers().add(web::http::header_names::content_type, header_value_content_type_json);
        return request;
    }

    web::http::http_request create_file_directory(const cloud_metadata& metadata, const cloud_file_directory_properties& properties, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_directory, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        add_metadata(request, metadata);
        add_additional_properties(request, properties, file_operation_type::create);
        return request;
    }

    web::http::http_request delete_file_directory(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_directory, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::DEL, uri_builder, timeout, context));
        return request;
    }

    web::http::http_request get_file_directory_properties(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_directory, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::HEAD, uri_builder, timeout, context));
        return request;
    }

    web::http::http_request set_file_directory_properties(const cloud_file_directory_properties& properties, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_directory, /* do_encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_properties, /* do_encoding */ false));

        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        add_additional_properties(request, properties, file_operation_type::update);

        return request;
    }
    
    web::http::http_request set_file_directory_metadata(const cloud_metadata& metadata, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_directory, /* do_encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_metadata, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        add_metadata(request, metadata);
        return request;
    }

    web::http::http_request list_files_and_directories(const utility::string_t& prefix, int64_t max_results, const continuation_token& token, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_directory, /* do_encoding */ false));
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

        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        return request;
    }

    web::http::http_request create_file(const int64_t length, const cloud_metadata& metadata, const cloud_file_properties& properties, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));

        add_metadata(request, metadata);
        add_file_properties(request, properties);
        add_additional_properties(request, properties, file_operation_type::create);

        add_optional_header(request.headers(), _XPLATSTR("x-ms-type"), _XPLATSTR("file"));
        request.headers()[ms_header_content_length] = core::convert_to_string(length);

        return request;
    }

    web::http::http_request delete_file(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request(base_request(web::http::methods::DEL, uri_builder, timeout, context));
        return request;
    }

    web::http::http_request get_file_properties(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request(base_request(web::http::methods::HEAD, uri_builder, timeout, context));
        return request;
    }
    
    web::http::http_request set_file_properties(const cloud_file_properties& properties, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_properties, /* do_encoding */ false));

        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        //Note that setting file properties with a length won't resize the file.
        //If resize is needed, user should call azure::storage::cloud_file::resize instead.
        add_file_properties(request, properties);
        add_additional_properties(request, properties, file_operation_type::update);

        return request;
    }

    web::http::http_request resize_with_properties(const cloud_file_properties & properties, web::http::uri_builder uri_builder, const std::chrono::seconds & timeout, operation_context context)
    {
        auto request = set_file_properties(properties, uri_builder, timeout, context);

        request.headers()[ms_header_content_length] = core::convert_to_string(properties.length());
        return request;
    }
    
    web::http::http_request set_file_metadata(const cloud_metadata& metadata, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_metadata, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        add_metadata(request, metadata);
        return request;
    }

    
        web::http::http_request copy_file(const web::http::uri& source, const cloud_metadata& metadata, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        request.headers().add(ms_header_copy_source, source.to_string());
        add_metadata(request, metadata);

        return request;
    }

    web::http::http_request copy_file_from_blob(const web::http::uri& source, const access_condition& condition, const cloud_metadata& metadata, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        request.headers().add(ms_header_copy_source, source.to_string());
        add_source_access_condition(request, condition);
        add_metadata(request, metadata);

        return request;
    }

    web::http::http_request abort_copy_file(const utility::string_t& copy_id, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_copy, /* do_encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_copy_id, copy_id, /* do_encoding */ false));

        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        request.headers().add(ms_header_copy_action, header_value_copy_abort);
        return request;
    }

    web::http::http_request list_file_ranges(utility::size64_t start_offset, utility::size64_t length, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_range_list, /* do_encoding */ false));
        
        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        add_file_range(request, start_offset, length);

        return request;
    }

    web::http::http_request put_file_range(file_range range, file_range_write write, utility::string_t content_md5, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_range, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        
        web::http::http_headers& headers = request.headers();
        headers.add(ms_header_range, range.to_string());

        switch (write)
        {
        case file_range_write::update:
            headers.add(_XPLATSTR("x-ms-write"), _XPLATSTR("update"));
            add_optional_header(headers, web::http::header_names::content_md5, content_md5);
            break;

        case file_range_write::clear:
            headers.add(_XPLATSTR("x-ms-write"), _XPLATSTR("clear"));
            break;
        }
        return request;
    }

    web::http::http_request get_file(utility::size64_t start_offset, utility::size64_t length, bool md5_validation, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        web::http::http_headers& headers = request.headers();
        add_file_range(request, start_offset, length);

        if (start_offset < std::numeric_limits<utility::size64_t>::max() && md5_validation)
        {
            headers.add(ms_header_range_get_content_md5, header_value_true);
        }
        return request;
    }
}}}