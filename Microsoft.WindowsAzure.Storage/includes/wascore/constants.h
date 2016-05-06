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

namespace azure { namespace storage { namespace protocol {

    // size constants
    const size_t max_block_size = 4 * 1024 * 1024;
    const size_t default_buffer_size = 64 * 1024;
    const utility::size64_t default_single_blob_upload_threshold = 32 * 1024 * 1024;

    // double special values
    const utility::string_t double_not_a_number(_XPLATSTR("NaN"));
    const utility::string_t double_infinity(_XPLATSTR("Infinity"));
    const utility::string_t double_negative_infinity(_XPLATSTR("-Infinity"));

    // duration constants
    const std::chrono::seconds default_retry_interval(3);
    // The following value must be less than 2147482, which is the highest
    // that Casablanca 2.2.0 on Linux can accept, which is derived from
    // the maximum value for a signed long on g++, divided by 1000.
    // Choosing to set it to 24 days to align with .NET.
    const std::chrono::seconds default_maximum_execution_time(24 * 24 * 60 * 60);
    // For the following value, "0" means "don't send a timeout to the service"
    const std::chrono::seconds default_server_timeout(0);

    // lease break period and duration constants
    const std::chrono::seconds minimum_lease_break_period(0);
    const std::chrono::seconds maximum_lease_break_period(60);
    const std::chrono::seconds minimum_fixed_lease_duration(15);
    const std::chrono::seconds maximum_fixed_lease_duration(60);

    // service names
    const utility::string_t service_blob(_XPLATSTR("blob"));
    const utility::string_t service_table(_XPLATSTR("table"));
    const utility::string_t service_queue(_XPLATSTR("queue"));

    // uri query parameters
    const utility::string_t uri_query_timeout(_XPLATSTR("timeout"));
    const utility::string_t uri_query_resource_type(_XPLATSTR("restype"));
    const utility::string_t uri_query_snapshot(_XPLATSTR("snapshot"));
    const utility::string_t uri_query_component(_XPLATSTR("comp"));
    const utility::string_t uri_query_block_id(_XPLATSTR("blockid"));
    const utility::string_t uri_query_block_list_type(_XPLATSTR("blocklisttype"));
    const utility::string_t uri_query_prefix(_XPLATSTR("prefix"));
    const utility::string_t uri_query_delimiter(_XPLATSTR("delimiter"));
    const utility::string_t uri_query_marker(_XPLATSTR("marker"));
    const utility::string_t uri_query_max_results(_XPLATSTR("maxresults"));
    const utility::string_t uri_query_include(_XPLATSTR("include"));
    const utility::string_t uri_query_copy_id(_XPLATSTR("copyid"));

    // SAS query parameters
    const utility::string_t uri_query_sas_start(_XPLATSTR("st"));
    const utility::string_t uri_query_sas_expiry(_XPLATSTR("se"));
    const utility::string_t uri_query_sas_resource(_XPLATSTR("sr"));
    const utility::string_t uri_query_sas_table_name(_XPLATSTR("tn"));
    const utility::string_t uri_query_sas_permissions(_XPLATSTR("sp"));
    const utility::string_t uri_query_sas_start_partition_key(_XPLATSTR("spk"));
    const utility::string_t uri_query_sas_start_row_key(_XPLATSTR("srk"));
    const utility::string_t uri_query_sas_end_partition_key(_XPLATSTR("epk"));
    const utility::string_t uri_query_sas_end_row_key(_XPLATSTR("erk"));
    const utility::string_t uri_query_sas_identifier(_XPLATSTR("si"));
    const utility::string_t uri_query_sas_version(_XPLATSTR("sv"));
    const utility::string_t uri_query_sas_signature(_XPLATSTR("sig"));
    const utility::string_t uri_query_sas_cache_control(_XPLATSTR("rscc"));
    const utility::string_t uri_query_sas_content_type(_XPLATSTR("rsct"));
    const utility::string_t uri_query_sas_content_encoding(_XPLATSTR("rsce"));
    const utility::string_t uri_query_sas_content_language(_XPLATSTR("rscl"));
    const utility::string_t uri_query_sas_content_disposition(_XPLATSTR("rscd"));
    const utility::string_t uri_query_sas_api_version(_XPLATSTR("api-version"));
    const utility::string_t uri_query_sas_services(_XPLATSTR("ss"));
    const utility::string_t uri_query_sas_resource_types(_XPLATSTR("srt"));
    const utility::string_t uri_query_sas_ip(_XPLATSTR("sip"));
    const utility::string_t uri_query_sas_protocol(_XPLATSTR("spr"));

