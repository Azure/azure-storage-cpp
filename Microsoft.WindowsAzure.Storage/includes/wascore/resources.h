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

namespace wa { namespace storage { namespace protocol {

    const utility::string_t error_blob_type_mismatch(U("Blob type of the blob reference doesn't match blob type of the blob."));
    const utility::string_t error_closed_stream(U("Cannot access a closed stream."));
    const utility::string_t error_lease_id_on_source(U("A lease condition cannot be specified on the source of a copy."));
    const utility::string_t error_incorrect_length(U("Incorrect number of bytes received."));
    const utility::string_t error_md5_mismatch(U("Calculated MD5 does not match existing property."));
    const utility::string_t error_missing_md5(U("MD5 does not exist. If you do not want to force validation, please disable use_transactional_md5."));
    const utility::string_t error_sas_missing_credentials(U("Cannot create Shared Access Signature unless Shared Key credentials are used."));
    const utility::string_t error_client_timeout(U("The client could not finish the operation within specified timeout."));
    const utility::string_t error_cannot_modify_snapshot(U("Cannot perform this operation on a blob representing a snapshot."));
    const utility::string_t error_page_blob_size_unknown(U("The size of the page blob could not be determined, because stream is not seekable and a length argument is not provided."));
    const utility::string_t error_stream_short(U("The requested number of bytes exceeds the length of the stream remaining from the specified position."));
    const utility::string_t error_unsupported_text_blob(U("Only plain text with utf-8 encoding is supported."));
    const utility::string_t error_multiple_snapshots(U("Cannot provide snapshot time as part of the address and as constructor parameter. Either pass in the address or use a different constructor."));
    const utility::string_t error_multiple_credentials(U("Cannot provide credentials as part of the address and as constructor parameter. Either pass in the address or use a different constructor."));
    const utility::string_t error_uri_missing_location(U("The Uri for the target storage location is not specified. Please consider changing the request's location mode."));
    const utility::string_t error_primary_only_command(U("This operation can only be executed against the primary storage location."));
    const utility::string_t error_secondary_only_command(U("This operation can only be executed against the secondary storage location."));
    const utility::string_t error_md5_not_possible(U("MD5 cannot be calculated for an existing page blob because it would require reading the existing data. Please disable StoreBlobContentMD5."));
    const utility::string_t error_missing_params_for_sas(U("Missing mandatory parameters for valid Shared Access Signature"));
    const utility::string_t error_md5_options_mismatch(U("When uploading a blob in a single request, store_blob_content_md5 must be set to true if use_transactional_md5 is true, because the MD5 calculated for the transaction will be stored in the blob."));
    const utility::string_t error_storage_uri_mismatch(U("Primary and secondary location URIs in a StorageUri must point to the same resource."));

}}} // namespace wa::storage::protocol
