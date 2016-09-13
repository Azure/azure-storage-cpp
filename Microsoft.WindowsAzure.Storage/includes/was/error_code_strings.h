// -----------------------------------------------------------------------------------------
// <copyright file="error_code_strings.h" company="Microsoft">
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

namespace azure { namespace storage { namespace protocol {

#pragma region storage_error_code_strings

    // This section provides error code strings that are common to all storage services.

    /// <summary>
    /// The specified HTTP verb is not supported.
    /// </summary>
    const utility::string_t error_code_unsupported_http_verb(_XPLATSTR("UnsupportedHttpVerb"));

    /// <summary>
    /// The Content-Length header is required for this request.
    /// </summary>
    const utility::string_t error_code_missing_content_length_header(_XPLATSTR("MissingContentLengthHeader"));

    /// <summary>
    /// A required header was missing.
    /// </summary>
    const utility::string_t error_code_missing_required_header(_XPLATSTR("MissingRequiredHeader"));

    /// <summary>
    /// A required XML node was missing.
    /// </summary>
    const utility::string_t error_code_missing_required_xml_node(_XPLATSTR("MissingRequiredXmlNode"));

    /// <summary>
    /// One or more header values are not supported.
    /// </summary>
    const utility::string_t error_code_unsupported_header(_XPLATSTR("UnsupportedHeader"));

    /// <summary>
    /// One or more XML nodes are not supported.
    /// </summary>
    const utility::string_t error_code_unsupported_xml_node(_XPLATSTR("UnsupportedXmlNode"));

    /// <summary>
    /// One or more header values are invalid.
    /// </summary>
    const utility::string_t error_code_invalid_header_value(_XPLATSTR("InvalidHeaderValue"));

    /// <summary>
    /// One or more XML node values are invalid.
    /// </summary>
    const utility::string_t error_code_invalid_xml_node_value(_XPLATSTR("InvalidXmlNodeValue"));

    /// <summary>
    /// A required query parameter is missing.
    /// </summary>
    const utility::string_t error_code_missing_required_query_parameter(_XPLATSTR("MissingRequiredQueryParameter"));

    /// <summary>
    /// One or more query parameters is not supported.
    /// </summary>
    const utility::string_t error_code_unsupported_query_parameter(_XPLATSTR("UnsupportedQueryParameter"));

    /// <summary>
    /// One or more query parameters are invalid.
    /// </summary>
    const utility::string_t error_code_invalid_query_parameter_value(_XPLATSTR("InvalidQueryParameterValue"));

    /// <summary>
    /// One or more query parameters are out of range.
    /// </summary>
    const utility::string_t error_code_out_of_range_query_parameter_value(_XPLATSTR("OutOfRangeQueryParameterValue"));

    /// <summary>
    /// The URI is invalid.
    /// </summary>
    const utility::string_t error_code_invalid_uri(_XPLATSTR("InvalidUri"));

    /// <summary>
    /// The HTTP verb is invalid.
    /// </summary>
    const utility::string_t error_code_invalid_http_verb(_XPLATSTR("InvalidHttpVerb"));

    /// <summary>
    /// The metadata key is empty.
    /// </summary>
    const utility::string_t error_code_empty_metadata_key(_XPLATSTR("EmptyMetadataKey"));

    /// <summary>
    /// The request body is too large.
    /// </summary>
    const utility::string_t error_code_request_body_too_large(_XPLATSTR("RequestBodyTooLarge"));

    /// <summary>
    /// The specified XML document is invalid.
    /// </summary>
    const utility::string_t error_code_invalid_xml_document(_XPLATSTR("InvalidXmlDocument"));

    /// <summary>
    /// An internal error occurred.
    /// </summary>
    const utility::string_t error_code_internal_error(_XPLATSTR("InternalError"));

    /// <summary>
    /// Authentication failed.
    /// </summary>
    const utility::string_t error_code_authentication_failed(_XPLATSTR("AuthenticationFailed"));

