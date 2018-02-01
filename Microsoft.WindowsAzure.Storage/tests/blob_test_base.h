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

#include "cpprest/filestream.h"

#include "test_base.h"
#include "was/blob.h"

const utility::string_t dummy_md5(_XPLATSTR("MDAwMDAwMDA="));

class blob_service_test_base : public test_base
{
public:

    blob_service_test_base()
        : m_client(test_config::instance().account().create_cloud_blob_client()),
        m_premium_client(test_config::instance().premium_account().create_cloud_blob_client()),
        m_blob_storage_client(test_config::instance().blob_storage_account().create_cloud_blob_client())
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
    static void check_blob_equal(const azure::storage::cloud_blob& expected, const azure::storage::cloud_blob& actual);
    static void check_blob_copy_state_equal(const azure::storage::copy_state& expected, const azure::storage::copy_state& actual);
    static void check_blob_properties_equal(const azure::storage::cloud_blob_properties& expected, const azure::storage::cloud_blob_properties& actual, bool check_settable_only = false);

    std::vector<azure::storage::cloud_blob_container> list_all_containers(const utility::string_t& prefix, azure::storage::container_listing_details::values includes, int max_results, const azure::storage::blob_request_options& options);
    std::vector<azure::storage::cloud_blob> list_all_blobs_from_client(const utility::string_t& prefix, azure::storage::blob_listing_details::values includes, int max_results, const azure::storage::blob_request_options& options);

    azure::storage::cloud_blob_client m_client;
    azure::storage::cloud_blob_client m_premium_client;
    azure::storage::cloud_blob_client m_blob_storage_client;
};

class temp_file : public blob_service_test_base
{
public:

    temp_file(size_t file_size)
    {
        m_path = get_random_container_name(8);

        std::vector<uint8_t> buffer;
        buffer.resize(file_size);
        m_content_md5 = fill_buffer_and_get_md5(buffer);

        auto stream = concurrency::streams::file_stream<uint8_t>::open_ostream(m_path).get();
        stream.streambuf().putn_nocopy(buffer.data(), buffer.size()).wait();
        stream.close().wait();
    }

    ~temp_file()
    {
        std::remove(utility::conversions::to_utf8string(m_path).c_str());
    }

    const utility::string_t& path() const
    {
        return m_path;
    }

    const utility::string_t& content_md5() const
    {
        return m_content_md5;
    }

private:

    utility::string_t m_path;
    utility::string_t m_content_md5;
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
            catch (const azure::storage::storage_exception&)
            {
            }
        }

        for (auto iter = m_containers_to_delete.begin(); iter != m_containers_to_delete.end(); ++iter)
        {
            try
            {
                iter->delete_container();
            }
            catch (const azure::storage::storage_exception&)
            {
            }
        }
    }

protected:
    void create_containers(const utility::string_t& prefix, std::size_t num, azure::storage::blob_container_public_access_type public_access_type = azure::storage::blob_container_public_access_type::off);
    void create_blobs(const azure::storage::cloud_blob_container& container, const utility::string_t& prefix, std::size_t num);
    void check_container_list(const std::vector<azure::storage::cloud_blob_container>& list, const utility::string_t& prefix, bool check_found);
    void check_blob_list(const std::vector<azure::storage::cloud_blob>& list);

    std::vector<azure::storage::cloud_blob> m_blobs_to_delete;
    std::vector<azure::storage::cloud_blob_container> m_containers_to_delete;
};

class container_test_base : public blob_service_test_base
{
public:

    container_test_base()
    {
        m_container = m_client.get_container_reference(get_random_container_name());
        m_premium_container = m_premium_client.get_container_reference(get_random_container_name());/* manage create and delete in test case since it's not for all test cases*/
        m_blob_storage_container = m_blob_storage_client.get_container_reference(get_random_container_name());/* manage create and delete in test case since it's not for all test cases*/
    }

    ~container_test_base()
    {
        try
        {
            m_container.delete_container_if_exists(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        }
        catch (const azure::storage::storage_exception&)
        {
        }
    }

protected:

    void check_public_access(azure::storage::blob_container_public_access_type access);
    std::vector<azure::storage::cloud_blob> list_all_blobs(const utility::string_t& prefix, azure::storage::blob_listing_details::values includes, int max_results, const azure::storage::blob_request_options& options);
    std::vector<azure::storage::cloud_blob> list_all_blobs(const azure::storage::cloud_blob_container & container, const utility::string_t & prefix, azure::storage::blob_listing_details::values includes, int max_results, const azure::storage::blob_request_options & options);
    void check_lease_access(azure::storage::cloud_blob_container& container, azure::storage::lease_state state, const utility::string_t& lease_id, bool fake, bool allow_delete);
    static void check_container_no_stale_property(azure::storage::cloud_blob_container& container);

    azure::storage::cloud_blob_container m_container;
    azure::storage::cloud_blob_container m_premium_container;
    azure::storage::cloud_blob_container m_blob_storage_container;
};

class blob_test_base : public container_test_base
{
public:

    blob_test_base()
    {
        m_container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);
    }

    ~blob_test_base()
    {
    }

protected:

    bool wait_for_copy(azure::storage::cloud_blob& blob);
    static azure::storage::operation_context upload_and_download(azure::storage::cloud_blob& blob, size_t buffer_size, size_t buffer_offset, size_t blob_size, bool use_seekable_stream, const azure::storage::blob_request_options& options, size_t expected_request_count, bool expect_md5_header);
    void check_access(const utility::string_t& sas_token, uint8_t permissions, const azure::storage::cloud_blob_shared_access_headers& headers, const azure::storage::cloud_blob& original_blob);
    void check_lease_access(azure::storage::cloud_blob& blob, azure::storage::lease_state state, const utility::string_t& lease_id, bool fake);
    static void check_blob_no_stale_property(azure::storage::cloud_blob& blob);
};

class block_blob_test_base : public blob_test_base
{
public:

    block_blob_test_base()
    {
        m_blob = m_container.get_block_blob_reference(_XPLATSTR("blockblob"));
    }

    ~block_blob_test_base()
    {
    }

protected:

    static utility::string_t get_block_id(uint16_t block_index);
    void check_block_list_equal(const std::vector<azure::storage::block_list_item>& committed_put_block_list, const std::vector<azure::storage::block_list_item>& uncommitted_put_block_list);

    azure::storage::cloud_block_blob m_blob;
};

class page_blob_test_base : public blob_test_base
{
public:

    page_blob_test_base()
    {
        m_blob = m_container.get_page_blob_reference(_XPLATSTR("pageblob"));
    }

    ~page_blob_test_base()
    {
    }

protected:

    void check_page_ranges_equal(const std::vector<azure::storage::page_range>& page_ranges);

    azure::storage::cloud_page_blob m_blob;
};

class append_blob_test_base : public blob_test_base
{
public:

    append_blob_test_base()
    {
        m_blob = m_container.get_append_blob_reference(_XPLATSTR("appendblob"));
    }

    ~append_blob_test_base()
    {
    }

protected:

    azure::storage::cloud_append_blob m_blob;
};
