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
#include "wascore/protocol.h"
#include "wascore/util.h"
#include "wascore/constants.h"

namespace azure { namespace storage {

    request_result::request_result(utility::datetime start_time, storage_location target_location, const web::http::http_response& response, bool parse_body_as_error)
        : m_is_response_available(true),
        m_start_time(start_time),
        m_target_location(target_location),
        m_end_time(utility::datetime::utc_now()),
        m_http_status_code(response.status_code()),
        m_content_length(std::numeric_limits<utility::size64_t>::max())
    {
        parse_headers(response.headers());
        if (parse_body_as_error)
        {
            parse_body(response);
        }
    }

    request_result::request_result(utility::datetime start_time, storage_location target_location, const web::http::http_response& response, web::http::status_code http_status_code, storage_extended_error extended_error)
        : m_is_response_available(true),
        m_start_time(start_time),
        m_target_location(target_location),
        m_end_time(utility::datetime::utc_now()),
        m_http_status_code(http_status_code),
        m_extended_error(std::move(extended_error)),
        m_content_length(std::numeric_limits<utility::size64_t>::max())
    {
        parse_headers(response.headers());
    }

    void request_result::parse_headers(const web::http::http_headers& headers)
    {
        headers.match(protocol::ms_header_request_id, m_service_request_id);
        headers.match(web::http::header_names::content_length, m_content_length);
        headers.match(web::http::header_names::content_md5, m_content_md5);
        headers.match(web::http::header_names::etag, m_etag);

        utility::string_t request_server_encrypted;
        if (headers.match(protocol::ms_header_request_server_encrypted, request_server_encrypted))
        {
            m_request_server_encrypted = (request_server_encrypted == _XPLATSTR("true"));
        }

        utility::string_t request_date;
        if (headers.match(web::http::header_names::date, request_date))
        {
            m_request_date = utility::datetime::from_string(request_date, utility::datetime::date_format::RFC_1123);
        }
    }

    void request_result::parse_body(const web::http::http_response& response)
    {
        m_extended_error = protocol::parse_extended_error(response);
    }

}} // namespace azure::storage
