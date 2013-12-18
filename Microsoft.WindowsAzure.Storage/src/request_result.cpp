// -----------------------------------------------------------------------------------------
// <copyright file="request_result.cpp" company="Microsoft">
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
#include "was/common.h"
#include "wascore/protocol_xml.h"
#include "wascore/constants.h"

namespace wa { namespace storage {

    request_result::request_result()
        : m_is_response_available(false)
    {
    }

    request_result::request_result(const utility::datetime& start_time, storage_location target_location)
        : m_start_time(start_time),
        m_end_time(utility::datetime::utc_now()),
        m_target_location(target_location),
        m_is_response_available(false),
        m_http_status_code(0)
    {
    }

    request_result::request_result(const utility::datetime& start_time, storage_location target_location, const web::http::http_response& response, bool parse_body_as_error)
        : m_start_time(start_time),
        m_end_time(utility::datetime::utc_now()),
        m_target_location(target_location),
        m_is_response_available(true),
        m_http_status_code(response.status_code())
    {
        parse_headers(response.headers());
        if (parse_body_as_error)
        {
            parse_body(response.body());
        }
    }

    void request_result::parse_headers(const web::http::http_headers& headers)
    {
        headers.match(protocol::ms_header_request_id, m_service_request_id);
        headers.match(web::http::header_names::content_md5, m_content_md5);
        headers.match(web::http::header_names::etag, m_etag);

        utility::string_t request_date;
        if (headers.match(web::http::header_names::date, request_date))
        {
            m_request_date = utility::datetime::from_string(request_date, utility::datetime::date_format::RFC_1123);
        }
    }

    void request_result::parse_body(const concurrency::streams::istream& body)
    {
        protocol::storage_error_reader reader(body);
        m_extended_error = storage_extended_error(reader.extract_error_code(),
            reader.extract_error_message(),
            std::unordered_map<utility::string_t, utility::string_t>());
    }

}} // namespace wa::storage
