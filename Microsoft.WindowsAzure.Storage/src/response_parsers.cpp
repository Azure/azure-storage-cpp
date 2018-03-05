// -----------------------------------------------------------------------------------------
// <copyright file="response_parsers.cpp" company="Microsoft">
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
#include "wascore/protocol_xml.h"
#include "wascore/protocol_json.h"
#include "wascore/constants.h"
#include "wascore/resources.h"

namespace azure { namespace storage { namespace protocol {

    void preprocess_response_void(const web::http::http_response& response, const request_result& result, operation_context context)
    {
        preprocess_response<char>(0, response, result, context);
    }

    utility::string_t get_header_value(const web::http::http_headers& headers, const utility::string_t& header)
    {
        utility::string_t value;
        headers.match(header, value);
        return value;
    }

    utility::string_t get_header_value(const web::http::http_response& response, const utility::string_t& header)
    {
        return get_header_value(response.headers(), header);
    }

    utility::size64_t parse_quota(const web::http::http_response& response)
    {
        utility::size64_t value;
        utility::istringstream_t iss(get_header_value(response, protocol::ms_header_share_quota));
        iss >> value;
        return value;
    }

    utility::string_t parse_etag(const web::http::http_response& response)
    {
        return get_header_value(response, web::http::header_names::etag);
    }

    utility::datetime parse_last_modified(const utility::string_t& value)
    {
        return utility::datetime::from_string(value, utility::datetime::date_format::RFC_1123);
    }

    utility::datetime parse_last_modified(const web::http::http_response& response)
    {
        utility::string_t value;
        if (response.headers().match(web::http::header_names::last_modified, value))
        {
            return parse_last_modified(value);
        }
        else
        {
            return utility::datetime();
        }
    }

    utility::string_t parse_lease_id(const web::http::http_response& response)
    {
        return get_header_value(response, ms_header_lease_id);
    }

    lease_status parse_lease_status(const utility::string_t& value)
    {
        if (value == header_value_locked)
        {
            return lease_status::locked;
        }
        else if (value == header_value_unlocked)
        {
            return lease_status::unlocked;
        }
        else
        {
            return lease_status::unspecified;
        }
    }

    lease_status parse_lease_status(const web::http::http_response& response)
    {
        return parse_lease_status(get_header_value(response, ms_header_lease_status));
    }

    lease_state parse_lease_state(const utility::string_t& value)
    {
        if (value == header_value_lease_available)
        {
            return lease_state::available;
        }
        else if (value == header_value_lease_breaking)
        {
            return lease_state::breaking;
        }
        else if (value == header_value_lease_broken)
        {
            return lease_state::broken;
        }
        else if (value == header_value_lease_expired)
        {
            return lease_state::expired;
        }
        else if (value == header_value_lease_leased)
        {
            return lease_state::leased;
        }
        else
        {
            return lease_state::unspecified;
        }
    }

    lease_state parse_lease_state(const web::http::http_response& response)
    {
        return parse_lease_state(get_header_value(response, ms_header_lease_state));
    }

    lease_duration parse_lease_duration(const utility::string_t& value)
    {
        if (value == header_value_lease_infinite)
        {
            return lease_duration::infinite;
        }
        else if (value == header_value_lease_fixed)
        {
            return lease_duration::fixed;
        }
        else
        {
            return lease_duration::unspecified;
        }
    }

    lease_duration parse_lease_duration(const web::http::http_response& response)
    {
        return parse_lease_duration(get_header_value(response, ms_header_lease_duration));
    }

    std::chrono::seconds parse_lease_time(const web::http::http_response& response)
    {
        utility::string_t value;
        if (response.headers().match(ms_header_lease_time, value))
        {
            int64_t seconds = utility::conversions::scan_string<int64_t>(value);
            return std::chrono::seconds(seconds);
        }
        else
        {
            return std::chrono::seconds();
        }
    }

    int parse_approximate_messages_count(const web::http::http_response& response)
    {
        utility::string_t value;
        if (response.headers().match(ms_header_approximate_messages_count, value))
        {
            return utility::conversions::scan_string<int>(value);
        }

        return -1;
    }

    utility::string_t parse_pop_receipt(const web::http::http_response& response)
    {
        utility::string_t value;
        if (response.headers().match(ms_header_pop_receipt, value))
        {
            return value;
        }

        return utility::string_t();
    }

    utility::datetime parse_next_visible_time(const web::http::http_response& response)
    {
        utility::string_t value;
        if (response.headers().match(ms_header_time_next_visible, value))
        {
            return utility::datetime::from_string(value, utility::datetime::RFC_1123);
        }

        return utility::datetime();
    }