    /// <summary>
    /// The specified MD5 hash does not match the server value.
    /// </summary>
    const utility::string_t error_code_md5_mismatch(_XPLATSTR("Md5Mismatch"));

    /// <summary>
    /// The specified MD5 hash is invalid.
    /// </summary>
    const utility::string_t error_code_invalid_md5(_XPLATSTR("InvalidMd5"));

    /// <summary>
    /// The input is out of range.
    /// </summary>
    const utility::string_t error_code_out_of_range_input(_XPLATSTR("OutOfRangeInput"));

    /// <summary>
    /// The input is invalid.
    /// </summary>
    const utility::string_t error_code_invalid_input(_XPLATSTR("InvalidInput"));

    /// <summary>
    /// The operation timed out.
    /// </summary>
    const utility::string_t error_code_operation_timed_out(_XPLATSTR("OperationTimedOut"));

    /// <summary>
    /// The specified resource was not found.
    /// </summary>
    const utility::string_t error_code_resource_not_found(_XPLATSTR("ResourceNotFound"));

    /// <summary>
    /// The specified metadata is invalid.
    /// </summary>
    const utility::string_t error_code_invalid_metadata(_XPLATSTR("InvalidMetadata"));

    /// <summary>
    /// The specified metadata is too large.
    /// </summary>
    const utility::string_t error_code_metadata_too_large(_XPLATSTR("MetadataTooLarge"));

    /// <summary>
    /// The specified condition was not met.
    /// </summary>
    const utility::string_t error_code_condition_not_met(_XPLATSTR("ConditionNotMet"));

    /// <summary>
    /// The specified range is invalid.
    /// </summary>
    const utility::string_t error_code_invalid_range(_XPLATSTR("InvalidRange"));

    /// <summary>
    /// The server is busy.
    /// </summary>
    const utility::string_t error_code_server_busy(_XPLATSTR("ServerBusy"));

    /// <summary>
    /// The url in the request could not be parsed.
    /// </summary>
    const utility::string_t error_code_request_url_failed_to_parse(_XPLATSTR("RequestUrlFailedToParse"));

    /// <summary>
    /// The authentication information was not provided in the correct format. Verify the value of Authorization header.
    /// </summary>
    const utility::string_t error_code_invalid_authentication_info(_XPLATSTR("InvalidAuthenticationInfo"));

    /// <summary>
    /// The specified resource name contains invalid characters.
    /// </summary>
    const utility::string_t error_code_invalid_resource_name(_XPLATSTR("InvalidResourceName"));

    /// <summary>
    /// Condition headers are not supported.
    /// </summary>
    const utility::string_t error_code_condition_headers_not_supported(_XPLATSTR("ConditionHeadersNotSupported"));

    /// <summary>
    /// Multiple conditional headers are not supported.
    /// </summary>
    const utility::string_t error_code_multiple_condition_headers_not_supported(_XPLATSTR("MultipleConditionHeadersNotSupported"));

    /// <summary>
    /// Read-access geo-redundant replication is not enabled for the account, write operations to the secondary location are not allowed, 
    /// or the account being accessed does not have sufficient permissions to execute this operation.
    /// </summary>
    const utility::string_t error_code_insufficient_account_permissions(_XPLATSTR("InsufficientAccountPermissions"));

    /// <summary>
    /// The specified account is disabled.
    /// </summary>
    const utility::string_t error_code_account_is_disabled(_XPLATSTR("AccountIsDisabled"));

    /// <summary>
    /// The specified account already exists.
    /// </summary>
    const utility::string_t error_code_account_already_exists(_XPLATSTR("AccountAlreadyExists"));

    /// <summary>
    /// The specified account is in the process of being created.
    /// </summary>
    const utility::string_t error_code_account_being_created(_XPLATSTR("AccountBeingCreated"));

    /// <summary>
    /// The specified resource already exists.
    /// </summary>
    const utility::string_t error_code_resource_already_exists(_XPLATSTR("ResourceAlreadyExists"));

