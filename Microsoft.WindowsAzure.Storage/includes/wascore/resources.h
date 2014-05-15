// -----------------------------------------------------------------------------------------
// <copyright file="resources.h" company="Microsoft">
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

    const std::string error_blob_type_mismatch("Blob type of the blob reference doesn't match blob type of the blob.");
    const std::string error_closed_stream("Cannot access a closed stream.");
    const std::string error_lease_id_on_source("A lease condition cannot be specified on the source of a copy.");
    const std::string error_incorrect_length("Incorrect number of bytes received.");
    const std::string error_md5_mismatch("Calculated MD5 does not match existing property.");
    const std::string error_missing_md5("MD5 does not exist. If you do not want to force validation, please disable use_transactional_md5.");
    const std::string error_sas_missing_credentials("Cannot create Shared Access Signature unless Shared Key credentials are used.");
    const std::string error_client_timeout("The client could not finish the operation within specified timeout.");
    const std::string error_cannot_modify_snapshot("Cannot perform this operation on a blob representing a snapshot.");
    const std::string error_page_blob_size_unknown("The size of the page blob could not be determined, because stream is not seekable and a length argument is not provided.");
    const std::string error_stream_short("The requested number of bytes exceeds the length of the stream remaining from the specified position.");
    const std::string error_unsupported_text_blob("Only plain text with utf-8 encoding is supported.");
    const std::string error_multiple_snapshots("Cannot provide snapshot time as part of the address and as constructor parameter. Either pass in the address or use a different constructor.");
    const std::string error_multiple_credentials("Cannot provide credentials as part of the address and as constructor parameter. Either pass in the address or use a different constructor.");
    const std::string error_uri_missing_location("The Uri for the target storage location is not specified. Please consider changing the request's location mode.");
    const std::string error_primary_only_command("This operation can only be executed against the primary storage location.");
    const std::string error_secondary_only_command("This operation can only be executed against the secondary storage location.");
    const std::string error_md5_not_possible("MD5 is not supported for an existing page blobs.");
    const std::string error_missing_params_for_sas("Missing mandatory parameters for valid Shared Access Signature.");
    const std::string error_md5_options_mismatch("When uploading a blob in a single request, store_blob_content_md5 must be set to true if use_transactional_md5 is true, because the MD5 calculated for the transaction will be stored in the blob.");
    const std::string error_storage_uri_empty("Primary or secondary location URI must be supplied.");
    const std::string error_storage_uri_mismatch("Primary and secondary location URIs must point to the same resource.");

    const std::string error_empty_batch_operation("The batch operation cannot be empty.");
    const std::string error_batch_operation_partition_key_mismatch("The batch operation cannot contain entities with different partition keys.");
    const std::string error_batch_operation_retrieve_count("The batch operation cannot contain more than one retrieve operation.");
    const std::string error_batch_operation_retrieve_mix("The batch operation cannot contain any other operations when it contains a retrieve operation.");
    const std::string error_entity_property_not_binary("The type of the entity property is not binary.");
    const std::string error_entity_property_not_boolean("The type of the entity property is not boolean.");
    const std::string error_parse_boolean("An error occurred parsing the boolean.");
    const std::string error_parse_double("An error occurred parsing the double.");
    const std::string error_entity_property_not_datetime("The type of the entity property is not date/time.");
    const std::string error_parse_datetime("An error occurred parsing the date/time.");
    const std::string error_entity_property_not_double("The type of the entity property is not double.");
    const std::string error_entity_property_not_guid("The type of the entity property is not GUID.");
    const std::string error_entity_property_not_int32("The type of the entity property is not 32-bit integer.");
    const std::string error_parse_int32("An error occurred parsing the 32-bit integer.");
    const std::string error_entity_property_not_int64("The type of the entity property is not 64-bit integer.");
    const std::string error_entity_property_not_string("The type of the entity property is not string.");

    const std::string error_non_positive_time_to_live("The time to live cannot be zero or negative.");
    const std::string error_large_time_to_live("The time to live cannot be greater than 604800.");
    const std::string error_negative_initial_visibility_timeout("The initial visibility timeout cannot be negative.");
    const std::string error_large_initial_visibility_timeout("The initial visibility timeout cannot be greater than 604800.");
    const std::string error_negative_visibility_timeout("The visibility timeout cannot be negative.");
    const std::string error_large_visibility_timeout("The visibility timeout cannot be greater than 604800.");
    const std::string error_large_message_count("The message count cannot be greater than 32.");
    const std::string error_empty_message_id("The message ID cannot be empty.");
    const std::string error_empty_message_pop_receipt("The message pop receipt cannot be empty.");

    const std::string error_create_uuid("An error occurred creating the UUID.");
    const std::string error_serialize_uuid("An error occurred serializing the UUID.");
    const std::string error_free_uuid("An error occurred freeing the UUID string.");
    const std::string error_parse_uuid("An error occurred parsing the UUID.");

    const std::string error_empty_metadata_value("The metadata value cannot be empty or consist entirely of whitespace.");
    const std::string error_hash_on_closed_streambuf("Hash is calculated when the streambuf is closed.");
    const std::string error_invalid_settings_form("Settings must be of the form \"name=value\".");

}}} // namespace azure::storage::protocol
