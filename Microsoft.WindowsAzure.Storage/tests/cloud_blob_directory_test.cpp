// -----------------------------------------------------------------------------------------
// <copyright file="cloud_blob_directory_test.cpp" company="Microsoft">
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
#include "blob_test_base.h"
#include "check_macros.h"

const utility::string_t delimiters[] = { U("$"), U("@"), U("-"), U("%"), U("/"), U("||") };

#pragma region Fixture

void create_blob_tree(const wa::storage::cloud_blob_container& container, const std::vector<wa::storage::cloud_blob>& blobs, const utility::string_t& delimiter, wa::storage::operation_context context)
{
    for (auto iter = blobs.begin(); iter != blobs.end(); ++iter)
    {
        utility::string_t name(iter->name());
        std::vector<utility::string_t> splitted_name;
        utility::string_t::size_type pos(0);

        pos -= delimiter.size();
        do
        {
            name = name.substr(pos + delimiter.size());
            pos = name.find(delimiter);
            splitted_name.push_back(name.substr(0, pos));
        } while (pos != utility::string_t::npos);

        wa::storage::cloud_blob blob;
        if (splitted_name.size() == 1)
        {
            blob = container.get_blob_reference(splitted_name.back());
        }
        else
        {
            auto directory = container.get_directory_reference(splitted_name[0]);
            for (size_t i = 1; i < splitted_name.size() - 1; i++)
            {
                directory = directory.get_subdirectory_reference(splitted_name[i]);
            }

            blob = directory.get_blob_reference(splitted_name.back());
        }

        if (iter->type() == wa::storage::blob_type::page_blob)
        {
            wa::storage::cloud_page_blob page_blob(blob);
            page_blob.create(0, wa::storage::access_condition(), wa::storage::blob_request_options(), context);
        }
        else
        {
            wa::storage::cloud_block_blob block_blob(blob);
            block_blob.upload_block_list(std::vector<wa::storage::block_list_item>(), wa::storage::access_condition(), wa::storage::blob_request_options(), context);
        }
    }
}

int list_entire_blob_tree_helper(std::vector<wa::storage::cloud_blob>& blobs, const wa::storage::cloud_blob_directory& directory, const wa::storage::blob_listing_includes& includes, int max_results, const wa::storage::blob_request_options& options, wa::storage::operation_context context)
{
    wa::storage::blob_continuation_token token;
    int max_depth = 0;

    do
    {
        auto results = directory.list_blobs_segmented(false, includes, max_results, token, options, context);

        if (max_results > 0)
        {
            CHECK((results.blobs().size() + results.directories().size()) <= static_cast<size_t>(max_results));
        }

        std::copy(results.blobs().begin(), results.blobs().end(), std::back_inserter(blobs));

        for (auto iter = results.directories().begin(); iter != results.directories().end(); ++iter)
        {
            int depth = list_entire_blob_tree_helper(blobs, *iter, includes, max_results, options, context);
            if (depth > max_depth)
            {
                max_depth = depth;
            }
        }

        token = results.continuation_token();
    } while (!token.empty());

    return max_depth + 1;
}

std::vector<wa::storage::cloud_blob> list_entire_blob_tree(const wa::storage::cloud_blob_container& container, const wa::storage::blob_listing_includes& includes, int max_results, int& max_depth, const wa::storage::blob_request_options& options, wa::storage::operation_context context)
{
    std::vector<wa::storage::cloud_blob> blobs;
    wa::storage::blob_continuation_token token;
    max_depth = 0;

    do
    {
        auto results = container.list_blobs_segmented(utility::string_t(), false, includes, max_results, token, options, context);

        if (max_results > 0)
        {
            CHECK((results.blobs().size() + results.directories().size()) <= static_cast<size_t>(max_results));
        }

        std::copy(results.blobs().begin(), results.blobs().end(), std::back_inserter(blobs));

        for (auto iter = results.directories().begin(); iter != results.directories().end(); ++iter)
        {
            int depth = list_entire_blob_tree_helper(blobs, *iter, includes, max_results, options, context);
            if (depth > max_depth)
            {
                max_depth = depth;
            }
        }

        token = results.continuation_token();
    } while (!token.empty());

    return blobs;
}

