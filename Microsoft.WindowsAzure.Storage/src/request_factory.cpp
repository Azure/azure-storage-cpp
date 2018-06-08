// -----------------------------------------------------------------------------------------
// <copyright file="request_factory.cpp" company="Microsoft">
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

namespace azure { namespace storage { namespace protocol {

    web::http::http_request base_request(web::http::method method, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        UNREFERENCED_PARAMETER(context);
        if (timeout.count() > 0)
        {
            uri_builder.append_query(core::make_query_parameter(uri_query_timeout, timeout.count(), /* do_encoding */ false));
        }

        web::http::http_request request(method);
        request.set_request_uri(uri_builder.to_uri());

        web::http::http_headers& headers = request.headers();
        headers.add(web::http::header_names::user_agent, header_value_user_agent);
        headers.add(ms_header_version, header_value_storage_version);

        if (method == web::http::methods::PUT)
        {
            headers.set_content_length(0);
        }

        return request;
    }

    web::http::http_request get_service_properties(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_service, /* do_encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_properties, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        return request;
    }

    web::http::http_request set_service_properties(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_resource_type, resource_service, /* do_encoding */ false));
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_properties, /* do_encoding */ false));
        web::http::http_request request(base_request(web::http::methods::PUT, uri_builder, timeout, context));
        return request;
    }

    web::http::http_request get_service_stats(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(uri_query_resource_type, resource_service);
        uri_builder.append_query(uri_query_component, component_stats);
        web::http::http_request request(base_request(web::http::methods::GET, uri_builder, timeout, context));
        return request;
    }

    void add_optional_header(web::http::http_headers& headers, const utility::string_t& header, const utility::string_t& value)
    {
        if (!value.empty())
        {
            headers.add(header, value);
        }
    }

    void add_metadata(web::http::http_request& request, const cloud_metadata& metadata)
    {
        web::http::http_headers& headers = request.headers();
        for (cloud_metadata::const_iterator it = metadata.cbegin(); it != metadata.cend(); ++it)
        {
            if (core::has_whitespace_or_empty(it->first))
            {
                throw std::invalid_argument(protocol::error_empty_whitespace_metadata_name);
            }
            if (core::is_empty_or_whitespace(it->second))
            {
                throw std::invalid_argument(protocol::error_empty_metadata_value);
            }
            if (isspace(*it->second.begin()) || isspace(*it->second.rbegin()))
            {
                headers.add(ms_header_metadata_prefix + it->first, core::str_trim_starting_trailing_whitespaces(it->second));
            }
            else
            {
                headers.add(ms_header_metadata_prefix + it->first, it->second);
            }
            
        }
    }

}}} // namespace azure::storage::protocol
