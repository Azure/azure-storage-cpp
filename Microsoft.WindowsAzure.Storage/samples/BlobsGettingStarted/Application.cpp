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
#include "cpprest/filestream.h"
#include "cpprest/containerstream.h"

namespace azure { namespace storage { namespace samples {

    utility::string_t to_string(const std::vector<uint8_t>& data)
    {
        return utility::string_t(data.cbegin(), data.cend());
    }

    void blobs_getting_started_sample()
    {
        try
        {
            // Initialize storage account
            azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(storage_connection_string);

            // Create a blob container
            azure::storage::cloud_blob_client blob_client = storage_account.create_cloud_blob_client();
            azure::storage::cloud_blob_container container = blob_client.get_container_reference(U("my-sample-container"));
           
            // Return value is true if the container did not exist and was successfully created.
            container.create_if_not_exists();

            // Make the blob container publicly accessible
            azure::storage::blob_container_permissions permissions;
            permissions.set_public_access(azure::storage::blob_container_public_access_type::blob);
            container.upload_permissions(permissions);

            // Upload a blob from a file
            concurrency::streams::istream input_stream = concurrency::streams::file_stream<uint8_t>::open_istream(U("DataFile.txt")).get();
            azure::storage::cloud_block_blob blob1 = container.get_block_blob_reference(U("my-blob-1"));
            blob1.upload_from_stream(input_stream);
            input_stream.close().wait();

            // Upload some blobs from text
            azure::storage::cloud_block_blob blob2 = container.get_block_blob_reference(U("my-blob-2"));
            blob2.upload_text(U("more text"));
            azure::storage::cloud_block_blob blob3 = container.get_block_blob_reference(U("my-directory/my-sub-directory/my-blob-3"));
            blob3.upload_text(U("other text"));

            // List blobs in the blob container
            azure::storage::continuation_token token;
            do
            {
                azure::storage::blob_result_segment result = container.list_blobs_segmented(token);
                std::vector<azure::storage::cloud_blob> blobs = result.blobs();
                for (std::vector<azure::storage::cloud_blob>::const_iterator it = blobs.cbegin(); it != blobs.cend(); ++it)
                {
                    ucout << U("Blob: ") << it->uri().primary_uri().to_string() << std::endl;
                }
                std::vector<azure::storage::cloud_blob_directory> directories = result.directories();
                for (std::vector<azure::storage::cloud_blob_directory>::const_iterator it = directories.cbegin(); it != directories.cend(); ++it)
                {
                    ucout << U("Directory: ") << it->uri().primary_uri().to_string() << std::endl;
                }
                token = result.continuation_token();
            }
            while (!token.empty());

            // Download a blob to a stream
            concurrency::streams::container_buffer<std::vector<uint8_t>> buffer;
            concurrency::streams::ostream output_stream(buffer);
            azure::storage::cloud_block_blob binary_blob = container.get_block_blob_reference(U("my-blob-1"));
            binary_blob.download_to_stream(output_stream);
            ucout << U("Stream: ") << to_string(buffer.collection()) << std::endl;

            // Download a blob as text
            azure::storage::cloud_block_blob text_blob = container.get_block_blob_reference(U("my-blob-2"));
            utility::string_t text = text_blob.download_text();
            ucout << U("Text: ") << text << std::endl;

            // Delete the blobs
            blob1.delete_blob();
            blob2.delete_blob();
            blob3.delete_blob();            

            // Delete the blob container
            // Return value is true if the container did exist and was successfully deleted.
            container.delete_container_if_exists();
        }
        catch (const azure::storage::storage_exception& e)
        {
            ucout << U("Error: ") << e.what() << std::endl;

            azure::storage::request_result result = e.result();
            azure::storage::storage_extended_error extended_error = result.extended_error();
            if (!extended_error.message().empty())
            {
                ucout << extended_error.message() << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            ucout << U("Error: ") << e.what() << std::endl;
        }
    }

}}} // namespace azure::storage::samples

int main(int argc, const char *argv[])
{
    azure::storage::samples::blobs_getting_started_sample();
    return 0;
}

