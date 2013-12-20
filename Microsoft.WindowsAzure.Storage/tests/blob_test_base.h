// -----------------------------------------------------------------------------------------
// <copyright file="blob_test_base.h" company="Microsoft">
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

#include <iomanip>
#include "CurrentTest.h"

#include "test_base.h"
#include "was/blob.h"

const utility::string_t dummy_md5(U("MDAwMDAwMDA="));

class blob_service_test_base : public test_base
{
public:
    blob_service_test_base()
        : m_client(test_config::instance().account().create_cloud_blob_client())
    {
    }

    ~blob_service_test_base()
    {
    }

protected:

    static web::http::uri defiddler(const web::http::uri& uri);
    static utility::string_t fill_buffer_and_get_md5(std::vector<uint8_t>& buffer);
    static utility::string_t fill_buffer_and_get_md5(std::vector<uint8_t>& buffer, size_t offset, size_t count);
    static utility::string_t get_random_container_name(size_t length = 10);
    static void check_parallelism(const wa::storage::operation_context& context, int expected_parallelism);
    static void check_blob_equal(const wa::storage::cloud_blob& expected, const wa::storage::cloud_blob& actual);
    static void check_blob_copy_state_equal(const wa::storage::copy_state& expected, const wa::storage::copy_state& actual);
    static void check_blob_properties_equal(const wa::storage::cloud_blob_properties& expected, const wa::storage::cloud_blob_properties& actual);

    std::vector<wa::storage::cloud_blob_container> list_all_containers(const utility::string_t& prefix, const wa::storage::container_listing_includes& includes, int max_results, const wa::storage::blob_request_options& options);
    std::vector<wa::storage::cloud_blob> list_all_blobs_from_client(const utility::string_t& prefix, const wa::storage::blob_listing_includes& includes, int max_results, const wa::storage::blob_request_options& options);

    wa::storage::cloud_blob_client m_client;
};

class blob_service_test_base_with_objects_to_delete : public blob_service_test_base
{
public:
    
    blob_service_test_base_with_objects_to_delete()
    {
    }

    ~blob_service_test_base_with_objects_to_delete()
    {
        for (auto iter = m_blobs_to_delete.begin(); iter != m_blobs_to_delete.end(); ++iter)
        {
            try
            {
                iter->delete_blob();
            }
            catch (const wa::storage::storage_exception&)
            {
            }
        }

        for (auto iter = m_containers_to_delete.begin(); iter != m_containers_to_delete.end(); ++iter)
        {
            try
            {
                iter->delete_container();
            }
            catch (const wa::storage::storage_exception&)
            {
            }
        }
    }

protected:
    
    std::vector<wa::storage::cloud_blob> m_blobs_to_delete;
    std::vector<wa::storage::cloud_blob_container> m_containers_to_delete;
};

class container_test_base : public blob_service_test_base
{
public:
    container_test_base()
    {
        m_container = m_client.get_container_reference(get_random_container_name());
    }

    ~container_test_base()
    {
        try
        {
            m_container.delete_container(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        }
        catch (const wa::storage::storage_exception&)
        {
        }
    }

protected:

    void check_public_access(wa::storage::blob_container_public_access_type access);
    std::vector<wa::storage::cloud_blob> list_all_blobs(const utility::string_t& prefix, const wa::storage::blob_listing_includes& includes, int max_results, const wa::storage::blob_request_options& options);
    void check_lease_access(wa::storage::cloud_blob_container& container, wa::storage::lease_state state, const utility::string_t& lease_id, bool fake, bool allow_delete);

    wa::storage::cloud_blob_container m_container;
};

class blob_test_base : public container_test_base
{
public:
    blob_test_base()
    {
        m_container.create(wa::storage::blob_container_public_access_type::off, wa::storage::blob_request_options(), m_context);
    }

    ~blob_test_base()
    {
    }

protected:

    bool wait_for_copy(wa::storage::cloud_blob& blob);
    static wa::storage::operation_context upload_and_download(wa::storage::cloud_blob& blob, size_t buffer_size, size_t buffer_offset, size_t blob_size, bool use_seekable_stream, const wa::storage::blob_request_options& options, size_t expected_request_count, bool expect_md5_header);
    void check_access(const utility::string_t& sas_token, uint8_t permissions, const wa::storage::cloud_blob_shared_access_headers& headers, const wa::storage::cloud_blob& original_blob);
    void check_lease_access(wa::storage::cloud_blob& blob, wa::storage::lease_state state, const utility::string_t& lease_id, bool fake);
};

class block_blob_test_base : public blob_test_base
{
public:
    block_blob_test_base()
    {
        m_blob = m_container.get_block_blob_reference(U("blockblob"));
    }

    ~block_blob_test_base()
    {
    }

protected:

    static utility::string_t get_block_id(uint16_t block_index);
    void check_block_list_equal(const std::vector<wa::storage::block_list_item>& committed_put_block_list, const std::vector<wa::storage::block_list_item>& uncommitted_put_block_list);

    wa::storage::cloud_block_blob m_blob;
};

class page_blob_test_base : public blob_test_base
{
public:
    page_blob_test_base()
    {
        m_blob = m_container.get_page_blob_reference(U("pageblob"));
    }

    ~page_blob_test_base()
    {
    }

protected:

    void check_page_ranges_equal(const std::vector<wa::storage::page_range>& page_ranges);

    wa::storage::cloud_page_blob m_blob;
};
