// -----------------------------------------------------------------------------------------
// <copyright file="result_iterator_test.cpp" company="Microsoft">
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
#include "check_macros.h"
#include "test_base.h"
#include "wascore/util.h"

typedef std::function<azure::storage::result_segment<int>(const azure::storage::continuation_token &, size_t)> result_generator_type;

class test_result_provider
{
public:
    test_result_provider(size_t total_results, size_t max_results_per_segment, bool return_full_segment)
        : m_returned_results(0), m_total_results(total_results), m_max_results_per_segment(max_results_per_segment), m_return_full_segment(return_full_segment)
    {
    }

    azure::storage::result_segment<int> get_next_segment(const azure::storage::continuation_token &token, size_t maximum_results)
    {
        if (token.next_marker() != m_expected_token.next_marker())
        {
            throw std::runtime_error("unexpected continuation token");
        }

        if (m_returned_results >= m_total_results)
        {
            // all results have been returned
            return azure::storage::result_segment<int>();
        }

        if (m_max_results_per_segment != 0)
        {
            CHECK(maximum_results <= m_max_results_per_segment);
        }

        if (maximum_results == 0)
        {
            // each segment return up to "m_max_results_per_segment" results
            maximum_results = m_max_results_per_segment;
        }

        size_t res_segment_size = (size_t)std::min(m_total_results - m_returned_results, maximum_results);
        if (!m_return_full_segment)
        {
            // return less results
            res_segment_size = std::rand() % (res_segment_size + 1);
        }

        std::vector<int> res_vec;
        res_vec.reserve(res_segment_size);
        for (size_t i = 1; i <= res_segment_size; ++i) {
            res_vec.push_back((int)(m_returned_results + i));
        }

        m_returned_results += res_segment_size;

        if (m_returned_results == m_total_results)
        {
            m_expected_token = azure::storage::continuation_token();
        }
        else
        {
            m_expected_token = azure::storage::continuation_token(azure::storage::core::convert_to_string(m_returned_results));
        }

        return azure::storage::result_segment<int>(res_vec, m_expected_token);
    }

private:
    size_t m_returned_results;
    size_t m_total_results;
    size_t m_max_results_per_segment;
    bool m_return_full_segment;
    azure::storage::continuation_token m_expected_token;
};

result_generator_type get_result_generator(test_result_provider& result_provider)
{
    return [&result_provider](const azure::storage::continuation_token &token, size_t max_results_per_segment) -> azure::storage::result_segment<int>
    {
        return result_provider.get_next_segment(token, max_results_per_segment);
    };
}

void result_iterator_check_result_number(result_generator_type generator, size_t max_results, size_t max_results_per_segment, size_t num_of_results)
{
    azure::storage::result_iterator<int> iter(generator, max_results, max_results_per_segment);
    int count = 0;
    for (auto& item : iter)
    {
        count++;
        CHECK(item == count);
    }

    CHECK_EQUAL(num_of_results, (size_t)count);
}

