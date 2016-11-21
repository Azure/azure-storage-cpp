// -----------------------------------------------------------------------------------------
// <copyright file="retry_policy_test.cpp" company="Microsoft">
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
#include "test_base.h"
#include "check_macros.h"
#include "was/blob.h"
#include "was/table.h"
#include "was/queue.h"

azure::storage::storage_location get_initial_location(azure::storage::location_mode mode)
{
    switch (mode)
    {
    case azure::storage::location_mode::primary_only:
    case azure::storage::location_mode::primary_then_secondary:
        return azure::storage::storage_location::primary;
    
    case azure::storage::location_mode::secondary_only:
    case azure::storage::location_mode::secondary_then_primary:
        return azure::storage::storage_location::secondary;

    default:
        throw std::invalid_argument("mode");
    }
}

azure::storage::storage_location get_next_location(azure::storage::location_mode mode, azure::storage::storage_location current_location)
{
    switch (mode)
    {
    case azure::storage::location_mode::primary_only:
        return azure::storage::storage_location::primary;

    case azure::storage::location_mode::secondary_only:
        return azure::storage::storage_location::secondary;

    case azure::storage::location_mode::primary_then_secondary:
    case azure::storage::location_mode::secondary_then_primary:
        switch (current_location)
        {
        case azure::storage::storage_location::primary:
            return azure::storage::storage_location::secondary;

        case azure::storage::storage_location::secondary:
            return azure::storage::storage_location::primary;

        default:
            throw std::invalid_argument("current_location");
        }

    default:
        throw std::invalid_argument("mode");
    }
}

void verify_retry_results(azure::storage::retry_policy policy, web::http::status_code primary_status_code, web::http::status_code secondary_status_code, azure::storage::location_mode mode, std::function<std::chrono::milliseconds (int)> allowed_delta, std::vector<azure::storage::retry_info> expected_retry_info_list, azure::storage::operation_context context)
{
    auto initial_location = get_initial_location(mode);
    auto next_location = get_next_location(mode, initial_location);

    azure::storage::operation_context op_context = context;
    azure::storage::request_result result(utility::datetime::utc_now(),
        initial_location,
        web::http::http_response(initial_location == azure::storage::storage_location::secondary ? secondary_status_code : primary_status_code),
        false);

    int retry_count = 0;
    for (auto iter = expected_retry_info_list.cbegin(); iter != expected_retry_info_list.cend(); ++iter)
    {
        auto retry_info = policy.evaluate(azure::storage::retry_context(retry_count++, result, next_location, mode), op_context);

        CHECK(retry_info.should_retry());
        CHECK(iter->target_location() == retry_info.target_location());
        CHECK(iter->updated_location_mode() == retry_info.updated_location_mode());
        CHECK_CLOSE(iter->retry_interval().count(), retry_info.retry_interval().count(), allowed_delta(retry_count).count());

        std::this_thread::sleep_for(retry_info.retry_interval());

        result = azure::storage::request_result(utility::datetime::utc_now(),
            retry_info.target_location(),
            web::http::http_response(retry_info.target_location() == azure::storage::storage_location::secondary ? secondary_status_code : primary_status_code),
            false);
        mode = retry_info.updated_location_mode();
        next_location = get_next_location(mode, next_location);
    }

    auto retry_info = policy.evaluate(azure::storage::retry_context(retry_count++, result, next_location, mode), op_context);
    CHECK(!retry_info.should_retry());
}

static azure::storage::retry_info create_fake_retry_info(azure::storage::storage_location target_location, azure::storage::location_mode updated_mode, std::chrono::milliseconds interval)
{
    azure::storage::retry_context dummy_context(0, azure::storage::request_result(), target_location, updated_mode);
    azure::storage::retry_info fake_info(dummy_context);
    fake_info.set_retry_interval(interval);
    return fake_info;
}

