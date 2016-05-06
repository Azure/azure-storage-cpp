// -----------------------------------------------------------------------------------------
// <copyright file="unicode_test.cpp" company="Microsoft">
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
#include "check_macros.h"
#include "blob_test_base.h"
#include "queue_test_base.h"
#include "table_test_base.h"

const std::string error_invalid_characters_in_resource_name("The specifed resource name contains invalid characters.");
const std::string error_bad_request("Bad Request");

SUITE(Core)
{
    TEST_FIXTURE(container_test_base, container_name_unicode)
    {
        auto container = m_client.get_container_reference(_XPLATSTR("容器1"));
        CHECK_STORAGE_EXCEPTION(container.exists(), error_invalid_characters_in_resource_name);
        CHECK_STORAGE_EXCEPTION(container.create_if_not_exists(), error_invalid_characters_in_resource_name);
    }

    TEST_FIXTURE(blob_test_base, directory_name_unicode)
    {
        utility::string_t dir_name(_XPLATSTR("目录1"));
        utility::string_t blob_name(_XPLATSTR("block_blob"));
        auto dir = m_container.get_directory_reference(dir_name);
        auto blob = dir.get_block_blob_reference(blob_name);
        blob.upload_text(_XPLATSTR("test"));
        CHECK(blob.exists());
        CHECK(blob.name() == dir_name + _XPLATSTR("/") + blob_name);
    }

    TEST_FIXTURE(blob_test_base, blob_name_unicode)
    {
        utility::string_t blob_name(_XPLATSTR("文件1"));
        auto blob = m_container.get_block_blob_reference(blob_name);
        blob.upload_text(_XPLATSTR("test2"));
        CHECK(blob.exists());
        CHECK(blob.name() == blob_name);
    }

    TEST_FIXTURE(queue_service_test_base, queue_name_unicode)
    {
        utility::string_t queue_name(_XPLATSTR("队列1"));
        azure::storage::cloud_queue_client client = get_queue_client();
        azure::storage::cloud_queue queue = client.get_queue_reference(queue_name);
        CHECK_STORAGE_EXCEPTION(queue.exists(), error_invalid_characters_in_resource_name);
        CHECK_STORAGE_EXCEPTION(queue.create(), error_invalid_characters_in_resource_name);
    }

    TEST_FIXTURE(table_service_test_base, table_name_unicode)
    {
        utility::string_t table_name(_XPLATSTR("表格1"));
        azure::storage::cloud_table_client client = get_table_client();
        azure::storage::cloud_table table = client.get_table_reference(table_name);
        CHECK(false == table.exists()); 
        CHECK_STORAGE_EXCEPTION(table.create(), error_bad_request);
    }
}
