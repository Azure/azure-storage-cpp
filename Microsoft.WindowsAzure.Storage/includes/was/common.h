// -----------------------------------------------------------------------------------------
// <copyright file="common.h" company="Microsoft">
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

#include <iterator>
#include <unordered_map>

#include "core.h"
#include "retry_policies.h"

#ifndef _WIN32
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/asio/ip/address.hpp>
#endif

#pragma push_macro("min")
#undef min

namespace azure { namespace storage {

    namespace protocol
    {
        class service_stats_reader;
        class response_parsers;
        class list_blobs_reader;
    }

    template<typename result_type> class result_iterator;

    /// <summary>
    /// Represents the user meta-data for queues, containers and blobs.
    /// </summary>
    typedef std::unordered_map<utility::string_t, utility::string_t> cloud_metadata;

    /// <summary>
    /// Represents a continuation token for listing operations. 
    /// </summary>
    /// <remarks>A method that may return a partial set of results via a result segment object also returns a continuation token, 
    /// which can be used in a subsequent call to return the next set of available results.</remarks>
    class continuation_token
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::continuation_token" /> class.
        /// </summary>
        continuation_token()
            : m_target_location(storage_location::unspecified)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::continuation_token" /> class.
        /// </summary>
        /// <param name="next_marker">The next_marker.</param>
        explicit continuation_token(utility::string_t next_marker)
            : m_next_marker(std::move(next_marker)), m_target_location(storage_location::unspecified)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::continuation_token" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::continuation_token" /> object.</param>
        continuation_token(continuation_token&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::continuation_token" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::continuation_token" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::continuation_token" /> object with properties set.</returns>
        continuation_token& operator=(continuation_token&& other)
        {
            if (this != &other)
            {
                m_next_marker = std::move(other.m_next_marker);
                m_target_location = std::move(other.m_target_location);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets the next marker for continuing results for enumeration operations.
        /// </summary>
        /// <returns>The next marker for continuing results for enumeration operations.</returns>
        const utility::string_t& next_marker() const
        {
            return m_next_marker;
        }

        /// <summary>
        /// Sets the next marker for continuing results for enumeration operations.
        /// </summary>
        /// <param name="next_marker">The next marker for continuing results for enumeration operations.</param>
        void set_next_marker(utility::string_t next_marker)
        {
            m_next_marker = std::move(next_marker);
        }

        /// <summary>
        /// Gets the location that the token applies to.
        /// </summary>
        /// <returns>The location that the token applies to.</returns>
        storage_location target_location() const
        {
            return m_target_location;
        }

        /// <summary>
        /// Sets the location that the token applies to.
        /// </summary>
        /// <param name="value">The location that the token applies to.</param>
        void set_target_location(storage_location value)
        {
            m_target_location = value;
        }

        /// <summary>
        /// Gets a value indicating whether the continuation token is empty.
        /// </summary>
        /// <returns><c>true</c> if the continuation token is empty; otherwise, <c>false</c>.</returns>
        bool empty() const
        {
            return m_next_marker.empty();
        }

    private:

        utility::string_t m_next_marker;
        storage_location m_target_location;
    };

    /// <summary>
    /// Represents a result segment retrieved from the total set of possible results.
    /// </summary>
    /// <typeparam name="result_type">The type of the result.</typeparam>
    template<typename result_type>
    class result_segment
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::result_segment{result_type}" /> class.
        /// </summary>
        result_segment()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::result_segment{result_type}" /> class.
        /// </summary>
        /// <param name="results">An enumerable collection of results.</param>
        /// <param name="token">The continuation token.</param>
        result_segment(std::vector<result_type> results, continuation_token token)
            : m_results(std::move(results)), m_continuation_token(std::move(token))
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::result_segment" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::result_segment" /> object.</param>
        result_segment(result_segment&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::result_segment" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::result_segment" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::result_segment" /> object with properties set.</returns>
        result_segment& operator=(result_segment&& other)
        {
            if (this != &other)
            {
                m_results = std::move(other.m_results);
                m_continuation_token = std::move(other.m_continuation_token);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets an enumerable collection of results.
        /// </summary>
        /// <returns>An enumerable collection of results.</returns>
        const std::vector<result_type>& results() const
        {
            return m_results;
        }

        /// <summary>
        /// Gets the continuation token used to retrieve the next segment of results.
        /// </summary>
        /// <returns>The continuation token.</returns>
        const azure::storage::continuation_token& continuation_token() const
        {
            return m_continuation_token;
        }

    private:

        result_type& results(size_t index)
        {
            if(index < m_results.size())
                return m_results[index];
            throw std::runtime_error("index is out of the results range");
        }

        std::vector<result_type> m_results;
        azure::storage::continuation_token m_continuation_token;

        friend class result_iterator<result_type>;
    };

    /// <summary>
    /// Represents a result iterator that could be used to enumerate results in the result set in a lazy way.
    /// </summary>
    /// <typeparam name="result_type">The type of the result.</typeparam>
    template<typename result_type>
    class result_iterator : public std::iterator<std::input_iterator_tag, result_type>
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::result_iterator{result_type}" /> class.
        /// </summary>
        result_iterator() :
            m_result_generator(nullptr),
            m_segment_index(0),
            m_returned_results(0),
            m_max_results(0),
            m_max_results_per_segment(0)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::result_iterator{result_type}" /> class.
        /// </summary>
        /// <param name="result_generator">The result segment generator.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned 
        /// by the result iterator. If this value is 0, the maximum possible number of results will be returned.</param>
        /// <param name="max_results_per_segment">A non-negative integer value that indicates the maximum number of results to 
        /// be returned in one segment. If this value is 0, the maximum possible number of results returned in a segment will be
        ///  determined by individual service.</param>
        result_iterator(std::function<result_segment<result_type>(const continuation_token &, size_t)> result_generator, utility::size64_t max_results, size_t max_results_per_segment) :
            m_result_generator(std::move(result_generator)),
            m_segment_index(0),
            m_returned_results(0),
            m_max_results(max_results),
            m_max_results_per_segment(max_results_per_segment)
        {
            fetch_first_segment();
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::result_iterator" /> class.
        /// </summary>
        /// <param name="other">A reference to a set of <see cref="azure::storage::result_iterator" /> on which to base the new instance.</param>
        result_iterator(result_iterator&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::result_iterator" /> object.
        /// </summary>
        /// <param name="other">A reference to a set of <see cref="azure::storage::result_iterator" /> to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::result_iterator" /> object with properties set.</returns>
        result_iterator& operator=(result_iterator&& other)
        {
            if (this != &other)
            {
                m_result_generator = std::move(other.m_result_generator);
                m_result_segment = std::move(other.m_result_segment);
                m_segment_index = other.m_segment_index;
                m_returned_results = other.m_returned_results;
                m_max_results = other.m_max_results;
                m_max_results_per_segment = other.m_max_results_per_segment;
            }

            return *this;
        }
#endif

        const result_type& operator*() const
        {
            if (passed_the_end())
            {
                throw std::runtime_error("cannot dereference past-the-end iterator");
            }

            return m_result_segment.results()[m_segment_index];
        }

        result_type& operator*()
        {
            if (passed_the_end())
            {
                throw std::runtime_error("cannot dereference past-the-end iterator");
            }

            return m_result_segment.results(m_segment_index);
        }

        const result_type* operator->() const
        {
            return (std::pointer_traits<const result_type*>::pointer_to(**this));
        }

        result_type* operator->()
        {
            return (std::pointer_traits<result_type*>::pointer_to(**this));
        }

        result_iterator& operator++()
        {
            // preincrement
            // - if the iterator has passed the end, do nothing
            // - if any exception is thrown within the method, do nothing
            if (!passed_the_end())
            {
                if (m_segment_index + 1 == m_result_segment.results().size() && !m_result_segment.continuation_token().empty())
                {
                    fetch_next_segment();
                }
                else
                {
                    m_segment_index++;
                }

                m_returned_results++;
            }

            return (*this);
        }

        result_iterator operator++(int)
        {
            // postincrement
            result_iterator tmp = *this;
            ++*this;
            return (tmp);
        }

        bool operator==(const result_iterator& rhs) const
        {
            // test for iterator equality
            // - comparison of two past-the-end iterators returns true
            // - comparison of a non-end iterator and a past-the-end iterators returns false
            // - comparison of two non-end iterators returns false
            return passed_the_end() && rhs.passed_the_end();
        }

        bool operator!=(const result_iterator& rhs) const
        {
            return (!(*this == rhs));
        }

    private:

        void fetch_first_segment()
        {
            if (nullptr != m_result_generator)
            {
                m_result_segment = m_result_generator(continuation_token(), get_remaining_results_num());
                m_segment_index = 0;

                if (m_result_segment.results().empty())
                {
                    // continue if returned result segment is empty
                    fetch_next_segment();
                }
            }
        }

        void fetch_next_segment()
        {
            if (nullptr != m_result_generator && !m_result_segment.continuation_token().empty())
            {
                auto tmp_segment = m_result_generator(m_result_segment.continuation_token(), get_remaining_results_num());
                while (tmp_segment.results().empty() && !tmp_segment.continuation_token().empty())
                {
                    tmp_segment = m_result_generator(tmp_segment.continuation_token(), get_remaining_results_num());
                }

                m_result_segment = std::move(tmp_segment);
                m_segment_index = 0;
            }
        }

        size_t get_remaining_results_num() const
        {
            if (m_max_results == 0)
            {
                // 0 indicates to return maximum possible number of results
                return m_max_results_per_segment;
            }

            return (size_t)std::min(m_max_results - m_returned_results, (utility::size64_t)m_max_results_per_segment);
        }

        bool passed_the_end() const
        {
            return (m_segment_index == m_result_segment.results().size() && m_result_segment.continuation_token().empty()) ||
                   (m_max_results > 0 && m_returned_results >= m_max_results);
        }

        std::function<result_segment<result_type>(const continuation_token &, size_t)> m_result_generator;
        result_segment<result_type> m_result_segment;
        size_t m_segment_index;
        utility::size64_t m_returned_results;
        utility::size64_t m_max_results;
        size_t m_max_results_per_segment;
    };

    /// <summary>
    /// begin method that treats a result iterator as a range.
    /// </summary>
    /// <typeparam name="result_type">The type of the result.</typeparam>
    /// <param name="range">An <see cref="azure::storage::result_iterator{result_type}" /> object that represents a range starting from the specified iterator.</param>
    /// <returns>An <see cref="azure::storage::result_iterator{result_type}" /> object that represents begin of the range.</returns>
    template<typename result_type>
    result_iterator<result_type> begin(result_iterator<result_type> range)
    {
        return std::move(range);
    }

    /// <summary>
    /// end method that treats a result iterator as a range.
    /// </summary>
    /// <typeparam name="result_type">The type of the result.</typeparam>
    /// <returns>An <see cref="azure::storage::result_iterator{result_type}" /> object that represents end of the range.</returns>
    template<typename result_type>
    result_iterator<result_type> end(result_iterator<result_type>)
    {
        return result_iterator<result_type>();
    }

    /// <summary>
    /// Specifies which items to include when setting service properties.
    /// </summary>
    class service_properties_includes
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::service_properties_includes" /> class.
        /// </summary>
        service_properties_includes()
            : m_logging(false), m_hour_metrics(false), m_minute_metrics(false), m_cors(false)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::service_properties_includes" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::service_properties_includes" /> object.</param>
        service_properties_includes(service_properties_includes&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::service_properties_includes" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::service_properties_includes" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::service_properties_includes" /> object with properties set.</returns>
        service_properties_includes& operator=(service_properties_includes&& other)
        {
            if (this != &other)
            {
                m_logging = std::move(other.m_logging);
                m_hour_metrics = std::move(other.m_hour_metrics);
                m_minute_metrics = std::move(other.m_minute_metrics);
                m_cors = std::move(other.m_cors);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets an <see cref="azure::storage::service_properties_includes" /> object that includes all available service properties.
        /// </summary>
        /// <returns>An <see cref="azure::storage::service_properties_includes" /> object with all properties set to <c>true</c>.</returns>
        static service_properties_includes all()
        {
            service_properties_includes includes;
            includes.set_logging(true);
            includes.set_hour_metrics(true);
            includes.set_minute_metrics(true);
            includes.set_cors(true);
            return includes;
        }

        /// <summary>
        /// Gets an <see cref="azure::storage::service_properties_includes" /> object that includes all available file service properties.
        /// </summary>
        /// <returns>An <see cref="azure::storage::service_properties_includes" /> object with all file service properties set to <c>true</c>.</returns>
        static service_properties_includes file()
        {
            service_properties_includes includes;
            includes.set_logging(false);
            includes.set_hour_metrics(true);
            includes.set_minute_metrics(true);
            includes.set_cors(true);
            return includes;
        }

        /// <summary>
        /// Indicates whether logging properties are to be included when the service properties are next modified.
        /// </summary>
        /// <returns><c>true</c> if logging properties are to be included; otherwise, <c>false</c>.</returns>
        bool logging() const
        {
            return m_logging;
        }

        /// <summary>
        /// Specifies whether logging properties are to be included when the service properties are next modified.
        /// </summary>
        /// <param name="value"><c>true</c> if logging properties are to be included; otherwise, <c>false</c></param>
        void set_logging(bool value)
        {
            m_logging = value;
        }

        /// <summary>
        /// Indicates whether hour metrics properties are to be included when the service properties are next modified.
        /// </summary>
        /// <returns><c>true</c> if hour metrics properties are to be included; otherwise, <c>false</c>.</returns>
        bool hour_metrics() const
        {
            return m_hour_metrics;
        }

        /// <summary>
        /// Specifies whether hour metrics properties are to be included when the service properties are next modified.
        /// </summary>
        /// <param name="value"><c>true</c> if hour metrics properties are to be included; otherwise, <c>false</c></param>
        void set_hour_metrics(bool value)
        {
            m_hour_metrics = value;
        }

        /// <summary>
        /// Indicates whether minute metrics properties are to be included when the service properties are next modified.
        /// </summary>
        /// <returns><c>true</c> if minute metrics properties are to be included; otherwise, <c>false</c>.</returns>
        bool minute_metrics() const
        {
            return m_minute_metrics;
        }

        /// <summary>
        /// Specifies whether minute metrics properties are to be included when the service properties are next modified.
        /// </summary>
        /// <param name="value"><c>true</c> if minute metrics properties are to be included; otherwise, <c>false</c></param>
        void set_minute_metrics(bool value)
        {
            m_minute_metrics = value;
        }

        /// <summary>
        /// Indicates whether CORS properties are to be included when the service properties are next modified.
        /// </summary>
        /// <returns><c>true</c> if CORS properties are to be included; otherwise, <c>false</c>.</returns>
        bool cors() const
        {
            return m_cors;
        }

        /// <summary>
        /// Specifies whether CORS properties are to be included when the service properties are next modified.
        /// </summary>
        /// <param name="value"><c>true</c> if CORS properties are to be included; otherwise, <c>false</c></param>
        void set_cors(bool value)
        {
            m_cors = value;
        }

    private:

        bool m_logging;
        bool m_hour_metrics;
        bool m_minute_metrics;
        bool m_cors;
    };

    /// <summary>
    /// Class representing a set of properties pertaining to the Blob, Queue, or Table service.
    /// </summary>
    class service_properties
    {
    public:

        /// <summary>
        /// An object representing the service properties pertaining to logging.
        /// </summary>
        class logging_properties
        {
        public:

            /// <summary>
            /// Initializes a new instance of the <see cref="azure::storage::service_properties::logging_properties" /> struct.
            /// </summary>
            logging_properties()
                : m_read_enabled(false), m_write_enabled(false), m_delete_enabled(false), m_retention_enabled(false), m_retention_days(0)
            {
            }

#if defined(_MSC_VER) && _MSC_VER < 1900
            // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
            // have implicitly-declared move constructor and move assignment operator.

            /// <summary>
            /// Initializes a new instance of the <see cref="azure::storage::service_properties::logging_properties" /> class based on an existing instance.
            /// </summary>
            /// <param name="other">An existing <see cref="azure::storage::service_properties::logging_properties" /> object.</param>
            logging_properties(logging_properties&& other)
            {
                *this = std::move(other);
            }

            /// <summary>
            /// Returns a reference to an <see cref="azure::storage::service_properties::logging_properties" /> object.
            /// </summary>
            /// <param name="other">An existing <see cref="azure::storage::service_properties::logging_properties" /> object to use to set properties.</param>
            /// <returns>An <see cref="azure::storage::service_properties::logging_properties" /> object with properties set.</returns>
            logging_properties& operator=(logging_properties&& other)
            {
                if (this != &other)
                {
                    m_version = std::move(other.m_version);
                    m_read_enabled = std::move(other.m_read_enabled);
                    m_write_enabled = std::move(other.m_write_enabled);
                    m_delete_enabled = std::move(other.m_delete_enabled);
                    m_retention_enabled = std::move(other.m_retention_enabled);
                    m_retention_days = std::move(other.m_retention_days);
                }
                return *this;
            }
#endif

            /// <summary>
            /// Gets the version of Storage Analytics in use.
            /// </summary>
            /// <returns>A string specifying the version of Storage Analytics to use. Set this value to "1.0".</returns>
            const utility::string_t& version() const
            {
                return m_version;
            }

            /// <summary>
            /// Gets the version of Storage Analytics to use.
            /// </summary>
            /// <param name="value">A string specifying the version of Storage Analytics to use. Set this value to "1.0".</param>
            void set_version(utility::string_t value)
            {
                m_version = std::move(value);
            }

            /// <summary>
            /// Gets a value indicating whether all read requests should be logged.
            /// </summary>
            /// <returns><c>true</c> if all read requests should be logged; otherwise, <c>false</c>.</returns>
            bool read_enabled() const
            {
                return m_read_enabled;
            }

            /// <summary>
            /// Sets a value indicating whether all read requests should be logged.
            /// </summary>
            /// <param name="value">Use <c>true</c> if all read requests should be logged; otherwise, <c>false</c>.</param>
            void set_read_enabled(bool value)
            {
                m_read_enabled = value;
            }

            /// <summary>
            /// Gets a value indicating whether all write requests should be logged.
            /// </summary>
            /// <returns><c>true</c> if all write requests should be logged; otherwise, <c>false</c>.</returns>
            bool write_enabled() const
            {
                return m_write_enabled;
            }

            /// <summary>
            /// Sets a value indicating whether all write requests should be logged.
            /// </summary>
            /// <param name="value">Use <c>true</c> if all write requests should be logged; otherwise, <c>false</c>.</param>
            void set_write_enabled(bool value)
            {
                m_write_enabled = value;
            }

            /// <summary>
            /// Gets a value indicating whether all delete requests should be logged.
            /// </summary>
            /// <returns><c>true</c> if all delete requests should be logged; otherwise, <c>false</c>.</returns>
            bool delete_enabled() const
            {
                return m_delete_enabled;
            }

            /// <summary>
            /// Sets a value indicating whether all delete requests should be logged.
            /// </summary>
            /// <param name="value">Use <c>true</c> if all delete requests should be logged; otherwise, <c>false</c>.</param>
            void set_delete_enabled(bool value)
            {
                m_delete_enabled = value;
            }

            /// <summary>
            /// Gets a value indicating whether a retention policy is enabled for service logs.
            /// </summary>
            /// <returns><c>true</c> if a retention policy is enabled; otherwise, <c>false</c>.</returns>
            bool retention_policy_enabled() const
            {
                return m_retention_enabled;
            }

            /// <summary>
            /// Sets a value indicating whether a retention policy is enabled for service logs.
            /// </summary>
            /// <param name="value">Use <c>true</c> if a retention policy is enabled; otherwise, <c>false</c>.</param>
            void set_retention_policy_enabled(bool value)
            {
                m_retention_enabled = value;
            }

            /// <summary>
            /// Gets the number of days that logging data should be retained.
            /// </summary>
            /// <returns>The number of days to retain the logging data. If this value is 0, the retention policy is disabled.</returns>
            int retention_days() const
            {
                return m_retention_days;
            }

            /// <summary>
            /// Sets the number of days that logging data should be retained.
            /// </summary>
            /// <param name="value">The number of days to retain the logging data, or 0 to disable the retention policy.</param>
            void set_retention_days(int value)
            {
                m_retention_days = value;
            }

        private:

            utility::string_t m_version;
            bool m_read_enabled;
            bool m_write_enabled;
            bool m_delete_enabled;
            bool m_retention_enabled;
            int m_retention_days;
        };

        /// <summary>
        /// An object representing the service properties pertaining to metrics.
        /// </summary>
        class metrics_properties
        {
        public:

            /// <summary>
            /// Initializes a new instance of the <see cref="azure::storage::service_properties::metrics_properties" /> class.
            /// </summary>
            metrics_properties()
                : m_include_apis(false), m_retention_enabled(false), m_retention_days(0)
            {
            }

#if defined(_MSC_VER) && _MSC_VER < 1900
            // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
            // have implicitly-declared move constructor and move assignment operator.

            /// <summary>
            /// Initializes a new instance of the <see cref="azure::storage::service_properties::metrics_properties" /> class based on an existing instance.
            /// </summary>
            /// <param name="other">An existing <see cref="azure::storage::service_properties::metrics_properties" /> object.</param>
            metrics_properties(metrics_properties&& other)
            {
                *this = std::move(other);
            }

            /// <summary>
            /// Returns a reference to an <see cref="azure::storage::service_properties::metrics_properties" /> object.
            /// </summary>
            /// <param name="other">An existing <see cref="azure::storage::service_properties::metrics_properties" /> object to use to set properties.</param>
            /// <returns>An <see cref="azure::storage::service_properties::metrics_properties" /> object with properties set.</returns>
            metrics_properties& operator=(metrics_properties&& other)
            {
                if (this != &other)
                {
                    m_version = std::move(other.m_version);
                    m_enabled = std::move(other.m_enabled);
                    m_include_apis = std::move(other.m_include_apis);
                    m_retention_enabled = std::move(other.m_retention_enabled);
                    m_retention_days = std::move(other.m_retention_days);
                }
                return *this;
            }
#endif

            /// <summary>
            /// Gets the version of Storage Analytics in use.
            /// </summary>
            /// <returns>A string specifying the version of Storage Analytics to use. Set this value to "1.0".</returns>
            const utility::string_t& version() const
            {
                return m_version;
            }

            /// <summary>
            /// Gets the version of Storage Analytics to use.
            /// </summary>
            /// <param name="value">A string specifying the version of Storage Analytics to use. Set this value to "1.0".</param>
            void set_version(utility::string_t value)
            {
                m_version = std::move(value);
            }

            /// <summary>
            /// Gets a value indicating whether metrics is enabled for the storage service.
            /// </summary>
            /// <returns><c>true</c> if metrics is enabled for the storage service; otherwise, <c>false</c>.</returns>
            bool enabled() const
            {
                return m_enabled;
            }

            /// <summary>
            /// Sets a value indicating whether metrics is enabled for the storage service.
            /// </summary>
            /// <param name="value">Use <c>true</c> to enable metrics for the storage service; otherwise, <c>false</c>.</param>
            void set_enabled(bool value)
            {
                m_enabled = value;
            }

            /// <summary>
            /// Gets a value indicating whether metrics should generate summary statistics for called API operations.
            /// </summary>
            /// <returns><c>true</c> if metrics should generate summary statistics for called API operations; otherwise, <c>false</c>.</returns>
            bool include_apis() const
            {
                return m_include_apis;
            }

            /// <summary>
            /// Sets a value indicating whether metrics should generate summary statistics for called API operations.
            /// </summary>
            /// <param name="value">Use <c>true</c> if metrics should generate summary statistics for called API operations; otherwise, <c>false</c>.</param>
            void set_include_apis(bool value)
            {
                m_include_apis = value;
            }

            /// <summary>
            /// Gets a value indicating whether a retention policy is enabled for metrics.
            /// </summary>
            /// <returns><c>true</c> if a retention policy is enabled; otherwise, <c>false</c>.</returns>
            bool retention_policy_enabled() const
            {
                return m_retention_enabled;
            }

            /// <summary>
            /// Sets a value indicating whether a retention policy is enabled for metrics.
            /// </summary>
            /// <param name="value">Use <c>true</c> to enable retention policy; otherwise, <c>false</c>.</param>
            void set_retention_policy_enabled(bool value)
            {
                m_retention_enabled = value;
            }

            /// <summary>
            /// Gets the number of days that metrics data should be retained.
            /// </summary>
            /// <returns>The number of days to retain the metrics data. If this value is 0, the retention policy is disabled.</returns>
            int retention_days() const
            {
                return m_retention_days;
            }

            /// <summary>
            /// Sets the number of days that metrics data should be retained.
            /// </summary>
            /// <param name="value">The number of days to retain the metrics data, or 0 to disable the retention policy.</param>
            void set_retention_days(int value)
            {
                m_retention_days = value;
            }

        private:

            utility::string_t m_version;
            bool m_enabled;
            bool m_include_apis;
            bool m_retention_enabled;
            int m_retention_days;
        };

        /// <summary>
        /// Class representing the service properties pertaining to CORS.
        /// </summary>
        class cors_rule
        {
        public:

            /// <summary>
            /// Initializes a new instance of the <see cref="azure::storage::service_properties::cors_rule" /> class.
            /// </summary>
            cors_rule()
            {
            }

#if defined(_MSC_VER) && _MSC_VER < 1900
            // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
            // have implicitly-declared move constructor and move assignment operator.

            /// <summary>
            /// Initializes a new instance of the <see cref="azure::storage::service_properties::cors_rule" /> class based on an existing instance.
            /// </summary>
            /// <param name="other">An existing <see cref="azure::storage::service_properties::cors_rule" /> object.</param>
            cors_rule(cors_rule&& other)
            {
                *this = std::move(other);
            }

            /// <summary>
            /// Returns a reference to an <see cref="azure::storage::service_properties::cors_rule" /> object.
            /// </summary>
            /// <param name="other">An existing <see cref="azure::storage::service_properties::cors_rule" /> object to use to set properties.</param>
            /// <returns>An <see cref="azure::storage::service_properties::cors_rule" /> object with properties set.</returns>
            cors_rule& operator=(cors_rule&& other)
            {
                if (this != &other)
                {
                    m_allowed_origins = std::move(other.m_allowed_origins);
                    m_exposed_headers = std::move(other.m_exposed_headers);
                    m_allowed_headers = std::move(other.m_allowed_headers);
                    m_allowed_methods = std::move(other.m_allowed_methods);
                    m_max_age = std::move(other.m_max_age);
                }
                return *this;
            }
#endif

            /// <summary>
            /// Gets domain names allowed via CORS.
            /// </summary>
            /// <returns>A collection of strings containing the allowed domain names, limited to 64.</returns>
            const std::vector<utility::string_t>& allowed_origins() const
            {
                return m_allowed_origins;
            }

            /// <summary>
            /// Gets domain names allowed via CORS.
            /// </summary>
            /// <returns>A collection of strings containing the allowed domain names, limited to 64.</returns>
            std::vector<utility::string_t>& allowed_origins()
            {
                return m_allowed_origins;
            }

            /// <summary>
            /// Sets domain names allowed via CORS.
            /// </summary>
            /// <param name="value">A collection of strings containing the allowed domain names, limited to 64.</param>
            void set_allowed_origins(std::vector<utility::string_t> value)
            {
                m_allowed_origins = std::move(value);
            }

            /// <summary>
            /// Gets response headers that should be exposed to client via CORS.
            /// </summary>
            /// <returns>A collection of strings containing exposed headers, limited to 64 defined headers and two prefixed headers.</returns>
            const std::vector<utility::string_t>& exposed_headers() const
            {
                return m_exposed_headers;
            }

            /// <summary>
            /// Gets response headers that should be exposed to client via CORS.
            /// </summary>
            /// <returns>A collection of strings containing exposed headers, limited to 64 defined headers and two prefixed headers.</returns>
            std::vector<utility::string_t>& exposed_headers()
            {
                return m_exposed_headers;
            }

            /// <summary>
            /// Sets response headers that should be exposed to client via CORS.
            /// </summary>
            /// <param name="value">A collection of strings containing exposed headers, limited to 64 defined headers and two prefixed headers.</param>
            void set_exposed_headers(std::vector<utility::string_t> value)
            {
                m_exposed_headers = std::move(value);
            }

            /// <summary>
            /// Gets headers allowed to be part of the CORS request.
            /// </summary>
            /// <returns>A collection of strings containing allowed headers, limited to 64 defined headers and two prefixed headers.</returns>
            const std::vector<utility::string_t>& allowed_headers() const
            {
                return m_allowed_headers;
            }

            /// <summary>
            /// Gets headers allowed to be part of the CORS request.
            /// </summary>
            /// <returns>A collection of strings containing allowed headers, limited to 64 defined headers and two prefixed headers.</returns>
            std::vector<utility::string_t>& allowed_headers()
            {
                return m_allowed_headers;
            }

            /// <summary>
            /// Sets headers allowed to be part of the CORS request.
            /// </summary>
            /// <param name="value">A collection of strings containing allowed headers, limited to 64 defined headers and two prefixed headers.</param>
            void set_allowed_headers(std::vector<utility::string_t> value)
            {
                m_allowed_headers = std::move(value);
            }

            /// <summary>
            /// Gets the HTTP methods permitted to execute for this origin.
            /// </summary>
            /// <returns>The allowed HTTP methods.</returns>
            const std::vector<web::http::method>& allowed_methods() const
            {
                return m_allowed_methods;
            }

            /// <summary>
            /// Gets the HTTP methods permitted to execute for this origin.
            /// </summary>
            /// <returns>The allowed HTTP methods.</returns>
            std::vector<web::http::method>& allowed_methods()
            {
                return m_allowed_methods;
            }

            /// <summary>
            /// Sets the HTTP methods permitted to execute for this origin.
            /// </summary>
            /// <param name="value">The allowed HTTP methods.</param>
            void set_allowed_methods(std::vector<web::http::method> value)
            {
                m_allowed_methods = std::move(value);
            }

            /// <summary>
            /// Gets the length of time in seconds that a preflight response should be cached by browser.
            /// </summary>
            /// <returns>The maximum number of seconds to cache the response.</returns>
            const std::chrono::seconds max_age() const
            {
                return m_max_age;
            }

            /// <summary>
            /// Sets the length of time in seconds that a preflight response should be cached by browser.
            /// </summary>
            /// <param name="value">The maximum number of seconds to cache the response.</param>
            void set_max_age(const std::chrono::seconds value)
            {
                m_max_age = value;
            }

        private:

            std::vector<utility::string_t> m_allowed_origins;
            std::vector<utility::string_t> m_exposed_headers;
            std::vector<utility::string_t> m_allowed_headers;
            std::vector<web::http::method> m_allowed_methods;
            std::chrono::seconds m_max_age;
        };

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::service_properties" /> class.
        /// </summary>
        service_properties()
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::service_properties" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::service_properties" /> object.</param>
        service_properties(service_properties&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::service_properties" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::service_properties" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::service_properties" /> object with properties set.</returns>
        service_properties& operator=(service_properties&& other)
        {
            if (this != &other)
            {
                m_logging = std::move(other.m_logging);
                m_minute_metrics = std::move(other.m_minute_metrics);
                m_hour_metrics = std::move(other.m_hour_metrics);
                m_cors_rules = std::move(other.m_cors_rules);
                m_default_service_version = std::move(other.m_default_service_version);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets the logging properties for the service.
        /// </summary>
        /// <returns>The logging properties for the service.</returns>
        const logging_properties& logging() const
        {
            return m_logging;
        }

        /// <summary>
        /// Gets the logging properties for the service.
        /// </summary>
        /// <returns>The logging properties for the service.</returns>
        logging_properties& logging()
        {
            return m_logging;
        }

        /// <summary>
        /// Sets the logging properties for the service.
        /// </summary>
        /// <param name="value">The logging properties for the service.</param>
        void set_logging(logging_properties value)
        {
            m_logging = std::move(value);
        }

        /// <summary>
        /// Gets the hour metrics properties for the service.
        /// </summary>
        /// <returns>The hour metrics properties for the service.</returns>
        const metrics_properties& hour_metrics() const
        {
            return m_hour_metrics;
        }

        /// <summary>
        /// Gets the hour metrics properties for the service.
        /// </summary>
        /// <returns>The hour metrics properties for the service.</returns>
        metrics_properties& hour_metrics()
        {
            return m_hour_metrics;
        }

        /// <summary>
        /// Sets the hour metrics properties for the service.
        /// </summary>
        /// <param name="value">The hour metrics properties for the service.</param>
        void set_hour_metrics(metrics_properties value)
        {
            m_hour_metrics = std::move(value);
        }

        /// <summary>
        /// Gets the minute metrics properties for the service.
        /// </summary>
        /// <returns>The minute metrics properties for the service.</returns>
        const metrics_properties& minute_metrics() const
        {
            return m_minute_metrics;
        }

        /// <summary>
        /// Gets the minute metrics properties for the service.
        /// </summary>
        /// <returns>The minute metrics properties for the service.</returns>
        metrics_properties& minute_metrics()
        {
            return m_minute_metrics;
        }

        /// <summary>
        /// Sets the minute metrics properties for the service.
        /// </summary>
        /// <param name="value">The minute metrics properties for the service.</param>
        void set_minute_metrics(metrics_properties value)
        {
            m_minute_metrics = std::move(value);
        }

        /// <summary>
        /// Gets the Cross Origin Resource Sharing (CORS) properties for the service.
        /// </summary>
        /// <returns>The CORS properties for the service.</returns>
        const std::vector<cors_rule>& cors() const
        {
            return m_cors_rules;
        }

        /// <summary>
        /// Gets the Cross Origin Resource Sharing (CORS) properties for the service.
        /// </summary>
        /// <returns>The CORS properties for the service.</returns>
        std::vector<cors_rule>& cors()
        {
            return m_cors_rules;
        }

        /// <summary>
        /// Sets the Cross Origin Resource Sharing (CORS) properties for the service.
        /// </summary>
        /// <param name="value">The CORS properties for the service.</param>
        void set_cors(std::vector<cors_rule> value)
        {
            m_cors_rules = std::move(value);
        }

        /// <summary>
        /// Gets the default service version for the Blob service.
        /// </summary>
        /// <returns>The default service version for the Blob service.</returns>
        const utility::string_t& default_service_version() const
        {
            return m_default_service_version;
        }

        /// <summary>
        /// Sets the default service version for the Blob service.
        /// </summary>
        /// <param name="value">The default service version for the Blob service.</param>
        void set_default_service_version(utility::string_t value)
        {
            m_default_service_version = std::move(value);
        }

    private:

        logging_properties m_logging;
        metrics_properties m_minute_metrics;
        metrics_properties m_hour_metrics;
        std::vector<cors_rule> m_cors_rules;
        utility::string_t m_default_service_version;
    };

    /// <summary>
    /// Enumeration representing the status of geo-replication for the storage account.
    /// </summary>
    enum class geo_replication_status
    {
        /// <summary>
        /// The status of geo-replication is unavailable for the storage account.
        /// </summary>
        unavailable,

        /// <summary>
        /// Geo-replication is live for the storage account.
        /// </summary>
        live,

        /// <summary>
        /// Data is being bootstrapped from the primary location to the secondary location.
        /// </summary>
        bootstrap,
    };

    /// <summary>
    /// Represents a set of stats pertaining to the Blob, Queue, or Table service.
    /// </summary>
    class service_stats
    {
    public:

        /// <summary>
        /// Represents the geo-replication stats for the service.
        /// </summary>
        class geo_replication_stats
        {
        public:

            /// <summary>
            /// Initializes a new instance of the <see cref="azure::storage::service_stats::geo_replication_stats" /> struct.
            /// </summary>
            geo_replication_stats()
                : m_status(geo_replication_status::unavailable)
            {
            }

#if defined(_MSC_VER) && _MSC_VER < 1900
            // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
            // have implicitly-declared move constructor and move assignment operator.

            /// <summary>
            /// Initializes a new instance of the <see cref="azure::storage::service_stats::geo_replication_stats" /> class based on an existing instance.
            /// </summary>
            /// <param name="other">An existing <see cref="azure::storage::service_stats::geo_replication_stats" /> object.</param>
            geo_replication_stats(geo_replication_stats&& other)
            {
                *this = std::move(other);
            }

            /// <summary>
            /// Returns a reference to an <see cref="azure::storage::service_stats::geo_replication_stats" /> object.
            /// </summary>
            /// <param name="other">An existing <see cref="azure::storage::service_stats::geo_replication_stats" /> object to use to set properties.</param>
            /// <returns>An <see cref="azure::storage::service_stats::geo_replication_stats" /> object with properties set.</returns>
            geo_replication_stats& operator=(geo_replication_stats&& other)
            {
                if (this != &other)
                {
                    m_status = std::move(other.m_status);
                    m_last_sync_time = std::move(other.m_last_sync_time);
                }
                return *this;
            }
#endif

            /// <summary>
            /// Gets the status of geo-replication.
            /// </summary>
            /// <returns>The status of geo-replication.</returns>
            geo_replication_status status() const
            {
                return m_status;
            }

            /// <summary>
            /// Gets the last synchronization time.
            /// </summary>
            /// <returns>The last synchronization time.</returns>
            /// <remarks>All primary writes preceding this value are guaranteed to be available for read operations. Primary writes following this point in time may or may not be available for reads.</remarks>
            const utility::datetime last_sync_time() const
            {
                return m_last_sync_time;
            }

        private:

            /// <summary>
            /// Sets the status of geo-replication.
            /// </summary>
            /// <param name="value">The status of geo-replication.</param>
            void set_status(geo_replication_status value)
            {
                m_status = value;
            }

            /// <summary>
            /// Sets the last synchronization time.
            /// </summary>
            /// <param name="value">The last synchronization time.</param>
            void set_last_sync_time(utility::datetime value)
            {
                m_last_sync_time = value;
            }

            geo_replication_status m_status;
            utility::datetime m_last_sync_time;

            friend class protocol::service_stats_reader;
        };

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::service_stats" /> class.
        /// </summary>
        service_stats()
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::service_stats" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::service_stats" /> object.</param>
        service_stats(service_stats&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::service_stats" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::service_stats" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::service_stats" /> object with properties set.</returns>
        service_stats& operator=(service_stats&& other)
        {
            if (this != &other)
            {
                m_geo_replication = std::move(other.m_geo_replication);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets the geo-replication stats.
        /// </summary>
        /// <returns>The geo-replication stats.</returns>
        const geo_replication_stats& geo_replication() const
        {
            return m_geo_replication;
        }

    private:

        /// <summary>
        /// Gets the geo-replication stats.
        /// </summary>
        /// <returns>The geo-replication stats.</returns>
        geo_replication_stats& geo_replication_private()
        {
            return m_geo_replication;
        }

        geo_replication_stats m_geo_replication;

        friend class protocol::service_stats_reader;
    };

    /// <summary>
    /// Specifies what types of messages to write to the log.
    /// </summary>
    enum client_log_level
    {
        /// <summary>
        /// Output no tracing and debugging messages.
        /// </summary>
        log_level_off = 0,

        /// <summary>
        /// Output error-handling messages.
        /// </summary>
        log_level_error,

        /// <summary>
        /// Output warnings and error-handling messages.
        /// </summary>
        log_level_warning,

        /// <summary>
        /// Output informational messages, warnings, and error-handling messages.
        /// </summary>
        log_level_informational,

        /// <summary>
        /// Output all debugging and tracing messages.
        /// </summary>
        log_level_verbose,
    };

    class operation_context;

    class _operation_context
    {
    public:

        /// <summary>
        /// Gets a string containing the client request ID.
        /// </summary>
        /// <returns>A string containing the client request ID.</returns>
        const utility::string_t& client_request_id() const
        {
            return m_client_request_id;
        }

        /// <summary>
        /// Sets the client request ID.
        /// </summary>
        /// <param name="client_request_id">A string containing the client request ID.</param>
        void set_client_request_id(utility::string_t client_request_id)
        {
            m_client_request_id = std::move(client_request_id);
        }

        /// <summary>
        /// Gets the start time of the request.
        /// </summary>
        /// <returns>The start time of the request.</returns>
        utility::datetime start_time() const
        {
            return m_start_time;
        }

        /// <summary>
        /// Sets the start time of the requeset.
        /// </summary>
        /// <param name="start_time">The start time of the request.</param>
        void set_start_time(utility::datetime start_time)
        {
            m_start_time = start_time;
        }

        /// <summary>
        /// Gets the end time of the request.
        /// </summary>
        /// <returns>The end time of the request.</returns>
        utility::datetime end_time() const
        {
            return m_end_time;
        }

        /// <summary>
        /// Sets the end time of the requeset.
        /// </summary>
        /// <param name="start_time">The end time of the request.</param>
        void set_end_time(utility::datetime end_time)
        {
            m_end_time = end_time;
        }

        /// <summary>
        /// Gets the level at which messages are logged.
        /// </summary>
        /// <returns>An <see cref="azure::storage::client_log_level" /> object indicating the level at which messages are logged.</returns>
        client_log_level log_level() const
        {
            return m_log_level;
        }

        /// <summary>
        /// Sets the level at which messages are logged.
        /// </summary>
        /// <param name="log_level">An <see cref="azure::storage::client_log_level" /> object indicating the level at which messages are to be logged.</param>
        void set_log_level(client_log_level log_level)
        {
            m_log_level = log_level;
        }

        /// <summary>
        /// Gets the proxy.
        /// </summary>
        /// <returns>An <see cref="web::web_proxy" /> object indicating the proxy.</returns>
        const web::web_proxy &proxy() const
        {
            return m_proxy;
        }

        /// <summary>
        /// Sets the proxy.
        /// </summary>
        /// <param name="proxy">An <see cref="web::web_proxy" /> object indicating the proxy.</param>
        /// <remarks>
        /// The proxy uri should be in the format "//host[:port]"
        /// </remarks>
        void set_proxy(web::web_proxy proxy)
        {
            m_proxy = std::move(proxy);
        }

        /// <summary>
        /// Gets the user headers provided for the request.
        /// </summary>
        /// <returns>A <see cref="web::http::http_headers" /> object containing user headers.</returns>
        web::http::http_headers& user_headers()
        {
            return m_user_headers;
        }

        /// <summary>
        /// Gets the user headers provided for the request.
        /// </summary>
        /// <returns>A <see cref="web::http::http_headers" /> object containing user headers.</returns>
        const web::http::http_headers& user_headers() const
        {
            return m_user_headers;
        }

        /// <summary>
        /// Gets the results of the request.
        /// </summary>
        /// <returns>An enumerable collection of <see cref="azure::storage::request_result" /> objects.</returns>
        const std::vector<request_result>& request_results() const
        {
            return m_request_results;
        }

        /// <summary>
        /// Adds a request result to the set of results.
        /// </summary>
        /// <param name="result">An <see cref="azure::storage::request_result" /> object.</param>
        void add_request_result(request_result result)
        {
            pplx::extensibility::scoped_critical_section_t l(m_request_results_lock);
            m_request_results.push_back(std::move(result));
        }

        /// <summary>
        /// Gets the function to call when sending a request.
        /// </summary>
        /// <returns>A pointer to a function that takes an <see cref="web::http::http_request" /> object 
        /// and an <see cref="azure::storage::operation_context" /> object.</returns>
        std::function<void(web::http::http_request &, operation_context)> sending_request() const
        {
            return m_sending_request;
        }

        /// <summary>
        /// Sets the function to call when sending a request.
        /// </summary>
        /// <param name="value">A pointer to a function that takes an <see cref="web::http::http_request" /> object 
        /// and an <see cref="azure::storage::operation_context" /> object.</param>
        void set_sending_request(std::function<void(web::http::http_request &, operation_context)> value)
        {
            m_sending_request = value;
        }

        /// <summary>
        /// Gets the function to call when receiving a response from a request.
        /// </summary>
        /// <returns>A pointer to a function that takes an <see cref="web::http::http_request" /> object, 
        /// an <see cref="web::http::http_response" /> object, and an <see cref="azure::storage::operation_context" /> object.</returns>
        std::function<void(web::http::http_request &, const web::http::http_response &, operation_context)> response_received() const
        {
            return m_response_received;
        }

        /// <summary>
        /// Sets the function to call when receiving a response from a request.
        /// </summary>
        /// <param name="value">A pointer to a function that takes an <see cref="web::http::http_request" /> object,
        /// an <see cref="web::http::http_response" /> object, and an <see cref="azure::storage::operation_context" /> object.</param>
        void set_response_received(std::function<void(web::http::http_request &, const web::http::http_response &, operation_context)> value)
        {
            m_response_received = value;
        }

#ifndef _WIN32
        /// <summary>
        /// Gets the logger object on this operation context.
        /// </summary>
        /// <returns>The <see cref="boost::log::sources::severity_logger{boost::log::trivial::severity_level}" /> object used by this operation context.</returns>
        boost::log::sources::severity_logger<boost::log::trivial::severity_level>& logger()
        {
            return m_logger;
        }

        /// <summary>
        /// Gets the logger object on this operation context.
        /// </summary>
        /// <returns>The <see cref="boost::log::sources::severity_logger{boost::log::trivial::severity_level}" /> object used by this operation context.</returns>
        const boost::log::sources::severity_logger<boost::log::trivial::severity_level>& logger() const
        {
            return m_logger;
        }

        /// <summary>
        /// Sets the logger object on this operation context.
        /// </summary>
        /// <param name="logger">The <see cref="boost::log::sources::severity_logger{boost::log::trivial::severity_level}" /> object to use for requests made by this operation context.</param>
        void set_logger(boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger)
        {
            m_logger = std::move(logger);
        }
#endif

    private:

        std::function<void(web::http::http_request &, operation_context)> m_sending_request;
        std::function<void(web::http::http_request &, const web::http::http_response &, operation_context)> m_response_received;
        utility::string_t m_client_request_id;
        web::http::http_headers m_user_headers;
        utility::datetime m_start_time;
        utility::datetime m_end_time;
        client_log_level m_log_level;
        web::web_proxy m_proxy;
        std::vector<request_result> m_request_results;
        pplx::extensibility::critical_section_t m_request_results_lock;
#ifndef _WIN32
        boost::log::sources::severity_logger<boost::log::trivial::severity_level> m_logger;
#endif
    };

    /// <summary>
    /// Represents the context for a request to the Windows Azure storage services, and provides additional runtime information about its execution.
    /// </summary>
    class operation_context
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::operation_context" /> class.
        /// </summary>
        WASTORAGE_API operation_context();

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::operation_context" /> class.
        /// </summary>
        /// <param name="context">A reference to an <see cref="azure::storage::operation_context" /> object.</param>
        operation_context(const operation_context& context)
            : m_impl(context.m_impl)
        {
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::operation_context" /> object.
        /// </summary>
        /// <param name="context">A reference to an <see cref="azure::storage::operation_context" /> object.</param>
        /// <returns>An <see cref="azure::storage::operation_context" /> object.</returns>
        operation_context& operator=(const operation_context& context)
        {
            m_impl = context.m_impl;
            return *this;
        }

        /// <summary>
        /// Gets the client request ID.
        /// </summary>
        /// <returns>The client request ID.</returns>
        const utility::string_t& client_request_id() const
        {
            return m_impl->client_request_id();
        }

        /// <summary>
        /// Sets the client request ID.
        /// </summary>
        /// <param name="client_request_id">The client request ID.</param>
        void set_client_request_id(utility::string_t client_request_id)
        {
            return m_impl->set_client_request_id(std::move(client_request_id));
        }

        /// <summary>
        /// Gets the start time of the operation.
        /// </summary>
        /// <returns>The start time of the operation.</returns>
        utility::datetime start_time() const
        {
            return m_impl->start_time();
        }

        /// <summary>
        /// Sets the start time of the operation.
        /// </summary>
        /// <param name="start_time">The start time of the operation.</param>
        void set_start_time(utility::datetime start_time)
        {
            return m_impl->set_start_time(start_time);
        }

        /// <summary>
        /// Gets the end time of the operation.
        /// </summary>
        /// <returns>The end time of the operation.</returns>
        utility::datetime end_time() const
        {
            return m_impl->end_time();
        }

        /// <summary>
        /// Sets the end time of the operation.
        /// </summary>
        /// <param name="end_time">The end time of the operation.</param>
        void set_end_time(utility::datetime end_time)
        {
            return m_impl->set_end_time(end_time);
        }

        /// <summary>
        /// Gets the default logging level to be used for subsequently created instances of the <see cref="azure::storage::operation_context" /> class.
        /// </summary>
        /// <returns>A value of type <see cref="azure::storage::client_log_level" /> that specifies which events are logged by default by instances of the <see cref="azure::storage::operation_context" />.</returns>
        WASTORAGE_API static client_log_level default_log_level();

        /// <summary>
        /// Sets the default logging level to be used for subsequently created instances of the <see cref="azure::storage::operation_context" /> class.
        /// </summary>
        /// <param name="log_level">A value of type <see cref="azure::storage::client_log_level" /> that specifies which events are logged by default by instances of the <see cref="azure::storage::operation_context" />.</param>
        WASTORAGE_API static void set_default_log_level(client_log_level log_level);

        /// <summary>
        /// Gets the logging level to be used for an instance of the <see cref="azure::storage::operation_context" /> class.
        /// </summary>
        /// <returns>A value of type <see cref="azure::storage::client_log_level" /> that specifies which events are logged by the <see cref="azure::storage::operation_context" />.</returns>
        client_log_level log_level() const
        {
            return m_impl->log_level();
        }

        /// <summary>
        /// Sets the logging level to be used for an instance of the <see cref="azure::storage::operation_context" /> class.
        /// </summary>
        /// <param name="log_level">A value of type <see cref="azure::storage::client_log_level" /> that specifies which events are logged by the <see cref="azure::storage::operation_context" />.</param>
        void set_log_level(client_log_level log_level)
        {
            m_impl->set_log_level(log_level);
        }

        /// <summary>
        /// Gets or sets additional headers on the request, for example, for proxy or logging information.
        /// </summary>
        /// <returns>A <see cref="web::http::http_headers" /> reference containing additional header information.</returns>
        web::http::http_headers& user_headers()
        {
            return m_impl->user_headers();
        }

        /// <summary>
        /// Gets or sets additional headers on the request, for example, for proxy or logging information.
        /// </summary>
        /// <returns>A <see cref="web::http::http_headers" /> reference containing additional header information.</returns>
        const web::http::http_headers& user_headers() const
        {
            return m_impl->user_headers();
        }

        /// <summary>
        /// Gets the set of request results that the current operation has created.
        /// </summary>
        /// <returns>A <see cref="std::vector" /> object that contains <see cref="azure::storage::request_result" /> objects that represent the request results created by the current operation.</returns>
        const std::vector<request_result>& request_results() const
        {
            return m_impl->request_results();
        }

        /// <summary>
        /// Sets the function to call when sending a request.
        /// </summary>
        /// <param name="value">A pointer to a function that takes an <see cref="web::http::http_request" /> object 
        /// and an <see cref="azure::storage::operation_context" /> object.</param>
        void set_sending_request(std::function<void(web::http::http_request &, operation_context)> value)
        {
            m_impl->set_sending_request(value);
        }

        /// <summary>
        /// Sets the function that is called when a response is received from the server.
        /// </summary>
        /// <param name="value">A pointer to a function that takes an <see cref="web::http::http_request" /> object,
        /// an <see cref="web::http::http_response" /> object, and an <see cref="azure::storage::operation_context" /> object.</param>
        void set_response_received(std::function<void(web::http::http_request &, const web::http::http_response &, operation_context)> value)
        {
            m_impl->set_response_received(value);
        }

        /// <summary>
        /// Gets the default proxy to be used for subsequently created instances of the <see cref="azure::storage::operation_context" /> class.
        /// </summary>
        /// <returns>A value of type <see cref="web::web_proxy" /> that specifies the default proxy by instances of the <see cref="azure::storage::operation_context" />.</returns>
        WASTORAGE_API static const web::web_proxy &default_proxy();

        /// <summary>
        /// Sets the default proxy to be used for subsequently created instances of the <see cref="azure::storage::operation_context" /> class.
        /// </summary>
        /// <param name="proxy">A value of type <see cref="web::proxy" /> that specifies default proxy by instances of the <see cref="azure::storage::operation_context" />.</param>
        /// <remarks>
        /// The proxy uri should be in the format "//host[:port]"
        /// </remarks>
        WASTORAGE_API static void set_default_proxy(web::web_proxy proxy);

        /// <summary>
        /// Gets the proxy.
        /// </summary>
        /// <returns>The proxy.</returns>
        const web::web_proxy &proxy() const
        {
            return m_impl->proxy();
        }

        /// <summary>
        /// Sets the proxy.
        /// </summary>
        /// <param name="proxy">The proxy.</param>
        /// <remarks>
        /// The proxy uri should be in the format "//host[:port]"
        /// </remarks>
        void set_proxy(web::web_proxy proxy)
        {
            m_impl->set_proxy(std::move(proxy));
        }

#ifndef _WIN32
        /// <summary>
        /// Gets the logger object on this operation context.
        /// </summary>
        /// <returns>The <see cref="boost::log::sources::severity_logger{boost::log::trivial::severity_level}" /> object used by this operation context.</returns>
        boost::log::sources::severity_logger<boost::log::trivial::severity_level>& logger()
        {
            return m_impl->logger();
        }

        /// <summary>
        /// Gets the logger object on this operation context.
        /// </summary>
        /// <returns>The <see cref="boost::log::sources::severity_logger{boost::log::trivial::severity_level}" /> object used by this operation context.</returns>
        const boost::log::sources::severity_logger<boost::log::trivial::severity_level>& logger() const
        {
            return m_impl->logger();
        }

        /// <summary>
        /// Sets the logger object on this operation context.
        /// </summary>
        /// <param name="logger">The <see cref="boost::log::sources::severity_logger{boost::log::trivial::severity_level}" /> object to use for requests made by this operation context.</param>
        void set_logger(boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger)
        {
            m_impl->set_logger(std::move(logger));
        }
#endif

        std::shared_ptr<_operation_context> _get_impl() const
        {
            return m_impl;
        }

    private:

        std::shared_ptr<_operation_context> m_impl;
        static client_log_level m_global_log_level;
        static web::web_proxy m_global_proxy;
    };

    /// <summary>
    /// Represents a shared access policy, which specifies the start time, expiry time, 
    /// and permissions for a shared access signature.
    /// </summary>
    class shared_access_policy
    {
    public:

        /// <summary>
        /// Specifies the set of possible signed protocols for a shared access account policy.
        /// </summary>
        enum protocols
        {
            /// <summary>
            /// Permission to use SAS only through https granted.
            /// </summary>
            https_only = 0x1,

            /// <summary>
            /// Permission to use SAS through https or http granted. Equivalent to not specifying any permission at all.
            /// </summary>
            https_or_http = 0x2
        };

        /// <summary>
        /// Get a canonical string representation of the protocols for a shared access policy.
        /// </summary>
        utility::string_t protocols_to_string() const
        {
            if (m_protocol == https_only)
            {
                return _XPLATSTR("https");
            }
            else
            {
                return _XPLATSTR("https,http");
            }
        }

        /// <summary>
        /// Specifies either a single IP Address or a single range of IP Addresses (a minimum and a maximum, inclusive.)
        /// </summary>
        class ip_address_or_range
        {
        public:

            /// <summary>
            /// Initializes a default new instance of the <see cref="azure::storage::shared_access_policy::ip_address_or_range" /> class.
            /// </summary>
            ip_address_or_range()
            : m_single_address(true)
            {}

            /// <summary>
            /// Initializes a new instance of the <see cref="azure::storage::shared_access_policy::ip_address_or_range" /> class using the a single ip address.
            /// </summary>
            /// <param name="address">The single IP address to use.</param>
            ip_address_or_range(utility::string_t address)
            : m_address(std::move(address)), m_single_address(true)
            {
                try_parse(m_address);
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="azure::storage::shared_access_policy::ip_address_or_range" /> class using
            /// a range of ip address specified by a minimum ip address and a maximum ip address.
            /// </summary>
            /// <param name="minimum_address">The minimum IP address of a range to use.</param>
            /// <param name="maximum_address">The maximum IP address of a range to use.</param>
            ip_address_or_range(utility::string_t minimum_address, utility::string_t maximum_address)
            : m_minimum_address(std::move(minimum_address)), m_maximum_address(std::move(maximum_address)), m_single_address(false)
            {
                validate_range();
            }

#if defined(_MSC_VER) && _MSC_VER < 1900
            // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
            // have implicitly-declared move constructor and move assignment operator.

            /// <summary>
            /// Initializes a new instance of the <see cref="azure::storage::shared_access_policy::ip_address_or_range" /> class based on an existing instance.
            /// </summary>
            /// <param name="other">An existing <see cref="azure::storage::shared_access_policy::ip_address_or_range" /> object.</param>
            ip_address_or_range(ip_address_or_range&& other)
            {
                *this = std::move(other);
            }

            /// <summary>
            /// Returns a reference to an <see cref="azure::storage::shared_access_policy::ip_address_or_range" /> object.
            /// </summary>
            /// <param name="other">An existing <see cref="azure::storage::shared_access_policy::ip_address_or_range" /> object to use to set properties.</param>
            /// <returns>An <see cref="azure::storage::shared_access_policy::ip_address_or_range" /> object with properties set.</returns>
            ip_address_or_range& operator=(ip_address_or_range&& other)
            {
                if (this != &other)
                {
                    m_address = std::move(other.m_address);
                    m_minimum_address = std::move(other.m_minimum_address);
                    m_maximum_address = std::move(other.m_maximum_address);
                    m_single_address = std::move(other.m_single_address);
                }
                return *this;
            }
#endif

            /// <summary>
            /// Gets the IP Address.
            /// Returns empty string if this object represents a range of IP addresses.
            /// </summary>
            /// <returns>The IP Address.</returns>
            const utility::string_t &address() const
            {
                return m_address;
            }

            /// <summary>
            /// Gets the minimum IP Address for the range, inclusive.
            /// Returns empty string if this object represents a single IP address.
            /// </summary>
            /// <returns>The minimum IP Address.</returns>
            const utility::string_t &minimum_address() const
            {
                return m_minimum_address;
            }

            /// <summary>
            /// Gets the maximum IP Address for the range, inclusive.
            /// Returns empty string if this object represents a single IP address.
            /// </summary>
            /// <returns>The maximum IP Address.</returns>
            const utility::string_t &maximum_address() const
            {
                return m_maximum_address;
            }

            /// <summary>
            /// True if this object represents a single IP Address, false if it represents a range.
            /// </summary>
            bool is_single_address() const
            {
                return m_single_address;
            }
            
            /// <summary>
            /// Get a canonical string representation of the ip address or range for a shared access policy.
            /// </summary>
            utility::string_t to_string() const
            {
                if (m_single_address)
                {
                    return m_address;
                }
                else
                {
                    return m_minimum_address + _XPLATSTR("-") + m_maximum_address;
                }
            }

        private:

            utility::string_t m_address;
            utility::string_t m_minimum_address;
            utility::string_t m_maximum_address;
            bool m_single_address;
#ifdef _WIN32
            struct ip_address
            {
                bool ipv4;
                unsigned long addr;
                unsigned short addr6[8];
            };
#else
            using ip_address = boost::asio::ip::address;
#endif
            WASTORAGE_API static ip_address try_parse(const utility::string_t &address);
            WASTORAGE_API void validate_range();
        };

        /// <summary>
        /// Get a canonical string representation of the permissions for a shared access policy.
        /// </summary>
        /// <remarks>
        // The order in which permissions are specified in the string is important. 
        // A good permission set is an in-order subset such as 'rwd', 'rwdl', 'raud', or 'raup'.
        // For example, 'rup', 'rl', and 'rw' are valid permission strings, but 'paur', 'dr', and 'ld' are not.
        /// </remarks>
        utility::string_t permissions_to_string() const
        {
            utility::string_t permissions;
            if (m_permission != none)
            {
                if (m_permission & read)
                {
                    permissions.push_back(_XPLATSTR('r'));
                }

                if (m_permission & add)
                {
                    permissions.push_back(_XPLATSTR('a'));
                }
                
                if (m_permission & create)
                {
                    permissions.push_back(_XPLATSTR('c'));
                }

                if (m_permission & write)
                {
                    permissions.push_back(_XPLATSTR('w'));
                }

                if (m_permission & update)
                {
                    permissions.push_back(_XPLATSTR('u'));
                }

                if (m_permission & del)
                {
                    permissions.push_back(_XPLATSTR('d'));
                }

                if (m_permission & process)
                {
                    permissions.push_back(_XPLATSTR('p'));
                }

                if (m_permission & list)
                {
                    permissions.push_back(_XPLATSTR('l'));
                }
            }

            return permissions;
        }

        /// <summary>
        /// Sets the permissions from the given string.
        /// </summary>
        /// <param name="value">The permissions for the shared access policy.</param>
        void set_permissions_from_string(const utility::string_t& value)
        {
            m_permission = 0;

            for (auto iter = value.cbegin(); iter != value.cend(); ++iter)
            {
                switch (*iter)
                {
                case _XPLATSTR('r'):
                    m_permission |= read;
                    break;

                case _XPLATSTR('w'):
                    m_permission |= write;
                    break;

                case _XPLATSTR('d'):
                    m_permission |= del;
                    break;

                case _XPLATSTR('l'):
                    m_permission |= list;
                    break;

                case _XPLATSTR('a'):
                    m_permission |= add;
                    break;

                case _XPLATSTR('u'):
                    m_permission |= update;
                    break;

                case _XPLATSTR('p'):
                    m_permission |= process;
                    break;

                case _XPLATSTR('c'):
                    m_permission |= create;
                    break;
                }
            }
        }

        /// <summary>
        /// Sets the permissions from the specified permissions.
        /// </summary>
        /// <param name="value">The permissions for the shared access policy.</param>
        void set_permissions(uint8_t value)
        {
            m_permission = value;
        }

        /// <summary>
        /// Gets the permissions for the shared access policy.
        /// </summary>
        /// <returns>The permissions for the shared access policy.</returns>
        uint8_t permission() const
        {
            return m_permission;
        }

        /// <summary>
        /// Sets the start time for the shared access policy.
        /// </summary>
        /// <param name="value">The start time for the access policy.</param>
        void set_start(utility::datetime value)
        {
            m_start = value;
        }

        /// <summary>
        /// Gets the start time for the shared access policy.
        /// </summary>
        /// <returns>The start time for the access policy.</returns>
        utility::datetime start() const
        {
            return m_start;
        }

        /// <summary>
        /// Sets the expiry time for the shared access policy.
        /// </summary>
        /// <param name="value">The expiry time for the shared access policy.</param>
        void set_expiry(utility::datetime value)
        {
            m_expiry = value;
        }

        /// <summary>
        /// Gets the expiry time for the shared access policy.
        /// </summary>
        /// <returns>The expiry time for the shared access policy.</returns>
        utility::datetime expiry() const
        {
            return m_expiry;
        }

        /// <summary>
        /// Indicates whether the <see cref="azure::storage::shared_access_policy" /> object is valid.
        /// </summary>
        /// <returns><c>true</c> if the <see cref="azure::storage::shared_access_policy" /> object is valid; otherwise, <c>false</c>.</returns>
        bool is_valid() const
        {
            return m_expiry.is_initialized() && (m_permission != none);
        }

        /// <summary>
        /// Sets the allowed protocols for a shared access signature associated with this shared access policy.
        /// </summary>
        /// <param name="value">The allowed protocols for the shared access policy.</param>
        void set_protocol(protocols value)
        {
            m_protocol = value;
        }

        /// <summary>
        /// Gets the allowed protocols for a shared access signature associated with this shared access policy.
        /// </summary>
        /// <returns>The allowed protocols for the shared access policy.</returns>
        protocols protocol() const
        {
            return m_protocol;
        }

        /// <summary>
        /// Sets the allowed IP address or IP address range for a shared access signature associated with this shared access policy.
        /// </summary>
        /// <param name="value">The allowed IP address or IP address range for the shared access policy.</param>
        void set_address_or_range(ip_address_or_range value)
        {
            m_ip_address_or_range = std::move(value);
        }

        /// <summary>
        /// Gets the allowed IP address or IP address range for a shared access signature associated with this shared access policy.
        /// </summary>
        /// <returns>The allowed IP address or IP address range for the shared access policy.</returns>
        const ip_address_or_range &address_or_range() const
        {
            return m_ip_address_or_range;
        }

    protected:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::shared_access_policy" /> class.
        /// </summary>
        shared_access_policy()
            : m_protocol(protocols::https_or_http), m_permission(none)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::shared_access_policy" /> class.
        /// </summary>
        /// <param name="expiry">The expiration date and time for the shared access policy.</param>
        /// <param name="permission">A mask specifying permissions for the shared access policy.</param>
        shared_access_policy(utility::datetime expiry, uint8_t permission)
            : m_expiry(expiry), m_protocol(protocols::https_or_http), m_permission(permission)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::shared_access_policy" /> class.
        /// </summary>
        /// <param name="start">The start date and time for the shared access policy.</param>
        /// <param name="expiry">The expiration date and time for the shared access policy.</param>
        /// <param name="permission">A mask specifying permissions for the shared access policy.</param>
        shared_access_policy(utility::datetime start, utility::datetime expiry, uint8_t permission)
            : m_start(start), m_expiry(expiry), m_protocol(protocols::https_or_http), m_permission(permission)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::shared_access_policy" /> class.
        /// </summary>
        /// <param name="start">The start date and time for the shared access policy.</param>
        /// <param name="expiry">The expiration date and time for the shared access policy.</param>
        /// <param name="permission">A mask specifying permissions for the shared access policy.</param>
        /// <param name="protocol">The allowed protocols for a shared access signature associated with this shared access policy.</param>
        /// <param name="address">The allowed IP address for a shared access signature associated with this shared access policy.</param>
        shared_access_policy(utility::datetime start, utility::datetime expiry, uint8_t permission, protocols protocol, utility::string_t address)
            : m_start(start), m_expiry(expiry), m_protocol(protocol), m_ip_address_or_range(ip_address_or_range(std::move(address))), m_permission(permission)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::shared_access_policy" /> class.
        /// </summary>
        /// <param name="start">The start date and time for the shared access policy.</param>
        /// <param name="expiry">The expiration date and time for the shared access policy.</param>
        /// <param name="permission">A mask specifying permissions for the shared access policy.</param>
        /// <param name="protocol">The allowed protocols for the shared access policy.</param>
        /// <param name="minimum_address">The minimum allowed address for an IP range for the shared access policy.</param>
        /// <param name="maximum_address">The maximum allowed address for an IP range for the shared access policy.</param>
        shared_access_policy(utility::datetime start, utility::datetime expiry, uint8_t permission, protocols protocol, utility::string_t minimum_address, utility::string_t maximum_address)
            : m_start(start), m_expiry(expiry), m_protocol(protocol), m_ip_address_or_range(ip_address_or_range(std::move(minimum_address), std::move(maximum_address))), m_permission(permission)
        {
        }

    private:

        enum permissions
        {
            none = 0, read = 1, write = 2, del = 4, list = 8, add = 0x10, update = 0x20, process = 0x40, create = 0x80
        };

        utility::datetime m_start;
        utility::datetime m_expiry;
        protocols m_protocol;
        ip_address_or_range m_ip_address_or_range;
        uint8_t m_permission;
    };

    template<typename Policy>
    class shared_access_policies : public std::map<utility::string_t, Policy>
    {
    };

    /// <summary>
    /// Represents the set of shared access policies for a Windows Azure Storage resource.
    /// </summary>
    template<typename Policy>
    class cloud_permissions
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_permissions" /> class.
        /// </summary>
        cloud_permissions()
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_permissions" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_permissions" /> object.</param>
        cloud_permissions(cloud_permissions&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_permissions" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_permissions" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::cloud_permissions" /> object with properties set.</returns>
        cloud_permissions& operator=(cloud_permissions&& other)
        {
            if (this != &other)
            {
                m_policies = std::move(other.m_policies);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets the set of shared access policies for the specified object.
        /// </summary>
        /// <returns>The set of shared access policies for the specified object.</returns>
        const shared_access_policies<Policy>& policies() const
        {
            return m_policies;
        }

        /// <summary>
        /// Gets the set of shared access policies for the specified object.
        /// </summary>
        /// <returns>The set of shared access policies for the specified object.</returns>
        shared_access_policies<Policy>& policies()
        {
            return m_policies;
        }

        /// <summary>
        /// Sets the set of shared access policies for the specified object.
        /// </summary>
        /// <param name="value">The set of shared access policies for the specified object.</param>
        void set_policies(shared_access_policies<Policy> value)
        {
            m_policies = std::move(value);
        }

    private:

        shared_access_policies<Policy> m_policies;
    };

    /// <summary>
    /// Represents a set of timeout and retry policy options that may be specified for an operation request.
    /// </summary>
    class request_options
    {
    public:

        // TODO: Optimize request_options to make copying and duplicating these objects unnecesary (maybe make it immutable)
        // TODO: Consider not overwriting unset values in request_options with the service's defaults because it is a confusing interface (the service's defaults would be used only when the user does not supply a request_options parameter)

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::request_options" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::request_options" /> object.</param>
        request_options(request_options&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::request_options" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::request_options" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::request_options" /> object with properties set.</returns>
        request_options& operator=(request_options&& other)
        {
            if (this != &other)
            {
                m_operation_expiry_time = std::move(other.m_operation_expiry_time);
                m_retry_policy = std::move(other.m_retry_policy);
                m_noactivity_timeout = std::move(other.m_noactivity_timeout);
                m_server_timeout = std::move(other.m_server_timeout);
                m_maximum_execution_time = std::move(other.m_maximum_execution_time);
                m_location_mode = std::move(other.m_location_mode);
                m_http_buffer_size = std::move(other.m_http_buffer_size);
            }
            return *this;
        }
#endif
        
        /// <summary>
        /// Gets the retry policy for the request.
        /// </summary>
        /// <returns>The retry policy for the request.</returns>
        azure::storage::retry_policy retry_policy() const
        {
            return m_retry_policy;
        }

        /// <summary>
        /// Sets the retry policy for the request.
        /// </summary>
        /// <param name="retry_policy">The retry policy for the request.</param>
        void set_retry_policy(azure::storage::retry_policy retry_policy)
        {
            m_retry_policy = retry_policy;
        }

        const std::chrono::seconds noactivity_timeout() const
        {
            return m_noactivity_timeout;
        }

        void set_noactivity_timeout(std::chrono::seconds noactivity_timeout)
        {
            m_noactivity_timeout = noactivity_timeout;
        }

        /// <summary>
        /// Gets the server timeout for the request. 
        /// </summary>
        /// <returns>The server timeout for the request.</returns>
        const std::chrono::seconds server_timeout() const
        {
            return m_server_timeout;
        }

        /// <summary>
        /// Sets the server timeout for the request. 
        /// </summary>
        /// <param name="server_timeout">The server timeout for the request.</param>
        void set_server_timeout(std::chrono::seconds server_timeout)
        {
            m_server_timeout = server_timeout;
        }

        /// <summary>
        /// Gets the maximum execution time across all potential retries.
        /// </summary>
        /// <returns>The maximum execution time.</returns>
        const std::chrono::seconds maximum_execution_time() const
        {
            return m_maximum_execution_time;
        }

        /// <summary>
        /// Sets the maximum execution time across all potential retries.
        /// </summary>
        /// <param name="maximum_execution_time">The maximum execution time.</param>
        void set_maximum_execution_time(std::chrono::seconds maximum_execution_time)
        {
            m_maximum_execution_time = maximum_execution_time;
        }

        /// <summary>
        /// Gets the location mode of the request.
        /// </summary>
        /// <returns>The location mode of the request.</returns>
        azure::storage::location_mode location_mode() const
        {
            return m_location_mode;
        }

        /// <summary>
        /// Sets the location mode of the request.
        /// </summary>
        /// <param name="location_mode">The location mode of the request.</param>
        void set_location_mode(azure::storage::location_mode location_mode)
        {
            m_location_mode = location_mode;
        }

        /// <summary>
        /// Gets the number of bytes to buffer when reading from and writing to a network stream.
        /// </summary>
        /// <returns>The number of bytes to buffer when reading from and writing to a network stream.</returns>
        /// <remarks>
        /// Using a larger buffer size is more efficient when downloading and uploading larger blobs because it will reduce CPU usage.
        /// </remarks>
        size_t http_buffer_size() const
        {
            return m_http_buffer_size;
        }

        /// <summary>
        /// Sets the number of bytes to buffer when reading from and writing to a network stream.
        /// </summary>
        /// <param name="http_buffer_size">The number of bytes to buffer when reading from and writing to a network stream.</param>
        /// <remarks>
        /// Using a larger buffer size is more efficient when downloading and uploading larger blobs because it will reduce CPU usage.
        /// </remarks>
        void set_http_buffer_size(size_t http_buffer_size)
        {
            m_http_buffer_size = http_buffer_size;
        }

        /// <summary>
        /// Gets the expiry time across all potential retries for the request.
        /// </summary>
        /// <returns>The expiry time.</returns>
        utility::datetime operation_expiry_time() const
        {
            return m_operation_expiry_time;
        }

    protected:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::request_options" /> class.
        /// </summary>
        WASTORAGE_API request_options();

        /// <summary>
        /// Applies the default set of request options.
        /// </summary>
        /// <param name="other">A reference to a set of <see cref="azure::storage::request_options" />.</param>
        /// <param name="apply_expiry">Specifies that an expiry time be applied to the
        /// request options. This parameter is used internally.</param>
        void apply_defaults(const request_options& other, bool apply_expiry)
        {
            if (!m_retry_policy.is_valid())
            {
                m_retry_policy = other.m_retry_policy;
            }

            m_noactivity_timeout.merge(other.m_noactivity_timeout);
            m_server_timeout.merge(other.m_server_timeout);
            m_maximum_execution_time.merge(other.m_maximum_execution_time);
            m_location_mode.merge(other.m_location_mode);
            m_http_buffer_size.merge(other.m_http_buffer_size);

            if (apply_expiry)
            {
                auto expiry_in_seconds = static_cast<std::chrono::seconds>(m_maximum_execution_time).count();
                if (!m_operation_expiry_time.is_initialized() && (expiry_in_seconds > 0))
                {
                    // This should not be copied from the other options, since
                    // this value should never have a default. Only if it has
                    // not been initialized by the copy constructor, now is the
                    // time to initialize it.
                    m_operation_expiry_time = utility::datetime::utc_now() + utility::datetime::from_seconds(static_cast<unsigned int>(expiry_in_seconds));
                }
            }
        }

    private:

        utility::datetime m_operation_expiry_time;
        azure::storage::retry_policy m_retry_policy;
        option_with_default<std::chrono::seconds> m_noactivity_timeout;
        option_with_default<std::chrono::seconds> m_server_timeout;
        option_with_default<std::chrono::seconds> m_maximum_execution_time;
        option_with_default<azure::storage::location_mode> m_location_mode;
        option_with_default<size_t> m_http_buffer_size;
    };

    /// <summary>
    /// Represents the status of a blob copy operation.
    /// </summary>
    enum class copy_status
    {
        /// <summary>
        /// The copy status is invalid.
        /// </summary>
        invalid,

        /// <summary>
        /// The copy operation is pending.
        /// </summary>
        pending,

        /// <summary>
        /// The copy operation succeeded.
        /// </summary>
        success,

        /// <summary>
        /// The copy operation has been aborted.
        /// </summary>
        aborted,

        /// <summary>
        /// The copy operation encountered an error.
        /// </summary>
        failed
    };

    /// <summary>
    /// Represents the attributes of a copy blob operation.
    /// </summary>
    class copy_state
    {
    public:

        copy_state()
        {
        }

        copy_state(const copy_state& other)
        {
            *this = other;
        }

        copy_state& operator=(const copy_state& other)
        {
            if (this != &other)
            {
                m_copy_id = other.m_copy_id;
                m_completion_time = other.m_completion_time;
                m_status_description = other.m_status_description;
                m_bytes_copied = other.m_bytes_copied;
                m_total_bytes = other.m_total_bytes;
                m_status = other.m_status;
                m_source = other.m_source;
                *m_source_uri = *other.m_source_uri;
                m_destination_snapshot_time = other.m_destination_snapshot_time;
            }
            return *this;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::copy_state" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::copy_state" /> object.</param>
        copy_state(copy_state&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::copy_state" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::copy_state" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::copy_state" /> object with properties set.</returns>
        copy_state& operator=(copy_state&& other)
        {
            if (this != &other)
            {
                m_copy_id = std::move(other.m_copy_id);
                m_completion_time = std::move(other.m_completion_time);
                m_status_description = std::move(other.m_status_description);
                m_bytes_copied = std::move(other.m_bytes_copied);
                m_total_bytes = std::move(other.m_total_bytes);
                m_status = std::move(other.m_status);
                m_source = std::move(other.m_source);
                *m_source_uri = std::move(*other.m_source_uri);
                m_destination_snapshot_time = std::move(other.m_destination_snapshot_time);
            }
            return *this;
        }

        /// <summary>
        /// Gets the ID of the copy blob operation.
        /// </summary>
        /// <returns>An ID string for the copy operation.</returns>
        const utility::string_t& copy_id() const
        {
            return m_copy_id;
        }

        /// <summary>
        /// Gets the time that the copy blob operation completed, and indicates whether completion was due 
        /// to a successful copy, whether the operation was cancelled, or whether the operation failed.
        /// </summary>
        /// <returns>A <see cref="utility::datetime" /> containing the completion time.</returns>
        utility::datetime completion_time() const
        {
            return m_completion_time;
        }

        /// <summary>
        /// Gets the status of the copy blob operation.
        /// </summary>
        /// <returns>An <see cref="azure::storage::copy_status" /> enumeration indicating the status of the copy operation.</returns>
        copy_status status() const
        {
            return m_status;
        }

        /// <summary>
        /// Gets the URI of the source blob for a copy operation.
        /// </summary>
        /// <returns>A <see cref="web::http::uri" /> indicating the source of a copy operation.</returns>
        const web::http::uri& source() const
        {
            if (m_source_uri->is_empty())
            {
                *m_source_uri = m_source;
            }
            return *m_source_uri;
        }

        /// <summary>
        /// Gets the URI string of the source blob for a copy operation.
        /// </summary>
        /// <returns>A <see cref="utility::string_t" /> indicating the source of a copy operation.</returns>
        const utility::string_t& source_raw() const
        {
            return m_source;
        }

        /// <summary>
        /// Gets the number of bytes copied in the operation so far.
        /// </summary>
        /// <returns>The number of bytes copied in the operation so far.</returns>
        int64_t bytes_copied() const
        {
            return m_bytes_copied;
        }

        /// <summary>
        /// Gets the total number of bytes in the source blob for the copy operation.
        /// </summary>
        /// <returns>The number of bytes in the source blob.</returns>
        int64_t total_bytes() const
        {
            return m_total_bytes;
        }

        /// <summary>
        /// Gets the description of the current status of the copy blob operation, if status is available.
        /// </summary>
        /// <returns>A status description string.</returns>
        const utility::string_t& status_description() const
        {
            return m_status_description;
        }

        /// <summary>
        /// Gets the incremental destination snapshot time for the latest incremental copy, if the time is available.
        /// </summary>
        /// <returns>A <see cref="utility::datetime" /> containing the destination snapshot time for the latest incremental copy.</returns>
        utility::datetime destination_snapshot_time() const
        {
            return m_destination_snapshot_time;
        }

    private:

        utility::string_t m_copy_id;
        utility::datetime m_completion_time;
        utility::string_t m_status_description;
        int64_t m_bytes_copied = 0;
        int64_t m_total_bytes = 0;
        copy_status m_status = copy_status::invalid;
        utility::string_t m_source;
        std::unique_ptr<web::http::uri> m_source_uri = std::unique_ptr<web::http::uri>(new web::http::uri());
        utility::datetime m_destination_snapshot_time;

        friend class protocol::response_parsers;
        friend class protocol::list_blobs_reader;
    };

}} // namespace azure::storage

#pragma pop_macro("min")