SUITE(Core)
{
    TEST_FIXTURE(test_base, result_iterator_default_constructor)
    {
        azure::storage::result_iterator<int> iter;
        CHECK_THROW(*iter, std::runtime_error);

        CHECK(iter++ == iter);
        CHECK(++iter == iter);
    }

    TEST_FIXTURE(test_base, result_iterator_get_results)
    {
        size_t test_data[][6] = {
                /*{total_results, segment_size, return_full_segment, max_results, max_results_per_segment, num_of_results_returned}*/
                { 0, 1000, 1, 0, 0, 0 },            // empty result
                { 1, 1000, 1, 0, 0, 1 },            // one result
                { 99, 1000, 1, 0, 0, 99 },          // num_of_results = max_results_per_segment - 1
                { 100, 1000, 1, 0, 0, 100 },        // num_of_results = max_results_per_segment
                { 101, 1000, 1, 0, 0, 101 },        // num_of_results = max_results_per_segment + 1
                { 200, 1000, 1, 0, 0, 200 },        // num_of_results = max_results_per_segment * 2
                { 3199, 1000, 1, 0, 0, 3199 },      // num_of_results = max_results_per_segment * 32 - 1
                { 3200, 1000, 1, 0, 0, 3200 },      // num_of_results = max_results_per_segment * 32
                { 3201, 1000, 1, 0, 0, 3201 },      // num_of_results = max_results_per_segment * 32 + 1
                { 100, 1000, 1, 1, 0, 1 },          // return partial results: max_results = 1
                { 100, 1000, 1, 50, 0, 50 },        // return partial results: max_results = 50
                { 100, 1000, 1, 99, 0, 99 },        // return partial results: max_results = 99
                { 100, 1000, 1, 100, 0, 100 },      // return all results: max_results = 100
                { 100, 1000, 1, 1, 1, 1 },          // max_results_per_segment = 1
                { 100, 1000, 1, 50, 1, 50 },        // max_results_per_segment = 1
                { 100, 1000, 1, 50, 2, 50 },        // max_results_per_segment = 2
                { 100, 1000, 1, 99, 50, 99 },       // max_results_per_segment = 50
                { 100, 1000, 1, 90, 200, 90 },      // max_results_per_segment = 200
                { 100, 1000, 1, 90, 1000, 90 },     // max_results_per_segment = 1000
                { 100, 1000, 1, 90, 2000, 90 },     // max_results_per_segment = 2000
                { 250, 1000, 1, 251, 10, 250 },     // max_results > total_results
                { 250, 1000, 1, 500, 50, 250 },     // max_results > total_results
                { 250, 1000, 1, 1000, 1000, 250 },  // max_results > total_results
                { 500, 100, 0, 20, 50, 20 },        // return_full_segment = false
                { 500, 100, 0, 400, 100, 400 },     // return_full_segment = false
                { 500, 100, 0, 1000, 100, 500 },    // return_full_segment = false
        };

        for (auto& data : test_data)
        {
            test_result_provider provider(data[0], data[1], data[2] != 0);
            auto generator = get_result_generator(provider);

            result_iterator_check_result_number(generator, data[3], data[4], data[5]);
        }
    }

    TEST_FIXTURE(test_base, result_iterator_fail_to_fetch_first_segment)
    {
        auto generator = [](const azure::storage::continuation_token &, size_t ) -> azure::storage::result_segment<int>
        {
            throw std::runtime_error("result_iterator first segment error");
        };

        CHECK_THROW(azure::storage::result_iterator<int>(generator, 0, 0), std::runtime_error);
    }

    TEST_FIXTURE(test_base, result_iterator_fail_to_fetch_next_segment)
    {
        test_result_provider provider(1000, 200, true);
        bool throw_exception_in_result_generator = false;
        auto generator = [&provider, &throw_exception_in_result_generator](const azure::storage::continuation_token &token, size_t max_results_per_segment) -> azure::storage::result_segment<int>
        {
            if (throw_exception_in_result_generator)
            {
                throw std::runtime_error("result_iterator next segment error");
            }
            else
            {
                return provider.get_next_segment(token, max_results_per_segment);
            }
        };

        azure::storage::result_iterator<int> end_of_results;
        azure::storage::result_iterator<int> iter(generator, 0, 0);

        int count = 0;
        try
        {
            for (; iter != end_of_results; ++iter)
            {
                count++;
                CHECK(*iter == (int)count);

                if (count == 600)
                {
                    throw_exception_in_result_generator = true;
                }
            }
        }
        catch (std::runtime_error&)
        {
        }

        CHECK_EQUAL(600, count);

        throw_exception_in_result_generator = false;

        // retry to continue. the iterator shall be able to read out remaining results upon successful retry
        ++iter;
        for (; iter != end_of_results; ++iter)
        {
            count++;
            CHECK(*iter == (int)count);
        }
    }
}
