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
#include <was/file.h>
#include <cpprest/filestream.h>

namespace azure { namespace storage { namespace samples {

    void files_getting_started_sample()
    {
        try
        {
            // Initialize storage account
            azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(storage_connection_string);

            // Create share, directory
            azure::storage::cloud_file_client file_client = storage_account.create_cloud_file_client();
            azure::storage::cloud_file_share share = file_client.get_share_reference(_XPLATSTR("my-sample-share"));
            azure::storage::cloud_file_directory directory = share.get_directory_reference(_XPLATSTR("my-sample-directory"));

            // Return value is true if the share did not exist and was successfully created.
            share.create_if_not_exists();

            // Return value is true if the directory did not exist and was successfully created.
            directory.create_if_not_exists();

            // Create a sub-directory.
            azure::storage::cloud_file_directory subdirectory = directory.get_subdirectory_reference(_XPLATSTR("my-sample-subdirectory"));
            subdirectory.create_if_not_exists();

            // Upload a file from a stream.
            concurrency::streams::istream input_stream = concurrency::streams::file_stream<uint8_t>::open_istream(_XPLATSTR("DataFile.txt")).get();
            azure::storage::cloud_file file1 = directory.get_file_reference(_XPLATSTR("my-sample-file-1"));
            file1.upload_from_stream(input_stream);

            // Upload some files from text.
            azure::storage::cloud_file file2 = directory.get_file_reference(_XPLATSTR("my-sample-file-2"));
            file2.upload_text(_XPLATSTR("more text"));
            azure::storage::cloud_file file3 = directory.get_file_reference(_XPLATSTR("my-sample-file-3"));
            file3.upload_text(_XPLATSTR("other text"));

            // Upload a file from a file.
            azure::storage::cloud_file file4 = directory.get_file_reference(_XPLATSTR("my-sample-file-4"));
            file4.upload_from_file(_XPLATSTR("DataFile.txt"));

            // List files and directories in the directory
            azure::storage::continuation_token token;
            do
            {
                azure::storage::list_file_and_directory_result_segment result = directory.list_files_and_directories_segmented(token);
                for (auto& item : result.results())
                {
                    if (item.is_file())
                    {
                        ucout << "File: " << item.as_file().uri().primary_uri().to_string() << std::endl;
                    }
                    if (item.is_directory())
                    {
                        ucout << "Directory: " << item.as_directory().uri().primary_uri().to_string() << std::endl;
                    }
                }
            } 
            while (!token.empty());

            // Download text file.
            azure::storage::cloud_file text_file = directory.get_file_reference(_XPLATSTR("my-sample-file-2"));
            utility::string_t text = text_file.download_text();
            ucout << "File Text: " << text << std::endl;

            // Delete the files.
            file1.delete_file();
            file2.delete_file();
            file3.delete_file();
            file4.delete_file();

            // Delete the directories;
            subdirectory.delete_directory();
            directory.delete_directory();

            // Delete the file share.
            share.delete_share_if_exists();
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
    azure::storage::samples::files_getting_started_sample();
    return 0;
}