    /// <summary>
    /// The specified resource type does not match the type of the existing resource.
    /// </summary>
    const utility::string_t error_code_resource_type_mismatch(_XPLATSTR("ResourceTypeMismatch"));

#pragma endregion

#pragma region blob_error_code_strings

    // This section provides error code strings that are specific to the Blob service.

    /// <summary>
    /// The specified append offset is invalid.
    /// </summary>
    const utility::string_t error_code_invalid_append_condition(_XPLATSTR("AppendPositionConditionNotMet"));

    /// <summary>
    /// The specified maximum blob size is invalid.
    /// </summary>
    const utility::string_t error_invalid_max_blob_size_condition(_XPLATSTR("MaxBlobSizeConditionNotMet"));

    /// <summary>
    /// The specified block or blob is invalid.
    /// </summary>
    const utility::string_t error_code_invalid_blob_or_block(_XPLATSTR("InvalidBlobOrBlock"));

    /// <summary>
    /// The block ID is invalid.
    /// </summary>
    const utility::string_t error_code_invalid_block_id(_XPLATSTR("InvalidBlockId"));

    /// <summary>
    /// The block list is invalid.
    /// </summary>
    const utility::string_t error_code_invalid_block_list(_XPLATSTR("InvalidBlockList"));

    /// <summary>
    /// The specified container was not found.
    /// </summary>
    const utility::string_t error_code_container_not_found(_XPLATSTR("ContainerNotFound"));

    /// <summary>
    /// The blob with the specified address cannot be found.
    /// </summary>
    const utility::string_t error_code_blob_not_found(_XPLATSTR("BlobNotFound"));

    /// <summary>
    /// The specified container already exists.
    /// </summary>
    const utility::string_t error_code_container_already_exists(_XPLATSTR("ContainerAlreadyExists"));

    /// <summary>
    /// The specified container is disabled.
    /// </summary>
    const utility::string_t error_code_container_disabled(_XPLATSTR("ContainerDisabled"));

    /// <summary>
    /// The specified container is being deleted.
    /// </summary>
    const utility::string_t error_code_container_being_deleted(_XPLATSTR("ContainerBeingDeleted"));

    /// <summary>
    /// The specified blob already exists.
    /// </summary>
    const utility::string_t error_code_blob_already_exists(_XPLATSTR("BlobAlreadyExists"));

    /// <summary>
    /// There is currently no lease on the blob.
    /// </summary>
    const utility::string_t error_code_lease_not_present_with_blob_operation(_XPLATSTR("LeaseNotPresentWithBlobOperation"));

    /// <summary>
    /// There is currently no lease on the container.
    /// </summary>
    const utility::string_t error_code_lease_not_present_with_container_operation(_XPLATSTR("LeaseNotPresentWithContainerOperation"));

    /// <summary>
    /// The specified lease has expired.
    /// </summary>
    const utility::string_t error_code_lease_lost(_XPLATSTR("LeaseLost"));

    /// <summary>
    /// The specified lease ID does not match the lease ID for the blob.
    /// </summary>
    const utility::string_t error_code_lease_id_mismatch_with_blob_operation(_XPLATSTR("LeaseIdMismatchWithBlobOperation"));

    /// <summary>
    /// The specified lease ID does not match the lease ID for the container.
    /// </summary>
    const utility::string_t error_code_lease_id_mismatch_with_container_operation(_XPLATSTR("LeaseIdMismatchWithContainerOperation"));

    /// <summary>
    /// There is currently a lease on the resource and no lease ID was specified in the request.
    /// </summary>
    const utility::string_t error_code_lease_id_missing(_XPLATSTR("LeaseIdMissing"));

    /// <summary>
    /// There is currently no lease on the resource.
    /// </summary>
    const utility::string_t error_code_lease_not_present_with_lease_operation(_XPLATSTR("LeaseNotPresentWithLeaseOperation"));

    /// <summary>
    /// The specified lease ID specified does not match the lease operation.
    /// </summary>
    const utility::string_t error_code_lease_id_mismatch_with_lease_operation(_XPLATSTR("LeaseIdMismatchWithLeaseOperation"));

