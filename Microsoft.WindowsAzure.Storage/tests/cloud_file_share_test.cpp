// -----------------------------------------------------------------------------------------
// <copyright file="cloud_file_share_test.cpp" company="Microsoft">
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
    TEST_FIXTURE(file_share_test_base, share_create_delete)
    {
        CHECK(!m_share.exists(azure::storage::file_request_options(), m_context));
        CHECK(!m_share.delete_share_if_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));

        CHECK(m_share.create_if_not_exists(azure::storage::file_request_options(), m_context));
        CHECK(!m_share.create_if_not_exists(azure::storage::file_request_options(), m_context));

        CHECK(m_share.exists(azure::storage::file_request_options(), m_context));

        CHECK(m_share.delete_share_if_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));
        CHECK(!m_share.delete_share_if_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));

        CHECK(!m_share.exists(azure::storage::file_request_options(), m_context));
    }

    TEST_FIXTURE(file_share_test_base, share_create_delete_with_quotas)
    {
        size_t quota = rand() % 5120 + 1;

        CHECK(!m_share.exists(azure::storage::file_request_options(), m_context));
        CHECK(!m_share.delete_share_if_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));

        CHECK(m_share.create_if_not_exists(quota, azure::storage::file_request_options(), m_context));
        CHECK(!m_share.create_if_not_exists(quota, azure::storage::file_request_options(), m_context));
        CHECK_EQUAL(quota, m_share.properties().quota());

        // download attributes then check the size of share.
        m_share.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(quota, m_share.properties().quota());

        CHECK(m_share.exists(azure::storage::file_request_options(), m_context));

        CHECK(m_share.delete_share_if_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));
        CHECK(!m_share.delete_share_if_exists(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context));

        CHECK(!m_share.exists(azure::storage::file_request_options(), m_context));
    }


    TEST_FIXTURE(file_share_test_base, share_metadata)
    {
        auto value1 = this->get_random_string();
        auto value2 = this->get_random_string();
        auto value3 = this->get_random_string();
        auto value4 = this->get_random_string();

        // create 2 pairs
        m_share.metadata()[_XPLATSTR("key1")] = value1;
        m_share.metadata()[_XPLATSTR("key2")] = value2;
        m_share.create_if_not_exists(azure::storage::file_request_options(), m_context);
        CHECK_UTF8_EQUAL(m_share.metadata()[_XPLATSTR("key1")], value1);
        CHECK_UTF8_EQUAL(m_share.metadata()[_XPLATSTR("key2")], value2);

        auto same_share = m_client.get_share_reference(m_share.name());
        CHECK(same_share.metadata().empty());
        same_share.download_attributes();
        CHECK_EQUAL(2U, same_share.metadata().size());
        CHECK_UTF8_EQUAL(value1, same_share.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(value2, same_share.metadata()[_XPLATSTR("key2")]);

        // add 1 pair
        m_share.metadata()[_XPLATSTR("key3")] = value3;
        m_share.upload_metadata(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        same_share.metadata().clear();
        CHECK(same_share.metadata().empty());
        same_share.download_attributes();
        CHECK_EQUAL(3U, same_share.metadata().size());
        CHECK_UTF8_EQUAL(value1, same_share.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(value2, same_share.metadata()[_XPLATSTR("key2")]);
        CHECK_UTF8_EQUAL(value3, same_share.metadata()[_XPLATSTR("key3")]);

        // overwrite with 1 pair
        m_share.metadata().clear();
        m_share.metadata()[_XPLATSTR("key4")] = value4;
        m_share.upload_metadata(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        same_share.metadata().clear();
        CHECK(same_share.metadata().empty());
        same_share.download_attributes();
        CHECK_EQUAL(1U, same_share.metadata().size());
        CHECK_UTF8_EQUAL(value4, same_share.metadata()[_XPLATSTR("key4")]);

        // clear metadata
        m_share.metadata().clear();
        m_share.upload_metadata(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        same_share.metadata().clear();
        CHECK(same_share.metadata().empty());
        same_share.download_attributes();
        CHECK_EQUAL(0U, same_share.metadata().size());
    }

    TEST_FIXTURE(file_share_test_base, share_get_directory_reference)
    {
        auto dir_name = this->get_random_string();
        azure::storage::cloud_file_directory dir = m_share.get_directory_reference(dir_name);
        CHECK_UTF8_EQUAL(dir_name, dir.name());
        CHECK(dir.get_parent_share_reference().is_valid());
        check_equal(m_share, dir.get_parent_share_reference());
        CHECK(!dir.uri().primary_uri().is_empty());
        CHECK(dir.metadata().empty());
        CHECK(dir.properties().etag().empty());
        CHECK(!dir.properties().last_modified().is_initialized());
    }

    TEST_FIXTURE(file_share_test_base, share_resize)
    {
        m_share.create_if_not_exists(azure::storage::file_request_options(), m_context);
        m_share.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(m_share.properties().quota(), 5120U);

        auto quota = get_random_int32();
        if (quota < 0)
            quota = -quota;
        quota = quota % 5120 + 1;
        
        m_share.resize(quota, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        m_share.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(m_share.properties().quota(), quota);
    }

    TEST_FIXTURE(file_share_test_base, share_stored_policy)
    {
        m_share.create_if_not_exists(azure::storage::file_request_options(), m_context);
        auto stored_permissions = m_share.download_permissions(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(stored_permissions.policies().empty());

        auto now = utility::datetime::utc_now();
        auto aligned_now = now - (now.to_interval() % (10 * 1000 * 1000));

        azure::storage::file_shared_access_policy policy;
        policy.set_permissions(azure::storage::file_shared_access_policy::permissions::write);
        policy.set_start(aligned_now - utility::datetime::from_minutes(5));
        policy.set_expiry(aligned_now + utility::datetime::from_minutes(30));

        azure::storage::file_share_permissions permissions;
        permissions.policies()[_XPLATSTR("id1")] = policy;
        m_share.upload_permissions(permissions, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        std::this_thread::sleep_for(std::chrono::seconds(30));

        auto file = m_share.get_root_directory_reference().get_file_reference(_XPLATSTR("file"));
        file.upload_text(_XPLATSTR("test"), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        auto sas_token = file.get_shared_access_signature(azure::storage::file_shared_access_policy(), _XPLATSTR("id1"));
        check_access(sas_token, azure::storage::file_shared_access_policy::permissions::write, azure::storage::cloud_file_shared_access_headers(), file);

        stored_permissions = m_share.download_permissions(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(1U, stored_permissions.policies().size());
        auto stored_policy = stored_permissions.policies().find(_XPLATSTR("id1"));
        CHECK(stored_policy != stored_permissions.policies().end());
        CHECK_EQUAL(policy.permission(), stored_policy->second.permission());
        CHECK_EQUAL(policy.start().to_interval(), stored_policy->second.start().to_interval());
        CHECK_EQUAL(policy.expiry().to_interval(), stored_policy->second.expiry().to_interval());

        stored_permissions.policies().clear();
        m_share.upload_permissions(stored_permissions, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        std::this_thread::sleep_for(std::chrono::seconds(30));

        check_access(sas_token, azure::storage::file_shared_access_policy::permissions::none, azure::storage::cloud_file_shared_access_headers(), file);

        stored_permissions = m_share.download_permissions(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK(stored_permissions.policies().empty());
    }

    TEST_FIXTURE(file_share_test_base, share_stats)
    {
        m_share.create_if_not_exists(azure::storage::file_request_options(), m_context);
        auto quota = m_share.download_share_usage(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
        CHECK_EQUAL(0, quota);
    }

    // share level sas test
    TEST_FIXTURE(file_share_test_base, share_sas_token)
    {
        // simple sas token check
        {
            utility::size64_t quota = 512;
            m_share.create_if_not_exists(quota, azure::storage::file_request_options(), m_context);
            m_share.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
            CHECK_EQUAL(quota, m_share.properties().quota());

            azure::storage::file_shared_access_policy policy;
            policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));
            policy.set_permissions(0x0F);

            auto sas_token = m_share.get_shared_access_signature(policy);
            auto share = azure::storage::cloud_file_share(m_share.uri(), azure::storage::storage_credentials(sas_token));

            auto file = share.get_root_directory_reference().get_file_reference(_XPLATSTR("test"));
            utility::string_t content = _XPLATSTR("testtargetfile");
            auto content_length = content.length();
            file.create_if_not_exists(content.length());
            file.upload_text(content, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
            file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
            CHECK_EQUAL(file.properties().length(), content_length);
        }

        // full combinations of sas check.
        for (uint8_t i = 0; i < 16; i++)
        {
            auto permissions = i;

            azure::storage::file_shared_access_policy policy;
            policy.set_permissions(permissions);
            policy.set_start(utility::datetime::utc_now() - utility::datetime::from_minutes(5));
            policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));
            auto sas_token = m_share.get_shared_access_signature(policy);

            auto file = m_share.get_root_directory_reference().get_file_reference(_XPLATSTR("file") + utility::conversions::print_string((int)i));
            file.create_if_not_exists(512U, azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
            file.properties().set_cache_control(_XPLATSTR("no-transform"));
            file.properties().set_content_disposition(_XPLATSTR("attachment"));
            file.properties().set_content_encoding(_XPLATSTR("gzip"));
            file.properties().set_content_language(_XPLATSTR("tr,en"));
            file.properties().set_content_type(_XPLATSTR("text/html"));
            file.upload_text(_XPLATSTR("test"), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

            check_access(sas_token, permissions, azure::storage::cloud_file_shared_access_headers(), file);
        }
    }
}