// -----------------------------------------------------------------------------------------
// <copyright file="table_response_parsers.cpp" company="Microsoft">
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
#include "wascore/protocol_json.h"
#include "was/common.h"

namespace azure { namespace storage { namespace protocol {

    utility::string_t table_response_parsers::parse_etag(const web::http::http_response& response)
    {
        web::http::http_headers headers = response.headers();
        web::http::http_headers::const_iterator itr = headers.find(_XPLATSTR("ETag"));

        utility::string_t etag;
        if (itr != headers.end())
        {
            etag = itr->second;
        }

        return etag;
    }

    continuation_token table_response_parsers::parse_continuation_token(const web::http::http_response& response, const request_result& result)
    {
        web::http::uri_builder builder;

        web::http::http_headers headers = response.headers();

        web::http::http_headers::iterator next_partition_key_it = headers.find(ms_header_continuation_next_partition_key);
        if (next_partition_key_it != headers.end())
        {
            builder.append_query(core::make_query_parameter(table_query_next_partition_key, next_partition_key_it->second));
        }

        web::http::http_headers::iterator next_row_key_it = headers.find(ms_header_continuation_next_row_key);
        if (next_row_key_it != headers.end())
        {
            builder.append_query(core::make_query_parameter(table_query_next_row_key, next_row_key_it->second));
        }

        web::http::http_headers::iterator next_table_name_it = headers.find(ms_header_continuation_next_table_name);
        if (next_table_name_it != headers.end())
        {
            builder.append_query(core::make_query_parameter(table_query_next_table_name, next_table_name_it->second));
        }

        continuation_token token(builder.query());
        token.set_target_location(result.target_location());

        return token;
    }

