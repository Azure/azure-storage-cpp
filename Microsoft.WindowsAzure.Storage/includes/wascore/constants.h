// -----------------------------------------------------------------------------------------
// <copyright file="constants.h" company="Microsoft">
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

#include "cpprest/asyncrt_utils.h"

#include "wascore/basic_types.h"

namespace wa { namespace storage { namespace protocol {

    // size constants
    const size_t max_block_size = 4 * 1024 * 1024;
    const size_t default_buffer_size = 64 * 1024;
    const utility::size64_t default_single_blob_upload_threshold = 32 * 1024 * 1024;
    const size_t invalid_size_t = (size_t)-1;
    const utility::size64_t invalid_size64_t = (utility::size64_t)-1;

    // double special values
    const utility::string_t double_not_a_number(U("NaN"));
    const utility::string_t double_infinity(U("Infinity"));
    const utility::string_t double_negative_infinity(U("-Infinity"));

    // duration constants
    const std::chrono::seconds default_retry_interval(3);
    const std::chrono::seconds default_server_timeout(90);

    // uri query parameters
    const utility::string_t uri_query_timeout(U("timeout"));
    const utility::string_t uri_query_resource_type(U("restype"));
    const utility::string_t uri_query_snapshot(U("snapshot"));
    const utility::string_t uri_query_component(U("comp"));
    const utility::string_t uri_query_block_id(U("blockid"));
    const utility::string_t uri_query_block_list_type(U("blocklisttype"));
    const utility::string_t uri_query_prefix(U("prefix"));
    const utility::string_t uri_query_delimiter(U("delimiter"));
    const utility::string_t uri_query_marker(U("marker"));
    const utility::string_t uri_query_max_results(U("maxresults"));
    const utility::string_t uri_query_include(U("include"));
    const utility::string_t uri_query_copy_id(U("copyid"));

    // SAS query parameters
    const utility::string_t uri_query_sas_start(U("st"));
    const utility::string_t uri_query_sas_expiry(U("se"));
    const utility::string_t uri_query_sas_resource(U("sr"));
    const utility::string_t uri_query_sas_table_name(U("tn"));
    const utility::string_t uri_query_sas_permissions(U("sp"));
    const utility::string_t uri_query_sas_start_partition_key(U("spk"));
    const utility::string_t uri_query_sas_start_row_key(U("srk"));
    const utility::string_t uri_query_sas_end_partition_key(U("epk"));
    const utility::string_t uri_query_sas_end_row_key(U("erk"));
    const utility::string_t uri_query_sas_identifier(U("si"));
    const utility::string_t uri_query_sas_version(U("sv"));
    const utility::string_t uri_query_sas_signature(U("sig"));
    const utility::string_t uri_query_sas_cache_control(U("rscc"));
    const utility::string_t uri_query_sas_content_type(U("rsct"));
    const utility::string_t uri_query_sas_content_encoding(U("rsce"));
    const utility::string_t uri_query_sas_content_language(U("rscl"));
    const utility::string_t uri_query_sas_content_disposition(U("rscd"));

    // table query parameters
    const utility::string_t table_query_next_partition_key(U("NextPartitionKey"));
    const utility::string_t table_query_next_row_key(U("NextRowKey"));
    const utility::string_t table_query_next_table_name(U("NextTableName"));

    // resource types
    const utility::string_t resource_service(U("service"));
    const utility::string_t resource_container(U("container"));
    const utility::string_t resource_blob(U("blob"));
    const utility::string_t resource_block_list_all(U("all"));
    const utility::string_t resource_block_list_committed(U("committed"));
    const utility::string_t resource_block_list_uncommitted(U("uncommitted"));

    // components
    const utility::string_t component_list(U("list"));
    const utility::string_t component_properties(U("properties"));
    const utility::string_t component_metadata(U("metadata"));
    const utility::string_t component_snapshot(U("snapshot"));
    const utility::string_t component_snapshots(U("snapshots"));
    const utility::string_t component_uncommitted_blobs(U("uncommittedblobs"));
    const utility::string_t component_lease(U("lease"));
    const utility::string_t component_block(U("block"));
    const utility::string_t component_block_list(U("blocklist"));
    const utility::string_t component_page_list(U("pagelist"));
    const utility::string_t component_page(U("page"));
    const utility::string_t component_copy(U("copy"));
    const utility::string_t component_acl(U("acl"));

    // common resources
    const utility::string_t root_container(U("$root"));
    const utility::string_t directory_delimiter(U("/"));

