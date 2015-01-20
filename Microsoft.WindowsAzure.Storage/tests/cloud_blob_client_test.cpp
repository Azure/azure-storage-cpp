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

void blob_service_test_base_with_objects_to_delete::create_containers(const utility::string_t& prefix, std::size_t num)
{
    for (std::size_t i = 0; i < num; ++i)
    {
        auto index = utility::conversions::print_string(i);
        auto container = m_client.get_container_reference(prefix + index);
        m_containers_to_delete.push_back(container);
        container.metadata()[U("index")] = index;
        container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);
    }
}

void blob_service_test_base_with_objects_to_delete::create_blobs(const azure::storage::cloud_blob_container& container, const utility::string_t& prefix, std::size_t num)
{
    for (std::size_t i = 0; i < num; i++)
    {
        auto index = utility::conversions::print_string(i);
        auto blob = container.get_block_blob_reference(prefix + index);
        m_blobs_to_delete.push_back(blob);
        blob.upload_text(U("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
    }
}

void blob_service_test_base_with_objects_to_delete::check_container_list(const std::vector<azure::storage::cloud_blob_container>& list, const utility::string_t& prefix, bool check_found)
{
    auto container_list_sorted = std::is_sorted(list.cbegin(), list.cend(), [](const azure::storage::cloud_blob_container& a, const azure::storage::cloud_blob_container& b)
    {
        return a.name() < b.name();
    });
    CHECK(container_list_sorted);

    std::vector<azure::storage::cloud_blob_container> containers(m_containers_to_delete);

    for (auto list_iter = list.begin(); list_iter != list.end(); ++list_iter)
    {
        bool found = false;
        for (auto iter = containers.begin(); iter != containers.end(); ++iter)
        {
            if (iter->name() == list_iter->name())
            {
                auto index_str = list_iter->metadata().find(U("index"));
                CHECK(index_str != list_iter->metadata().end());
                CHECK_UTF8_EQUAL(iter->name(), prefix + index_str->second);
                containers.erase(iter);
                found = true;
                break;
            }
        }
        if (check_found)
        {
            CHECK(found);
        }
    }
    CHECK(containers.empty());
}

void blob_service_test_base_with_objects_to_delete::check_blob_list(const std::vector<azure::storage::cloud_blob>& list)
{
    auto blob_list_sorted = std::is_sorted(list.cbegin(), list.cend(), [](const azure::storage::cloud_blob& a, const azure::storage::cloud_blob& b)
    {
        return a.name() < b.name();
    });
    CHECK(blob_list_sorted);

    std::vector<azure::storage::cloud_blob> blobs(m_blobs_to_delete);

    for (auto list_iter = list.begin(); list_iter != list.end(); ++list_iter)
    {
        for (auto iter = blobs.begin(); iter != blobs.end(); ++iter)
        {
            if (iter->name() == list_iter->name())
            {
                blobs.erase(iter);
                break;
            }
        }
    }

    CHECK(blobs.empty());
}

std::vector<azure::storage::cloud_blob_container> blob_service_test_base::list_all_containers(const utility::string_t& prefix, azure::storage::container_listing_details::values includes, int max_results, const azure::storage::blob_request_options& options)
{
    std::vector<azure::storage::cloud_blob_container> containers;
    azure::storage::continuation_token token;

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

std::vector<azure::storage::cloud_blob> blob_service_test_base::list_all_blobs_from_client(const utility::string_t& prefix, azure::storage::blob_listing_details::values includes, int max_results, const azure::storage::blob_request_options& options)
{
    std::vector<azure::storage::cloud_blob> blobs;
    azure::storage::continuation_token token;

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
    TEST_FIXTURE(blob_service_test_base, get_container_reference)
    {
        utility::string_t container_name = get_random_container_name();
        azure::storage::cloud_blob_container container = m_client.get_container_reference(container_name);

        CHECK(!container.service_client().base_uri().primary_uri().is_empty());
        CHECK(container.service_client().credentials().is_shared_key());
        CHECK(container.name() == container_name);
        CHECK(!container.uri().primary_uri().is_empty());
        CHECK(container.metadata().empty());
        CHECK(container.properties().etag().empty());
        CHECK(!container.properties().last_modified().is_initialized());
        CHECK(container.properties().lease_status() == azure::storage::lease_status::unspecified);
        CHECK(container.properties().lease_state() == azure::storage::lease_state::unspecified);
        CHECK(container.properties().lease_duration() == azure::storage::lease_duration::unspecified);
        CHECK(container.is_valid());
    }

    TEST_FIXTURE(blob_service_test_base_with_objects_to_delete, list_containers_with_prefix)
    {
        auto prefix = get_random_container_name();

        create_containers(prefix, 1);

        auto listing = list_all_containers(prefix, azure::storage::container_listing_details::all, 1, azure::storage::blob_request_options());
        
        check_container_list(listing, prefix, true);
    }

    TEST_FIXTURE(blob_service_test_base_with_objects_to_delete, list_containers)
    {
        auto prefix = get_random_container_name();

        create_containers(prefix, 1);

        auto listing = list_all_containers(utility::string_t(), azure::storage::container_listing_details::all, 5001, azure::storage::blob_request_options());
        
        check_container_list(listing, prefix, false);
    }

    TEST_FIXTURE(blob_service_test_base_with_objects_to_delete, list_containers_with_continuation_token)
    {
        auto prefix = get_random_container_name();

        create_containers(prefix, 10);

        auto listing = list_all_containers(prefix, azure::storage::container_listing_details::all, 3, azure::storage::blob_request_options());

        check_container_list(listing, prefix, true);
    }

    TEST_FIXTURE(blob_service_test_base_with_objects_to_delete, list_blobs_from_client_root)
    {
        auto root_container = m_client.get_root_container_reference();
        root_container.create_if_not_exists();

        auto prefix = get_random_container_name();

        create_blobs(root_container, prefix, 1);

        auto listing = list_all_blobs_from_client(prefix, azure::storage::blob_listing_details::none, 1, azure::storage::blob_request_options());
        
        check_blob_list(listing);
    }

    TEST_FIXTURE(blob_test_base, list_blobs_from_client)
    {
        std::vector<azure::storage::cloud_blob> blobs;
        auto prefix = get_random_container_name();

        for (int i = 0; i < 1; i++)
        {
            auto index = utility::conversions::print_string(i);
            auto blob = m_container.get_block_blob_reference(prefix + index);
            blob.upload_text(U("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
            blobs.push_back(blob);
        }

        std::vector<azure::storage::cloud_blob> original_blobs(blobs);

        auto listing = list_all_blobs_from_client(m_container.name(), azure::storage::blob_listing_details::none, 1, azure::storage::blob_request_options());
        CHECK(listing.empty());

        listing = list_all_blobs_from_client(m_container.name() + U("/"), azure::storage::blob_listing_details::none, 1, azure::storage::blob_request_options());
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

        listing = list_all_blobs_from_client(m_container.name() + U("/") + prefix, azure::storage::blob_listing_details::none, 1, azure::storage::blob_request_options());
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
        client.set_authentication_scheme(azure::storage::authentication_scheme::shared_key_lite);
        client.list_containers_segmented(utility::string_t(), azure::storage::container_listing_details::none, 1, azure::storage::continuation_token(), azure::storage::blob_request_options(), m_context);
    }
}
