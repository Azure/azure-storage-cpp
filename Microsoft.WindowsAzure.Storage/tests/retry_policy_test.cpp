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

wa::storage::storage_location get_initial_location(wa::storage::location_mode mode)
{
    switch (mode)
    {
    case wa::storage::location_mode::primary_only:
    case wa::storage::location_mode::primary_then_secondary:
        return wa::storage::storage_location::primary;
    
    case wa::storage::location_mode::secondary_only:
    case wa::storage::location_mode::secondary_then_primary:
        return wa::storage::storage_location::secondary;

    default:
        throw std::invalid_argument("mode");
    }
}

wa::storage::storage_location get_next_location(wa::storage::location_mode mode, wa::storage::storage_location current_location)
{
    switch (mode)
    {
    case wa::storage::location_mode::primary_only:
        return wa::storage::storage_location::primary;

    case wa::storage::location_mode::secondary_only:
        return wa::storage::storage_location::secondary;

    case wa::storage::location_mode::primary_then_secondary:
    case wa::storage::location_mode::secondary_then_primary:
        switch (current_location)
        {
        case wa::storage::storage_location::primary:
            return wa::storage::storage_location::secondary;

        case wa::storage::storage_location::secondary:
            return wa::storage::storage_location::primary;

        default:
            throw std::invalid_argument("current_location");
        }

    default:
        throw std::invalid_argument("mode");
    }
}

void verify_retry_results(wa::storage::retry_policy policy, web::http::status_code primary_status_code, web::http::status_code secondary_status_code, wa::storage::location_mode mode, std::function<std::chrono::milliseconds (int)> allowed_delta, std::vector<wa::storage::retry_info> expected_retry_info_list)
{
    auto initial_location = get_initial_location(mode);
    auto next_location = get_next_location(mode, initial_location);

    wa::storage::operation_context op_context;
    wa::storage::request_result result(utility::datetime::utc_now(),
        initial_location,
        web::http::http_response(initial_location == wa::storage::storage_location::secondary ? secondary_status_code : primary_status_code),
        false);

    int retry_count = 0;
    for (auto iter = expected_retry_info_list.cbegin(); iter != expected_retry_info_list.cend(); ++iter)
    {
        auto retry_info = policy.evaluate(wa::storage::retry_context(retry_count++, result, next_location, mode), op_context);

        CHECK(retry_info.should_retry());
        CHECK(iter->target_location() == retry_info.target_location());
        CHECK(iter->updated_location_mode() == retry_info.updated_location_mode());
        CHECK_CLOSE(iter->retry_interval().count(), retry_info.retry_interval().count(), allowed_delta(retry_count).count());

        std::this_thread::sleep_for(retry_info.retry_interval());

        result = wa::storage::request_result(utility::datetime::utc_now(),
            retry_info.target_location(),
            web::http::http_response(retry_info.target_location() == wa::storage::storage_location::secondary ? secondary_status_code : primary_status_code),
            false);
        mode = retry_info.updated_location_mode();
        next_location = get_next_location(mode, next_location);
    }

    auto retry_info = policy.evaluate(wa::storage::retry_context(retry_count++, result, next_location, mode), op_context);
    CHECK(!retry_info.should_retry());
}

static wa::storage::retry_info create_fake_retry_info(wa::storage::storage_location target_location, wa::storage::location_mode updated_mode, std::chrono::milliseconds interval)
{
    wa::storage::retry_context dummy_context(0, wa::storage::request_result(), target_location, updated_mode);
    wa::storage::retry_info fake_info(dummy_context);
    fake_info.set_retry_interval(interval);
    return fake_info;
}

