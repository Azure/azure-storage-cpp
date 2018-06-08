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

#include <map>

#ifndef _WIN32
    #include "pplx/threadpool.h"
#endif
#include "cpprest/streams.h"

#include "was/core.h"

#pragma push_macro("max")
#undef max

namespace azure { namespace storage { namespace core {

#pragma region Navigation Helpers

    bool use_path_style(const storage_uri& uri);
    utility::string_t::size_type find_path_start(const web::http::uri& uri);
    bool parse_container_uri(const storage_uri& uri, utility::string_t& container_name);
    bool parse_blob_uri(const storage_uri& uri, utility::string_t& container_name, utility::string_t& blob_name);
    bool parse_file_directory_uri(const storage_uri& uri, utility::string_t& share_name, utility::string_t& directory_name);
    bool parse_file_uri(const storage_uri& uri, utility::string_t& share_name, utility::string_t& directory_name, utility::string_t& file_name);
    storage_uri create_stripped_uri(const storage_uri& uri);
    void parse_query_and_verify(const storage_uri& uri, storage_credentials& credentials, bool require_signed_resource);
    storage_uri verify_blob_uri(const storage_uri& uri, storage_credentials& credentials, utility::string_t& snapshot);
    storage_uri append_path_to_uri(const storage_uri& uri, const utility::string_t& path);
    utility::string_t get_parent_name(utility::string_t name, const utility::string_t& delimiter);
    bool parse_object_uri(const storage_uri& uri, utility::string_t& object_name);
    storage_uri get_service_client_uri(const storage_uri& uri);

#pragma endregion

#pragma region MIME Helpers

    utility::string_t generate_boundary_name(const utility::string_t& prefix);
    void write_boundary(utility::string_t& body_text, const utility::string_t& boundary_name, bool is_closure = false);
    void write_mime_changeset_headers(utility::string_t& body_text);
    void write_request_line(utility::string_t& body_text, const web::http::method& method, const web::http::uri& uri);
    void write_request_headers(utility::string_t& body_text, const web::http::http_headers& headers);
    void write_request_payload(utility::string_t& body_text, const web::json::value& json_object);

#pragma endregion

#pragma region Common Utilities

    utility::string_t make_query_parameter(const utility::string_t& parameter_name, const utility::string_t& parameter_value, bool do_encoding = true);
    utility::size64_t get_remaining_stream_length(concurrency::streams::istream stream);
    pplx::task<utility::size64_t> stream_copy_async(concurrency::streams::istream istream, concurrency::streams::ostream ostream, utility::size64_t length, utility::size64_t max_length = std::numeric_limits<utility::size64_t>::max());
    pplx::task<void> complete_after(std::chrono::milliseconds timeout);
    std::vector<utility::string_t> string_split(const utility::string_t& string, const utility::string_t& separator);
    bool is_empty_or_whitespace(const utility::string_t& value);
    bool has_whitespace_or_empty(const utility::string_t& str);
    utility::string_t single_quote(const utility::string_t& value);
    bool is_nan(double value);
    bool is_finite(double value);
    bool is_integral(const utility::string_t& value);
    utility::datetime truncate_fractional_seconds(utility::datetime value);
    utility::string_t convert_to_string(double value);
    utility::string_t convert_to_string(const std::vector<uint8_t>& value);
    utility::string_t convert_to_string_with_fixed_length_fractional_seconds(utility::datetime value);
    utility::char_t utility_char_tolower(const utility::char_t& character);
    utility::string_t str_trim_starting_trailing_whitespaces(const utility::string_t& str);

    template<typename T>
    utility::string_t convert_to_string(T value)
    {
        utility::ostringstream_t buffer;
        buffer << value;
        return buffer.str();
    }

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

    template<typename T>
    utility::string_t make_query_parameter(const utility::string_t& parameter_name, T parameter_value, bool do_encoding = true)
    {
        return make_query_parameter(parameter_name, convert_to_string(parameter_value), do_encoding);
    }

#pragma endregion

#ifndef _WIN32
    class http_client_reusable
    {
    public:
        WASTORAGE_API static std::shared_ptr<web::http::client::http_client> get_http_client(const web::uri& uri);
        WASTORAGE_API static std::shared_ptr<web::http::client::http_client> get_http_client(const web::uri& uri, const web::http::client::http_client_config& config);

    private:
        static const boost::asio::io_service& s_service;
        WASTORAGE_API static std::map<utility::string_t, std::shared_ptr<web::http::client::http_client>> s_http_clients;
        WASTORAGE_API static std::mutex s_mutex;
    };
#endif

}}} // namespace azure::storage::core

#pragma pop_macro("max")
