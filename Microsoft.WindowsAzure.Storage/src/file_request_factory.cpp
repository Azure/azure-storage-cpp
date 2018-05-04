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
            headers.add(_XPLATSTR("x-ms-content-type"), properties.content_type());
        }
        if (!core::is_empty_or_whitespace(properties.content_encoding()))
        {
            headers.add(_XPLATSTR("x-ms-content-encoding"), properties.content_encoding());
        }
        if (!core::is_empty_or_whitespace(properties.content_language()))
        {
            headers.add(_XPLATSTR("x-ms-content-language"), properties.content_language());
        }
        if (!core::is_empty_or_whitespace(properties.cache_control()))
        {
            headers.add(_XPLATSTR("x-ms-cache-control"), properties.cache_control());
        }
        if (!core::is_empty_or_whitespace(properties.content_md5()))
        {
            headers.add(_XPLATSTR("x-ms-content-md5"), properties.content_md5());
        }
        if (!core::is_empty_or_whitespace(properties.content_disposition()))
        {
            headers.add(_XPLATSTR("x-ms-content-disposition"), properties.content_disposition());
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
        if (max_size <= maximum_share_quota)
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
        if (properties.quota() <= protocol::maximum_share_quota)
        {
            request.headers().add(protocol::ms_header_share_quota, properties.quota());
        }
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

    web::http::http_request create_file_directory(const cloud_metadata& metadata, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_directory, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        add_metadata(request, metadata);
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

        add_optional_header(request.headers(), _XPLATSTR("x-ms-type"), _XPLATSTR("file"));
        request.headers()[_XPLATSTR("x-ms-content-length")] = utility::conversions::print_string(length);

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

        return request;
    }

    web::http::http_request resize_with_properties(const cloud_file_properties & properties, web::http::uri_builder uri_builder, const std::chrono::seconds & timeout, operation_context context)
    {
        auto request = set_file_properties(properties, uri_builder, timeout, context);

        request.headers()[_XPLATSTR("x-ms-content-length")] = utility::conversions::print_string(properties.length());
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
            headers.add(_XPLATSTR("x-ms-range-get-content-md5"), _XPLATSTR("true"));
        }
        return request;
    }
}}}