// -----------------------------------------------------------------------------------------
// <copyright file="blob_lease_test.cpp" company="Microsoft">
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

const utility::string_t fake_guid(_XPLATSTR("d30a3ff7-d28b-4ce5-a381-c5d7f94be411"));

void blob_test_base::check_lease_access(azure::storage::cloud_blob& blob, azure::storage::lease_state state, const utility::string_t& lease_id, bool fake)
{
    bool locked = (state == azure::storage::lease_state::leased) || (state == azure::storage::lease_state::breaking);
    auto lease_condition = azure::storage::access_condition::generate_lease_condition(lease_id);
    auto empty_condition = azure::storage::access_condition();

    blob.download_attributes(empty_condition, azure::storage::blob_request_options(), m_context);
    CHECK(state == blob.properties().lease_state());
    
    auto status = locked ? azure::storage::lease_status::locked : azure::storage::lease_status::unlocked;
    CHECK(status == blob.properties().lease_status());

    if (locked)
    {
        CHECK_THROW(blob.upload_properties(empty_condition, azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
    }
    else
    {
        blob.upload_properties(empty_condition, azure::storage::blob_request_options(), m_context);
    }

    if (locked && !fake)
    {
        blob.upload_properties(lease_condition, azure::storage::blob_request_options(), m_context);
        blob.download_attributes(lease_condition, azure::storage::blob_request_options(), m_context);
    }
    else
    {
        CHECK_THROW(blob.upload_properties(lease_condition, azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
        CHECK_THROW(blob.download_attributes(lease_condition, azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
    }
}

void container_test_base::check_lease_access(azure::storage::cloud_blob_container& container, azure::storage::lease_state state, const utility::string_t& lease_id, bool fake, bool allow_delete)
{
    bool locked = (state == azure::storage::lease_state::leased) || (state == azure::storage::lease_state::breaking);
    auto lease_condition = azure::storage::access_condition::generate_lease_condition(lease_id);
    auto empty_condition = azure::storage::access_condition();

    container.download_attributes(empty_condition, azure::storage::blob_request_options(), m_context);
    CHECK(state == container.properties().lease_state());

    auto status = locked ? azure::storage::lease_status::locked : azure::storage::lease_status::unlocked;
    CHECK(status == container.properties().lease_status());

    if (locked)
    {
        CHECK_THROW(container.delete_container(empty_condition, azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
    }
    else
    {
        if (allow_delete)
        {
            container.delete_container(empty_condition, azure::storage::blob_request_options(), m_context);
        }
    }

    if (locked && !fake)
    {
        container.download_attributes(lease_condition, azure::storage::blob_request_options(), m_context);
        if (allow_delete)
        {
            container.delete_container(lease_condition, azure::storage::blob_request_options(), m_context);
        }
    }
    else
    {
        CHECK_THROW(container.delete_container(lease_condition, azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
        CHECK_THROW(container.download_attributes(lease_condition, azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
    }
}

SUITE(Blob)
{
    TEST_FIXTURE(block_blob_test_base, blob_lease_acquire_infinite)
    {
        m_blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto my_lease_id = utility::uuid_to_string(utility::new_uuid());
        check_lease_access(m_blob, azure::storage::lease_state::available, my_lease_id, true);

        auto lease_id = m_blob.acquire_lease(azure::storage::lease_time(), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        check_lease_access(m_blob, azure::storage::lease_state::leased, lease_id, false);
        check_lease_access(m_blob, azure::storage::lease_state::leased, my_lease_id, true);

        CHECK(m_blob.properties().lease_duration() == azure::storage::lease_duration::infinite);

        std::this_thread::sleep_for(std::chrono::seconds(70));
        check_lease_access(m_blob, azure::storage::lease_state::leased, lease_id, false);
        check_lease_access(m_blob, azure::storage::lease_state::leased, my_lease_id, true);
    }

    TEST_FIXTURE(block_blob_test_base, blob_lease_acquire_non_infinite)
    {
        m_blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        check_lease_access(m_blob, azure::storage::lease_state::available, fake_guid, true);

        auto lease_id = m_blob.acquire_lease(azure::storage::lease_time(std::chrono::seconds(40)), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        check_lease_access(m_blob, azure::storage::lease_state::leased, lease_id, false);
        check_lease_access(m_blob, azure::storage::lease_state::leased, fake_guid, true);

        CHECK(m_blob.properties().lease_duration() == azure::storage::lease_duration::fixed);
        
        std::this_thread::sleep_for(std::chrono::seconds(20));
        check_lease_access(m_blob, azure::storage::lease_state::leased, lease_id, false);
        check_lease_access(m_blob, azure::storage::lease_state::leased, fake_guid, true);

        std::this_thread::sleep_for(std::chrono::seconds(30));
        check_lease_access(m_blob, azure::storage::lease_state::expired, lease_id, false);
        check_lease_access(m_blob, azure::storage::lease_state::available, fake_guid, true);
    }

    TEST_FIXTURE(block_blob_test_base, blob_lease_acquire_proposed_id)
    {
        m_blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto lease_id = m_blob.acquire_lease(azure::storage::lease_time(), fake_guid, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_UTF8_EQUAL(fake_guid, lease_id);
        check_lease_access(m_blob, azure::storage::lease_state::leased, fake_guid, false);
    }

    TEST_FIXTURE(block_blob_test_base, blob_lease_acquire_invalid_duration)
    {
        m_blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        CHECK_THROW(m_blob.acquire_lease(azure::storage::lease_time(std::chrono::seconds(14)), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);
        CHECK_THROW(m_blob.acquire_lease(azure::storage::lease_time(std::chrono::seconds(61)), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);
    }

    TEST_FIXTURE(block_blob_test_base, blob_lease_renew)
    {
        m_blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto lease_id = m_blob.acquire_lease(azure::storage::lease_time(std::chrono::seconds(40)), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        auto lease_condition = azure::storage::access_condition::generate_lease_condition(lease_id);

        std::this_thread::sleep_for(std::chrono::seconds(30));
        CHECK_THROW(m_blob.renew_lease(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);
        m_blob.renew_lease(lease_condition, azure::storage::blob_request_options(), m_context);

        std::this_thread::sleep_for(std::chrono::seconds(30));
        check_lease_access(m_blob, azure::storage::lease_state::leased, lease_id, false);

        std::this_thread::sleep_for(std::chrono::seconds(20));
        check_lease_access(m_blob, azure::storage::lease_state::expired, lease_id, false);
        CHECK_THROW(m_blob.renew_lease(lease_condition, azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
    }

    TEST_FIXTURE(block_blob_test_base, blob_lease_change)
    {
        m_blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto lease_id = m_blob.acquire_lease(azure::storage::lease_time(), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        CHECK_THROW(m_blob.change_lease(fake_guid, azure::storage::access_condition::generate_lease_condition(fake_guid), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
        CHECK_THROW(m_blob.change_lease(utility::string_t(), azure::storage::access_condition::generate_lease_condition(lease_id), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);

        CHECK_THROW(m_blob.change_lease(fake_guid, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);
        auto new_lease_id = m_blob.change_lease(fake_guid, azure::storage::access_condition::generate_lease_condition(lease_id), azure::storage::blob_request_options(), m_context);
        CHECK_UTF8_EQUAL(fake_guid, new_lease_id);
        check_lease_access(m_blob, azure::storage::lease_state::leased, lease_id, true);
        check_lease_access(m_blob, azure::storage::lease_state::leased, fake_guid, false);
    }

    TEST_FIXTURE(block_blob_test_base, blob_lease_release)
    {
        m_blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto lease_id = m_blob.acquire_lease(azure::storage::lease_time(), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_THROW(m_blob.release_lease(azure::storage::access_condition::generate_lease_condition(fake_guid), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
        CHECK_THROW(m_blob.release_lease(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);
        m_blob.release_lease(azure::storage::access_condition::generate_lease_condition(lease_id), azure::storage::blob_request_options(), m_context);
        check_lease_access(m_blob, azure::storage::lease_state::available, lease_id, false);
    }

    TEST_FIXTURE(block_blob_test_base, blob_lease_break_infinite_immediately)
    {
        m_blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto lease_id = m_blob.acquire_lease(azure::storage::lease_time(), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        m_blob.break_lease(azure::storage::lease_break_period(), azure::storage::access_condition::generate_lease_condition(fake_guid), azure::storage::blob_request_options(), m_context);
        check_lease_access(m_blob, azure::storage::lease_state::broken, lease_id, false);
    }

    TEST_FIXTURE(block_blob_test_base, blob_lease_break_infinite_period)
    {
        m_blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto lease_id = m_blob.acquire_lease(azure::storage::lease_time(), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        m_blob.break_lease(azure::storage::lease_break_period(std::chrono::seconds(20)), azure::storage::access_condition::generate_lease_condition(fake_guid), azure::storage::blob_request_options(), m_context);
        check_lease_access(m_blob, azure::storage::lease_state::breaking, lease_id, false);

        std::this_thread::sleep_for(std::chrono::seconds(30));
        check_lease_access(m_blob, azure::storage::lease_state::broken, lease_id, false);
    }

    TEST_FIXTURE(block_blob_test_base, blob_lease_break_non_infinite_remaining)
    {
        m_blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto lease_id = m_blob.acquire_lease(azure::storage::lease_time(std::chrono::seconds(60)), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        m_blob.break_lease(azure::storage::lease_break_period(), azure::storage::access_condition::generate_lease_condition(fake_guid), azure::storage::blob_request_options(), m_context);
        check_lease_access(m_blob, azure::storage::lease_state::breaking, lease_id, false);

        std::this_thread::sleep_for(std::chrono::seconds(80));
        check_lease_access(m_blob, azure::storage::lease_state::broken, lease_id, false);
    }

    TEST_FIXTURE(block_blob_test_base, blob_lease_break_non_infinite_period)
    {
        m_blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto lease_id = m_blob.acquire_lease(azure::storage::lease_time(std::chrono::seconds(60)), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        m_blob.break_lease(azure::storage::lease_break_period(std::chrono::seconds(20)), azure::storage::access_condition::generate_lease_condition(fake_guid), azure::storage::blob_request_options(), m_context);
        check_lease_access(m_blob, azure::storage::lease_state::breaking, lease_id, false);

        std::this_thread::sleep_for(std::chrono::seconds(40));
        check_lease_access(m_blob, azure::storage::lease_state::broken, lease_id, false);
    }

    TEST_FIXTURE(container_test_base, container_lease_acquire_infinite)
    {
        m_container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);

        auto my_lease_id = utility::uuid_to_string(utility::new_uuid());
        check_lease_access(m_container, azure::storage::lease_state::available, my_lease_id, true, false);

        auto lease_id = m_container.acquire_lease(azure::storage::lease_time(), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        check_lease_access(m_container, azure::storage::lease_state::leased, lease_id, false, false);
        check_lease_access(m_container, azure::storage::lease_state::leased, my_lease_id, true, false);

        CHECK(m_container.properties().lease_duration() == azure::storage::lease_duration::infinite);

        std::this_thread::sleep_for(std::chrono::seconds(70));
        check_lease_access(m_container, azure::storage::lease_state::leased, my_lease_id, true, false);
        check_lease_access(m_container, azure::storage::lease_state::leased, lease_id, false, true);
    }

    TEST_FIXTURE(container_test_base, container_lease_acquire_non_infinite)
    {
        m_container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);

        check_lease_access(m_container, azure::storage::lease_state::available, fake_guid, true, false);

        auto lease_id = m_container.acquire_lease(azure::storage::lease_time(std::chrono::seconds(40)), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        check_lease_access(m_container, azure::storage::lease_state::leased, lease_id, false, false);
        check_lease_access(m_container, azure::storage::lease_state::leased, fake_guid, true, false);

        CHECK(m_container.properties().lease_duration() == azure::storage::lease_duration::fixed);

        std::this_thread::sleep_for(std::chrono::seconds(20));
        check_lease_access(m_container, azure::storage::lease_state::leased, lease_id, false, false);
        check_lease_access(m_container, azure::storage::lease_state::leased, fake_guid, true, false);

        std::this_thread::sleep_for(std::chrono::seconds(30));
        check_lease_access(m_container, azure::storage::lease_state::expired, fake_guid, true, false);
        check_lease_access(m_container, azure::storage::lease_state::expired, lease_id, false, true);
    }

    TEST_FIXTURE(container_test_base, container_lease_acquire_proposed_id)
    {
        m_container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);

        auto lease_id = m_container.acquire_lease(azure::storage::lease_time(), fake_guid, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_UTF8_EQUAL(fake_guid, lease_id);
        check_lease_access(m_container, azure::storage::lease_state::leased, fake_guid, false, true);
    }

    TEST_FIXTURE(container_test_base, container_lease_acquire_invalid_duration)
    {
        m_container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);

        CHECK_THROW(m_container.acquire_lease(azure::storage::lease_time(std::chrono::seconds(14)), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);
        CHECK_THROW(m_container.acquire_lease(azure::storage::lease_time(std::chrono::seconds(61)), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);
    }

    TEST_FIXTURE(container_test_base, container_lease_renew)
    {
        m_container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);

        auto lease_id = m_container.acquire_lease(azure::storage::lease_time(std::chrono::seconds(40)), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        auto lease_condition = azure::storage::access_condition::generate_lease_condition(lease_id);

        std::this_thread::sleep_for(std::chrono::seconds(30));
        CHECK_THROW(m_container.renew_lease(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);
        m_container.renew_lease(lease_condition, azure::storage::blob_request_options(), m_context);

        std::this_thread::sleep_for(std::chrono::seconds(30));
        check_lease_access(m_container, azure::storage::lease_state::leased, lease_id, false, false);

        std::this_thread::sleep_for(std::chrono::seconds(20));
        check_lease_access(m_container, azure::storage::lease_state::expired, lease_id, false, false);
        m_container.renew_lease(lease_condition, azure::storage::blob_request_options(), m_context);
    }

    TEST_FIXTURE(container_test_base, container_lease_change)
    {
        m_container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);

        auto lease_id = m_container.acquire_lease(azure::storage::lease_time(), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        CHECK_THROW(m_container.change_lease(fake_guid, azure::storage::access_condition::generate_lease_condition(fake_guid), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
        CHECK_THROW(m_container.change_lease(utility::string_t(), azure::storage::access_condition::generate_lease_condition(lease_id), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);

        CHECK_THROW(m_container.change_lease(fake_guid, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);
        auto new_lease_id = m_container.change_lease(fake_guid, azure::storage::access_condition::generate_lease_condition(lease_id), azure::storage::blob_request_options(), m_context);
        CHECK_UTF8_EQUAL(fake_guid, new_lease_id);
        check_lease_access(m_container, azure::storage::lease_state::leased, lease_id, true, false);
        check_lease_access(m_container, azure::storage::lease_state::leased, fake_guid, false, true);
    }

    TEST_FIXTURE(container_test_base, container_lease_release)
    {
        m_container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);

        auto lease_id = m_container.acquire_lease(azure::storage::lease_time(), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_THROW(m_container.release_lease(azure::storage::access_condition::generate_lease_condition(fake_guid), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
        CHECK_THROW(m_container.release_lease(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);
        m_container.release_lease(azure::storage::access_condition::generate_lease_condition(lease_id), azure::storage::blob_request_options(), m_context);
        check_lease_access(m_container, azure::storage::lease_state::available, lease_id, false, true);
    }

    TEST_FIXTURE(container_test_base, container_lease_break_infinite_immediately)
    {
        m_container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);

        auto lease_id = m_container.acquire_lease(azure::storage::lease_time(), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        m_container.break_lease(azure::storage::lease_break_period(), azure::storage::access_condition::generate_lease_condition(fake_guid), azure::storage::blob_request_options(), m_context);
        check_lease_access(m_container, azure::storage::lease_state::broken, lease_id, false, true);
    }

    TEST_FIXTURE(container_test_base, container_lease_break_infinite_period)
    {
        m_container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);

        auto lease_id = m_container.acquire_lease(azure::storage::lease_time(), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        m_container.break_lease(azure::storage::lease_break_period(std::chrono::seconds(20)), azure::storage::access_condition::generate_lease_condition(fake_guid), azure::storage::blob_request_options(), m_context);
        check_lease_access(m_container, azure::storage::lease_state::breaking, lease_id, false, false);

        std::this_thread::sleep_for(std::chrono::seconds(30));
        check_lease_access(m_container, azure::storage::lease_state::broken, lease_id, false, true);
    }

    TEST_FIXTURE(container_test_base, container_lease_break_non_infinite_remaining)
    {
        m_container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);

        auto lease_id = m_container.acquire_lease(azure::storage::lease_time(std::chrono::seconds(60)), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        m_container.break_lease(azure::storage::lease_break_period(), azure::storage::access_condition::generate_lease_condition(fake_guid), azure::storage::blob_request_options(), m_context);
        check_lease_access(m_container, azure::storage::lease_state::breaking, lease_id, false, false);

        std::this_thread::sleep_for(std::chrono::seconds(80));
        check_lease_access(m_container, azure::storage::lease_state::broken, lease_id, false, true);
    }

    TEST_FIXTURE(container_test_base, container_lease_break_non_infinite_period)
    {
        m_container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);

        auto lease_id = m_container.acquire_lease(azure::storage::lease_time(std::chrono::seconds(60)), utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        m_container.break_lease(azure::storage::lease_break_period(std::chrono::seconds(20)), azure::storage::access_condition::generate_lease_condition(fake_guid), azure::storage::blob_request_options(), m_context);
        check_lease_access(m_container, azure::storage::lease_state::breaking, lease_id, false, false);

        std::this_thread::sleep_for(std::chrono::seconds(40));
        check_lease_access(m_container, azure::storage::lease_state::broken, lease_id, false, true);
    }

    TEST_FIXTURE(blob_test_base, lease_time_constructor)
    {
        azure::storage::lease_time lease_time1;
        CHECK_EQUAL(-1, lease_time1.seconds().count());

        int valid_times[] = { -1, 15, 20, 60 };
        for (int valid_time : valid_times)
        {
            CHECK_EQUAL(valid_time, azure::storage::lease_time(std::chrono::seconds(valid_time)).seconds().count());
        }

        int invalid_times[] = { -2, 0, 1, 14, 61 };
        for (int invalid_time : invalid_times)
        {
            CHECK_THROW(azure::storage::lease_time(std::chrono::seconds(invalid_time)), std::invalid_argument);
        }
    }

    TEST_FIXTURE(blob_test_base, lease_break_period_constructor)
    {
        CHECK_EQUAL(std::numeric_limits<std::chrono::seconds::rep>::max(), azure::storage::lease_break_period().seconds().count());
        CHECK_EQUAL(std::numeric_limits<std::chrono::seconds::rep>::max(), azure::storage::lease_break_period(std::chrono::seconds(std::numeric_limits<std::chrono::seconds::rep>::max())).seconds().count());
        CHECK_THROW(azure::storage::lease_break_period(std::chrono::seconds(std::numeric_limits<std::chrono::seconds::rep>::min())), std::invalid_argument);

        int valid_periods[] = { 0, 1, 60};
        for (int valid_period : valid_periods)
        {
            CHECK_EQUAL(valid_period, azure::storage::lease_break_period(std::chrono::seconds(valid_period)).seconds().count());
        }

        int invalid_periods[] = { -1, -2, 61 };
        for (int invalid_period : invalid_periods)
        {
            CHECK_THROW(azure::storage::lease_break_period(std::chrono::seconds(invalid_period)), std::invalid_argument);
        }
    }
}