    /// <summary>
    /// There is already a lease present.
    /// </summary>
    const utility::string_t error_code_lease_already_present(_XPLATSTR("LeaseAlreadyPresent"));

    /// <summary>
    /// The lease has already been broken and cannot be broken again.
    /// </summary>
    const utility::string_t error_code_lease_already_broken(_XPLATSTR("LeaseAlreadyBroken"));

    /// <summary>
    /// The lease has been broken explicitly and cannot be renewed.
    /// </summary>
    const utility::string_t error_code_lease_is_broken_and_cannot_be_renewed(_XPLATSTR("LeaseIsBrokenAndCannotBeRenewed"));

    /// <summary>
    /// The lease is breaking and cannot be acquired.
    /// </summary>
    const utility::string_t error_code_lease_is_breaking_and_cannot_be_acquired(_XPLATSTR("LeaseIsBreakingAndCannotBeAcquired"));

    /// <summary>
    /// The lease is breaking and cannot be changed.
    /// </summary>
    const utility::string_t error_code_lease_is_breaking_and_cannot_be_changed(_XPLATSTR("LeaseIsBreakingAndCannotBeChanged"));

    /// <summary>
    /// The destination of a copy operation has a lease of fixed duration.
    /// </summary>
    const utility::string_t error_code_infinite_lease_duration_required(_XPLATSTR("InfiniteLeaseDurationRequired"));

    /// <summary>
    /// The operation is not permitted because the blob has snapshots.
    /// </summary>
    const utility::string_t error_code_snapshots_present(_XPLATSTR("SnapshotsPresent"));

    /// <summary>
    /// The blob type is invalid for this operation.
    /// </summary>
    const utility::string_t error_code_invalid_blob_type(_XPLATSTR("InvalidBlobType"));

    /// <summary>
    /// The operation on page blobs uses a version prior to 2009-09-19.
    /// </summary>
    const utility::string_t error_code_invalid_version_for_page_blob_operation(_XPLATSTR("InvalidVersionForPageBlobOperation"));

    /// <summary>
    /// The page range specified is invalid.
    /// </summary>
    const utility::string_t error_code_invalid_page_range(_XPLATSTR("InvalidPageRange"));

    /// <summary>
    /// The sequence number condition specified was not met.
    /// </summary>
    const utility::string_t error_code_sequence_number_condition_not_met(_XPLATSTR("SequenceNumberConditionNotMet"));

    /// <summary>
    /// The sequence number increment cannot be performed because it would result in overflow of the sequence number.
    /// </summary>
    const utility::string_t error_code_sequence_number_increment_too_large(_XPLATSTR("SequenceNumberIncrementTooLarge"));

    /// <summary>
    /// The source condition specified using HTTP conditional header(s) is not met.
    /// </summary>
    const utility::string_t error_code_source_condition_not_met(_XPLATSTR("SourceConditionNotMet"));

    /// <summary>
    /// The target condition specified using HTTP conditional header(s) is not met.
    /// </summary>
    const utility::string_t error_code_target_condition_not_met(_XPLATSTR("TargetConditionNotMet"));

    /// <summary>
    ///  The source and destination accounts for the copy operation are not the same.
    /// </summary>
    const utility::string_t error_code_copy_across_accounts_not_supported(_XPLATSTR("CopyAcrossAccountsNotSupported"));

    /// <summary>
    /// The source for the copy operation cannot be accessed.
    /// </summary>
    const utility::string_t error_code_cannot_verify_copy_source(_XPLATSTR("CannotVerifyCopySource"));

    /// <summary>
    /// An attempt to modify the destination of a pending copy operation is made.
    /// </summary>
    const utility::string_t error_code_pending_copy_operation(_XPLATSTR("PendingCopyOperation"));

    /// <summary>
    /// An Abort Copy operation is called when there is no pending copy operation.
    /// </summary>
    const utility::string_t error_code_no_pending_copy_operation(_XPLATSTR("NoPendingCopyOperation"));

