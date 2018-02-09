// -----------------------------------------------------------------------------------------
// <copyright file="cloud_file_directory_test.cpp" company="Microsoft">
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
#include "file_test_base.h"
#include "check_macros.h"

#pragma region Fixture

#pragma endregion

SUITE(File)
{
    TEST_FIXTURE(file_directory_test_base, directory_create_delete)
    {
        CHECK(!m_directory.exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));
        CHECK(!m_directory.delete_directory_if_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));
    
        CHECK(m_directory.create_if_not_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));
        CHECK(!m_directory.create_if_not_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));

        CHECK(m_directory.exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));
        
        CHECK(m_directory.delete_directory_if_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));
        CHECK(!m_directory.delete_directory_if_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));
        
        CHECK(!m_directory.exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));
    }

    TEST_FIXTURE(file_directory_test_base, directory_metadata)
    {
        auto value1 = this->get_random_string();
        auto value2 = this->get_random_string();
        auto value3 = this->get_random_string();
        auto value4 = this->get_random_string();

        // create 2 pairs
        m_directory.metadata()[_XPLATSTR("key1")] = value1;
        m_directory.metadata()[_XPLATSTR("key2")] = value2;
        m_directory.create_if_not_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_UTF8_EQUAL(m_directory.metadata()[_XPLATSTR("key1")], value1);
        CHECK_UTF8_EQUAL(m_directory.metadata()[_XPLATSTR("key2")], value2);

        auto same_directory = m_directory.get_parent_share_reference().get_directory_reference(m_directory.name());
        CHECK(same_directory.metadata().empty());
        same_directory.download_attributes();
        CHECK_EQUAL(2U, same_directory.metadata().size());
        CHECK_UTF8_EQUAL(value1, same_directory.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(value2, same_directory.metadata()[_XPLATSTR("key2")]);

        // add 1 pair
        m_directory.metadata()[_XPLATSTR("key3")] = value3;
        m_directory.upload_metadata(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        same_directory.metadata().clear();
        CHECK(same_directory.metadata().empty());
        same_directory.download_attributes();
        CHECK_EQUAL(3U, same_directory.metadata().size());
        CHECK_UTF8_EQUAL(value1, same_directory.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(value2, same_directory.metadata()[_XPLATSTR("key2")]);
        CHECK_UTF8_EQUAL(value3, same_directory.metadata()[_XPLATSTR("key3")]);

        // overwrite with 1 pair
        m_directory.metadata().clear();
        m_directory.metadata()[_XPLATSTR("key4")] = value4;
        m_directory.upload_metadata(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        same_directory.metadata().clear();
        CHECK(same_directory.metadata().empty());
        same_directory.download_attributes();
        CHECK_EQUAL(1U, same_directory.metadata().size());
        CHECK_UTF8_EQUAL(value4, same_directory.metadata()[_XPLATSTR("key4")]);

        // clear metadata
        m_directory.metadata().clear();
        m_directory.upload_metadata(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        same_directory.metadata().clear();
        CHECK(same_directory.metadata().empty());
        same_directory.download_attributes();
        CHECK_EQUAL(0U, same_directory.metadata().size());
    }

    TEST_FIXTURE(file_directory_test_base, directory_attributes)
    {
        CHECK(m_directory.get_parent_share_reference().is_valid());
        check_equal(m_share, m_directory.get_parent_share_reference());
        CHECK(!m_directory.uri().primary_uri().is_empty());
        CHECK(m_directory.metadata().empty());
        CHECK(m_directory.properties().etag().empty());
        CHECK(!m_directory.properties().last_modified().is_initialized());
        CHECK(!m_directory.properties().server_encrypted());

        m_directory.create_if_not_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        m_directory.download_attributes();
        CHECK(m_directory.properties().server_encrypted());

        CHECK(m_directory.get_parent_share_reference().is_valid());
        check_equal(m_share, m_directory.get_parent_share_reference());
        CHECK(!m_directory.uri().primary_uri().is_empty());
        CHECK(m_directory.metadata().empty());
        CHECK(!m_directory.properties().etag().empty());
        CHECK(m_directory.properties().last_modified().is_initialized());
    }

    TEST_FIXTURE(file_directory_test_base, directory_list_files_and_directories)
    {
        m_directory.create_if_not_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        azure::storage::list_file_and_diretory_result_iterator end_of_list;
        for (auto iter = m_share.get_root_directory_reference().list_files_and_directories(); iter != end_of_list; ++iter)
        {
            CHECK_EQUAL(iter->is_directory(), true);
            CHECK_EQUAL(iter->is_file(), false);
            auto directory = iter->as_directory();

            check_equal(directory, m_directory);

            CHECK(directory.get_parent_share_reference().is_valid());
            check_equal(m_share, directory.get_parent_share_reference());

            CHECK(!directory.uri().primary_uri().is_empty());
            CHECK(directory.metadata().empty());
            CHECK(directory.properties().etag().empty());

            CHECK(!directory.properties().last_modified().is_initialized());
        }

        // more complicated file structure.
        const size_t size = 2;
        std::vector<utility::string_t> directories;
        std::vector<utility::string_t> files;
        for (size_t i = 0; i < size; ++i)
        {
            directories.push_back(_XPLATSTR("directory") + get_random_string(10));
        }
        for (size_t i = 0; i < size; ++i)
        {
            files.push_back(_XPLATSTR("file") + get_random_string(10));
        }
        for (size_t i = 0; i < size; ++i)
        {
            auto directory = m_share.get_directory_reference(directories[i]);
            directory.create_if_not_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

            for (size_t j = 0; j < size; ++j)
            {
                auto subdirectory = directory.get_subdirectory_reference(directories[j]);
                subdirectory.create_if_not_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
                for (size_t k = 0; k < size; ++k)
                {
                    auto file = subdirectory.get_file_reference(files[k]);
                    file.create_if_not_exists(512U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
                }
            }
            for (size_t j = 0; j < size; ++j)
            {
                auto file = directory.get_file_reference(files[j]);
                file.create_if_not_exists(512U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
            }
            auto file = m_share.get_root_directory_reference().get_file_reference(files[i]);
            file.create_if_not_exists(512U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        }

        auto direcotries_one = directories;
        auto files_one = files;
        for (auto iter = m_share.get_root_directory_reference().list_files_and_directories(); iter != end_of_list; ++iter)
        {
            if (iter->is_directory())
            {
                auto directory2 = iter->as_directory();
                CHECK(directory2.get_parent_share_reference().is_valid());
                check_equal(m_share, directory2.get_parent_share_reference());
                CHECK(!directory2.uri().primary_uri().is_empty());
                CHECK(directory2.metadata().empty());
                CHECK(directory2.properties().etag().empty());
                CHECK(!directory2.properties().last_modified().is_initialized());

                auto found = false;
                for (auto directory_name = direcotries_one.begin(); directory_name != direcotries_one.end(); directory_name++)
                {
                    if (*directory_name == directory2.name())
                    {
                        direcotries_one.erase(directory_name);
                        found = true;
                        break;
                    }
                }

                auto direcotries_two = directories;
                auto files_two = files;
                for (auto iter2 = directory2.list_files_and_directories(); found && iter2 != end_of_list; ++iter2)
                {
                    if (iter2->is_directory())
                    {
                        auto directory3 = iter2->as_directory();
                        CHECK(directory3.get_parent_share_reference().is_valid());
                        check_equal(m_share, directory3.get_parent_share_reference());
                        CHECK(!directory3.uri().primary_uri().is_empty());
                        CHECK(directory3.metadata().empty());
                        CHECK(directory3.properties().etag().empty());
                        CHECK(!directory3.properties().last_modified().is_initialized());

                        for (auto directory_name = direcotries_two.begin(); directory_name != direcotries_two.end(); directory_name++)
                        {
                            if (*directory_name == directory3.name())
                            {
                                direcotries_two.erase(directory_name);
                                break;
                            }
                        }

                        auto files_three = files;
                        for (auto iter3 = directory3.list_files_and_directories(); iter3 != end_of_list; ++iter3)
                        {
                            CHECK(iter3->is_file());
                            auto file = iter3->as_file();
                            CHECK(file.get_parent_share_reference().is_valid());
                            check_equal(m_share, file.get_parent_share_reference());
                            CHECK(!file.uri().primary_uri().is_empty());
                            CHECK(file.metadata().empty());
                            CHECK(file.properties().etag().empty());
                            CHECK(!file.properties().last_modified().is_initialized());

                            for (auto file_name = files_three.begin(); file_name != files_three.end(); file_name++)
                            {
                                if (*file_name == file.name())
                                {
                                    files_three.erase(file_name);
                                    break;
                                }
                            }
                        }
                        CHECK(files_three.empty());
                    }
                    else if (iter2->is_file())
                    {
                        auto file = iter2->as_file();
                        CHECK(file.get_parent_share_reference().is_valid());
                        check_equal(m_share, file.get_parent_share_reference());
                        CHECK(!file.uri().primary_uri().is_empty());
                        CHECK(file.metadata().empty());
                        CHECK(file.properties().etag().empty());
                        CHECK(!file.properties().last_modified().is_initialized());

                        for (auto file_name = files_two.begin(); file_name != files_two.end(); file_name++)
                        {
                            if (*file_name == file.name())
                            {
                                files_two.erase(file_name);
                                break;
                            }
                        }
                    }

                }
                CHECK(!found || direcotries_two.empty());
                CHECK(!found || files_two.empty());
            }
            else if (iter->is_file())
            {
                auto file = iter->as_file();
                CHECK(file.get_parent_share_reference().is_valid());
                check_equal(m_share, file.get_parent_share_reference());
                CHECK(!file.uri().primary_uri().is_empty());
                CHECK(file.metadata().empty());
                CHECK(file.properties().etag().empty());
                CHECK(!file.properties().last_modified().is_initialized());

                for (auto file_name = files_one.begin(); file_name != files_one.end(); file_name++)
                {
                    if (*file_name == file.name())
                    {
                        files_one.erase(file_name);
                        break;
                    }
                }
            }
        }

        CHECK(direcotries_one.empty());
        CHECK(files_one.empty());
    }

    TEST_FIXTURE(file_directory_test_base, directory_list_files_and_directories_with_prefix)
    {
        m_directory.create_if_not_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        auto prefix = _XPLATSTR("t") + get_random_string(3);
        auto dir_prefix = prefix + _XPLATSTR("dir");
        auto file_prefix = prefix + _XPLATSTR("file");
        auto exclude_prefix = _XPLATSTR("exclude");

        std::vector<azure::storage::cloud_file_directory> directories;
        std::vector<azure::storage::cloud_file> files;
        for (int i = 0; i < get_random_int32() % 3 + 1; ++i)
        {
            auto subdirectory = m_directory.get_subdirectory_reference(dir_prefix + utility::conversions::print_string(i));
            subdirectory.create();
            directories.push_back(subdirectory);

            auto file = m_directory.get_file_reference(file_prefix + utility::conversions::print_string(i));
            file.create(1);
            files.push_back(file);

            m_directory.get_subdirectory_reference(exclude_prefix + utility::conversions::print_string(i)).create();
        }

        int num_items_expected = directories.size() + files.size();
        int num_items_actual = 0;
        for (auto&& item : m_directory.list_files_and_directories(prefix))
        {
            ++num_items_actual;
            if (item.is_directory())
            {
                auto actual = item.as_directory();
                CHECK(actual.get_parent_share_reference().is_valid());
                check_equal(m_share, actual.get_parent_share_reference());
                
                auto it_found = std::find_if(directories.begin(), directories.end(), [&actual](const azure::storage::cloud_file_directory& expect)
                {
                    return actual.name() == expect.name();
                });
                CHECK(it_found != directories.end());
                check_equal(*it_found, actual);
                directories.erase(it_found);
            }
            else if (item.is_file())
            {
                auto actual = item.as_file();
                CHECK(actual.get_parent_share_reference().is_valid());
                check_equal(m_share, actual.get_parent_share_reference());

                auto it_found = std::find_if(files.begin(), files.end(), [&actual](const azure::storage::cloud_file& expect)
                {
                    return actual.name() == expect.name();
                });
                CHECK(it_found != files.end());
                check_equal(*it_found, actual);
                files.erase(it_found);
            }
        }

        CHECK_EQUAL(num_items_expected, num_items_actual);
        CHECK(directories.empty());
        CHECK(files.empty());
    }

    TEST_FIXTURE(file_directory_test_base, directory_get_directory_ref)
    {
        m_directory.create_if_not_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        auto directory_name = _XPLATSTR("directory");
        auto directory = m_directory.get_subdirectory_reference(directory_name);
        CHECK(directory.name() == directory_name);

        CHECK(directory.get_parent_share_reference().is_valid());
        check_equal(m_share, directory.get_parent_share_reference());

        check_equal(m_directory, directory.get_parent_directory_reference());

        CHECK(!directory.uri().primary_uri().is_empty());
        CHECK(directory.metadata().empty());
        CHECK(directory.properties().etag().empty());

        CHECK(!directory.properties().last_modified().is_initialized());

        // In root directory
        directory = m_share.get_root_directory_reference().get_subdirectory_reference(directory_name);
        CHECK(directory.name() == directory_name);

        CHECK(directory.get_parent_share_reference().is_valid());
        check_equal(m_share, directory.get_parent_share_reference());

        check_equal(m_share.get_root_directory_reference(), directory.get_parent_directory_reference());

        CHECK(!directory.uri().primary_uri().is_empty());
        CHECK(directory.metadata().empty());
        CHECK(directory.properties().etag().empty());

        CHECK(!directory.properties().last_modified().is_initialized());
    }

    TEST_FIXTURE(file_directory_test_base, directory_get_file_ref)
    {
        m_directory.create_if_not_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        auto file_name = _XPLATSTR("file");
        auto file = m_directory.get_file_reference(file_name);

        CHECK_UTF8_EQUAL(file_name, file.name());
        CHECK(file.get_parent_share_reference().is_valid());
        check_equal(m_share, file.get_parent_share_reference());

        check_equal(m_directory, file.get_parent_directory_reference());

        CHECK(!file.uri().primary_uri().is_empty());
        CHECK(file.metadata().empty());
        CHECK(file.properties().etag().empty());
        CHECK(file.properties().cache_control().empty());
        CHECK(file.properties().content_disposition().empty());
        CHECK(file.properties().content_encoding().empty());
        CHECK(file.properties().content_language().empty());
        CHECK(file.properties().content_md5().empty());
        CHECK(file.properties().content_type().empty());
        CHECK(file.properties().type().empty());
        CHECK_EQUAL(file.properties().length(), 0);

        CHECK(!file.properties().last_modified().is_initialized());

        // In root directory
        file = m_share.get_root_directory_reference().get_file_reference(file_name);

        CHECK_UTF8_EQUAL(file_name, file.name());
        CHECK(file.get_parent_share_reference().is_valid());
        check_equal(m_share, file.get_parent_share_reference());

        check_equal(m_share.get_root_directory_reference(), file.get_parent_directory_reference());

        CHECK(!file.uri().primary_uri().is_empty());
        CHECK(file.metadata().empty());
        CHECK(file.properties().etag().empty());
        CHECK(file.properties().cache_control().empty());
        CHECK(file.properties().content_disposition().empty());
        CHECK(file.properties().content_encoding().empty());
        CHECK(file.properties().content_language().empty());
        CHECK(file.properties().content_md5().empty());
        CHECK(file.properties().content_type().empty());
        CHECK(file.properties().type().empty());
        CHECK_EQUAL(file.properties().length(), 0);

        CHECK(!file.properties().last_modified().is_initialized());
    }

    TEST_FIXTURE(file_directory_test_base, directory_get_parent_directory_ref)
    {
        m_directory.create_if_not_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        auto directory_name = _XPLATSTR("directory");
        auto directory = m_directory.get_subdirectory_reference(directory_name);
        auto parent_directory = directory.get_parent_directory_reference();

        CHECK_UTF8_EQUAL(directory_name, directory.name());
        CHECK(directory.get_parent_share_reference().is_valid());
        check_equal(m_share, directory.get_parent_share_reference());

        check_equal(m_directory, parent_directory);

        CHECK(!directory.uri().primary_uri().is_empty());
        CHECK(directory.metadata().empty());
        CHECK(directory.properties().etag().empty());
        CHECK(!directory.properties().last_modified().is_initialized());

        CHECK(!parent_directory.uri().primary_uri().is_empty());
        CHECK(parent_directory.metadata().empty());
        CHECK(parent_directory.properties().etag().empty());
        CHECK(!parent_directory.properties().last_modified().is_initialized());

        // Check if get_root_directory_refence works for root directory's sub-directory.
        auto root_direcotry = m_share.get_root_directory_reference();
        directory_name = _XPLATSTR("directory");
        directory = root_direcotry.get_subdirectory_reference(directory_name);
        parent_directory = directory.get_parent_directory_reference();

        CHECK_UTF8_EQUAL(directory_name, directory.name());
        CHECK(directory.get_parent_share_reference().is_valid());
        check_equal(m_share, directory.get_parent_share_reference());

        check_equal(root_direcotry, parent_directory);

        CHECK(!directory.uri().primary_uri().is_empty());
        CHECK(directory.metadata().empty());
        CHECK(directory.properties().etag().empty());
        CHECK(!directory.properties().last_modified().is_initialized());

        CHECK(!parent_directory.uri().primary_uri().is_empty());
        CHECK(parent_directory.metadata().empty());
        CHECK(parent_directory.properties().etag().empty());
        CHECK(!parent_directory.properties().last_modified().is_initialized());

        // Check if get_root_directory_refence works for root directory.
        root_direcotry = m_share.get_root_directory_reference();
        directory = root_direcotry.get_parent_directory_reference();

        check_equal(root_direcotry, parent_directory);
    }
}