    // headers
    const utility::string_t http_version(U("HTTP/1.1"));
    const utility::string_t header_content_disposition(U("Content-Disposition"));
    const utility::string_t header_max_data_service_version(U("MaxDataServiceVersion"));
    const utility::string_t header_prefer(U("Prefer"));
    //const utility::string_t header_http_method(U("X-HTTP-Method"));
    const utility::string_t header_content_transfer_encoding(U("Content-Transfer-Encoding"));
    const utility::string_t header_content_id(U("Content-ID"));
    const utility::string_t ms_header_prefix(U("x-ms-"));
    const utility::string_t ms_header_date(U("x-ms-date"));
    const utility::string_t ms_header_version(U("x-ms-version"));
    const utility::string_t ms_header_blob_public_access(U("x-ms-blob-public-access"));
    const utility::string_t ms_header_blob_type(U("x-ms-blob-type"));
    const utility::string_t ms_header_blob_cache_control(U("x-ms-blob-cache-control"));
    const utility::string_t ms_header_blob_content_disposition(U("x-ms-blob-content-disposition"));
    const utility::string_t ms_header_blob_content_encoding(U("x-ms-blob-content-encoding"));
    const utility::string_t ms_header_blob_content_language(U("x-ms-blob-content-language"));
    const utility::string_t ms_header_blob_content_length(U("x-ms-blob-content-length"));
    const utility::string_t ms_header_blob_content_md5(U("x-ms-blob-content-md5"));
    const utility::string_t ms_header_blob_content_type(U("x-ms-blob-content-type"));
    const utility::string_t ms_header_blob_sequence_number(U("x-ms-blob-sequence-number"));
    const utility::string_t ms_header_sequence_number_action(U("x-ms-sequence-number-action"));
    const utility::string_t ms_header_copy_id(U("x-ms-copy-id"));
    const utility::string_t ms_header_copy_completion_time(U("x-ms-copy-completion-time"));
    const utility::string_t ms_header_copy_action(U("x-ms-copy-action"));
    const utility::string_t ms_header_copy_status(U("x-ms-copy-status"));
    const utility::string_t ms_header_copy_progress(U("x-ms-copy-progress"));
    const utility::string_t ms_header_copy_status_description(U("x-ms-copy-status-description"));
    const utility::string_t ms_header_copy_source(U("x-ms-copy-source"));
    const utility::string_t ms_header_delete_snapshots(U("x-ms-delete-snapshots"));
    const utility::string_t ms_header_request_id(U("x-ms-request-id"));
    const utility::string_t ms_header_client_request_id(U("x-ms-client-request-id"));
    const utility::string_t ms_header_range(U("x-ms-range"));
    const utility::string_t ms_header_page_write(U("x-ms-page-write"));
    const utility::string_t ms_header_range_get_content_md5(U("x-ms-range-get-content-md5"));
    const utility::string_t ms_header_lease_id(U("x-ms-lease-id"));
    const utility::string_t ms_header_lease_action(U("x-ms-lease-action"));
    const utility::string_t ms_header_lease_state(U("x-ms-lease-state"));
    const utility::string_t ms_header_lease_status(U("x-ms-lease-status"));
    const utility::string_t ms_header_lease_duration(U("x-ms-lease-duration"));
    const utility::string_t ms_header_lease_time(U("x-ms-lease-time"));
    const utility::string_t ms_header_lease_break_period(U("x-ms-lease-break-period"));
    const utility::string_t ms_header_lease_proposed_id(U("x-ms-proposed-lease-id"));
    const utility::string_t ms_header_metadata_prefix(U("x-ms-meta-"));
    const utility::string_t ms_header_snapshot(U("x-ms-snapshot"));
    const utility::string_t ms_header_if_sequence_number_le(U("x-ms-if-sequence-number-le"));
    const utility::string_t ms_header_if_sequence_number_lt(U("x-ms-if-sequence-number-lt"));
    const utility::string_t ms_header_if_sequence_number_eq(U("x-ms-if-sequence-number-eq"));
    const utility::string_t ms_header_source_if_match(U("x-ms-source-if-match"));
    const utility::string_t ms_header_source_if_none_match(U("x-ms-source-if-none-match"));
    const utility::string_t ms_header_source_if_modified_since(U("x-ms-source-if-modified-since"));
    const utility::string_t ms_header_source_if_unmodified_since(U("x-ms-source-if-unmodified-since"));
    const utility::string_t ms_header_continuation_next_partition_key(U("x-ms-continuation-NextPartitionKey"));
    const utility::string_t ms_header_continuation_next_row_key(U("x-ms-continuation-NextRowKey"));
    const utility::string_t ms_header_continuation_next_table_name(U("x-ms-continuation-NextTableName"));
    const utility::string_t ms_header_approximate_messages_count(U("x-ms-approximate-messages-count"));
    const utility::string_t ms_header_pop_receipt(U("x-ms-popreceipt"));
    const utility::string_t ms_header_time_next_visible(U("x-ms-time-next-visible"));