    // table query parameters
    const utility::string_t table_query_next_partition_key(_XPLATSTR("NextPartitionKey"));
    const utility::string_t table_query_next_row_key(_XPLATSTR("NextRowKey"));
    const utility::string_t table_query_next_table_name(_XPLATSTR("NextTableName"));

    // queue query parameters
    const utility::string_t queue_query_next_marker(_XPLATSTR("marker"));

    // resource types
    const utility::string_t resource_service(_XPLATSTR("service"));
    const utility::string_t resource_container(_XPLATSTR("container"));
    const utility::string_t resource_blob(_XPLATSTR("blob"));
    const utility::string_t resource_block_list_all(_XPLATSTR("all"));
    const utility::string_t resource_block_list_committed(_XPLATSTR("committed"));
    const utility::string_t resource_block_list_uncommitted(_XPLATSTR("uncommitted"));

    // components
    const utility::string_t component_list(_XPLATSTR("list"));
    const utility::string_t component_stats(_XPLATSTR("stats"));
    const utility::string_t component_properties(_XPLATSTR("properties"));
    const utility::string_t component_metadata(_XPLATSTR("metadata"));
    const utility::string_t component_snapshot(_XPLATSTR("snapshot"));
    const utility::string_t component_snapshots(_XPLATSTR("snapshots"));
    const utility::string_t component_uncommitted_blobs(_XPLATSTR("uncommittedblobs"));
    const utility::string_t component_lease(_XPLATSTR("lease"));
    const utility::string_t component_block(_XPLATSTR("block"));
    const utility::string_t component_block_list(_XPLATSTR("blocklist"));
    const utility::string_t component_page_list(_XPLATSTR("pagelist"));
    const utility::string_t component_page(_XPLATSTR("page"));
    const utility::string_t component_append_block(_XPLATSTR("appendblock"));
    const utility::string_t component_copy(_XPLATSTR("copy"));
    const utility::string_t component_acl(_XPLATSTR("acl"));

    // common resources
    const utility::string_t root_container(_XPLATSTR("$root"));
    const utility::string_t directory_delimiter(_XPLATSTR("/"));

