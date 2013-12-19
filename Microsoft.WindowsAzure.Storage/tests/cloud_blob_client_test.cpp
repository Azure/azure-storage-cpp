// -----------------------------------------------------------------------------------------
// <copyright file="cloud_blob_client_test.cpp" company="Microsoft">
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

#pragma region Fixture

std::vector<wa::storage::cloud_blob_container> blob_service_test_base::list_all_containers(const utility::string_t& prefix, const wa::storage::container_listing_includes& includes, int max_results, const wa::storage::blob_request_options& options)
{
    std::vector<wa::storage::cloud_blob_container> containers;
    wa::storage::blob_continuation_token token;

    do
    {
        auto results = m_client.list_containers_segmented(prefix, includes, max_results, token, options, m_context);

        if (max_results > 0)
        {
            CHECK(results.results().size() <= static_cast<size_t>(max_results));
        }

        std::copy(results.results().begin(), results.results().end(), std::back_inserter(containers));
        token = results.continuation_token();
    } while (!token.empty());

    return containers;
}

std::vector<wa::storage::cloud_blob> blob_service_test_base::list_all_blobs_from_client(const utility::string_t& prefix, const wa::storage::blob_listing_includes& includes, int max_results, const wa::storage::blob_request_options& options)
{
    std::vector<wa::storage::cloud_blob> blobs;
    wa::storage::blob_continuation_token token;

    do
    {
        auto results = m_client.list_blobs_segmented(prefix, true, includes, max_results, token, options, m_context);

        if (max_results > 0)
        {
            CHECK(results.blobs().size() <= static_cast<size_t>(max_results));
        }

        std::copy(results.blobs().begin(), results.blobs().end(), std::back_inserter(blobs));
        token = results.continuation_token();
    } while (!token.empty());

    return blobs;
}

#pragma endregion

