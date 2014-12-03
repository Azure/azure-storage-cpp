// -----------------------------------------------------------------------------------------
// <copyright file="retry_policies.cpp" company="Microsoft">
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

#include "was/common.h"
#include "was/retry_policies.h"

namespace azure { namespace storage {

    retry_info retry_policy::evaluate(const retry_context& retry_context, operation_context context)
    {
        if (m_policy != nullptr)
        {
            return m_policy->evaluate(retry_context, context);
        }

        return retry_info();
    }

    retry_info basic_no_retry_policy::evaluate(const retry_context& retry_context, operation_context context)
    {
        UNREFERENCED_PARAMETER(retry_context);
        UNREFERENCED_PARAMETER(context);
        return retry_info();
    }

    retry_info basic_common_retry_policy::evaluate(const retry_context& retry_context, operation_context context)
    {
        UNREFERENCED_PARAMETER(context);
        if (retry_context.current_retry_count() >= m_max_attempts)
        {
            return retry_info();
        }

        // Retry interval of a request to a location must take the time spent sending requests
        // to other locations into account. For example, assume a request was sent to the primary
        // location first, then to the secondary, and then to the primary again. If it
        // was supposed to wait 10 seconds between requests to the primary and the request to
        // the secondary took 3 seconds in total, retry interval should only be 7 seconds, because
        // in total, the requests will be 10 seconds apart from the primary locations' point of view.
        // For this calculation, current instance of the retry policy stores timestamp of the last
        // request to a specific location.
        switch (retry_context.last_request_result().target_location())
        {
        case azure::storage::storage_location::primary:
            m_last_primary_attempt = retry_context.last_request_result().end_time();
            break;

        case azure::storage::storage_location::secondary:
            m_last_secondary_attempt = retry_context.last_request_result().end_time();
            break;
        }

        bool secondary_not_found = (retry_context.last_request_result().http_status_code() == web::http::status_codes::NotFound) &&
            (retry_context.last_request_result().target_location() == storage_location::secondary);

        // Anything between 300 and 500 other than 408 should not be retried
        if (retry_context.last_request_result().http_status_code() >= 300 &&
            retry_context.last_request_result().http_status_code() < 500 &&
            retry_context.last_request_result().http_status_code() != web::http::status_codes::RequestTimeout &&
            !secondary_not_found)
        {
            return retry_info();
        }

        // Explicitly handle some 500 level status codes
        if ((retry_context.last_request_result().http_status_code() == web::http::status_codes::NotImplemented) ||
            (retry_context.last_request_result().http_status_code() == web::http::status_codes::HttpVersionNotSupported))
        {
            return retry_info();
        }

        retry_info result(retry_context);

        if (secondary_not_found && (retry_context.current_location_mode() != location_mode::secondary_only))
        {
            result.set_updated_location_mode(location_mode::primary_only);
            result.set_target_location(storage_location::primary);
        }

        return result;
    }

    void basic_common_retry_policy::align_retry_interval(retry_info& retry_info)
    {
        utility::datetime last_attempt;
        switch (retry_info.target_location())
        {
        case azure::storage::storage_location::primary:
            last_attempt = m_last_primary_attempt;
            break;

        case azure::storage::storage_location::secondary:
            last_attempt = m_last_secondary_attempt;
            break;

        default:
            return;
        }

        if (last_attempt.is_initialized())
        {
            auto since_last_attempt = std::chrono::seconds(utility::datetime::utc_now() - last_attempt);
            retry_info.set_retry_interval(std::max(std::chrono::milliseconds::zero(), retry_info.retry_interval() - since_last_attempt));
        }
        else
        {
            retry_info.set_retry_interval(std::chrono::milliseconds::zero());
        }
    }

    retry_info basic_linear_retry_policy::evaluate(const retry_context& retry_context, operation_context context)
    {
        auto result = basic_common_retry_policy::evaluate(retry_context, context);
        
        if (result.should_retry())
        {
            result.set_retry_interval(m_delta_backoff);
            align_retry_interval(result);
        }

        return result;
    }

    retry_info basic_exponential_retry_policy::evaluate(const retry_context& retry_context, operation_context context)
    {
        auto result = basic_common_retry_policy::evaluate(retry_context, context);

        if (result.should_retry())
        {
            auto random_backoff = m_rand_distribution(m_rand_engine);
            std::chrono::milliseconds increment(static_cast<std::chrono::milliseconds::rep>((std::pow(2, retry_context.current_retry_count()) - 1) * random_backoff * 1000));
            auto interval = increment < std::chrono::milliseconds::zero() ? max_exponential_retry_interval : min_exponential_retry_interval + increment;
            result.set_retry_interval(std::min(interval, max_exponential_retry_interval));
            align_retry_interval(result);
        }

        return result;
    }

}} // namespace azure::storage
