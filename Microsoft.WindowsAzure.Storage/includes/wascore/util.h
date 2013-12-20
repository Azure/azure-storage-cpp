// -----------------------------------------------------------------------------------------
// <copyright file="util.h" company="Microsoft">
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

#pragma once

#include "cpprest/streams.h"

#include "was/core.h"

namespace wa { namespace storage { namespace core {

#pragma region Navigation Helpers

    bool use_path_style(const storage_uri& uri);
    bool parse_container_uri(const storage_uri& uri, utility::string_t& container_name);
    bool parse_blob_uri(const storage_uri& uri, utility::string_t& container_name, utility::string_t& blob_name);
    storage_uri verify_blob_uri(const storage_uri& uri, storage_credentials& credentials, utility::string_t& snapshot);
    storage_uri append_path_to_uri(const storage_uri& uri, const utility::string_t& path);
    utility::string_t get_parent_name(utility::string_t name, const utility::string_t& delimiter);
    storage_uri get_service_client_uri(const storage_uri& uri);

#pragma endregion

#pragma region MIME Helpers

    utility::string_t generate_boundary_name(const utility::string_t& prefix);
    void write_boundary(utility::string_t& body_text, const utility::string_t& boundary_name, bool is_closure = false);
    void write_mime_multipart_headers(utility::string_t& body_text);
    void write_request_line(utility::string_t& body_text, const web::http::method& method, const web::http::uri& uri);
    void write_request_headers(utility::string_t& body_text, const web::http::http_headers& headers);
    //void write_content_type_request_header(utility::string_t& body_text, const utility::string_t& boundary_name);
    //void write_content_id_request_header(utility::string_t& body_text, int content_id);
    //void write_request_header_closure(utility::string_t& body_text);
    void write_request_payload(utility::string_t& body_text, web::json::value json_object);

#pragma endregion

#pragma region Common Utilities

    utility::size64_t get_remaining_stream_length(concurrency::streams::istream stream);
    pplx::task<utility::size64_t> stream_copy_async(concurrency::streams::istream istream, concurrency::streams::ostream ostream, utility::size64_t length);
    pplx::task<void> complete_after(std::chrono::milliseconds timeout);
    utility::string_t single_quote(const utility::string_t& value);
    bool is_nan(double value);
    bool is_finite(double value);
    utility::datetime truncate_fractional_seconds(const utility::datetime& value);
    utility::string_t convert_to_string(int value);
    utility::string_t convert_to_string(const std::vector<uint8_t>& value);
    std::vector<utility::string_t> string_split(const utility::string_t& string, const utility::string_t& separator);
    utility::string_t convert_to_string(utility::datetime value);
    utility::datetime parse_datetime(utility::string_t value);

    template<typename T>
    utility::string_t string_join(const std::vector<T>& vector, const utility::string_t& separator)
    {
        if (vector.empty())
        {
            return utility::string_t();
        }

        utility::ostringstream_t str;
        auto iter = vector.cbegin();
        str << *iter;
        for (++iter; iter != vector.cend(); ++iter)
        {
            str << separator << *iter;
        }

        return str.str();
    }

#pragma endregion

}}} // namespace wa::storage::core