SUITE(Blob)
{
    TEST_FIXTURE(blob_service_test_base_with_objects_to_delete, list_containers_with_prefix)
    {
        auto prefix = get_random_container_name();

        for (int i = 0; i < 1; i++)
        {
            auto index = utility::conversions::print_string(i);
            auto container = m_client.get_container_reference(prefix + index);
            m_containers_to_delete.push_back(container);
            container.metadata()[U("index")] = index;
            container.create(wa::storage::blob_container_public_access_type::off, wa::storage::blob_request_options(), m_context);
        }

        std::vector<wa::storage::cloud_blob_container> containers(m_containers_to_delete);

        auto listing = list_all_containers(prefix, wa::storage::container_listing_includes::all(), 1, wa::storage::blob_request_options());
        for (auto listing_iter = listing.begin(); listing_iter != listing.end(); ++listing_iter)
        {
            bool found = false;
            for (auto iter = containers.begin(); iter != containers.end(); ++iter)
            {
                if (iter->name() == listing_iter->name())
                {
                    auto index_str = listing_iter->metadata().find(U("index"));
                    CHECK(index_str != listing_iter->metadata().end());
                    CHECK_UTF8_EQUAL(iter->name(), prefix + index_str->second);
                    containers.erase(iter);
                    found = true;
                    break;
                }
            }

            CHECK(found);
        }

        CHECK(containers.empty());
    }

    TEST_FIXTURE(blob_service_test_base_with_objects_to_delete, list_containers)
    {
        auto prefix = get_random_container_name();

        for (int i = 0; i < 1; i++)
        {
            auto index = utility::conversions::print_string(i);
            auto container = m_client.get_container_reference(prefix + index);
            m_containers_to_delete.push_back(container);
            container.metadata()[U("index")] = index;
            container.create(wa::storage::blob_container_public_access_type::off, wa::storage::blob_request_options(), m_context);
        }

        std::vector<wa::storage::cloud_blob_container> containers(m_containers_to_delete);

        auto listing = list_all_containers(utility::string_t(), wa::storage::container_listing_includes::all(), 1, wa::storage::blob_request_options());
        for (auto listing_iter = listing.begin(); listing_iter != listing.end(); ++listing_iter)
        {
            for (auto iter = containers.begin(); iter != containers.end(); ++iter)
            {
                if (iter->name() == listing_iter->name())
                {
                    auto index_str = listing_iter->metadata().find(U("index"));
                    CHECK(index_str != listing_iter->metadata().end());
                    CHECK_UTF8_EQUAL(iter->name(), prefix + index_str->second);
                    containers.erase(iter);
                    break;
                }
            }
        }

        CHECK(containers.empty());
    }

    TEST_FIXTURE(blob_service_test_base_with_objects_to_delete, list_blobs_from_client_root)
    {
        auto root_container = m_client.get_root_container_reference();
        root_container.create_if_not_exists();

        auto prefix = get_random_container_name();

        for (int i = 0; i < 1; i++)
        {
            auto index = utility::conversions::print_string(i);
            auto blob = root_container.get_block_blob_reference(prefix + index);
            m_blobs_to_delete.push_back(blob);
            blob.upload_text(U("test"), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        }

        std::vector<wa::storage::cloud_blob> blobs(m_blobs_to_delete);

        auto listing = list_all_blobs_from_client(prefix, wa::storage::blob_listing_includes(), 1, wa::storage::blob_request_options());
        for (auto listing_iter = listing.begin(); listing_iter != listing.end(); ++listing_iter)
        {
            for (auto iter = blobs.begin(); iter != blobs.end(); ++iter)
            {
                if (iter->name() == listing_iter->name())
                {
                    blobs.erase(iter);
                    break;
                }
            }
        }

        CHECK(blobs.empty());
    }

    TEST_FIXTURE(blob_test_base, list_blobs_from_client)
    {
        std::vector<wa::storage::cloud_blob> blobs;
        auto prefix = get_random_container_name();

        for (int i = 0; i < 1; i++)
        {
            auto index = utility::conversions::print_string(i);
            auto blob = m_container.get_block_blob_reference(prefix + index);
            blob.upload_text(U("test"), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
            blobs.push_back(blob);
        }

        std::vector<wa::storage::cloud_blob> original_blobs(blobs);

        auto listing = list_all_blobs_from_client(m_container.name(), wa::storage::blob_listing_includes(), 1, wa::storage::blob_request_options());
        CHECK(listing.empty());

        listing = list_all_blobs_from_client(m_container.name() + U("/"), wa::storage::blob_listing_includes(), 1, wa::storage::blob_request_options());
        for (auto listing_iter = listing.begin(); listing_iter != listing.end(); ++listing_iter)
        {
            for (auto iter = blobs.begin(); iter != blobs.end(); ++iter)
            {
                if (iter->name() == listing_iter->name())
                {
                    blobs.erase(iter);
                    break;
                }
            }
        }

        CHECK(blobs.empty());

        blobs = original_blobs;

        listing = list_all_blobs_from_client(m_container.name() + U("/") + prefix, wa::storage::blob_listing_includes(), 1, wa::storage::blob_request_options());
        for (auto listing_iter = listing.begin(); listing_iter != listing.end(); ++listing_iter)
        {
            for (auto iter = blobs.begin(); iter != blobs.end(); ++iter)
            {
                if (iter->name() == listing_iter->name())
                {
                    blobs.erase(iter);
                    break;
                }
            }
        }

        CHECK(blobs.empty());
    }

    TEST_FIXTURE(test_base, blob_shared_key_lite)
    {
        auto client = test_config::instance().account().create_cloud_blob_client();
        client.set_authentication_scheme(wa::storage::authentication_scheme::shared_key_lite);
        client.list_containers_segmented(utility::string_t(), wa::storage::container_listing_includes(), 1, wa::storage::blob_continuation_token(), wa::storage::blob_request_options(), m_context);
    }
}
