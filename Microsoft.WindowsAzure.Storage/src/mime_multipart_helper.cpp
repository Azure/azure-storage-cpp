// -----------------------------------------------------------------------------------------
// <copyright file="mime_multipart_helper.cpp" company="Microsoft">
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
#include "wascore/util.h"
#include "was/table.h"

namespace azure { namespace storage {  namespace core {

    utility::string_t generate_boundary_name(const utility::string_t& prefix)
    {
        utility::uuid id = utility::new_uuid();

        utility::string_t boundary_name;
        boundary_name.reserve(prefix.size() + 37);

        boundary_name.append(prefix);
        boundary_name.push_back(_XPLATSTR('_'));
        boundary_name.append(utility::uuid_to_string(id));

        return boundary_name;
    }

    void write_line_break(utility::string_t& body_text)
    {
        body_text.push_back(_XPLATSTR('\r'));
        body_text.push_back(_XPLATSTR('\n'));
    }

    void write_boundary(utility::string_t& body_text, const utility::string_t& boundary_name, bool is_closure)
    {
        body_text.append(_XPLATSTR("--"));
        body_text.append(boundary_name);
        if (is_closure)
        {
            body_text.append(_XPLATSTR("--"));
        }

        write_line_break(body_text);
    }

    void write_mime_changeset_headers(utility::string_t& body_text)
    {
        body_text.append(web::http::header_names::content_type);
        body_text.push_back(_XPLATSTR(':'));
        body_text.push_back(_XPLATSTR(' '));
        body_text.append(protocol::header_value_content_type_http);
        write_line_break(body_text);

        body_text.append(protocol::header_content_transfer_encoding);
        body_text.push_back(_XPLATSTR(':'));
        body_text.push_back(_XPLATSTR(' '));
        body_text.append(protocol::header_value_content_transfer_encoding_binary);
        write_line_break(body_text);

        write_line_break(body_text);
    }

    void write_request_line(utility::string_t& body_text, const web::http::method& method, const web::http::uri& uri)
    {
        body_text.append(method);
        body_text.push_back(_XPLATSTR(' '));
        body_text.append(uri.to_string());
        body_text.push_back(_XPLATSTR(' '));
        body_text.append(protocol::http_version);
        write_line_break(body_text);
    }

    void write_request_headers(utility::string_t& body_text, const web::http::http_headers& headers)
    {
        for (web::http::http_headers::const_iterator it = headers.begin(); it != headers.end(); ++it)
        {
            const utility::string_t& header_name = it->first;
            const utility::string_t& header_value = it->second;

            body_text.append(header_name);
            body_text.push_back(_XPLATSTR(':'));
            body_text.push_back(_XPLATSTR(' '));
            body_text.append(header_value);
            write_line_break(body_text);
        }

        write_line_break(body_text);
    }

    void write_request_payload(utility::string_t& body_text, const web::json::value& json_object)
    {
        if (!json_object.is_null())
        {
            body_text.append(json_object.serialize());
        }

        write_line_break(body_text);
    }

}}} // namespace azure::storage::core