    // headers
    const utility::string_t http_version(_XPLATSTR("HTTP/1.1"));
    const utility::string_t header_content_disposition(_XPLATSTR("Content-Disposition"));
    const utility::string_t header_max_data_service_version(_XPLATSTR("MaxDataServiceVersion"));
    const utility::string_t header_prefer(_XPLATSTR("Prefer"));
    const utility::string_t header_content_transfer_encoding(_XPLATSTR("Content-Transfer-Encoding"));
    const utility::string_t header_content_id(_XPLATSTR("Content-ID"));
    const utility::string_t ms_header_prefix(_XPLATSTR("x-ms-"));
    const utility::string_t ms_header_date(_XPLATSTR("x-ms-date"));
    const utility::string_t ms_header_version(_XPLATSTR("x-ms-version"));
    const utility::string_t ms_header_blob_public_access(_XPLATSTR("x-ms-blob-public-access"));
    const utility::string_t ms_header_blob_type(_XPLATSTR("x-ms-blob-type"));
    const utility::string_t ms_header_blob_cache_control(_XPLATSTR("x-ms-blob-cache-control"));
    const utility::string_t ms_header_blob_content_disposition(_XPLATSTR("x-ms-blob-content-disposition"));
    const utility::string_t ms_header_blob_content_encoding(_XPLATSTR("x-ms-blob-content-encoding"));
    const utility::string_t ms_header_blob_content_language(_XPLATSTR("x-ms-blob-content-language"));
    const utility::string_t ms_header_blob_content_length(_XPLATSTR("x-ms-blob-content-length"));
    const utility::string_t ms_header_blob_content_md5(_XPLATSTR("x-ms-blob-content-md5"));
    const utility::string_t ms_header_blob_content_type(_XPLATSTR("x-ms-blob-content-type"));
    const utility::string_t ms_header_blob_sequence_number(_XPLATSTR("x-ms-blob-sequence-number"));
    const utility::string_t ms_header_sequence_number_action(_XPLATSTR("x-ms-sequence-number-action"));
    const utility::string_t ms_header_blob_condition_maxsize(_XPLATSTR("x-ms-blob-condition-maxsize"));
    const utility::string_t ms_header_blob_condition_appendpos(_XPLATSTR("x-ms-blob-condition-appendpos"));
    const utility::string_t ms_header_blob_append_offset(_XPLATSTR("x-ms-blob-append-offset"));
    const utility::string_t ms_header_blob_committed_block_count(_XPLATSTR("x-ms-blob-committed-block-count"));
    const utility::string_t ms_header_copy_id(_XPLATSTR("x-ms-copy-id"));
    const utility::string_t ms_header_copy_completion_time(_XPLATSTR("x-ms-copy-completion-time"));
    const utility::string_t ms_header_copy_action(_XPLATSTR("x-ms-copy-action"));
    const utility::string_t ms_header_copy_status(_XPLATSTR("x-ms-copy-status"));
    const utility::string_t ms_header_copy_progress(_XPLATSTR("x-ms-copy-progress"));
    const utility::string_t ms_header_copy_status_description(_XPLATSTR("x-ms-copy-status-description"));
    const utility::string_t ms_header_copy_source(_XPLATSTR("x-ms-copy-source"));
    const utility::string_t ms_header_delete_snapshots(_XPLATSTR("x-ms-delete-snapshots"));
    const utility::string_t ms_header_request_id(_XPLATSTR("x-ms-request-id"));
    const utility::string_t ms_header_client_request_id(_XPLATSTR("x-ms-client-request-id"));
    const utility::string_t ms_header_range(_XPLATSTR("x-ms-range"));
    const utility::string_t ms_header_page_write(_XPLATSTR("x-ms-page-write"));
    const utility::string_t ms_header_range_get_content_md5(_XPLATSTR("x-ms-range-get-content-md5"));
    const utility::string_t ms_header_lease_id(_XPLATSTR("x-ms-lease-id"));
    const utility::string_t ms_header_lease_action(_XPLATSTR("x-ms-lease-action"));
    const utility::string_t ms_header_lease_state(_XPLATSTR("x-ms-lease-state"));
    const utility::string_t ms_header_lease_status(_XPLATSTR("x-ms-lease-status"));
    const utility::string_t ms_header_lease_duration(_XPLATSTR("x-ms-lease-duration"));
    const utility::string_t ms_header_lease_time(_XPLATSTR("x-ms-lease-time"));
    const utility::string_t ms_header_lease_break_period(_XPLATSTR("x-ms-lease-break-period"));
    const utility::string_t ms_header_lease_proposed_id(_XPLATSTR("x-ms-proposed-lease-id"));
    const utility::string_t ms_header_metadata_prefix(_XPLATSTR("x-ms-meta-"));
    const utility::string_t ms_header_snapshot(_XPLATSTR("x-ms-snapshot"));
    const utility::string_t ms_header_if_sequence_number_le(_XPLATSTR("x-ms-if-sequence-number-le"));
    const utility::string_t ms_header_if_sequence_number_lt(_XPLATSTR("x-ms-if-sequence-number-lt"));
    const utility::string_t ms_header_if_sequence_number_eq(_XPLATSTR("x-ms-if-sequence-number-eq"));
    const utility::string_t ms_header_source_if_match(_XPLATSTR("x-ms-source-if-match"));
    const utility::string_t ms_header_source_if_none_match(_XPLATSTR("x-ms-source-if-none-match"));
    const utility::string_t ms_header_source_if_modified_since(_XPLATSTR("x-ms-source-if-modified-since"));
    const utility::string_t ms_header_source_if_unmodified_since(_XPLATSTR("x-ms-source-if-unmodified-since"));
    const utility::string_t ms_header_continuation_next_partition_key(_XPLATSTR("x-ms-continuation-NextPartitionKey"));
    const utility::string_t ms_header_continuation_next_row_key(_XPLATSTR("x-ms-continuation-NextRowKey"));
    const utility::string_t ms_header_continuation_next_table_name(_XPLATSTR("x-ms-continuation-NextTableName"));
    const utility::string_t ms_header_approximate_messages_count(_XPLATSTR("x-ms-approximate-messages-count"));
    const utility::string_t ms_header_pop_receipt(_XPLATSTR("x-ms-popreceipt"));
    const utility::string_t ms_header_time_next_visible(_XPLATSTR("x-ms-time-next-visible"));