    /// <summary>
    /// The copy ID specified in an Abort Copy operation does not match the current pending copy operation ID.
    /// </summary>
    const utility::string_t error_code_copy_id_mismatch(_XPLATSTR("CopyIdMismatch"));

#pragma endregion

#pragma region table_error_code_strings

    // This section provides error code strings that are specific to the Table service.

    /// <summary>
    /// The request uses X-HTTP-Method with an HTTP verb other than POST.
    /// </summary>
    const utility::string_t error_code_x_method_not_using_post(_XPLATSTR("XMethodNotUsingPost"));

    /// <summary>
    /// The specified X-HTTP-Method is invalid.
    /// </summary>
    const utility::string_t error_code_x_method_incorrect_value(_XPLATSTR("XMethodIncorrectValue"));

    /// <summary>
    /// More than one X-HTTP-Method is specified.
    /// </summary>
    const utility::string_t error_code_x_method_incorrect_count(_XPLATSTR("XMethodIncorrectCount"));

    /// <summary>
    /// The specified table has no properties.
    /// </summary>
    const utility::string_t error_code_table_has_no_properties(_XPLATSTR("TableHasNoProperties"));

    /// <summary>
    /// A property is specified more than once.
    /// </summary>
    const utility::string_t error_code_duplicate_properties_specified(_XPLATSTR("DuplicatePropertiesSpecified"));

    /// <summary>
    /// The specified table has no such property.
    /// </summary>
    const utility::string_t error_code_table_has_no_such_property(_XPLATSTR("TableHasNoSuchProperty"));

    /// <summary>
    /// A duplicate key property was specified.
    /// </summary>
    const utility::string_t error_code_duplicate_key_property_specified(_XPLATSTR("DuplicateKeyPropertySpecified"));

    /// <summary>
    /// The specified table already exists.
    /// </summary>
    const utility::string_t error_code_table_already_exists(_XPLATSTR("TableAlreadyExists"));

    /// <summary>
    /// The specified table was not found.
    /// </summary>
    const utility::string_t error_code_table_not_found(_XPLATSTR("TableNotFound"));

    /// <summary>
    /// The specified entity was not found.
    /// </summary>
    const utility::string_t error_code_entity_not_found(_XPLATSTR("EntityNotFound"));

    /// <summary>
    /// The specified entity already exists.
    /// </summary>
    const utility::string_t error_code_entity_already_exists(_XPLATSTR("EntityAlreadyExists"));

    /// <summary>
    /// The partition key was not specified.
    /// </summary>
    const utility::string_t error_code_partition_key_not_specified(_XPLATSTR("PartitionKeyNotSpecified"));

    /// <summary>
    /// One or more specified operators are invalid.
    /// </summary>
    const utility::string_t error_code_operator_invalid(_XPLATSTR("OperatorInvalid"));

    /// <summary>
    /// The specified update condition was not satisfied.
    /// </summary>
    const utility::string_t error_code_update_condition_not_satisfied(_XPLATSTR("UpdateConditionNotSatisfied"));

    /// <summary>
    /// All properties must have values.
    /// </summary>
    const utility::string_t error_code_properties_need_value(_XPLATSTR("PropertiesNeedValue"));

    /// <summary>
    /// The partition key property cannot be updated.
    /// </summary>
    const utility::string_t error_code_partition_key_property_cannot_be_updated(_XPLATSTR("PartitionKeyPropertyCannotBeUpdated"));

    /// <summary>
    /// The entity contains more properties than allowed.
    /// </summary>
    const utility::string_t error_code_too_many_properties(_XPLATSTR("TooManyProperties"));

    /// <summary>
    /// The entity is larger than the maximum size permitted.
    /// </summary>
    const utility::string_t error_code_entity_too_large(_XPLATSTR("EntityTooLarge"));

    /// <summary>
    /// The property value is larger than the maximum size permitted.
    /// </summary>
    const utility::string_t error_code_property_value_too_large(_XPLATSTR("PropertyValueTooLarge"));

