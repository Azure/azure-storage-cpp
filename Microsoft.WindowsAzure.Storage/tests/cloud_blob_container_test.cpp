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

void container_test_base::check_public_access(azure::storage::blob_container_public_access_type access)
{
    auto blob = m_container.get_block_blob_reference(_XPLATSTR("blob"));
    blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

    azure::storage::cloud_blob_container public_container(m_container.uri());
    auto public_blob = public_container.get_blob_reference(blob.name());

    if (access != azure::storage::blob_container_public_access_type::off)
    {
        public_blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
    }
    else
    {
        CHECK_THROW(public_blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
    }

    if (access == azure::storage::blob_container_public_access_type::container)
    {
        public_container.list_blobs_segmented(utility::string_t(), true, azure::storage::blob_listing_details::all, 0, azure::storage::continuation_token(), azure::storage::blob_request_options(), m_context);
        public_container.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
    }
    else
    {
        CHECK_THROW(public_container.list_blobs_segmented(utility::string_t(), true, azure::storage::blob_listing_details::all, 0, azure::storage::continuation_token(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
        CHECK_THROW(public_container.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
    }

    auto perms = m_container.download_permissions(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
    CHECK(access == perms.public_access());
}

std::vector<azure::storage::cloud_blob> container_test_base::list_all_blobs(const utility::string_t& prefix, azure::storage::blob_listing_details::values includes, int max_results, const azure::storage::blob_request_options& options)
{
    std::vector<azure::storage::cloud_blob> blobs;
    azure::storage::list_blob_item_iterator end_of_result;
    auto iter = m_container.list_blobs(prefix, true, includes, max_results, options, m_context);
    for (; iter != end_of_result; ++iter)
    {
        if (iter->is_blob())
        {
            blobs.push_back(iter->as_blob());
        }
    }

    return blobs;
}

#pragma endregion

SUITE(Blob)
{
    TEST_FIXTURE(container_test_base, container_get_reference)
    {
        auto block_blob = m_container.get_block_blob_reference(_XPLATSTR("blob1"));
        CHECK_UTF8_EQUAL(m_container.uri().primary_uri().to_string(), block_blob.container().uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_container.uri().secondary_uri().to_string(), block_blob.container().uri().secondary_uri().to_string());

        auto page_blob = m_container.get_page_blob_reference(_XPLATSTR("blob2"));
        CHECK_UTF8_EQUAL(m_container.uri().primary_uri().to_string(), page_blob.container().uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_container.uri().secondary_uri().to_string(), page_blob.container().uri().secondary_uri().to_string());

        auto append_blob = m_container.get_append_blob_reference(_XPLATSTR("blob3"));
        CHECK_UTF8_EQUAL(m_container.uri().primary_uri().to_string(), append_blob.container().uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_container.uri().secondary_uri().to_string(), append_blob.container().uri().secondary_uri().to_string());

        auto directory = m_container.get_directory_reference(_XPLATSTR("dir"));
        CHECK_UTF8_EQUAL(m_container.uri().primary_uri().to_string(), directory.container().uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_container.uri().secondary_uri().to_string(), directory.container().uri().secondary_uri().to_string());
    }

    TEST_FIXTURE(container_test_base, container_create_delete)
    {
        CHECK(!m_container.exists(azure::storage::blob_request_options(), m_context));
        CHECK(!m_container.delete_container_if_exists(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context));
        CHECK(m_container.create_if_not_exists(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context));
        check_container_no_stale_property(m_container);
        CHECK(!m_container.create_if_not_exists(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context));
        CHECK(m_container.exists(azure::storage::blob_request_options(), m_context));
        CHECK(m_container.delete_container_if_exists(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context));
        CHECK(!m_container.delete_container_if_exists(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context));
        CHECK(!m_container.exists(azure::storage::blob_request_options(), m_context));
    }

    TEST_FIXTURE(container_test_base, container_create_public_off)
    {
        m_container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);
        CHECK(m_container.properties().public_access() == azure::storage::blob_container_public_access_type::off);
        check_public_access(azure::storage::blob_container_public_access_type::off);
        check_container_no_stale_property(m_container);
    }

    TEST_FIXTURE(container_test_base, container_create_public_blob)
    {
        m_container.create(azure::storage::blob_container_public_access_type::blob, azure::storage::blob_request_options(), m_context);
        CHECK(m_container.properties().public_access() == azure::storage::blob_container_public_access_type::blob);
        check_public_access(azure::storage::blob_container_public_access_type::blob);
        check_container_no_stale_property(m_container);
    }

    TEST_FIXTURE(container_test_base, container_create_public_container)
    {
        m_container.create(azure::storage::blob_container_public_access_type::container, azure::storage::blob_request_options(), m_context);
        CHECK(m_container.properties().public_access() == azure::storage::blob_container_public_access_type::container);
        check_public_access(azure::storage::blob_container_public_access_type::container);
        check_container_no_stale_property(m_container);
    }

    TEST_FIXTURE(container_test_base, container_set_public_access)
    {
        m_container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);
        check_container_no_stale_property(m_container);
        azure::storage::blob_container_permissions permissions;

        permissions.set_public_access(azure::storage::blob_container_public_access_type::container);
        m_container.upload_permissions(permissions, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        check_public_access(azure::storage::blob_container_public_access_type::container);

        permissions.set_public_access(azure::storage::blob_container_public_access_type::blob);
        m_container.upload_permissions(permissions, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        check_public_access(azure::storage::blob_container_public_access_type::blob);

        permissions.set_public_access(azure::storage::blob_container_public_access_type::off);
        m_container.upload_permissions(permissions, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        check_public_access(azure::storage::blob_container_public_access_type::off);
    }

    TEST_FIXTURE(container_test_base, container_metadata)
    {
        // Create with 2 pairs
        m_container.metadata()[_XPLATSTR("key1")] = _XPLATSTR("value1");
        m_container.metadata()[_XPLATSTR("key2")] = _XPLATSTR("value2");
        m_container.create(get_random_enum(azure::storage::blob_container_public_access_type::blob), azure::storage::blob_request_options(), m_context);
        check_container_no_stale_property(m_container);

        auto same_container = m_client.get_container_reference(m_container.name());
        CHECK(same_container.properties().public_access() == azure::storage::blob_container_public_access_type::off);
        CHECK(same_container.metadata().empty());
        same_container.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(same_container.properties().public_access() == m_container.properties().public_access());
        CHECK_EQUAL(2U, same_container.metadata().size());
        CHECK_UTF8_EQUAL(_XPLATSTR("value1"), same_container.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value2"), same_container.metadata()[_XPLATSTR("key2")]);

        // Add 1 pair
        same_container.metadata()[_XPLATSTR("key3")] = _XPLATSTR("value3");
        same_container.upload_metadata(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        m_container.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(3U, same_container.metadata().size());
        CHECK_UTF8_EQUAL(_XPLATSTR("value1"), m_container.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value2"), m_container.metadata()[_XPLATSTR("key2")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value3"), m_container.metadata()[_XPLATSTR("key3")]);

        // Overwrite with 1 pair
        m_container.metadata().clear();
        m_container.metadata()[_XPLATSTR("key4")] = _XPLATSTR("value4");
        m_container.upload_metadata(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        same_container.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(1U, same_container.metadata().size());
        CHECK_UTF8_EQUAL(_XPLATSTR("value4"), same_container.metadata()[_XPLATSTR("key4")]);

        // Clear all pairs
        same_container.metadata().clear();
        same_container.upload_metadata(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        m_container.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(m_container.metadata().empty());
    }

    TEST_FIXTURE(container_test_base, container_list_blobs)
    {
        m_container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);
        check_container_no_stale_property(m_container);
        std::map<utility::string_t, azure::storage::cloud_blob> blobs;

        for (int i = 0; i < 4; i++)
        {
            auto index = utility::conversions::print_string(i);
            auto blob = m_container.get_block_blob_reference(_XPLATSTR("blockblob") + index);
            blob.metadata()[_XPLATSTR("index")] = index;
            
            std::vector<uint8_t> buffer;
            buffer.resize(i * 16 * 1024);
            auto stream = concurrency::streams::container_stream<std::vector<uint8_t>>::open_istream(buffer);
            blob.upload_from_stream(stream, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
            blobs[blob.name()] = blob;
        }

        for (int i = 0; i < 3; i++)
        {
            auto index = utility::conversions::print_string(i);
            auto blob = m_container.get_page_blob_reference(_XPLATSTR("pageblob") + index);
            blob.metadata()[_XPLATSTR("index")] = index;
            
            blob.create(i * 512, 0, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
            check_container_no_stale_property(m_container);
            blobs[blob.name()] = blob;
        }

        for (int i = 0; i < 3; i++)
        {
            auto index = utility::conversions::print_string(i);
            auto blob = m_container.get_append_blob_reference(_XPLATSTR("appendblob") + index);
            blob.metadata()[_XPLATSTR("index")] = index;

            blob.create_or_replace(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
            check_container_no_stale_property(m_container);

            std::vector<uint8_t> buffer;
            buffer.resize((i + 1) * 8 * 1024);
            fill_buffer_and_get_md5(buffer);
            auto stream = concurrency::streams::container_stream<std::vector<uint8_t>>::open_istream(buffer);
            blob.append_block(stream, utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
            blobs[blob.name()] = blob;
        }

        auto listing1 = list_all_blobs(utility::string_t(), azure::storage::blob_listing_details::all, 0, azure::storage::blob_request_options());
        for (auto iter = listing1.begin(); iter != listing1.end(); ++iter)
        {
            auto blob = blobs.find(iter->name());
            CHECK(blob != blobs.end());

            CHECK_UTF8_EQUAL(blob->second.uri().primary_uri().to_string(), iter->uri().primary_uri().to_string());
            CHECK_UTF8_EQUAL(blob->second.uri().secondary_uri().to_string(), iter->uri().secondary_uri().to_string());

            auto index_str = blob->second.metadata().find(_XPLATSTR("index"));
            CHECK(index_str != blob->second.metadata().end());
            auto index = utility::conversions::scan_string<int>(index_str->second);

            switch (iter->type())
            {
            case azure::storage::blob_type::block_blob:
                CHECK_EQUAL(index * 16 * 1024, iter->properties().size());
                break;

            case azure::storage::blob_type::page_blob:
                CHECK_EQUAL(index * 512, iter->properties().size());
                break;

            case azure::storage::blob_type::append_blob:
                CHECK_EQUAL((index + 1) * 8 * 1024, iter->properties().size());
                break;

            default:
                CHECK(false);
                break;
            }

            blobs.erase(blob);
        }

        CHECK_EQUAL(0U, blobs.size());

        auto listing2 = list_all_blobs(_XPLATSTR("block"), azure::storage::blob_listing_details::none, 10, azure::storage::blob_request_options());
        CHECK_EQUAL(4U, listing2.size());
        for (auto iter = listing2.begin(); iter != listing2.end(); ++iter)
        {
            CHECK(iter->metadata().empty());
        }
    }

    TEST_FIXTURE(blob_test_base, container_stored_policy)
    {
        auto stored_permissions = m_container.download_permissions(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(stored_permissions.policies().empty());

        auto now = utility::datetime::utc_now();
        auto aligned_now = now - (now.to_interval() % (10 * 1000 * 1000));

        azure::storage::blob_shared_access_policy policy;
        policy.set_permissions(azure::storage::blob_shared_access_policy::permissions::write);
        policy.set_start(aligned_now - utility::datetime::from_minutes(5));
        policy.set_expiry(aligned_now + utility::datetime::from_minutes(30));

        azure::storage::blob_container_permissions permissions;
        permissions.policies()[_XPLATSTR("id1")] = policy;
        m_container.upload_permissions(permissions, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        std::this_thread::sleep_for(std::chrono::seconds(30));

        auto blob = m_container.get_block_blob_reference(_XPLATSTR("blob"));
        blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto sas_token = blob.get_shared_access_signature(azure::storage::blob_shared_access_policy(), _XPLATSTR("id1"));
        check_access(sas_token, azure::storage::blob_shared_access_policy::permissions::write, azure::storage::cloud_blob_shared_access_headers(), blob);

        stored_permissions = m_container.download_permissions(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(1U, stored_permissions.policies().size());
        auto stored_policy = stored_permissions.policies().find(_XPLATSTR("id1"));
        CHECK(stored_policy != stored_permissions.policies().end());
        CHECK_EQUAL(policy.permission(), stored_policy->second.permission());
        CHECK_EQUAL(policy.start().to_interval(), stored_policy->second.start().to_interval());
        CHECK_EQUAL(policy.expiry().to_interval(), stored_policy->second.expiry().to_interval());

        stored_permissions.policies().clear();
        m_container.upload_permissions(stored_permissions, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        std::this_thread::sleep_for(std::chrono::seconds(30));

        check_access(sas_token, azure::storage::blob_shared_access_policy::permissions::none, azure::storage::cloud_blob_shared_access_headers(), blob);

        stored_permissions = m_container.download_permissions(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(stored_permissions.policies().empty());
    }

    /// Test the blob name with corner characters.
    TEST_FIXTURE(blob_test_base, corner_blob_name)
    {
        // Initialize the chareset to generate random blob name.
        std::vector<utility::char_t> charset;
        utility::string_t characters = _XPLATSTR("`~!@#$%^&*()_+[{]}|;:\'\",<>?");
        for (size_t i = 0; i < characters.size(); ++i)
        {
            charset.push_back(characters[i]);
        }

        for (int i = 0; i < 16; ++i)
        {
            utility::string_t blob_name = get_random_string(charset, 20);
            azure::storage::cloud_block_blob blob = m_container.get_block_blob_reference(blob_name);
            auto content = get_random_string(charset, 20);
            blob.upload_text(content);

            // list the container to get the blob just created.
            auto listing = list_all_blobs(blob_name, azure::storage::blob_listing_details::all, 0, azure::storage::blob_request_options());
            CHECK(listing.size() == 1);

            // check the consistance of blob content.
            auto download_content = blob.download_text();
            CHECK(content == download_content);

            blob.delete_blob();
        }
    }
}