    // header values
    const utility::string_t header_value_storage_version(_XPLATSTR("2015-04-05"));
    const utility::string_t header_value_true(_XPLATSTR("true"));
    const utility::string_t header_value_false(_XPLATSTR("false"));
    const utility::string_t header_value_locked(_XPLATSTR("locked"));
    const utility::string_t header_value_unlocked(_XPLATSTR("unlocked"));
    const utility::string_t header_value_copy_abort(_XPLATSTR("abort"));
    const utility::string_t header_value_copy_pending(_XPLATSTR("pending"));
    const utility::string_t header_value_copy_success(_XPLATSTR("success"));
    const utility::string_t header_value_copy_aborted(_XPLATSTR("aborted"));
    const utility::string_t header_value_copy_failed(_XPLATSTR("failed"));
    const utility::string_t header_value_lease_available(_XPLATSTR("available"));
    const utility::string_t header_value_lease_leased(_XPLATSTR("leased"));
    const utility::string_t header_value_lease_expired(_XPLATSTR("expired"));
    const utility::string_t header_value_lease_breaking(_XPLATSTR("breaking"));
    const utility::string_t header_value_lease_broken(_XPLATSTR("broken"));
    const utility::string_t header_value_lease_infinite(_XPLATSTR("infinite"));
    const utility::string_t header_value_lease_fixed(_XPLATSTR("fixed"));
    const utility::string_t header_value_lease_acquire(_XPLATSTR("acquire"));
    const utility::string_t header_value_lease_renew(_XPLATSTR("renew"));
    const utility::string_t header_value_lease_release(_XPLATSTR("release"));
    const utility::string_t header_value_lease_break(_XPLATSTR("break"));
    const utility::string_t header_value_lease_change(_XPLATSTR("change"));
    const utility::string_t header_value_range_prefix(_XPLATSTR("bytes="));
    const utility::string_t header_value_page_write_update(_XPLATSTR("Update"));
    const utility::string_t header_value_page_write_clear(_XPLATSTR("Clear"));
    const utility::string_t header_value_blob_type_block(_XPLATSTR("BlockBlob"));
    const utility::string_t header_value_blob_type_page(_XPLATSTR("PageBlob"));
    const utility::string_t header_value_blob_type_append(_XPLATSTR("AppendBlob"));
    const utility::string_t header_value_snapshots_include(_XPLATSTR("include"));
    const utility::string_t header_value_snapshots_only(_XPLATSTR("only"));
    const utility::string_t header_value_sequence_max(_XPLATSTR("max"));
    const utility::string_t header_value_sequence_update(_XPLATSTR("update"));
    const utility::string_t header_value_sequence_increment(_XPLATSTR("increment"));
    const utility::string_t header_value_accept_application_json_minimal_metadata(_XPLATSTR("application/json;odata=minimalmetadata"));
    const utility::string_t header_value_accept_application_json_full_metadata(_XPLATSTR("application/json;odata=fullmetadata"));
    const utility::string_t header_value_accept_application_json_no_metadata(_XPLATSTR("application/json;odata=nometadata"));
    const utility::string_t header_value_charset_utf8(_XPLATSTR("UTF-8"));
    const utility::string_t header_value_data_service_version(_XPLATSTR("3.0;Native"));
    const utility::string_t header_value_content_type_json(_XPLATSTR("application/json"));
    const utility::string_t header_value_content_type_utf8(_XPLATSTR("text/plain; charset=utf-8"));
    const utility::string_t header_value_content_type_mime_multipart_prefix(_XPLATSTR("multipart/mixed; boundary="));
    const utility::string_t header_value_content_type_http(_XPLATSTR("application/http"));
    const utility::string_t header_value_content_transfer_encoding_binary(_XPLATSTR("binary"));