SUITE(Core)
{
    TEST_FIXTURE(test_base, retry_policy_default_constructor)
    {
        azure::storage::retry_policy null_policy;
        CHECK(!null_policy.is_valid());

        azure::storage::retry_context dummy_context(0, azure::storage::request_result(), azure::storage::storage_location::primary, azure::storage::location_mode::primary_only);
        azure::storage::retry_info info = null_policy.evaluate(dummy_context, m_context);
        CHECK(!info.should_retry());
        CHECK(info.target_location() == azure::storage::storage_location::unspecified);
        CHECK(info.updated_location_mode() == azure::storage::location_mode::unspecified);
        CHECK(info.retry_interval() == std::chrono::milliseconds());

        azure::storage::retry_policy null_policy2 = null_policy.clone();
        CHECK(!null_policy2.is_valid());
    }

    TEST_FIXTURE(test_base, retry_info)
    {
        {
            azure::storage::retry_info info;

            CHECK(!info.should_retry());
            CHECK(info.target_location() == azure::storage::storage_location::unspecified);
            CHECK(info.updated_location_mode() == azure::storage::location_mode::unspecified);
            CHECK(info.retry_interval() == std::chrono::milliseconds());
        }

        {
            int current_retry_count = 2;
            azure::storage::request_result last_request_result;
            azure::storage::storage_location next_location = azure::storage::storage_location::secondary;
            azure::storage::location_mode current_location_mode = azure::storage::location_mode::secondary_only;

            azure::storage::retry_context context(current_retry_count, last_request_result, next_location, current_location_mode);

            azure::storage::retry_info info(context);

            CHECK(info.should_retry());
            CHECK(info.target_location() == next_location);
            CHECK(info.updated_location_mode() == current_location_mode);
            CHECK(info.retry_interval() > std::chrono::milliseconds());
        }
    }

    TEST_FIXTURE(test_base, no_retry_results)
    {
        auto allowed_delta = [] (int) -> std::chrono::milliseconds
        {
            return std::chrono::milliseconds();
        };

        // Both locations return InternalError

        {
            azure::storage::no_retry_policy policy;
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, azure::storage::location_mode::primary_only, allowed_delta, expected_retry_info_list, m_context);
        }

        {
            azure::storage::no_retry_policy policy;
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, azure::storage::location_mode::secondary_only, allowed_delta, expected_retry_info_list, m_context);
        }

        {
            azure::storage::no_retry_policy policy;
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, azure::storage::location_mode::primary_then_secondary, allowed_delta, expected_retry_info_list, m_context);
        }

        {
            azure::storage::no_retry_policy policy;
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, azure::storage::location_mode::secondary_then_primary, allowed_delta, expected_retry_info_list, m_context);
        }

        // Primary location returns InternalError, while secondary location returns NotFound

        {
            azure::storage::no_retry_policy policy;
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, azure::storage::location_mode::secondary_only, allowed_delta, expected_retry_info_list, m_context);
        }

        {
            azure::storage::no_retry_policy policy;
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, azure::storage::location_mode::primary_then_secondary, allowed_delta, expected_retry_info_list, m_context);
        }

        {
            azure::storage::no_retry_policy policy;
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, azure::storage::location_mode::secondary_then_primary, allowed_delta, expected_retry_info_list, m_context);
        }
    }

    TEST_FIXTURE(test_base, exponential_retry_results)
    {
        auto allowed_delta = [] (int retry_count) -> std::chrono::milliseconds
        {
            return std::chrono::milliseconds(static_cast<std::chrono::milliseconds::rep>(1000 * ((std::pow(2, retry_count) - 1) * 0.2 + 0.1)));
        };

        // Both locations return InternalError

        {
            azure::storage::exponential_retry_policy policy(std::chrono::seconds(1), 4);
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(3)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(4)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(6)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(10)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, azure::storage::location_mode::primary_only, allowed_delta, expected_retry_info_list, m_context);
        }

        {
            azure::storage::exponential_retry_policy policy(std::chrono::seconds(1), 4);
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(3)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(4)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(6)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(10)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, azure::storage::location_mode::secondary_only, allowed_delta, expected_retry_info_list, m_context);
        }

        {
            azure::storage::exponential_retry_policy policy(std::chrono::seconds(1), 4);
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::primary_then_secondary, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_then_secondary, std::chrono::seconds(4)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::primary_then_secondary, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_then_secondary, std::chrono::seconds(8)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, azure::storage::location_mode::primary_then_secondary, allowed_delta, expected_retry_info_list, m_context);
        }

        {
            azure::storage::exponential_retry_policy policy(std::chrono::seconds(1), 4);
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::secondary_then_primary, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_then_primary, std::chrono::seconds(4)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::secondary_then_primary, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_then_primary, std::chrono::seconds(8)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, azure::storage::location_mode::secondary_then_primary, allowed_delta, expected_retry_info_list, m_context);
        }

        // Primary location returns InternalError, while secondary location returns NotFound

        {
            azure::storage::exponential_retry_policy policy(std::chrono::seconds(1), 4);
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(3)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(4)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(6)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(10)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, azure::storage::location_mode::secondary_only, allowed_delta, expected_retry_info_list, m_context);
        }

        {
            azure::storage::exponential_retry_policy policy(std::chrono::seconds(1), 4);
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::primary_then_secondary, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(4)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(6)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(10)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, azure::storage::location_mode::primary_then_secondary, allowed_delta, expected_retry_info_list, m_context);
        }

        {
            azure::storage::exponential_retry_policy policy(std::chrono::seconds(1), 4);
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(4)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(6)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(10)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, azure::storage::location_mode::secondary_then_primary, allowed_delta, expected_retry_info_list, m_context);
        }
    }

    TEST_FIXTURE(test_base, linear_retry_results)
    {
        auto allowed_delta = [] (int) -> std::chrono::milliseconds
        {
            return std::chrono::milliseconds(100);
        };

        // Both locations return InternalError

        {
            azure::storage::linear_retry_policy policy(std::chrono::seconds(2), 4);
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(2)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, azure::storage::location_mode::primary_only, allowed_delta, expected_retry_info_list, m_context);
        }

        {
            azure::storage::linear_retry_policy policy(std::chrono::seconds(2), 4);
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(2)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, azure::storage::location_mode::secondary_only, allowed_delta, expected_retry_info_list, m_context);
        }

        {
            azure::storage::linear_retry_policy policy(std::chrono::seconds(2), 4);
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::primary_then_secondary, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_then_secondary, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::primary_then_secondary, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_then_secondary, std::chrono::seconds(2)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, azure::storage::location_mode::primary_then_secondary, allowed_delta, expected_retry_info_list, m_context);
        }

        {
            azure::storage::linear_retry_policy policy(std::chrono::seconds(2), 4);
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::secondary_then_primary, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_then_primary, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::secondary_then_primary, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_then_primary, std::chrono::seconds(2)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, azure::storage::location_mode::secondary_then_primary, allowed_delta, expected_retry_info_list, m_context);
        }

        // Primary location returns InternalError, while secondary location returns NotFound

        {
            azure::storage::linear_retry_policy policy(std::chrono::seconds(2), 4);
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(2)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, azure::storage::location_mode::secondary_only, allowed_delta, expected_retry_info_list, m_context);
        }

        {
            azure::storage::linear_retry_policy policy(std::chrono::seconds(2), 4);
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::primary_then_secondary, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(2)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, azure::storage::location_mode::primary_then_secondary, allowed_delta, expected_retry_info_list, m_context);
        }

        {
            azure::storage::linear_retry_policy policy(std::chrono::seconds(2), 4);
            std::vector<azure::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(2)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, azure::storage::location_mode::secondary_then_primary, allowed_delta, expected_retry_info_list, m_context);
        }
    }

    TEST_FIXTURE(test_base, default_request_options_retry_policy)
    {
        azure::storage::blob_request_options b;
        azure::storage::table_request_options t;
        azure::storage::queue_request_options q;
        CHECK(!b.retry_policy().is_valid());
        CHECK(!t.retry_policy().is_valid());
        CHECK(!q.retry_policy().is_valid());
        b.apply_defaults(test_config::instance().account().create_cloud_blob_client().default_request_options(), azure::storage::blob_type::unspecified);
        CHECK(b.retry_policy().is_valid());
        t.apply_defaults(test_config::instance().account().create_cloud_table_client().default_request_options());
        CHECK(t.retry_policy().is_valid());
        q.apply_defaults(test_config::instance().account().create_cloud_queue_client().default_request_options());
        CHECK(q.retry_policy().is_valid());
    }
}
