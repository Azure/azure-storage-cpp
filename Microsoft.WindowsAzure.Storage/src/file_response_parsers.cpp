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

#include "cpprest/asyncrt_utils.h"

namespace azure { namespace storage { namespace protocol {

    cloud_file_share_properties file_response_parsers::parse_file_share_properties(const web::http::http_response& response)
    {
        cloud_file_share_properties properties;
        properties.m_quota = parse_quota(response);
        properties.m_etag = parse_etag(response);
        properties.m_last_modified = parse_last_modified(response);
        properties.m_next_allowed_quota_downgrade_time = parse_datetime_rfc1123(get_header_value(response.headers(), ms_header_share_next_allowed_quota_downgrade_time));
        response.headers().match(ms_header_share_provisioned_egress_mbps, properties.m_provisioned_egress);
        response.headers().match(ms_header_share_provisioned_ingress_mbps, properties.m_provisioned_ingress);
        response.headers().match(ms_header_share_provisioned_iops, properties.m_provisioned_iops);
        return properties;
    }

    cloud_file_directory_properties file_response_parsers::parse_file_directory_properties(const web::http::http_response& response)
    {
        cloud_file_directory_properties properties;
        properties.m_etag = parse_etag(response);
        properties.m_last_modified = parse_last_modified(response);
        const auto& headers = response.headers();
        properties.m_server_encrypted = response_parsers::parse_boolean(get_header_value(headers, ms_header_server_encrypted));
        properties.set_permission_key(get_header_value(headers, ms_header_file_permission_key));
        properties.m_attributes = parse_file_attributes(get_header_value(headers, ms_header_file_attributes));
        properties.set_creation_time(parse_datetime_iso8601(get_header_value(headers, ms_header_file_creation_time)));
        properties.set_last_write_time(parse_datetime_iso8601(get_header_value(headers, ms_header_file_last_write_time)));
        properties.m_change_time = parse_datetime_iso8601(get_header_value(headers, ms_header_file_change_time));
        properties.m_file_id = get_header_value(headers, ms_header_file_id);
        properties.m_parent_id = get_header_value(headers, ms_header_file_parent_id);
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
            return utility::conversions::details::scan_string<utility::size64_t>(value);
        }

        return headers.content_length();
    }

    cloud_file_properties file_response_parsers::parse_file_properties(const web::http::http_response& response)
    {
        cloud_file_properties properties;
        properties.m_etag = parse_etag(response);
        properties.m_last_modified = parse_last_modified(response);
        properties.m_length = parse_file_size(response);

        const auto& headers = response.headers();
        properties.m_cache_control = get_header_value(headers, web::http::header_names::cache_control);
        properties.m_content_disposition = get_header_value(headers, header_content_disposition);
        properties.m_content_encoding = get_header_value(headers, web::http::header_names::content_encoding);
        properties.m_content_language = get_header_value(headers, web::http::header_names::content_language);
        properties.m_content_type = get_header_value(headers, web::http::header_names::content_type);
        properties.m_type = get_header_value(headers, _XPLATSTR("x-ms-file-type"));
        properties.m_server_encrypted = response_parsers::parse_boolean(get_header_value(headers, ms_header_server_encrypted));
        // When content_range is not empty, it means the request is Get File with range specified, then 'Content-MD5' header should not be used.
        properties.m_content_md5 = get_header_value(headers, ms_header_content_md5);
        if (properties.m_content_md5.empty() && get_header_value(headers, web::http::header_names::content_range).empty())
        {
            properties.m_content_md5 = get_header_value(headers, web::http::header_names::content_md5);
        }
        properties.set_permission_key(get_header_value(headers, ms_header_file_permission_key));
        properties.m_attributes = parse_file_attributes(get_header_value(headers, ms_header_file_attributes));
        properties.set_creation_time(parse_datetime_iso8601(get_header_value(headers, ms_header_file_creation_time)));
        properties.set_last_write_time(parse_datetime_iso8601(get_header_value(headers, ms_header_file_last_write_time)));
        properties.m_change_time = parse_datetime_iso8601(get_header_value(headers, ms_header_file_change_time));
        properties.m_file_id = get_header_value(headers, ms_header_file_id);
        properties.m_parent_id = get_header_value(headers, ms_header_file_parent_id);

        properties.m_lease_status = parse_lease_status(response);
        properties.m_lease_state = parse_lease_state(response);
        properties.m_lease_duration = parse_lease_duration(response);

        return properties;
    }

}}} // namespace azure::storage::protocol