    // header values
    const utility::string_t header_value_storage_version(U("2013-08-15"));
    const utility::string_t header_value_true(U("true"));
    const utility::string_t header_value_false(U("false"));
    const utility::string_t header_value_locked(U("locked"));
    const utility::string_t header_value_unlocked(U("unlocked"));
    const utility::string_t header_value_copy_abort(U("abort"));
    const utility::string_t header_value_copy_pending(U("pending"));
    const utility::string_t header_value_copy_success(U("success"));
    const utility::string_t header_value_copy_aborted(U("aborted"));
    const utility::string_t header_value_copy_failed(U("failed"));
    const utility::string_t header_value_lease_available(U("available"));
    const utility::string_t header_value_lease_leased(U("leased"));
    const utility::string_t header_value_lease_expired(U("expired"));
    const utility::string_t header_value_lease_breaking(U("breaking"));
    const utility::string_t header_value_lease_broken(U("broken"));
    const utility::string_t header_value_lease_infinite(U("infinite"));
    const utility::string_t header_value_lease_fixed(U("fixed"));
    const utility::string_t header_value_lease_acquire(U("acquire"));
    const utility::string_t header_value_lease_renew(U("renew"));
    const utility::string_t header_value_lease_release(U("release"));
    const utility::string_t header_value_lease_break(U("break"));
    const utility::string_t header_value_lease_change(U("change"));
    const utility::string_t header_value_range_prefix(U("bytes="));
    const utility::string_t header_value_page_write_update(U("Update"));
    const utility::string_t header_value_page_write_clear(U("Clear"));
    const utility::string_t header_value_blob_type_block(U("BlockBlob"));
    const utility::string_t header_value_blob_type_page(U("PageBlob"));
    const utility::string_t header_value_snapshots_include(U("include"));
    const utility::string_t header_value_snapshots_only(U("only"));
    const utility::string_t header_value_sequence_max(U("max"));
    const utility::string_t header_value_sequence_update(U("update"));
    const utility::string_t header_value_sequence_increment(U("increment"));
    const utility::string_t header_value_accept_application_json_minimal_metadata(U("application/json;odata=minimalmetadata"));
    const utility::string_t header_value_accept_application_json_full_metadata(U("application/json;odata=fullmetadata"));
    const utility::string_t header_value_accept_application_json_no_metadata(U("application/json;odata=nometadata"));
    const utility::string_t header_value_charset_utf8(U("UTF-8"));
    const utility::string_t header_value_data_service_version(U("3.0;Native"));
    const utility::string_t header_value_content_type_json(U("application/json"));
    const utility::string_t header_value_content_type_utf8(U("text/plain; charset=utf-8"));
    const utility::string_t header_value_content_type_mime_multipart_prefix(U("multipart/mixed; boundary="));
    const utility::string_t header_value_content_type_http(U("application/http"));
    const utility::string_t header_value_content_transfer_encoding_binary(U("binary"));

