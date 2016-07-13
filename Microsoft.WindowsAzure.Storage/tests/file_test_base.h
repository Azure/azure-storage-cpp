// -----------------------------------------------------------------------------------------
// <copyright file="file_test_base.h" company="Microsoft">
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
#include "was/file.h"
#include "was/blob.h"

extern const utility::string_t dummy_md5;//(_XPLATSTR("MDAwMDAwMDA="));

class file_service_test_base : public test_base
{
public:

    file_service_test_base()
        : m_client(test_config::instance().account().create_cloud_file_client())
    {
    }

    ~file_service_test_base()
    {
    }

    void check_equal(const azure::storage::cloud_file_share& source, const azure::storage::cloud_file_share& target);
    void check_equal(const azure::storage::cloud_file_directory& source, const azure::storage::cloud_file_directory& target);
    void check_equal(const azure::storage::cloud_file& source, const azure::storage::cloud_file& target);

protected:

    static utility::string_t fill_buffer_and_get_md5(std::vector<uint8_t>& buffer);
    static utility::string_t fill_buffer_and_get_md5(std::vector<uint8_t>& buffer, size_t offset, size_t count);
    static utility::string_t get_random_share_name(size_t length = 10);

    std::vector<azure::storage::cloud_file_share> list_all_shares(const utility::string_t& prefix, bool get_metadata, int max_results, const azure::storage::file_request_options& options);

    azure::storage::cloud_file_client m_client;
};

class file_service_test_base_with_objects_to_delete : public file_service_test_base
{

public:

    file_service_test_base_with_objects_to_delete()
    {
    }

    ~file_service_test_base_with_objects_to_delete()
    {
        for (auto iter = m_shares_to_delete.begin(); iter != m_shares_to_delete.end(); ++iter)
        {
            try
            {
                iter->delete_share();
            }
            catch (const azure::storage::storage_exception&)
            {
            }
        }
    }

protected:
    void create_share(const utility::string_t& prefix, std::size_t num);
    void check_share_list(const std::vector<azure::storage::cloud_file_share>& list, const utility::string_t& prefix, bool check_found);

    std::vector<azure::storage::cloud_file_share> m_shares_to_delete;
};

class file_share_test_base : public file_service_test_base
{
public:
    file_share_test_base()
    {
        m_share = m_client.get_share_reference(get_random_share_name());
    }

    ~file_share_test_base()
    {
        try
        {
            m_share.delete_share();
        }
        catch (const azure::storage::storage_exception&)
        {
        }
    }

    void check_access(const utility::string_t& sas_token, uint8_t permissions, const azure::storage::cloud_file_shared_access_headers& headers, const azure::storage::cloud_file& original_file);

protected:
    static utility::string_t get_random_directory_name(size_t length = 10);

    azure::storage::cloud_file_share m_share;
};

class file_directory_test_base : public file_share_test_base
{
public:
    file_directory_test_base()
    {
        m_share.create_if_not_exists();
        m_directory = m_share.get_directory_reference(get_random_string());
    }

    ~file_directory_test_base()
    {
    }

protected:
    static utility::string_t get_random_file_name(size_t length = 10);

    azure::storage::cloud_file_directory m_directory;
};

class file_test_base : public file_directory_test_base
{
public:
    file_test_base()
    {
        m_directory.create_if_not_exists();
        m_file = m_directory.get_file_reference(get_random_string());
    }

    ~file_test_base()
    {
    }

protected:
    
    bool wait_for_copy(azure::storage::cloud_file& file);

    azure::storage::cloud_file m_file;
};
