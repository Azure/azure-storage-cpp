// -----------------------------------------------------------------------------------------
// <copyright file="cloud_blob_container_test.cpp" company="Microsoft">
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

void container_test_base::check_public_access(wa::storage::blob_container_public_access_type access)
{
    auto blob = m_container.get_block_blob_reference(U("blob"));
    blob.upload_text(U("test"), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);

    wa::storage::cloud_blob_container public_container(m_container.uri());
    auto public_blob = public_container.get_blob_reference(blob.name());

    if (access != wa::storage::blob_container_public_access_type::off)
    {
        public_blob.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
    }
    else
    {
        CHECK_THROW(public_blob.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context), wa::storage::storage_exception);
    }

    if (access == wa::storage::blob_container_public_access_type::container)
    {
        public_container.list_blobs_segmented(utility::string_t(), true, wa::storage::blob_listing_includes::all(), 0, wa::storage::blob_continuation_token(), wa::storage::blob_request_options(), m_context);
        public_container.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
    }
    else
    {
        CHECK_THROW(public_container.list_blobs_segmented(utility::string_t(), true, wa::storage::blob_listing_includes::all(), 0, wa::storage::blob_continuation_token(), wa::storage::blob_request_options(), m_context), wa::storage::storage_exception);
        CHECK_THROW(public_container.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context), wa::storage::storage_exception);
    }

    auto perms = m_container.download_permissions(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
    CHECK(access == perms.public_access());
}

