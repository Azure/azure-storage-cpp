// -----------------------------------------------------------------------------------------
// <copyright file="blob_response_parsers.cpp" company="Microsoft">
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

    cloud_file_share_properties file_response_parsers::parse_file_share_properties(const web::http::http_response& response)
    {
        cloud_file_share_properties properties;
        properties.m_quota = parse_quota(response);
        properties.m_etag = parse_etag(response);
        properties.m_last_modified = parse_last_modified(response);
        return properties;
    }

    cloud_file_directory_properties file_response_parsers::parse_file_directory_properties(const web::http::http_response& response)
    {
        cloud_file_directory_properties properties;
        properties.m_etag = parse_etag(response);
        properties.m_last_modified = parse_last_modified(response);
        properties.m_server_encrypted = response_parsers::parse_boolean(get_header_value(response.headers(), ms_header_server_encrypted));
        return properties;
    }

    utility::size64_t file_response_parsers::parse_file_size(const web::http::http_response& response)
    {
        auto& headers = response.headers();
        utility::string_t value;

        if (headers.match(web::http::header_names::content_range, value))
        {
            auto slash = value.find(_XPLATSTR('/'));
            value = value.substr(slash + 1);
            return utility::conversions::scan_string<utility::size64_t>(value);
        }

        return headers.content_length();
    }

    cloud_file_properties file_response_parsers::parse_file_properties(const web::http::http_response& response)
    {
        cloud_file_properties properties;
        properties.m_etag = parse_etag(response);
        properties.m_last_modified = parse_last_modified(response);
        properties.m_length = parse_file_size(response);

        auto& headers = response.headers();
        properties.m_cache_control = get_header_value(headers, web::http::header_names::cache_control);
        properties.m_content_disposition = get_header_value(headers, header_content_disposition);
        properties.m_content_encoding = get_header_value(headers, web::http::header_names::content_encoding);
        properties.m_content_language = get_header_value(headers, web::http::header_names::content_language);
        properties.m_content_type = get_header_value(headers, web::http::header_names::content_type);
        properties.m_type = get_header_value(headers, _XPLATSTR("x-ms-file-type"));
        properties.m_content_md5 = get_header_value(headers, ms_header_content_md5);
        properties.m_server_encrypted = response_parsers::parse_boolean(get_header_value(headers, ms_header_server_encrypted));
        if (properties.m_content_md5.empty())
        {
            properties.m_content_md5 = get_header_value(headers, web::http::header_names::content_md5);
        }

        return properties;
    }

}}} // namespace azure::storage::protocol