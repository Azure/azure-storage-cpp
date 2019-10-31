// -----------------------------------------------------------------------------------------
// <copyright file="FilesProperties.cpp" company="Microsoft">
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

#include "samples_common.h"

#include <was/storage_account.h>
#include <was/file.h>


namespace azure { namespace storage { namespace samples {

    SAMPLE(FilesProperties, files_properties_sample)
    void files_properties_sample()
    {
        try
        {
            // Initialize storage account
            azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(storage_connection_string);

            // Create share
            azure::storage::cloud_file_client file_client = storage_account.create_cloud_file_client();
            azure::storage::cloud_file_share share = file_client.get_share_reference(_XPLATSTR("my-sample-share"));
            share.create_if_not_exists();

            // azure-storage-cpp sdk treats permission as an opaque string. The string below is pretty much a default permission.
            utility::string_t permission = _XPLATSTR("O:S-1-5-21-2127521184-1604012920-1887927527-21560751G:S-1-5-21-2127521184-1604012920-1887927527-513D:(A;;FA;;;SY)(A;;FA;;;BA)(A;;0x1200a9;;;S-1-5-21-397955417-626881126-188441444-3053964)");
            utility::string_t permission_key = share.upload_file_permission(permission);

            azure::storage::cloud_file_directory directory = share.get_directory_reference(_XPLATSTR("my-sample-directory"));
            directory.delete_directory_if_exists();

            // Create a new directory with properties.
            directory.properties().set_attributes(azure::storage::cloud_file_attributes::directory | azure::storage::cloud_file_attributes::system);
            directory.properties().set_creation_time(azure::storage::cloud_file_directory_properties::now);
            directory.properties().set_last_write_time(utility::datetime::from_string(_XPLATSTR("Thu, 31 Oct 2019 06:42:18 GMT")));
            // You can specify either permission or permission key, but not both.
            directory.properties().set_permission(permission);
            //directory.properties().set_permission_key(permission_key);
            directory.create();

            // Upload a file.
            azure::storage::cloud_file file = directory.get_file_reference(_XPLATSTR("my-sample-file-1"));
            // Properties for file are pretty much the same as for directory.
            // You can leave properties unset to use default.
            file.properties().set_attributes(azure::storage::cloud_file_attributes::archive);
            //file.properties().set_permission(azure::storage::cloud_file_properties::inherit);
            file.properties().set_permission_key(permission_key);
            file.upload_text(_XPLATSTR("some text"));

            // Update properties for an existing file/directory.
            file.properties().set_creation_time(utility::datetime::from_string(_XPLATSTR("Wed, 10 Oct 2001 20:51:31 +0000")));
            file.upload_properties();

            file.delete_file();
            directory.delete_directory();
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

}}}  // namespace azure::storage::samples
