// -----------------------------------------------------------------------------------------
// <copyright file="retry_policies.h" company="Microsoft">
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

#pragma once

#include <random>

#include "core.h"

namespace azure { namespace storage {

    const int default_attempts(3);
    const std::chrono::milliseconds max_exponential_retry_interval(120 * 1000);
    const std::chrono::milliseconds min_exponential_retry_interval(protocol::default_retry_interval);

    /// <summary>
    /// Represents a retry policy that performs no retries.
    /// </summary>
    class basic_no_retry_policy : public basic_retry_policy
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::basic_no_retry_policy" /> class.
        /// </summary>
        basic_no_retry_policy()
            : basic_retry_policy()
        {
        }

        WASTORAGE_API retry_info evaluate(const retry_context& retry_context, operation_context context) override;

        /// <summary>
        /// Clones the retry policy.
        /// </summary>
        /// <returns>A cloned <see cref="azure::storage::retry_policy" />.</returns>
        retry_policy clone() const override
        {
            return retry_policy(std::make_shared<basic_no_retry_policy>());
        }
    };

    /// <summary>
    /// Represents a retry policy that performs no retries.
    /// </summary>
    class no_retry_policy : public retry_policy
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::no_retry_policy" /> class.
        /// </summary>
        no_retry_policy()
            : retry_policy(std::make_shared<basic_no_retry_policy>())
        {
        }
    };

    /// <summary>
    /// Represents a retry policy.
    /// </summary>
    class basic_common_retry_policy : public basic_retry_policy
    {
    public:

        WASTORAGE_API retry_info evaluate(const retry_context& retry_context, operation_context context) override;
    
    protected:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::basic_common_retry_policy" /> class.
        /// </summary>
        /// <param name="max_attempts">The maximum number of retries to attempt.</param>
        explicit basic_common_retry_policy(int max_attempts)
            : basic_retry_policy(), m_max_attempts(max_attempts)
        {
        }

        void align_retry_interval(retry_info& retry_info);

        /// <summary>
        /// The last attempt against the primary location.
        /// </summary>
        utility::datetime m_last_primary_attempt;
        /// <summary>
        /// The last attempt against the secondary location.
        /// </summary>
        utility::datetime m_last_secondary_attempt;
        /// <summary>
        /// The maximum number of retries to attempt.
        /// </summary>
        int m_max_attempts;
    };

    /// <summary>
    /// Represents a linear retry policy.
    /// </summary>
    class basic_linear_retry_policy : public basic_common_retry_policy
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::basic_linear_retry_policy" /> class.
        /// </summary>
        /// <param name="delta_backoff">The delta backoff.</param>
        /// <param name="max_attempts">The maximum number of retries to attempt.</param>
        basic_linear_retry_policy(std::chrono::seconds delta_backoff, int max_attempts)
            : basic_common_retry_policy(max_attempts), m_delta_backoff(delta_backoff)
        {
        }

        WASTORAGE_API retry_info evaluate(const retry_context& retry_context, operation_context context) override;

        /// <summary>
        /// Clones the retry policy.
        /// </summary>
        /// <returns>A cloned <see cref="azure::storage::retry_policy" />.</returns>
        retry_policy clone() const override
        {
            return retry_policy(std::make_shared<basic_linear_retry_policy>(m_delta_backoff, m_max_attempts));
        }

    private:

        std::chrono::seconds m_delta_backoff;
    };

    /// <summary>
    /// Represents a linear retry policy.
    /// </summary>
    class linear_retry_policy : public retry_policy
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::linear_retry_policy" /> class.
        /// </summary>
        linear_retry_policy()
            : retry_policy(std::make_shared<basic_linear_retry_policy>(protocol::default_retry_interval, default_attempts))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::linear_retry_policy" /> class.
        /// </summary>
        /// <param name="delta_backoff">The delta backoff.</param>
        /// <param name="max_attempts">The maximum number of retries to attempt.</param>
        linear_retry_policy(std::chrono::seconds delta_backoff, int max_attempts)
            : retry_policy(std::make_shared<basic_linear_retry_policy>(delta_backoff, max_attempts))
        {
        }
    };

    /// <summary>
    /// Represents an exponential retry policy.
    /// </summary>
    class basic_exponential_retry_policy : public basic_common_retry_policy
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::basic_exponential_retry_policy" /> class.
        /// </summary>
        /// <param name="delta_backoff">The delta backoff.</param>
        /// <param name="max_attempts">The maximum number of retries to attempt.</param>
        basic_exponential_retry_policy(std::chrono::seconds delta_backoff, int max_attempts)
            : basic_common_retry_policy(max_attempts),
            m_rand_distribution(static_cast<double>(delta_backoff.count()) * 0.8, static_cast<double>(delta_backoff.count()) * 1.2),
            m_delta_backoff(delta_backoff)
        {
        }

        WASTORAGE_API retry_info evaluate(const retry_context& retry_context, operation_context context) override;

        /// <summary>
        /// Clones the retry policy.
        /// </summary>
        /// <returns>A cloned <see cref="azure::storage::retry_policy" />.</returns>
        retry_policy clone() const override
        {
            return retry_policy(std::make_shared<basic_exponential_retry_policy>(m_delta_backoff, m_max_attempts));
        }

    private:
        
        std::uniform_real_distribution<> m_rand_distribution;
        std::default_random_engine m_rand_engine;
        std::chrono::seconds m_delta_backoff;
    };

    /// <summary>
    /// Represents an exponential retry policy.
    /// </summary>
    class exponential_retry_policy : public retry_policy
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::exponential_retry_policy" /> class.
        /// </summary>
        exponential_retry_policy()
            : retry_policy(std::make_shared<basic_exponential_retry_policy>(protocol::default_retry_interval, default_attempts))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::exponential_retry_policy" /> class.
        /// </summary>
        /// <param name="delta_backoff">The delta backoff.</param>
        /// <param name="max_attempts">The maximum number of retries to attempt.</param>
        exponential_retry_policy(std::chrono::seconds delta_backoff, int max_attempts)
            : retry_policy(std::make_shared<basic_exponential_retry_policy>(delta_backoff, max_attempts))
        {
        }
    };

}} // namespace azure::storage