void check_parents(const wa::storage::cloud_blob_container& container, const utility::string_t& delimiter)
{
    auto dir1 = container.get_directory_reference(U("dir1"));
    CHECK_UTF8_EQUAL(U("dir1") + delimiter, dir1.prefix());

    auto dir2 = dir1.get_subdirectory_reference(U("dir2"));
    CHECK_UTF8_EQUAL(U("dir1") + delimiter + U("dir2") + delimiter, dir2.prefix());

    auto dir3 = dir2.get_subdirectory_reference(U("dir3"));
    CHECK_UTF8_EQUAL(U("dir1") + delimiter + U("dir2") + delimiter + U("dir3") + delimiter, dir3.prefix());

    auto blob = dir3.get_blob_reference(U("blob"));
    CHECK_UTF8_EQUAL(U("dir1") + delimiter + U("dir2") + delimiter + U("dir3") + delimiter + U("blob"), blob.name());

    auto block_blob = dir3.get_block_blob_reference(U("block_blob"));
    CHECK(wa::storage::blob_type::block_blob == block_blob.type());
    CHECK_UTF8_EQUAL(U("dir1") + delimiter + U("dir2") + delimiter + U("dir3") + delimiter + U("block_blob"), block_blob.name());

    auto page_blob = dir3.get_page_blob_reference(U("page_blob"));
    CHECK(wa::storage::blob_type::page_blob == page_blob.type());
    CHECK_UTF8_EQUAL(U("dir1") + delimiter + U("dir2") + delimiter + U("dir3") + delimiter + U("page_blob"), page_blob.name());

    auto blob_parent = blob.get_parent_reference();
    CHECK_UTF8_EQUAL(dir3.prefix(), blob_parent.prefix());

    auto dir3_parent = blob_parent.get_parent_reference();
    CHECK_UTF8_EQUAL(dir2.prefix(), dir3_parent.prefix());

    auto dir2_parent = dir3_parent.get_parent_reference();
    CHECK_UTF8_EQUAL(dir1.prefix(), dir2_parent.prefix());

    auto dir1_parent = dir2_parent.get_parent_reference();
    CHECK(!dir1_parent.is_valid());

    auto root_blob = container.get_blob_reference(U("blob"));
    CHECK_UTF8_EQUAL(U("blob"), root_blob.name());

    auto root_blob_parent = root_blob.get_parent_reference();
    CHECK(!root_blob_parent.is_valid());
}

#pragma endregion

SUITE(Blob)
{
    TEST_FIXTURE(blob_test_base, directory_parent)
    {
        for (auto delimiter : delimiters)
        {
            m_client.set_directory_delimiter(delimiter);
            auto container = m_client.get_container_reference(m_container.name());
            check_parents(container, delimiter);
        }
    }

    TEST_FIXTURE(blob_service_test_base, directory_parent_in_root_container)
    {
        for (auto delimiter : delimiters)
        {
            m_client.set_directory_delimiter(delimiter);
            auto container = m_client.get_root_container_reference();
            check_parents(container, delimiter);
        }
    }

    TEST_FIXTURE(blob_test_base, directory_tree_listing)
    {
        for (auto delimiter : delimiters)
        {
            m_client.set_directory_delimiter(delimiter);
            auto container = m_client.get_container_reference(m_container.name());

            std::vector<wa::storage::cloud_blob> blobs;
            blobs.push_back(container.get_block_blob_reference(U("block_blobs") + delimiter + U("dir1") + delimiter + U("dir2") + delimiter + U("blob1")));
            blobs.push_back(container.get_block_blob_reference(U("block_blobs") + delimiter + U("dir1") + delimiter + U("dir2") + delimiter + U("blob2")));
            blobs.push_back(container.get_block_blob_reference(U("block_blobs") + delimiter + U("dir1") + delimiter + U("blob3")));
            blobs.push_back(container.get_block_blob_reference(U("block_blobs") + delimiter + U("dir1") + delimiter + U("blob4")));
            blobs.push_back(container.get_block_blob_reference(U("block_blobs") + delimiter + U("blob5")));
            blobs.push_back(container.get_page_blob_reference(U("page_blobs") + delimiter + U("dir1") + delimiter + U("dir2") + delimiter + U("blob6")));
            blobs.push_back(container.get_page_blob_reference(U("page_blobs") + delimiter + U("blob7")));

            create_blob_tree(container, blobs, delimiter, m_context);

            int depth;
            auto results = list_entire_blob_tree(container, wa::storage::blob_listing_includes(), 2, depth, wa::storage::blob_request_options(), m_context);
            CHECK_EQUAL(3, depth);

            for (auto iter = results.begin(); iter != results.end(); ++iter)
            {
                bool found = false;
                for (auto blobs_iter = blobs.begin(); blobs_iter != blobs.end(); ++blobs_iter)
                {
                    if (blobs_iter->name() == iter->name())
                    {
                        CHECK(blobs_iter->type() == iter->type());
                        blobs.erase(blobs_iter);
                        found = true;
                        break;
                    }
                }

                CHECK(found);

                iter->delete_blob(wa::storage::delete_snapshots_option::none, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
            }

            CHECK(blobs.empty());

            wa::storage::blob_listing_includes includes;
            includes.set_snapshots(true);
            CHECK_THROW(list_entire_blob_tree(container, includes, 1, depth, wa::storage::blob_request_options(), m_context), std::invalid_argument);
        }
    }
}
