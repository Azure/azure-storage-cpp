// -----------------------------------------------------------------------------------------
// <copyright file="protocol.h" company="Microsoft">
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

#include "wascore/basic_types.h"
#include "was/blob.h"
#include "was/queue.h"
#include "was/table.h"
#include "was/file.h"
#include "wascore/executor.h"

namespace azure { namespace storage { namespace protocol {

    // Common request factory methods

    web::http::http_request base_request(web::http::method method, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_service_properties(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request set_service_properties(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_service_stats(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    void add_optional_header(web::http::http_headers& headers, const utility::string_t& header, const utility::string_t& value);
    void add_metadata(web::http::http_request& request, const cloud_metadata& metadata);

    // Blob request factory methods

    web::http::http_request create_blob_container(blob_container_public_access_type access_type, const cloud_metadata& metadata, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request delete_blob_container(const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_blob_container_properties(const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request set_blob_container_metadata(const cloud_metadata& metadata, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_blob_container_acl(const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request set_blob_container_acl(blob_container_public_access_type access_type, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request list_containers(const utility::string_t& prefix, container_listing_details::values includes, int max_results, const continuation_token& token, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request list_blobs(const utility::string_t& prefix, const utility::string_t& delimiter, blob_listing_details::values includes, int max_results, const continuation_token& token, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request lease_blob_container(const utility::string_t& lease_action, const utility::string_t& proposed_lease_id, const lease_time& duration, const lease_break_period& break_period, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request lease_blob(const utility::string_t& lease_action, const utility::string_t& proposed_lease_id, const lease_time& duration, const lease_break_period& break_period, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request put_block(const utility::string_t& block_id, const utility::string_t& content_md5, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request put_block_list(const cloud_blob_properties& properties, const cloud_metadata& metadata, const utility::string_t& content_md5, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_block_list(block_listing_filter listing_filter, const utility::string_t& snapshot_time, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_page_ranges(utility::size64_t offset, utility::size64_t length, const utility::string_t& snapshot_time, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_page_ranges_diff(utility::string_t previous_snapshort_time, utility::size64_t offset, utility::size64_t length, const utility::string_t& snapshot_time, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request put_page(page_range range, page_write write, const utility::string_t& content_md5, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request append_block(const utility::string_t& content_md5, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request put_block_blob(const cloud_blob_properties& properties, const cloud_metadata& metadata, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request put_page_blob(utility::size64_t size, const utility::string_t& tier, int64_t sequence_number, const cloud_blob_properties& properties, const cloud_metadata& metadata, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request put_append_blob(const cloud_blob_properties& properties, const cloud_metadata& metadata, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_blob(utility::size64_t offset, utility::size64_t length, bool get_range_content_md5, const utility::string_t& snapshot_time, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_blob_properties(const utility::string_t& snapshot_time, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request set_blob_properties(const cloud_blob_properties& properties, const cloud_metadata& metadata, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request resize_page_blob(utility::size64_t size, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request set_page_blob_sequence_number(const azure::storage::sequence_number& sequence_number, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request snapshot_blob(const cloud_metadata& metadata, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request set_blob_metadata(const cloud_metadata& metadata, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request delete_blob(delete_snapshots_option snapshots_option, const utility::string_t& snapshot_time, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request copy_blob(const web::http::uri& source, const utility::string_t& tier, const access_condition& source_condition, const cloud_metadata& metadata, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request abort_copy_blob(const utility::string_t& copy_id, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request incremental_copy_blob(const web::http::uri& source, const access_condition& condition, const cloud_metadata& metadata, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request set_blob_tier(const utility::string_t& tier, const access_condition& condition, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    void add_lease_id(web::http::http_request& request, const access_condition& condition);
    void add_sequence_number_condition(web::http::http_request& request, const access_condition& condition);
    void add_access_condition(web::http::http_request& request, const access_condition& condition);
    void add_source_access_condition(web::http::http_request& request, const access_condition& condition);
    void add_append_condition(web::http::http_request& request, const access_condition& condition);

    // Table request factory methods

    storage_uri generate_table_uri(const cloud_table_client& service_client, const cloud_table& table);
    storage_uri generate_table_uri(const cloud_table_client& service_client, const cloud_table& table, bool create_table);
    storage_uri generate_table_uri(const cloud_table_client& service_client, const cloud_table& table, const table_operation& operation);
    storage_uri generate_table_uri(const cloud_table_client& service_client, const cloud_table& table, const table_batch_operation& operation);
    storage_uri generate_table_uri(const cloud_table_client& service_client, const cloud_table& table, const table_query& query, const continuation_token& token);
    web::http::http_request execute_table_operation(const cloud_table& table, table_operation_type operation_type, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request execute_operation(const table_operation& operation, table_payload_format payload_format, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request execute_batch_operation(Concurrency::streams::stringstreambuf& response_buffer, const cloud_table& table, const table_batch_operation& batch_operation, table_payload_format payload_format, bool is_query, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request execute_query(table_payload_format payload_format, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_table_acl(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request set_table_acl(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    utility::string_t get_property_type_name(edm_type property_type);
    utility::string_t get_multipart_content_type(const utility::string_t& boundary_name);

    // Queue request factory methods

    storage_uri generate_queue_uri(const cloud_queue_client& service_client, const cloud_queue& queue);
    storage_uri generate_queue_uri(const cloud_queue_client& service_client, const utility::string_t& prefix, bool get_metadata, int max_results, const continuation_token& token);
    storage_uri generate_queue_message_uri(const cloud_queue_client& service_client, const cloud_queue& queue);
    storage_uri generate_queue_message_uri(const cloud_queue_client& service_client, const cloud_queue& queue, const cloud_queue_message& message);
    web::http::http_request list_queues(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request create_queue(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request delete_queue(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request add_message(const cloud_queue_message& message, std::chrono::seconds time_to_live, std::chrono::seconds initial_visibility_timeout, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_messages(size_t message_count, std::chrono::seconds visibility_timeout, bool is_peek, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request delete_message(const cloud_queue_message& message, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request update_message(const cloud_queue_message& message, std::chrono::seconds visibility_timeout, bool update_contents, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request clear_messages(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request download_queue_metadata(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request upload_queue_metadata(const cloud_metadata& metadata, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_queue_acl(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request set_queue_acl(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context);

    // File request factory methods
    web::http::http_request list_shares(const utility::string_t& prefix, bool get_metadata, int max_results, const continuation_token& token, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request create_file_share(const utility::size64_t max_size, const cloud_metadata& metadata, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request delete_file_share(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_file_share_properties(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request set_file_share_properties(const cloud_file_share_properties& properties, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request set_file_share_metadata(const cloud_metadata& metadata, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_file_share_stats(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_file_share_acl(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request set_file_share_acl(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request list_files_and_directories(const utility::string_t& prefix, int64_t max_results, const continuation_token& token, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request create_file_directory(const cloud_metadata& metadata, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request delete_file_directory(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_file_directory_properties(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request set_file_directory_metadata(const cloud_metadata& metadata, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request create_file(const int64_t length, const cloud_metadata& metadata, const cloud_file_properties& properties,  web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request delete_file(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_file_properties(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request set_file_properties(const cloud_file_properties& properties, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request resize_with_properties(const cloud_file_properties& properties, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request set_file_metadata(const cloud_metadata& metadata, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request copy_file(const web::http::uri& source, const cloud_metadata& metadata, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request copy_file_from_blob(const web::http::uri& source, const access_condition& condition, const cloud_metadata& metadata, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request abort_copy_file(const utility::string_t& copy_id, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request list_file_ranges(utility::size64_t start_offset, utility::size64_t length, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request put_file_range(file_range range, file_range_write write, utility::string_t content_md5, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    web::http::http_request get_file(utility::size64_t start_offset, utility::size64_t length, bool md5_validation, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context);
    
    // Common response parsers

    template<typename T>
    T preprocess_response(T return_value, const web::http::http_response& response, const request_result&, operation_context)
    {
        switch (response.status_code())
        {
        case web::http::status_codes::OK:
        case web::http::status_codes::Created:
        case web::http::status_codes::Accepted:
        case web::http::status_codes::NoContent:
        case web::http::status_codes::PartialContent:
            break;

        default:
            // Throwing an empty message here will cause the executor to read the response
            // and try to come up with an error message using the status code and body.
            throw storage_exception("");
        }

        return return_value;
    }

    void preprocess_response_void(const web::http::http_response& response, const request_result& result, operation_context context);

    utility::datetime parse_last_modified(const utility::string_t& value);
    utility::string_t parse_lease_id(const utility::string_t& value);
    lease_status parse_lease_status(const utility::string_t& value);
    lease_state parse_lease_state(const utility::string_t& value);
    lease_duration parse_lease_duration(const utility::string_t& value);
    std::chrono::seconds parse_lease_time(const utility::string_t& value);
    int parse_approximate_messages_count(const web::http::http_response& response);
    utility::string_t parse_pop_receipt(const web::http::http_response& response);
    utility::datetime parse_next_visible_time(const web::http::http_response& response);
    blob_container_public_access_type parse_public_access_type(const utility::string_t& value);

    utility::string_t get_header_value(const web::http::http_response& response, const utility::string_t& header);
    utility::string_t get_header_value(const web::http::http_headers& headers, const utility::string_t& header);

    utility::size64_t parse_quota(const web::http::http_response& response);
    utility::string_t parse_etag(const web::http::http_response& response);
    utility::datetime parse_last_modified(const web::http::http_response& response);
    utility::string_t parse_lease_id(const web::http::http_response& response);
    lease_status parse_lease_status(const web::http::http_response& response);
    lease_state parse_lease_state(const web::http::http_response& response);
    lease_duration parse_lease_duration(const web::http::http_response& response);
    std::chrono::seconds parse_lease_time(const web::http::http_response& response);
    cloud_metadata parse_metadata(const web::http::http_response& response);
    storage_extended_error parse_extended_error(const web::http::http_response& response);
    blob_container_public_access_type parse_public_access_type(const web::http::http_response& response);

    class response_parsers
    {
    public:
        static copy_state parse_copy_state(const web::http::http_response& response);
        static bool parse_copy_progress(const utility::string_t& value, int64_t& bytes_copied, int64_t& bytes_total);
        static copy_status parse_copy_status(const utility::string_t& value);
        static bool parse_boolean(const utility::string_t& value);
        static utility::datetime parse_datetime(const utility::string_t& value, utility::datetime::date_format format = utility::datetime::date_format::RFC_1123);
        static standard_blob_tier parse_standard_blob_tier(const utility::string_t& value);
        static premium_blob_tier parse_premium_blob_tier(const utility::string_t& value);
    };

    class blob_response_parsers
    {
    public:
        static blob_type parse_blob_type(const utility::string_t& value);
        static standard_blob_tier parse_standard_blob_tier(const utility::string_t & value);
        static premium_blob_tier parse_premium_blob_tier(const utility::string_t & value);
        static utility::size64_t parse_blob_size(const web::http::http_response& response);
        static archive_status parse_archive_status(const utility::string_t& value);

        static cloud_blob_container_properties parse_blob_container_properties(const web::http::http_response& response);
        static cloud_blob_properties parse_blob_properties(const web::http::http_response& response);
    };

    class table_response_parsers
    {
    public:
        static utility::string_t parse_etag(const web::http::http_response& response);
        static continuation_token parse_continuation_token(const web::http::http_response& response, const request_result& result);
        static std::vector<table_result> parse_batch_results(const web::http::http_response& response, Concurrency::streams::stringstreambuf& response_buffer, bool is_query, size_t batch_size);
        static std::vector<table_entity> parse_query_results(const web::json::value& obj);
    };

    class file_response_parsers
    {
    public:
        static utility::size64_t parse_file_size(const web::http::http_response& response);

        static cloud_file_share_properties parse_file_share_properties(const web::http::http_response& response);
        static cloud_file_directory_properties parse_file_directory_properties(const web::http::http_response& response);
        static cloud_file_properties parse_file_properties(const web::http::http_response& response);
    };

}}} // namespace azure::storage::protocol