    cloud_metadata parse_metadata(const web::http::http_response& response)
    {
        cloud_metadata metadata;

        auto& headers = response.headers();
        for (auto it = headers.begin(); it != headers.end(); ++it)
        {
            const utility::char_t *key = it->first.c_str();
            size_t key_size = it->first.size();
            // disables warning 4996 to bypass the usage of std::equal;
            // a more secure usage of std::equal with 5 parameters is supported by c++14.
            // to be compatible with c++11, warning 4996 is disabled.
            if ((key_size > ms_header_metadata_prefix_size) &&
                std::equal(ms_header_metadata_prefix, ms_header_metadata_prefix + ms_header_metadata_prefix_size, key, [](const utility::char_t &c1, const utility::char_t &c2) {return c1 == c2;}))
            {
                metadata.insert(std::make_pair(it->first.substr(ms_header_metadata_prefix_size), it->second));
            }
        }

        return metadata;
    }

    copy_state response_parsers::parse_copy_state(const web::http::http_response& response)
    {
        copy_state state;

        auto& headers = response.headers();
        auto status = get_header_value(headers, ms_header_copy_status);
        if (!status.empty())
        {
            state.m_status = parse_copy_status(status);
            state.m_copy_id = get_header_value(headers, ms_header_copy_id);
            state.m_source = get_header_value(headers, ms_header_copy_source);
            state.m_completion_time = parse_datetime(get_header_value(headers, ms_header_copy_completion_time));
            state.m_status_description = get_header_value(headers, ms_header_copy_status_description);
            state.m_destination_snapshot_time = parse_datetime(get_header_value(headers, ms_header_copy_destination_snapshot), utility::datetime::date_format::ISO_8601);
            parse_copy_progress(get_header_value(headers, ms_header_copy_progress), state.m_bytes_copied, state.m_total_bytes);
        }

        return state;
    }

    bool response_parsers::parse_boolean(const utility::string_t& value)
    {
        return value == _XPLATSTR("true");
    }

    utility::datetime response_parsers::parse_datetime(const utility::string_t& value, utility::datetime::date_format format)
    {
        if (!value.empty())
        {
            return utility::datetime::from_string(value, format);
        }
        else
        {
            return utility::datetime();
        }
    }

    bool response_parsers::parse_copy_progress(const utility::string_t& value, int64_t& bytes_copied, int64_t& bytes_total)
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

    copy_status response_parsers::parse_copy_status(const utility::string_t& value)
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

    bool is_matching_content_type(const utility::string_t& actual, const utility::string_t& expected)
    {
        // Check if the response's actual content type matches the expected content type.
        // It is OK if the actual content type has additional parameters (e.g. application/json;odata=minimalmetadata;streaming=true;charset=utf-8).
        return (actual.size() == expected.size() || (actual.size() > expected.size() && actual.at(expected.size()) == _XPLATSTR(';'))) &&
            std::equal(expected.cbegin(), expected.cend(), actual.cbegin());
    }

    storage_extended_error parse_extended_error(const web::http::http_response& response)
    {
        const web::http::http_headers& headers = response.headers();

        utility::string_t content_type;
        headers.match(web::http::header_names::content_type, content_type);
        std::transform(content_type.begin(), content_type.end(), content_type.begin(), core::utility_char_tolower);

        if (is_matching_content_type(content_type, protocol::header_value_content_type_json)) // application/json
        {
            web::json::value document = response.extract_json().get();
            return protocol::parse_table_error(document);
        }
        else // application/xml
        {
            utility::string_t error_code;
            utility::string_t error_message;
            std::unordered_map<utility::string_t, utility::string_t> details;

            concurrency::streams::istream body_stream = response.body();
            protocol::storage_error_reader reader(body_stream);
            error_code = reader.move_error_code();
            error_message = reader.move_error_message();
            details = reader.move_details();

            return storage_extended_error(std::move(error_code), std::move(error_message), std::move(details));
        }
    }

    blob_container_public_access_type parse_public_access_type(const utility::string_t& value)
    {
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
    
    blob_container_public_access_type parse_public_access_type(const web::http::http_response& response)
    {
        return parse_public_access_type(get_header_value(response.headers(), ms_header_blob_public_access));
    }

    premium_blob_tier response_parsers::parse_premium_blob_tier(const utility::string_t& value)
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

    standard_blob_tier response_parsers::parse_standard_blob_tier(const utility::string_t& value)
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

}}} // namespace azure::storage::protocol
