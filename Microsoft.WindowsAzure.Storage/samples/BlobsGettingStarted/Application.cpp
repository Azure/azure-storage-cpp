// -----------------------------------------------------------------------------------------
// <copyright file="Application.cpp" company="Microsoft">
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
#include "samples_common.h"

#include "was/storage_account.h"
#include "was/blob.h"

namespace wa { namespace storage { namespace samples {

    void blobs_getting_started_sample()
    {
        try
        {
            // Initialize storage account
            wa::storage::cloud_storage_account storage_account = wa::storage::cloud_storage_account::parse(storage_connection_string);

            // Create a blob container
            wa::storage::cloud_blob_client blob_client = storage_account.create_cloud_blob_client();
            wa::storage::cloud_blob_container container = blob_client.get_container_reference(U("azure-native-client-library-sample-container"));
            bool created = container.create_if_not_exists();

            // Make the blob container publicly accessible
            wa::storage::blob_container_permissions permissions;
            permissions.set_public_access(wa::storage::blob_container_public_access_type::blob);
            container.upload_permissions(permissions);

            // Upload some blobs
            wa::storage::cloud_block_blob blob1 = container.get_block_blob_reference(U("my-blob-1"));
            blob1.upload_text(U("some text"));
            wa::storage::cloud_block_blob blob2 = container.get_block_blob_reference(U("my-blob-2"));
            blob2.upload_text(U("more text"));
            wa::storage::cloud_block_blob blob3 = container.get_block_blob_reference(U("my-directory/my-sub-directory/my-blob-3"));
            blob3.upload_text(U("other text"));

            // List blobs in the blob container
            wa::storage::blob_continuation_token continuation_token;
            do
            {
                wa::storage::blob_result_segment result = container.list_blobs_segmented(continuation_token);
                std::vector<wa::storage::cloud_blob> blobs = result.blobs();
                for (std::vector<wa::storage::cloud_blob>::const_iterator itr = blobs.cbegin(); itr != blobs.cend(); ++itr)
                {
                    ucout << U("Blob: ") << itr->uri().primary_uri().to_string() << std::endl;
                }
                std::vector<wa::storage::cloud_blob_directory> directories = result.directories();
                for (std::vector<wa::storage::cloud_blob_directory>::const_iterator itr = directories.cbegin(); itr != directories.cend(); ++itr)
                {
                    ucout << U("Directory: ") << itr->uri().primary_uri().to_string() << std::endl;
                }
                continuation_token = result.continuation_token();
            }
            while (!continuation_token.empty());

            // Download a blob
            wa::storage::cloud_block_blob blob4 = container.get_block_blob_reference(U("my-blob-1"));
            utility::string_t text = blob4.download_text();

            // Delete a blob
            wa::storage::cloud_block_blob blob5 = container.get_block_blob_reference(U("my-blob-2"));
            blob5.delete_blob();

            // Delete the blob container
            bool deleted = container.delete_container_if_exists();
        }
        catch (wa::storage::storage_exception& e)
        {
            ucout << U("Error: ") << e.what() << std::endl;

            wa::storage::request_result result = e.result();
            wa::storage::storage_extended_error extended_error = result.extended_error();
            if (!extended_error.message().empty())
            {
                ucout << extended_error.message() << std::endl;
            }
        }
        catch (std::exception& e)
        {
            ucout << U("Error: ") << e.what() << std::endl;
        }
    }

}}} // namespace wa::storage::samples

int _tmain(int argc, _TCHAR *argv[])
{
    wa::storage::samples::blobs_getting_started_sample();
    return 0;
}