    // xml strings
    const utility::string_t xml_last_modified(U("Last-Modified"));
    const utility::string_t xml_etag(U("Etag"));
    const utility::string_t xml_lease_status(U("LeaseStatus"));
    const utility::string_t xml_lease_state(U("LeaseState"));
    const utility::string_t xml_lease_duration(U("LeaseDuration"));
    const utility::string_t xml_content_length(U("Content-Length"));
    const utility::string_t xml_content_disposition(U("Content-Disposition"));
    const utility::string_t xml_content_type(U("Content-Type"));
    const utility::string_t xml_content_encoding(U("Content-Encoding"));
    const utility::string_t xml_content_language(U("Content-Language"));
    const utility::string_t xml_content_md5(U("Content-MD5"));
    const utility::string_t xml_cache_control(U("Cache-Control"));
    const utility::string_t xml_blob_sequence_number(U("x-ms-blob-sequence-number"));
    const utility::string_t xml_blob_type(U("BlobType"));
    const utility::string_t xml_copy_id(U("CopyId"));
    const utility::string_t xml_copy_status(U("CopyStatus"));
    const utility::string_t xml_copy_source(U("CopySource"));
    const utility::string_t xml_copy_progress(U("CopyProgress"));
    const utility::string_t xml_copy_completion_time(U("CopyCompletionTime"));
    const utility::string_t xml_copy_status_description(U("CopyStatusDescription"));
    const utility::string_t xml_next_marker(U("NextMarker"));
    const utility::string_t xml_containers(U("Containers"));
    const utility::string_t xml_container(U("Container"));
    const utility::string_t xml_blobs(U("Blobs"));
    const utility::string_t xml_blob(U("Blob"));
    const utility::string_t xml_blob_prefix(U("BlobPrefix"));
    const utility::string_t xml_properties(U("Properties"));
    const utility::string_t xml_metadata(U("Metadata"));
    const utility::string_t xml_snapshot(U("Snapshot"));
    const utility::string_t xml_enumeration_results(U("EnumerationResults"));
    const utility::string_t xml_service_endpoint(U("ServiceEndpoint"));
    const utility::string_t xml_container_name(U("ContainerName"));
    const utility::string_t xml_page_range(U("PageRange"));
    const utility::string_t xml_start(U("Start"));
    const utility::string_t xml_end(U("End"));
    const utility::string_t xml_committed_blocks(U("CommittedBlocks"));
    const utility::string_t xml_uncommitted_blocks(U("UncommittedBlocks"));
    const utility::string_t xml_committed(U("Committed"));
    const utility::string_t xml_uncommitted(U("Uncommitted"));
    const utility::string_t xml_block(U("Block"));
    const utility::string_t xml_name(U("Name"));
    const utility::string_t xml_size(U("Size"));
    const utility::string_t xml_code(U("Code"));
    const utility::string_t xml_message(U("Message"));
    const utility::string_t xml_block_list(U("BlockList"));
    const utility::string_t xml_latest(U("Latest"));
    const utility::string_t xml_signed_identifiers(U("SignedIdentifiers"));
    const utility::string_t xml_signed_identifier(U("SignedIdentifier"));
    const utility::string_t xml_signed_id(U("Id"));
    const utility::string_t xml_access_policy(U("AccessPolicy"));
    const utility::string_t xml_access_policy_start(U("Start"));
    const utility::string_t xml_access_policy_expiry(U("Expiry"));
    const utility::string_t xml_access_policy_permissions(U("Permission"));
    const utility::string_t xml_service_properties(U("StorageServiceProperties"));
    const utility::string_t xml_service_properties_version(U("Version"));
    const utility::string_t xml_service_properties_enabled(U("Enabled"));
    const utility::string_t xml_service_properties_logging(U("Logging"));
    const utility::string_t xml_service_properties_delete(U("Delete"));
    const utility::string_t xml_service_properties_read(U("Read"));
    const utility::string_t xml_service_properties_write(U("Write"));
    const utility::string_t xml_service_properties_retention(U("RetentionPolicy"));
    const utility::string_t xml_service_properties_retention_days(U("Days"));
    const utility::string_t xml_service_properties_hour_metrics(U("HourMetrics"));
    const utility::string_t xml_service_properties_minute_metrics(U("MinuteMetrics"));
    const utility::string_t xml_service_properties_include_apis(U("IncludeAPIs"));
    const utility::string_t xml_service_properties_cors(U("Cors"));
    const utility::string_t xml_service_properties_cors_rule(U("CorsRule"));
    const utility::string_t xml_service_properties_allowed_origins(U("AllowedOrigins"));
    const utility::string_t xml_service_properties_allowed_methods(U("AllowedMethods"));
    const utility::string_t xml_service_properties_max_age(U("MaxAgeInSeconds"));
    const utility::string_t xml_service_properties_exposed_headers(U("ExposedHeaders"));
    const utility::string_t xml_service_properties_allowed_headers(U("AllowedHeaders"));
    const utility::string_t xml_service_properties_default_service_version(U("DefaultServiceVersion"));
    const utility::string_t xml_url(U("Url"));

    // error codes
    const utility::string_t error_code_container_already_exists(U("ContainerAlreadyExists"));
    const utility::string_t error_code_container_not_found(U("ContainerNotFound"));
    const utility::string_t error_code_blob_not_found(U("BlobNotFound"));

    // user agent
#if defined(WIN32)
    const utility::string_t header_value_user_agent(U("WA-Storage/0.1.0 (Native; Windows)"));
#else
    const utility::string_t header_value_user_agent(U("WA-Storage/0.1.0 (Native)"));
#endif

}}} // namespace wa::storage::protocol