std::vector<wa::storage::cloud_blob> container_test_base::list_all_blobs(const utility::string_t& prefix, const wa::storage::blob_listing_includes& includes, int max_results, const wa::storage::blob_request_options& options)
{
    std::vector<wa::storage::cloud_blob> blobs;
    wa::storage::blob_continuation_token token;
    
    do
    {
        auto results = m_container.list_blobs_segmented(prefix, true, includes, max_results, token, options, m_context);
        
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
    TEST_FIXTURE(container_test_base, container_get_reference)
    {
        auto block_blob = m_container.get_block_blob_reference(U("blob1"));
        CHECK_UTF8_EQUAL(m_container.uri().primary_uri().to_string(), block_blob.container().uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_container.uri().secondary_uri().to_string(), block_blob.container().uri().secondary_uri().to_string());

        auto page_blob = m_container.get_page_blob_reference(U("blob2"));
        CHECK_UTF8_EQUAL(m_container.uri().primary_uri().to_string(), page_blob.container().uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_container.uri().secondary_uri().to_string(), page_blob.container().uri().secondary_uri().to_string());

        auto directory = m_container.get_directory_reference(U("dir"));
        CHECK_UTF8_EQUAL(m_container.uri().primary_uri().to_string(), directory.container().uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_container.uri().secondary_uri().to_string(), directory.container().uri().secondary_uri().to_string());
    }

    TEST_FIXTURE(container_test_base, container_create_delete)
    {
        CHECK(!m_container.exists(wa::storage::blob_request_options(), m_context));
        CHECK(!m_container.delete_container_if_exists(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context));
        CHECK(m_container.create_if_not_exists(wa::storage::blob_container_public_access_type::off, wa::storage::blob_request_options(), m_context));
        CHECK(!m_container.create_if_not_exists(wa::storage::blob_container_public_access_type::off, wa::storage::blob_request_options(), m_context));
        CHECK(m_container.exists(wa::storage::blob_request_options(), m_context));
        CHECK(m_container.delete_container_if_exists(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context));
        CHECK(!m_container.delete_container_if_exists(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context));
        CHECK(!m_container.exists(wa::storage::blob_request_options(), m_context));
    }

    TEST_FIXTURE(container_test_base, container_create_public_off)
    {
        m_container.create(wa::storage::blob_container_public_access_type::off, wa::storage::blob_request_options(), m_context);
        check_public_access(wa::storage::blob_container_public_access_type::off);
    }

    TEST_FIXTURE(container_test_base, container_create_public_blob)
    {
        m_container.create(wa::storage::blob_container_public_access_type::blob, wa::storage::blob_request_options(), m_context);
        check_public_access(wa::storage::blob_container_public_access_type::blob);
    }

    TEST_FIXTURE(container_test_base, container_create_public_container)
    {
        m_container.create(wa::storage::blob_container_public_access_type::container, wa::storage::blob_request_options(), m_context);
        check_public_access(wa::storage::blob_container_public_access_type::container);
    }

    TEST_FIXTURE(container_test_base, container_set_public_access)
    {
        m_container.create(wa::storage::blob_container_public_access_type::off, wa::storage::blob_request_options(), m_context);
        wa::storage::blob_container_permissions permissions;

        permissions.set_public_access(wa::storage::blob_container_public_access_type::container);
        m_container.upload_permissions(permissions, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        check_public_access(wa::storage::blob_container_public_access_type::container);

        permissions.set_public_access(wa::storage::blob_container_public_access_type::blob);
        m_container.upload_permissions(permissions, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        check_public_access(wa::storage::blob_container_public_access_type::blob);

        permissions.set_public_access(wa::storage::blob_container_public_access_type::off);
        m_container.upload_permissions(permissions, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        check_public_access(wa::storage::blob_container_public_access_type::off);
    }

    TEST_FIXTURE(container_test_base, container_metadata)
    {
        // Create with 2 pairs
        m_container.metadata()[U("key1")] = U("value1");
        m_container.metadata()[U("key2")] = U("value2");
        m_container.create(wa::storage::blob_container_public_access_type::off, wa::storage::blob_request_options(), m_context);

        auto same_container = m_client.get_container_reference(m_container.name());
        CHECK(same_container.metadata().empty());
        same_container.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2, same_container.metadata().size());
        CHECK_UTF8_EQUAL(U("value1"), same_container.metadata()[U("key1")]);
        CHECK_UTF8_EQUAL(U("value2"), same_container.metadata()[U("key2")]);

        // Add 1 pair
        same_container.metadata()[U("key3")] = U("value3");
        same_container.upload_metadata(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        m_container.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK_EQUAL(3, same_container.metadata().size());
        CHECK_UTF8_EQUAL(U("value1"), m_container.metadata()[U("key1")]);
        CHECK_UTF8_EQUAL(U("value2"), m_container.metadata()[U("key2")]);
        CHECK_UTF8_EQUAL(U("value3"), m_container.metadata()[U("key3")]);

        // Overwrite with 1 pair
        m_container.metadata().clear();
        m_container.metadata()[U("key4")] = U("value4");
        m_container.upload_metadata(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        same_container.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK_EQUAL(1, same_container.metadata().size());
        CHECK_UTF8_EQUAL(U("value4"), same_container.metadata()[U("key4")]);

        // Clear all pairs
        same_container.metadata().clear();
        same_container.upload_metadata(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        m_container.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK(m_container.metadata().empty());
    }

    TEST_FIXTURE(container_test_base, container_list_blobs)
    {
        m_container.create(wa::storage::blob_container_public_access_type::off, wa::storage::blob_request_options(), m_context);
        std::map<utility::string_t, wa::storage::cloud_blob> blobs;

        for (int i = 0; i < 4; i++)
        {
            auto index = utility::conversions::print_string(i);
            auto blob = m_container.get_block_blob_reference(U("blockblob") + index);
            blob.metadata()[U("index")] = index;
            
            std::vector<uint8_t> buffer;
            buffer.resize(i * 16 * 1024);
            auto stream = concurrency::streams::container_stream<std::vector<uint8_t>>::open_istream(buffer);
            blob.upload_from_stream(stream, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
            blobs[blob.name()] = blob;
        }

        for (int i = 0; i < 3; i++)
        {
            auto index = utility::conversions::print_string(i);
            auto blob = m_container.get_page_blob_reference(U("pageblob") + index);
            blob.metadata()[U("index")] = index;
            
            blob.create(i * 512, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
            blobs[blob.name()] = blob;
        }

        auto listing1 = list_all_blobs(utility::string_t(), wa::storage::blob_listing_includes::all(), 2, wa::storage::blob_request_options());
        for (auto iter = listing1.begin(); iter != listing1.end(); ++iter)
        {
            auto blob = blobs.find(iter->name());
            CHECK(blob != blobs.end());

            CHECK_UTF8_EQUAL(blob->second.uri().primary_uri().to_string(), iter->uri().primary_uri().to_string());
            CHECK_UTF8_EQUAL(blob->second.uri().secondary_uri().to_string(), iter->uri().secondary_uri().to_string());

            auto index_str = blob->second.metadata().find(U("index"));
            CHECK(index_str != blob->second.metadata().end());
            auto index = utility::conversions::scan_string<int>(index_str->second);

            switch (iter->type())
            {
            case wa::storage::blob_type::block_blob:
                CHECK_EQUAL(index * 16 * 1024, iter->properties().size());
                break;

            case wa::storage::blob_type::page_blob:
                CHECK_EQUAL(index * 512, iter->properties().size());
                break;

            default:
                CHECK(false);
                break;
            }

            blobs.erase(blob);
        }

        CHECK_EQUAL(0, blobs.size());

        auto listing2 = list_all_blobs(U("block"), wa::storage::blob_listing_includes(), 10, wa::storage::blob_request_options());
        CHECK_EQUAL(4, listing2.size());
        for (auto iter = listing2.begin(); iter != listing2.end(); ++iter)
        {
            CHECK(iter->metadata().empty());
        }
    }

    TEST_FIXTURE(blob_test_base, container_stored_policy)
    {
        auto stored_permissions = m_container.download_permissions(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK(stored_permissions.policies().empty());

        auto now = utility::datetime::utc_now();
        auto aligned_now = now - (now.to_interval() % (10 * 1000 * 1000));

        wa::storage::blob_shared_access_policy policy;
        policy.set_permissions(wa::storage::blob_shared_access_policy::permissions::write);
        policy.set_start(aligned_now - utility::datetime::from_minutes(5));
        policy.set_expiry(aligned_now + utility::datetime::from_minutes(30));

        wa::storage::blob_container_permissions permissions;
        permissions.policies()[U("id1")] = policy;
        m_container.upload_permissions(permissions, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);

        std::this_thread::sleep_for(std::chrono::seconds(30));

        auto blob = m_container.get_block_blob_reference(U("blob"));
        blob.upload_text(U("test"), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);

        auto sas_token = blob.get_shared_access_signature(wa::storage::blob_shared_access_policy(), U("id1"));
        check_access(sas_token, wa::storage::blob_shared_access_policy::permissions::write, wa::storage::cloud_blob_shared_access_headers(), blob);

        stored_permissions = m_container.download_permissions(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK_EQUAL(1, stored_permissions.policies().size());
        auto stored_policy = stored_permissions.policies().find(U("id1"));
        CHECK(stored_policy != stored_permissions.policies().end());
        CHECK_EQUAL(policy.permission(), stored_policy->second.permission());
        CHECK_EQUAL(policy.start().to_interval(), stored_policy->second.start().to_interval());
        CHECK_EQUAL(policy.expiry().to_interval(), stored_policy->second.expiry().to_interval());

        stored_permissions.policies().clear();
        m_container.upload_permissions(stored_permissions, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);

        std::this_thread::sleep_for(std::chrono::seconds(30));

        check_access(sas_token, wa::storage::blob_shared_access_policy::permissions::none, wa::storage::cloud_blob_shared_access_headers(), blob);

        stored_permissions = m_container.download_permissions(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK(stored_permissions.policies().empty());
    }
}