    /// <summary>
    /// One or more value types are invalid.
    /// </summary>
    const utility::string_t error_code_invalid_value_type(_XPLATSTR("InvalidValueType"));

    /// <summary>
    /// The specified table is being deleted.
    /// </summary>
    const utility::string_t error_code_table_being_deleted(_XPLATSTR("TableBeingDeleted"));

    /// <summary>
    /// The Table service server is out of memory.
    /// </summary>
    const utility::string_t error_code_table_server_out_of_memory(_XPLATSTR("TableServerOutOfMemory"));

    /// <summary>
    /// The type of the primary key property is invalid.
    /// </summary>
    const utility::string_t error_code_primary_key_property_is_invalid_type(_XPLATSTR("PrimaryKeyPropertyIsInvalidType"));

    /// <summary>
    /// The property name exceeds the maximum allowed length.
    /// </summary>
    const utility::string_t error_code_property_name_too_long(_XPLATSTR("PropertyNameTooLong"));

    /// <summary>
    /// The property name is invalid.
    /// </summary>
    const utility::string_t error_code_property_name_invalid(_XPLATSTR("PropertyNameInvalid"));

    /// <summary>
    /// Batch operations are not supported for this operation type.
    /// </summary>
    const utility::string_t error_code_batch_operation_not_supported(_XPLATSTR("BatchOperationNotSupported"));

    /// <summary>
    /// JSON format is not supported.
    /// </summary>
    const utility::string_t error_code_json_format_not_supported(_XPLATSTR("JsonFormatNotSupported"));

    /// <summary>
    /// The specified method is not allowed.
    /// </summary>
    const utility::string_t error_code_method_not_allowed(_XPLATSTR("MethodNotAllowed"));

    /// <summary>
    /// The specified operation is not yet implemented.
    /// </summary>
    const utility::string_t error_code_not_implemented(_XPLATSTR("NotImplemented"));

    /// <summary>
    /// The required host information is not present in the request. You must send a non-empty Host header or include the absolute URI in the request line.
    /// </summary>
    const utility::string_t error_code_host_information_not_present(_XPLATSTR("HostInformationNotPresent"));

#pragma endregion

#pragma region queue_error_code_strings

    // This section provides error code strings that are specific to the Queue service.

    /// <summary>
    /// The specified queue was not found.
    /// </summary>
    const utility::string_t error_code_queue_not_found(_XPLATSTR("QueueNotFound"));

    /// <summary>
    /// The specified queue is disabled.
    /// </summary>
    const utility::string_t error_code_queue_disabled(_XPLATSTR("QueueDisabled"));

    /// <summary>
    /// The specified queue already exists.
    /// </summary>
    const utility::string_t error_code_queue_already_exists(_XPLATSTR("QueueAlreadyExists"));

    /// <summary>
    /// The specified queue is not empty.
    /// </summary>
    const utility::string_t error_code_queue_not_empty(_XPLATSTR("QueueNotEmpty"));

    /// <summary>
    /// The specified queue is being deleted.
    /// </summary>
    const utility::string_t error_code_queue_being_deleted(_XPLATSTR("QueueBeingDeleted"));

    /// <summary>
    /// The specified pop receipt does not match.
    /// </summary>
    const utility::string_t error_code_pop_receipt_mismatch(_XPLATSTR("PopReceiptMismatch"));

    /// <summary>
    /// One or more request parameters are invalid.
    /// </summary>
    const utility::string_t error_code_invalid_parameter(_XPLATSTR("InvalidParameter"));

    /// <summary>
    /// The specified message was not found.
    /// </summary>
    const utility::string_t error_code_message_not_found(_XPLATSTR("MessageNotFound"));

    /// <summary>
    /// The specified message is too large.
    /// </summary>
    const utility::string_t error_code_message_too_large(_XPLATSTR("MessageTooLarge"));

    /// <summary>
    /// The specified marker is invalid.
    /// </summary>
    const utility::string_t error_code_invalid_marker(_XPLATSTR("InvalidMarker"));

#pragma endregion

}}} // namespace azure::storage::protocol
