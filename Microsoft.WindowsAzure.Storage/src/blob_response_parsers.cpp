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

    cloud_blob_container_properties blob_response_parsers::parse_blob_container_properties(const web::http::http_response& response)
    {
        cloud_blob_container_properties properties;
        properties.m_etag = parse_etag(response);
        properties.m_last_modified = parse_last_modified(response);
        properties.m_lease_status = parse_lease_status(response);
        properties.m_lease_state = parse_lease_state(response);
        properties.m_lease_duration = parse_lease_duration(response);
        properties.m_public_access = parse_public_access_type(response);
        return properties;
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

    standard_blob_tier blob_response_parsers::parse_standard_blob_tier(const utility::string_t& value)
    {
        if (value == header_value_access_tier_hot)
        {
            return standard_blob_tier::hot;
        }
        else if (value == header_value_access_tier_cool)
        {
            return standard_blob_tier::cool;
        }
        else if (value == header_value_access_tier_archive)
        {
            return standard_blob_tier::archive;
        }
        else
        {
            return standard_blob_tier::unknown;
        }
    }

    premium_blob_tier blob_response_parsers::parse_premium_blob_tier(const utility::string_t& value)
    {
        if (value == header_value_access_tier_p4)
        {
            return premium_blob_tier::p4;
        }
        else if (value == header_value_access_tier_p6)
        {
            return premium_blob_tier::p6;
        }
        else if (value == header_value_access_tier_p10)
        {
            return premium_blob_tier::p10;
        }
        else if (value == header_value_access_tier_p20)
        {
            return premium_blob_tier::p20;
        }
        else if (value == header_value_access_tier_p30)
        {
            return premium_blob_tier::p30;
        }
        else if (value == header_value_access_tier_p40)
        {
            return premium_blob_tier::p40;
        }
        else if (value == header_value_access_tier_p50)
        {
            return premium_blob_tier::p50;
        }
        else if (value == header_value_access_tier_p60)
        {
            return premium_blob_tier::p60;
        }
        else
        {
            return premium_blob_tier::unknown;
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

    archive_status blob_response_parsers::parse_archive_status(const utility::string_t& value)
    {
        if (value == header_value_archive_status_to_hot)
        {
            return archive_status::rehydrate_pending_to_hot;
        }
        else if (value == header_value_archive_status_to_cool)
        {
            return archive_status::rehydrate_pending_to_cool;
        }
        else
        {
            return archive_status::unknown;
        }
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
        properties.m_content_type = get_header_value(headers, web::http::header_names::content_type);
        properties.m_type = parse_blob_type(get_header_value(headers, ms_header_blob_type));
        properties.m_content_md5 = get_header_value(headers, ms_header_blob_content_md5);
        if (properties.m_content_md5.empty())
        {
            properties.m_content_md5 = get_header_value(headers, web::http::header_names::content_md5);
        }
        
        auto change_time_string = get_header_value(headers, ms_header_tier_change_time);
        if (!change_time_string.empty())
        {
            properties.m_access_tier_change_time = utility::datetime::from_string(change_time_string, utility::datetime::date_format::RFC_1123);
        }

        auto tier_string = get_header_value(headers, ms_header_access_tier);
        properties.m_standard_blob_tier = parse_standard_blob_tier(tier_string);
        properties.m_premium_blob_tier = parse_premium_blob_tier(tier_string);
        properties.m_archive_status = parse_archive_status(get_header_value(headers, ms_header_archive_status));
        properties.m_server_encrypted = response_parsers::parse_boolean(get_header_value(headers, ms_header_server_encrypted));
        properties.m_is_incremental_copy = response_parsers::parse_boolean(get_header_value(headers, ms_header_incremental_copy));
        properties.m_access_tier_inferred = response_parsers::parse_boolean(get_header_value(headers, ms_header_access_tier_inferred));

        return properties;
    }

}}} // namespace azure::storage::protocol