SUITE(Core)
{
    TEST(no_retry_results)
    {
        auto allowed_delta = [] (int retry_count) -> std::chrono::milliseconds
        {
            return std::chrono::milliseconds();
        };

        // Both locations return InternalError

        {
            wa::storage::no_retry_policy policy;
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, wa::storage::location_mode::primary_only, allowed_delta, expected_retry_info_list);
        }

        {
            wa::storage::no_retry_policy policy;
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, wa::storage::location_mode::secondary_only, allowed_delta, expected_retry_info_list);
        }

        {
            wa::storage::no_retry_policy policy;
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, wa::storage::location_mode::primary_then_secondary, allowed_delta, expected_retry_info_list);
        }

        {
            wa::storage::no_retry_policy policy;
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, wa::storage::location_mode::secondary_then_primary, allowed_delta, expected_retry_info_list);
        }

        // Primary location returns InternalError, while secondary location returns NotFound

        {
            wa::storage::no_retry_policy policy;
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, wa::storage::location_mode::secondary_only, allowed_delta, expected_retry_info_list);
        }

        {
            wa::storage::no_retry_policy policy;
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, wa::storage::location_mode::primary_then_secondary, allowed_delta, expected_retry_info_list);
        }

        {
            wa::storage::no_retry_policy policy;
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, wa::storage::location_mode::secondary_then_primary, allowed_delta, expected_retry_info_list);
        }
    }

    TEST(exponential_retry_results)
    {
        auto allowed_delta = [] (int retry_count) -> std::chrono::milliseconds
        {
            return std::chrono::milliseconds(static_cast<std::chrono::milliseconds::rep>(1000 * ((std::pow(2, retry_count) - 1) * 0.2 + 0.1)));
        };

        // Both locations return InternalError

        {
            wa::storage::exponential_retry_policy policy(std::chrono::seconds(1), 4);
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(3)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(4)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(6)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(10)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, wa::storage::location_mode::primary_only, allowed_delta, expected_retry_info_list);
        }

        {
            wa::storage::exponential_retry_policy policy(std::chrono::seconds(1), 4);
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_only, std::chrono::seconds(3)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_only, std::chrono::seconds(4)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_only, std::chrono::seconds(6)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_only, std::chrono::seconds(10)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, wa::storage::location_mode::secondary_only, allowed_delta, expected_retry_info_list);
        }

        {
            wa::storage::exponential_retry_policy policy(std::chrono::seconds(1), 4);
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::primary_then_secondary, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_then_secondary, std::chrono::seconds(4)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::primary_then_secondary, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_then_secondary, std::chrono::seconds(8)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, wa::storage::location_mode::primary_then_secondary, allowed_delta, expected_retry_info_list);
        }

        {
            wa::storage::exponential_retry_policy policy(std::chrono::seconds(1), 4);
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::secondary_then_primary, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_then_primary, std::chrono::seconds(4)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::secondary_then_primary, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_then_primary, std::chrono::seconds(8)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, wa::storage::location_mode::secondary_then_primary, allowed_delta, expected_retry_info_list);
        }

        // Primary location returns InternalError, while secondary location returns NotFound

        {
            wa::storage::exponential_retry_policy policy(std::chrono::seconds(1), 4);
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_only, std::chrono::seconds(3)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_only, std::chrono::seconds(4)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_only, std::chrono::seconds(6)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_only, std::chrono::seconds(10)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, wa::storage::location_mode::secondary_only, allowed_delta, expected_retry_info_list);
        }

        {
            wa::storage::exponential_retry_policy policy(std::chrono::seconds(1), 4);
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::primary_then_secondary, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(4)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(6)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(10)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, wa::storage::location_mode::primary_then_secondary, allowed_delta, expected_retry_info_list);
        }

        {
            wa::storage::exponential_retry_policy policy(std::chrono::seconds(1), 4);
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(4)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(6)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(10)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, wa::storage::location_mode::secondary_then_primary, allowed_delta, expected_retry_info_list);
        }
    }

    TEST(linear_retry_results)
    {
        auto allowed_delta = [] (int retry_count) -> std::chrono::milliseconds
        {
            return std::chrono::milliseconds(100);
        };

        // Both locations return InternalError

        {
            wa::storage::linear_retry_policy policy(std::chrono::seconds(2), 4);
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(2)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, wa::storage::location_mode::primary_only, allowed_delta, expected_retry_info_list);
        }

        {
            wa::storage::linear_retry_policy policy(std::chrono::seconds(2), 4);
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_only, std::chrono::seconds(2)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, wa::storage::location_mode::secondary_only, allowed_delta, expected_retry_info_list);
        }

        {
            wa::storage::linear_retry_policy policy(std::chrono::seconds(2), 4);
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::primary_then_secondary, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_then_secondary, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::primary_then_secondary, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_then_secondary, std::chrono::seconds(2)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, wa::storage::location_mode::primary_then_secondary, allowed_delta, expected_retry_info_list);
        }

        {
            wa::storage::linear_retry_policy policy(std::chrono::seconds(2), 4);
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::secondary_then_primary, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_then_primary, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::secondary_then_primary, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_then_primary, std::chrono::seconds(2)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::InternalError, wa::storage::location_mode::secondary_then_primary, allowed_delta, expected_retry_info_list);
        }

        // Primary location returns InternalError, while secondary location returns NotFound

        {
            wa::storage::linear_retry_policy policy(std::chrono::seconds(2), 4);
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::secondary_only, std::chrono::seconds(2)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, wa::storage::location_mode::secondary_only, allowed_delta, expected_retry_info_list);
        }

        {
            wa::storage::linear_retry_policy policy(std::chrono::seconds(2), 4);
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::secondary, wa::storage::location_mode::primary_then_secondary, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(2)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, wa::storage::location_mode::primary_then_secondary, allowed_delta, expected_retry_info_list);
        }

        {
            wa::storage::linear_retry_policy policy(std::chrono::seconds(2), 4);
            std::vector<wa::storage::retry_info> expected_retry_info_list;
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(0)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(2)));
            expected_retry_info_list.push_back(create_fake_retry_info(wa::storage::storage_location::primary, wa::storage::location_mode::primary_only, std::chrono::seconds(2)));
            verify_retry_results(policy, web::http::status_codes::InternalError, web::http::status_codes::NotFound, wa::storage::location_mode::secondary_then_primary, allowed_delta, expected_retry_info_list);
        }
    }
}