    // xml strings
    const utility::string_t xml_last_modified(_XPLATSTR("Last-Modified"));
    const utility::string_t xml_etag(_XPLATSTR("Etag"));
    const utility::string_t xml_lease_status(_XPLATSTR("LeaseStatus"));
    const utility::string_t xml_lease_state(_XPLATSTR("LeaseState"));
    const utility::string_t xml_lease_duration(_XPLATSTR("LeaseDuration"));
    const utility::string_t xml_content_length(_XPLATSTR("Content-Length"));
    const utility::string_t xml_content_disposition(_XPLATSTR("Content-Disposition"));
    const utility::string_t xml_content_type(_XPLATSTR("Content-Type"));
    const utility::string_t xml_content_encoding(_XPLATSTR("Content-Encoding"));
    const utility::string_t xml_content_language(_XPLATSTR("Content-Language"));
    const utility::string_t xml_content_md5(_XPLATSTR("Content-MD5"));
    const utility::string_t xml_cache_control(_XPLATSTR("Cache-Control"));
    const utility::string_t xml_blob_sequence_number(_XPLATSTR("x-ms-blob-sequence-number"));
    const utility::string_t xml_blob_type(_XPLATSTR("BlobType"));
    const utility::string_t xml_copy_id(_XPLATSTR("CopyId"));
    const utility::string_t xml_copy_status(_XPLATSTR("CopyStatus"));
    const utility::string_t xml_copy_source(_XPLATSTR("CopySource"));
    const utility::string_t xml_copy_progress(_XPLATSTR("CopyProgress"));
    const utility::string_t xml_copy_completion_time(_XPLATSTR("CopyCompletionTime"));
    const utility::string_t xml_copy_status_description(_XPLATSTR("CopyStatusDescription"));
    const utility::string_t xml_next_marker(_XPLATSTR("NextMarker"));
    const utility::string_t xml_containers(_XPLATSTR("Containers"));
    const utility::string_t xml_container(_XPLATSTR("Container"));
    const utility::string_t xml_blobs(_XPLATSTR("Blobs"));
    const utility::string_t xml_blob(_XPLATSTR("Blob"));
    const utility::string_t xml_blob_prefix(_XPLATSTR("BlobPrefix"));
    const utility::string_t xml_properties(_XPLATSTR("Properties"));
    const utility::string_t xml_metadata(_XPLATSTR("Metadata"));
    const utility::string_t xml_snapshot(_XPLATSTR("Snapshot"));
    const utility::string_t xml_enumeration_results(_XPLATSTR("EnumerationResults"));
    const utility::string_t xml_service_endpoint(_XPLATSTR("ServiceEndpoint"));
    const utility::string_t xml_container_name(_XPLATSTR("ContainerName"));
    const utility::string_t xml_page_range(_XPLATSTR("PageRange"));
    const utility::string_t xml_start(_XPLATSTR("Start"));
    const utility::string_t xml_end(_XPLATSTR("End"));
    const utility::string_t xml_committed_blocks(_XPLATSTR("CommittedBlocks"));
    const utility::string_t xml_uncommitted_blocks(_XPLATSTR("UncommittedBlocks"));
    const utility::string_t xml_committed(_XPLATSTR("Committed"));
    const utility::string_t xml_uncommitted(_XPLATSTR("Uncommitted"));
    const utility::string_t xml_block(_XPLATSTR("Block"));
    const utility::string_t xml_name(_XPLATSTR("Name"));
    const utility::string_t xml_size(_XPLATSTR("Size"));
    const utility::string_t xml_error_root(_XPLATSTR("Error"));
    const utility::string_t xml_code(_XPLATSTR("Code"));
    const utility::string_t xml_code_table(_XPLATSTR("code"));
    const utility::string_t xml_message(_XPLATSTR("Message"));
    const utility::string_t xml_message_table(_XPLATSTR("message"));
    const utility::string_t xml_innererror_table(_XPLATSTR("innererror"));
    const utility::string_t xml_block_list(_XPLATSTR("BlockList"));
    const utility::string_t xml_latest(_XPLATSTR("Latest"));
    const utility::string_t xml_signed_identifiers(_XPLATSTR("SignedIdentifiers"));
    const utility::string_t xml_signed_identifier(_XPLATSTR("SignedIdentifier"));
    const utility::string_t xml_signed_id(_XPLATSTR("Id"));
    const utility::string_t xml_access_policy(_XPLATSTR("AccessPolicy"));
    const utility::string_t xml_access_policy_start(_XPLATSTR("Start"));
    const utility::string_t xml_access_policy_expiry(_XPLATSTR("Expiry"));
    const utility::string_t xml_access_policy_permissions(_XPLATSTR("Permission"));
    const utility::string_t xml_service_properties(_XPLATSTR("StorageServiceProperties"));
    const utility::string_t xml_service_properties_version(_XPLATSTR("Version"));
    const utility::string_t xml_service_properties_enabled(_XPLATSTR("Enabled"));
    const utility::string_t xml_service_properties_logging(_XPLATSTR("Logging"));
    const utility::string_t xml_service_properties_delete(_XPLATSTR("Delete"));
    const utility::string_t xml_service_properties_read(_XPLATSTR("Read"));
    const utility::string_t xml_service_properties_write(_XPLATSTR("Write"));
    const utility::string_t xml_service_properties_retention(_XPLATSTR("RetentionPolicy"));
    const utility::string_t xml_service_properties_retention_days(_XPLATSTR("Days"));
    const utility::string_t xml_service_properties_hour_metrics(_XPLATSTR("HourMetrics"));
    const utility::string_t xml_service_properties_minute_metrics(_XPLATSTR("MinuteMetrics"));
    const utility::string_t xml_service_properties_include_apis(_XPLATSTR("IncludeAPIs"));
    const utility::string_t xml_service_properties_cors(_XPLATSTR("Cors"));
    const utility::string_t xml_service_properties_cors_rule(_XPLATSTR("CorsRule"));
    const utility::string_t xml_service_properties_allowed_origins(_XPLATSTR("AllowedOrigins"));
    const utility::string_t xml_service_properties_allowed_methods(_XPLATSTR("AllowedMethods"));
    const utility::string_t xml_service_properties_max_age(_XPLATSTR("MaxAgeInSeconds"));
    const utility::string_t xml_service_properties_exposed_headers(_XPLATSTR("ExposedHeaders"));
    const utility::string_t xml_service_properties_allowed_headers(_XPLATSTR("AllowedHeaders"));
    const utility::string_t xml_service_properties_default_service_version(_XPLATSTR("DefaultServiceVersion"));
    const utility::string_t xml_service_stats_geo_replication(_XPLATSTR("GeoReplication"));
    const utility::string_t xml_service_stats_geo_replication_status(_XPLATSTR("Status"));
    const utility::string_t xml_service_stats_geo_replication_status_live(_XPLATSTR("live"));
    const utility::string_t xml_service_stats_geo_replication_status_bootstrap(_XPLATSTR("bootstrap"));
    const utility::string_t xml_service_stats_geo_replication_last_sync_time(_XPLATSTR("LastSyncTime"));
    const utility::string_t xml_url(_XPLATSTR("Url"));

    // user agent
#if defined(WIN32)
#if defined(_MSC_VER)
    const utility::string_t header_value_user_agent(_XPLATSTR("Azure-Storage/2.3.0 (Native; Windows; MSC_VER ") + utility::conversions::to_string_t(std::to_string(_MSC_VER)) + _XPLATSTR(")"));
#else
    const utility::string_t header_value_user_agent(_XPLATSTR("Azure-Storage/2.3.0 (Native; Windows)"));
#endif
#else
    const utility::string_t header_value_user_agent(_XPLATSTR("Azure-Storage/2.3.0 (Native)"));
#endif

}}} // namespace azure::storage::protocol