    std::vector<table_result> table_response_parsers::parse_batch_results(const web::http::http_response& response, Concurrency::streams::stringstreambuf& response_buffer, bool is_query, size_t batch_size)
    {
        std::vector<table_result> batch_result;
        batch_result.reserve(batch_size);

        std::string& response_body = response_buffer.collection();

        // TODO: Make this Casablanca code more robust

        // More strings we will search for during the course of parsing
        std::string http_begin("HTTP"), etag_header("ETag: "), crlf("\r\n"), crlfcrlf("\r\n\r\n"), space(" ");

        if (is_query)
        {
            // Find the HTTP status line
            size_t response_begin = response_body.find(http_begin);

            // Find the status code within that line
            size_t status_code_begin = response_body.find(space, response_begin) + space.size();
            size_t status_code_end = response_body.find(space, status_code_begin);
            std::string status_code_string = response_body.substr(status_code_begin, status_code_end - status_code_begin);

            // Extract the status code as an integer
            int status_code = utility::conversions::scan_string<int>(utility::conversions::to_string_t(status_code_string));

            // Acceptable codes are 'Created' and 'NoContent'
            if (status_code == web::http::status_codes::OK || status_code == web::http::status_codes::Created || status_code == web::http::status_codes::Accepted || status_code == web::http::status_codes::NoContent || status_code == web::http::status_codes::PartialContent || status_code == web::http::status_codes::NotFound)
            {
                table_result result;
                result.set_http_status_code(status_code);

                // The operation succeeded. Try to find an ETag: header if it exists. Delete operations will not have an etag header
                // so copy the list of headers into a substring and search within the list, lest we cross a changeset boundary.
                size_t headersBegin = response_body.find(crlf, status_code_end) + crlf.size();
                size_t headersEnd = response_body.find(crlfcrlf, headersBegin) + crlf.size();
                std::string headers = response_body.substr(headersBegin, headersEnd - headersBegin);

                utility::string_t etag_value;

                size_t etagBegin = headers.find(etag_header);
                if (etagBegin != std::string::npos)
                {
                    etagBegin += etag_header.size();
                    size_t etagEnd = headers.find(crlf, etagBegin);
                    std::string etag = headers.substr(etagBegin, etagEnd - etagBegin);
                    etag_value = utility::conversions::to_string_t(etag);

                    result.set_etag(etag_value);
                }

                size_t content_begin_offset = response_body.find(crlfcrlf, status_code_end) + crlfcrlf.size();
                size_t content_end_offset = response_body.find("\r\n--batchresponse", content_begin_offset) + crlf.size();

                std::string content_string = response_body.substr(content_begin_offset, content_end_offset - content_begin_offset);

                web::json::value document = web::json::value::parse(utility::conversions::to_string_t(content_string));
                table_entity entity = protocol::parse_table_entity(document);
                entity.set_etag(etag_value);
                result.set_entity(entity);

                batch_result.push_back(result);
            }
            else
            {
                size_t status_message_begin = status_code_end + space.size();
                size_t status_message_end = response_body.find(crlf, status_message_begin);

                std::string status_message = response_body.substr(status_message_begin, status_message_end - status_message_begin);

                // An operation failed. Find the start of the XML block that contains information about the error
                size_t content_begin_offset = response_body.find(crlfcrlf, status_message_end) + crlfcrlf.size();
                size_t content_end_offset = response_body.find("\r\n--batchresponse", content_begin_offset) + crlf.size();

                std::string error_string = response_body.substr(content_begin_offset, content_end_offset - content_begin_offset);

                web::json::value document = web::json::value::parse(utility::conversions::to_string_t(error_string));
                storage_extended_error extended_error = protocol::parse_table_error(document);
                request_result request_result(utility::datetime(), storage_location::unspecified, response, (web::http::status_code) status_code, extended_error);
                throw storage_exception(status_message, request_result);
            }
        }
        else
        {
            // Strings we will search for during the course of parsing
            std::string content_type("Content-Type"), boundary_begin("boundary=");

            // Find the Content-Type header for the batch - it contains the changeset marker right after the occurence of "boundary="
            size_t content_typeOffset = response_body.find(content_type);
            size_t changeset_begin_offset = response_body.find(boundary_begin, content_typeOffset) + boundary_begin.size();
            size_t changeset_end_offset = response_body.find(crlf, changeset_begin_offset);

            std::string changeset_marker_prefix = "--" + response_body.substr(changeset_begin_offset, changeset_end_offset - changeset_begin_offset);
            std::string changeset_begin_marker = changeset_marker_prefix + crlf;

            size_t result_index = SIZE_MAX;

            // Loop through the response looking for changeset begin markers.
            size_t next_changeset = response_body.find(changeset_begin_marker, changeset_end_offset);

            while(next_changeset != std::string::npos)
            {
                ++result_index;
                // Find the HTTP status line
                size_t response_begin = response_body.find(http_begin, next_changeset + changeset_begin_marker.size());

                // Find the status code within that line
                size_t status_code_begin = response_body.find(space, response_begin) + space.size();
                size_t status_code_end = response_body.find(space, status_code_begin);
                std::string status_code_string = response_body.substr(status_code_begin, status_code_end - status_code_begin);

                // Extract the status code as an integer
                int status_code = utility::conversions::scan_string<int>(utility::conversions::to_string_t(status_code_string));

                // Acceptable codes are 'Created' and 'NoContent'
                if (status_code == web::http::status_codes::OK || status_code == web::http::status_codes::Created || status_code == web::http::status_codes::Accepted || status_code == web::http::status_codes::NoContent || status_code == web::http::status_codes::PartialContent)
                {
                    table_result result;
                    result.set_http_status_code(status_code);

                    // The operation succeeded. Try to find an ETag: header if it exists. Delete operations will not have an etag header
                    // so copy the list of headers into a substring and search within the list, lest we cross a changeset boundary.
                    size_t headersBegin = response_body.find(crlf, status_code_end) + crlf.size();
                    size_t headersEnd = response_body.find(crlfcrlf, headersBegin) + crlf.size();
                    std::string headers = response_body.substr(headersBegin, headersEnd - headersBegin);

                    size_t etagBegin = headers.find(etag_header);
                    if (etagBegin != std::string::npos)
                    {
                        etagBegin += etag_header.size();
                        size_t etagEnd = headers.find(crlf, etagBegin);
                        std::string etag = headers.substr(etagBegin, etagEnd - etagBegin);

                        result.set_etag(utility::conversions::to_string_t(etag));
                    }

                    batch_result.push_back(result);

                    next_changeset = response_body.find(changeset_begin_marker, headersEnd);
                }
                else
                {
                    size_t status_message_begin = status_code_end + space.size();
                    size_t status_message_end = response_body.find(crlf, status_message_begin);

                    std::string status_message = response_body.substr(status_message_begin, status_message_end - status_message_begin);

                    // An operation failed. Find the start of the XML block that contains information about the error
                    size_t content_begin_offset = response_body.find(crlfcrlf, status_message_end) + crlfcrlf.size();
                    size_t content_end_offset = response_body.find(changeset_marker_prefix, content_begin_offset);

                    std::string error_string = response_body.substr(content_begin_offset, content_end_offset - content_begin_offset);

                    web::json::value document = web::json::value::parse(utility::conversions::to_string_t(error_string));
                    storage_extended_error extended_error = protocol::parse_table_error(document);
                    request_result request_result(utility::datetime(), storage_location::unspecified, response, (web::http::status_code) status_code, extended_error);
                    throw storage_exception(status_message, request_result);
                }
            }
        }

        if (batch_result.size() != batch_size) {
            std::string str;
            str.reserve(128);
            str.append(protocol::error_batch_size_not_match_response).append(" Sent ").append(std::to_string(batch_size)).append(" batch operations and received ").append(std::to_string(batch_result.size())).append(" batch results.");
            throw storage_exception(str, false);
        }
        return batch_result;
    }

    std::vector<table_entity> table_response_parsers::parse_query_results(const web::json::value& document)
    {
        std::vector<table_entity> result;

        if (document.is_object())
        {
            const web::json::object& results_obj = document.as_object();
            web::json::object::const_iterator value_it = results_obj.find(_XPLATSTR("value"));
            if (value_it != results_obj.cend())
            {
                const web::json::value& value_obj = value_it->second;
                if (value_obj.is_array())
                {
                    const web::json::array& entities_array = value_obj.as_array();

                    for (web::json::array::const_iterator entity_it = entities_array.cbegin(); entity_it != entities_array.cend(); ++entity_it)
                    {
                        const web::json::value& entity_obj = *entity_it;
                        if (entity_obj.is_object() && entity_obj.size() > 0)
                        {
                            table_entity entity = parse_table_entity(entity_obj);
                            result.push_back(entity);
                        }
                    }
                }
            }
        }

        return result;
    }

}}} // namespace azure::storage::protocol
