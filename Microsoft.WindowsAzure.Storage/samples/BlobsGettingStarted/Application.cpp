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

#include <was/storage_account.h>
#include <was/blob.h>
#include <cpprest/filestream.h>
#include <cpprest/containerstream.h>

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
            azure::storage::cloud_blob_container container = blob_client.get_container_reference(_XPLATSTR("my-sample-container"));
           
            // Return value is true if the container did not exist and was successfully created.
            container.create_if_not_exists();

            // Make the blob container publicly accessible
            azure::storage::blob_container_permissions permissions;
            permissions.set_public_access(azure::storage::blob_container_public_access_type::blob);
            container.upload_permissions(permissions);

            // Upload a blob from a file
            concurrency::streams::istream input_stream = concurrency::streams::file_stream<uint8_t>::open_istream(_XPLATSTR("DataFile.txt")).get();
            azure::storage::cloud_block_blob blob1 = container.get_block_blob_reference(_XPLATSTR("my-blob-1"));
            blob1.upload_from_stream(input_stream);
            input_stream.close().wait();

            // Upload some blobs from text
            azure::storage::cloud_block_blob blob2 = container.get_block_blob_reference(_XPLATSTR("my-blob-2"));
            blob2.upload_text(_XPLATSTR("more text"));
            azure::storage::cloud_block_blob blob3 = container.get_block_blob_reference(_XPLATSTR("my-directory/my-sub-directory/my-blob-3"));
            blob3.upload_text(_XPLATSTR("other text"));

            // List blobs in the blob container
            azure::storage::continuation_token token;
            do
            {
                azure::storage::list_blob_item_segment result = container.list_blobs_segmented(token);
                for (auto& item : result.results())
                {
                    if (item.is_blob())
                    {
                        ucout << _XPLATSTR("Blob: ") << item.as_blob().uri().primary_uri().to_string() << std::endl;
                    }
                    else
                    {
                        ucout << _XPLATSTR("Directory: ") << item.as_directory().uri().primary_uri().to_string() << std::endl;
                    }
                }
                token = result.continuation_token();
            }
            while (!token.empty());

            // Download a blob to a stream
            concurrency::streams::container_buffer<std::vector<uint8_t>> buffer;
            concurrency::streams::ostream output_stream(buffer);
            azure::storage::cloud_block_blob binary_blob = container.get_block_blob_reference(_XPLATSTR("my-blob-1"));
            binary_blob.download_to_stream(output_stream);
            ucout << _XPLATSTR("Stream: ") << to_string(buffer.collection()) << std::endl;

            // Download a blob as text
            azure::storage::cloud_block_blob text_blob = container.get_block_blob_reference(_XPLATSTR("my-blob-2"));
            utility::string_t text = text_blob.download_text();
            ucout << _XPLATSTR("Text: ") << text << std::endl;

            // Delete the blobs
            blob1.delete_blob();
            blob2.delete_blob();
            blob3.delete_blob();
            
            // Create an append blob
            azure::storage::cloud_append_blob append_blob = container.get_append_blob_reference(_XPLATSTR("my-append-1"));
            append_blob.properties().set_content_type(_XPLATSTR("text/plain; charset=utf-8"));
            append_blob.create_or_replace();

            // Append two blocks
            concurrency::streams::istream append_input_stream1 = concurrency::streams::bytestream::open_istream(utility::conversions::to_utf8string(_XPLATSTR("some text.")));
            concurrency::streams::istream append_input_stream2 = concurrency::streams::bytestream::open_istream(utility::conversions::to_utf8string(_XPLATSTR("more text.")));
            append_blob.append_block(append_input_stream1, utility::string_t());
            append_blob.append_block(append_input_stream2, utility::string_t());
            append_input_stream1.close().wait();
            append_input_stream2.close().wait();

            // Download append blob as text
            utility::string_t append_text = append_blob.download_text();
            ucout << _XPLATSTR("Append Text: ") << append_text << std::endl;
            
            // Delete the blob
            append_blob.delete_blob();

            // Delete the blob container
            // Return value is true if the container did exist and was successfully deleted.
            container.delete_container_if_exists();
        }
        catch (const azure::storage::storage_exception& e)
        {
            ucout << _XPLATSTR("Error: ") << e.what() << std::endl;

            azure::storage::request_result result = e.result();
            azure::storage::storage_extended_error extended_error = result.extended_error();
            if (!extended_error.message().empty())
            {
                ucout << extended_error.message() << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            ucout << _XPLATSTR("Error: ") << e.what() << std::endl;
        }
    }

}}} // namespace azure::storage::samples

int main(int argc, const char *argv[])
{
    azure::storage::samples::blobs_getting_started_sample();
    return 0;
}

