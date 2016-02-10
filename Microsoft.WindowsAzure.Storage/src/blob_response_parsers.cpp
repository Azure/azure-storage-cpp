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

namespace azure { namespace storage { namespace protocol {

    cloud_blob_container_properties blob_response_parsers::parse_blob_container_properties(const web::http::http_response& response)
    {
        cloud_blob_container_properties properties;
        properties.m_etag = parse_etag(response);
        properties.m_last_modified = parse_last_modified(response);
        properties.m_lease_status = parse_lease_status(response);
        properties.m_lease_state = parse_lease_state(response);
        properties.m_lease_duration = parse_lease_duration(response);
        return properties;
    }

    blob_container_public_access_type blob_response_parsers::parse_public_access_type(const web::http::http_response& response)
    {
        auto value = get_header_value(response.headers(), ms_header_blob_public_access);
        if (value == resource_blob)
        {
            return blob_container_public_access_type::blob;
        }
        else if (value == resource_container)
        {
            return blob_container_public_access_type::container;
        }
        else
        {
            return blob_container_public_access_type::off;
        }
    }

    blob_type blob_response_parsers::parse_blob_type(const utility::string_t& value)
    {
        if (value == header_value_blob_type_block)
        {
            return blob_type::block_blob;
        }
        else if (value == header_value_blob_type_page)
        {
            return blob_type::page_blob;
        }
        else if (value == header_value_blob_type_append)
        {
            return blob_type::append_blob;
        }
        else
        {
            return blob_type::unspecified;
        }
    }

    utility::size64_t blob_response_parsers::parse_blob_size(const web::http::http_response& response)
    {
        auto& headers = response.headers();
        utility::string_t value;

        if (headers.match(web::http::header_names::content_range, value))
        {
            auto slash = value.find(_XPLATSTR('/'));
            value = value.substr(slash + 1);
            return utility::conversions::scan_string<utility::size64_t>(value);
        }

        if (headers.match(ms_header_blob_content_length, value))
        {
            return utility::conversions::scan_string<utility::size64_t>(value);
        }

        return headers.content_length();
    }

    cloud_blob_properties blob_response_parsers::parse_blob_properties(const web::http::http_response& response)
    {
        cloud_blob_properties properties;

        properties.m_etag = parse_etag(response);
        properties.m_last_modified = parse_last_modified(response);
        properties.m_lease_status = parse_lease_status(response);
        properties.m_lease_state = parse_lease_state(response);
        properties.m_lease_duration = parse_lease_duration(response);
        properties.m_size = parse_blob_size(response);

        auto& headers = response.headers();
        properties.m_page_blob_sequence_number = utility::conversions::scan_string<int64_t>(get_header_value(headers, ms_header_blob_sequence_number));
        properties.m_append_blob_committed_block_count = utility::conversions::scan_string<int>(get_header_value(headers, ms_header_blob_committed_block_count));
        properties.m_cache_control = get_header_value(headers, web::http::header_names::cache_control);
        properties.m_content_disposition = get_header_value(headers, header_content_disposition);
        properties.m_content_encoding = get_header_value(headers, web::http::header_names::content_encoding);
        properties.m_content_language = get_header_value(headers, web::http::header_names::content_language);
        properties.m_content_md5 = get_header_value(headers, web::http::header_names::content_md5);
        properties.m_content_type = get_header_value(headers, web::http::header_names::content_type);
        properties.m_type = parse_blob_type(get_header_value(headers, ms_header_blob_type));

        return properties;
    }

    copy_state blob_response_parsers::parse_copy_state(const web::http::http_response& response)
    {
        copy_state state;

        auto& headers = response.headers();
        auto status = get_header_value(headers, ms_header_copy_status);
        if (!status.empty())
        {
            state.m_status = parse_copy_status(status);
            state.m_copy_id = get_header_value(headers, ms_header_copy_id);
            state.m_source = get_header_value(headers, ms_header_copy_source);
            state.m_completion_time = parse_copy_completion_time(get_header_value(headers, ms_header_copy_completion_time));
            state.m_status_description = get_header_value(headers, ms_header_copy_status_description);
            parse_copy_progress(get_header_value(headers, ms_header_copy_progress), state.m_bytes_copied, state.m_total_bytes);
        }

        return state;
    }

    utility::datetime blob_response_parsers::parse_copy_completion_time(const utility::string_t& value)
    {
        if (!value.empty())
        {
            return utility::datetime::from_string(value, utility::datetime::date_format::RFC_1123);
        }
        else
        {
            return utility::datetime();
        }
    }

    bool blob_response_parsers::parse_copy_progress(const utility::string_t& value, int64_t& bytes_copied, int64_t& bytes_total)
    {
        if (!value.empty())
        {
            utility::istringstream_t str(value);
            utility::char_t slash;
            str >> bytes_copied >> slash >> bytes_total;
            return true;
        }
        else
        {
            return false;
        }
    }

    copy_status blob_response_parsers::parse_copy_status(const utility::string_t& value)
    {
        if (value == header_value_copy_pending)
        {
            return copy_status::pending;
        }
        else if (value == header_value_copy_success)
        {
            return copy_status::success;
        }
        else if (value == header_value_copy_aborted)
        {
            return copy_status::aborted;
        }
        else if (value == header_value_copy_failed)
        {
            return copy_status::failed;
        }
        else
        {
            return copy_status::invalid;
        }
    }

}}} // namespace azure::storage::protocol
