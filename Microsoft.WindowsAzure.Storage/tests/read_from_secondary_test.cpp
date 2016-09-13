// -----------------------------------------------------------------------------------------
// <copyright file="read_from_secondary_test.cpp" company="Microsoft">
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
#include "blob_test_base.h"

class basic_always_retry_policy : public azure::storage::basic_retry_policy
{
public:

    basic_always_retry_policy(std::vector<azure::storage::retry_context> expected_retry_context_list, std::vector<azure::storage::retry_info> retry_info_list)
        : azure::storage::basic_retry_policy(), m_current_retry_count(0),
        m_expected_retry_context_list(std::move(expected_retry_context_list)),
        m_retry_info_list(std::move(retry_info_list))
    {
        CHECK_EQUAL(m_expected_retry_context_list.size(), m_retry_info_list.size() + 1);
    }

    azure::storage::retry_info evaluate(const azure::storage::retry_context& retry_context, azure::storage::operation_context context) override
    {
        CHECK_EQUAL(m_current_retry_count++, retry_context.current_retry_count());
        CHECK(retry_context.current_retry_count() < static_cast<int>(m_expected_retry_context_list.size()));
        CHECK(retry_context.current_location_mode() == m_expected_retry_context_list[retry_context.current_retry_count()].current_location_mode());
        CHECK(retry_context.next_location() == m_expected_retry_context_list[retry_context.current_retry_count()].next_location());

        if (static_cast<size_t>(retry_context.current_retry_count()) < m_retry_info_list.size())
        {
            return m_retry_info_list[retry_context.current_retry_count()];
        }
        else
        {
            return azure::storage::retry_info();
        }
    }

    azure::storage::retry_policy clone() const override
    {
        return azure::storage::retry_policy(std::make_shared<basic_always_retry_policy>(m_expected_retry_context_list, m_retry_info_list));
    }

private:

    std::vector<azure::storage::retry_context> m_expected_retry_context_list;
    std::vector<azure::storage::retry_info> m_retry_info_list;
    int m_current_retry_count;
};

class multi_location_test_helper
{
public:

    multi_location_test_helper(const azure::storage::storage_uri& uri, azure::storage::storage_location initial_location, azure::storage::operation_context context, const std::vector<azure::storage::retry_context>& expected_retry_context_list, const std::vector<azure::storage::retry_info>& retry_info_list)
        : m_uri(uri), m_initial_location(initial_location), m_retry_info_list(retry_info_list),
        m_policy(std::make_shared<basic_always_retry_policy>(expected_retry_context_list, retry_info_list)),
        m_request_counter(0), m_error(false), m_context(context), m_context_results_offset((int)context.request_results().size())
    {
        m_context.set_sending_request([this] (web::http::http_request& request, azure::storage::operation_context)
        {
            if (!m_error)
            {
                azure::storage::storage_location location(m_request_counter == 0 ? m_initial_location : m_retry_info_list[m_request_counter - 1].target_location());
                const web::http::uri& uri = m_uri.get_location_uri(location);
                if (!request.request_uri().has_same_authority(uri))
                {
                    m_error = true;
                }
            }

            m_request_counter++;
        });
    }

    ~multi_location_test_helper()
    {
        m_context.set_sending_request(std::function<void(web::http::http_request&, azure::storage::operation_context)>());

        CHECK(!m_error);
        CHECK(m_initial_location == m_context.request_results()[m_context_results_offset].target_location());
        CHECK_EQUAL(m_retry_info_list.size() + 1, m_context.request_results().size() - m_context_results_offset);
        for (size_t i = 0; i < m_retry_info_list.size(); ++i)
        {
            CHECK(m_retry_info_list[i].target_location() == m_context.request_results()[m_context_results_offset + i + 1].target_location());

            // This check assumes that datetime::to_interval() returns the time in microseconds/10
            std::chrono::microseconds interval((m_context.request_results()[m_context_results_offset + i + 1].start_time().to_interval() - m_context.request_results()[m_context_results_offset + i].end_time().to_interval()) / 10);
            CHECK(m_retry_info_list[i].retry_interval() < interval);
        }
    }

    azure::storage::retry_policy policy() const
    {
        return m_policy;
    }

private:

    std::vector<azure::storage::retry_info> m_retry_info_list;
    azure::storage::storage_uri m_uri;
    azure::storage::storage_location m_initial_location;
    azure::storage::retry_policy m_policy;
    azure::storage::operation_context m_context;
    int m_context_results_offset;
    int m_request_counter;
    bool m_error;
};

void test_container_download_attributes(azure::storage::cloud_blob_container& container, azure::storage::location_mode mode, azure::storage::storage_location initial_location, azure::storage::operation_context context, const std::vector<azure::storage::retry_context>& expected_retry_context_list, const std::vector<azure::storage::retry_info>& retry_info_list)
{
    multi_location_test_helper helper(container.service_client().base_uri(), initial_location, context, expected_retry_context_list, retry_info_list);
    azure::storage::blob_request_options options;
    options.set_location_mode(mode);
    options.set_retry_policy(helper.policy());
    CHECK_THROW(container.download_attributes(azure::storage::access_condition(), options, context), azure::storage::storage_exception);
}

static azure::storage::retry_info create_fake_retry_info(azure::storage::storage_location target_location, azure::storage::location_mode updated_mode, std::chrono::milliseconds interval)
{
    azure::storage::retry_context dummy_context(0, azure::storage::request_result(), target_location, updated_mode);
    azure::storage::retry_info fake_info(dummy_context);
    fake_info.set_retry_interval(interval);
    return fake_info;
}

void add_updated_location_mode_list(std::vector<azure::storage::retry_context>& expected_retry_context_list, std::vector<azure::storage::retry_info>& retry_info_list)
{
    retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::secondary_only, std::chrono::seconds(4)));
    expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only));
    retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::primary_only, std::chrono::seconds(1)));
    expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::primary, azure::storage::location_mode::primary_only));
    retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::primary_then_secondary, std::chrono::seconds(1)));
    expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::primary, azure::storage::location_mode::primary_then_secondary));
    retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_then_primary, std::chrono::seconds(1)));
    expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::primary, azure::storage::location_mode::secondary_then_primary));
}

void test_multi_location_retries(std::function<void(azure::storage::location_mode, azure::storage::storage_location, const std::vector<azure::storage::retry_context>&, const std::vector<azure::storage::retry_info>&)> test)
{
    {
        std::vector<azure::storage::retry_context> expected_retry_context_list;
        std::vector<azure::storage::retry_info> retry_info_list;
        expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::primary, azure::storage::location_mode::primary_only));
        test(azure::storage::location_mode::primary_only, azure::storage::storage_location::primary, expected_retry_context_list, retry_info_list);
    }

    {
        std::vector<azure::storage::retry_context> expected_retry_context_list;
        std::vector<azure::storage::retry_info> retry_info_list;
        expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only));
        test(azure::storage::location_mode::secondary_only, azure::storage::storage_location::secondary, expected_retry_context_list, retry_info_list);
    }

    {
        std::vector<azure::storage::retry_context> expected_retry_context_list;
        std::vector<azure::storage::retry_info> retry_info_list;
        expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::secondary, azure::storage::location_mode::primary_then_secondary));
        test(azure::storage::location_mode::primary_then_secondary, azure::storage::storage_location::primary, expected_retry_context_list, retry_info_list);
    }

    {
        std::vector<azure::storage::retry_context> expected_retry_context_list;
        std::vector<azure::storage::retry_info> retry_info_list;
        expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::primary, azure::storage::location_mode::secondary_then_primary));
        test(azure::storage::location_mode::secondary_then_primary, azure::storage::storage_location::secondary, expected_retry_context_list, retry_info_list);
    }

    {
        std::vector<azure::storage::retry_context> expected_retry_context_list;
        std::vector<azure::storage::retry_info> retry_info_list;
        expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::primary, azure::storage::location_mode::primary_only));
        retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(6)));
        expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::primary, azure::storage::location_mode::primary_only));
        retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_only, std::chrono::seconds(1)));
        expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::primary, azure::storage::location_mode::primary_only));
        add_updated_location_mode_list(expected_retry_context_list, retry_info_list);
        test(azure::storage::location_mode::primary_only, azure::storage::storage_location::primary, expected_retry_context_list, retry_info_list);
    }

    {
        std::vector<azure::storage::retry_context> expected_retry_context_list;
        std::vector<azure::storage::retry_info> retry_info_list;
        expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only));
        retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(6)));
        expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only));
        retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only, std::chrono::seconds(1)));
        expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_only));
        add_updated_location_mode_list(expected_retry_context_list, retry_info_list);
        test(azure::storage::location_mode::secondary_only, azure::storage::storage_location::secondary, expected_retry_context_list, retry_info_list);
    }

    {
        std::vector<azure::storage::retry_context> expected_retry_context_list;
        std::vector<azure::storage::retry_info> retry_info_list;
        expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::secondary, azure::storage::location_mode::primary_then_secondary));
        retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::primary_then_secondary, std::chrono::seconds(6)));
        expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::primary, azure::storage::location_mode::primary_then_secondary));
        retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::primary_then_secondary, std::chrono::seconds(1)));
        expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::secondary, azure::storage::location_mode::primary_then_secondary));
        add_updated_location_mode_list(expected_retry_context_list, retry_info_list);
        test(azure::storage::location_mode::primary_then_secondary, azure::storage::storage_location::primary, expected_retry_context_list, retry_info_list);
    }

    {
        std::vector<azure::storage::retry_context> expected_retry_context_list;
        std::vector<azure::storage::retry_info> retry_info_list;
        expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::primary, azure::storage::location_mode::secondary_then_primary));
        retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::primary, azure::storage::location_mode::secondary_then_primary, std::chrono::seconds(6)));
        expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_then_primary));
        retry_info_list.push_back(create_fake_retry_info(azure::storage::storage_location::secondary, azure::storage::location_mode::secondary_then_primary, std::chrono::seconds(1)));
        expected_retry_context_list.push_back(azure::storage::retry_context(0, azure::storage::request_result(), azure::storage::storage_location::primary, azure::storage::location_mode::secondary_then_primary));
        add_updated_location_mode_list(expected_retry_context_list, retry_info_list);
        test(azure::storage::location_mode::secondary_then_primary, azure::storage::storage_location::secondary, expected_retry_context_list, retry_info_list);
    }
}

SUITE(Core)
{
    TEST_FIXTURE(container_test_base, blob_multi_location_retries)
    {
        test_multi_location_retries(std::bind(test_container_download_attributes, m_container, std::placeholders::_1, std::placeholders::_2, m_context, std::placeholders::_3, std::placeholders::_4));
    }

    TEST_FIXTURE(blob_test_base, location_lock)
    {
        for (int i = 0; i < 2; i++)
        {
            auto index = utility::conversions::print_string(i);
            auto blob = m_container.get_block_blob_reference(_XPLATSTR("blockblob") + index);

            blob.upload_text(blob.name(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        }

        azure::storage::blob_request_options options;
        options.set_retry_policy(azure::storage::no_retry_policy());

        options.set_location_mode(azure::storage::location_mode::primary_only);
        auto results = m_container.list_blobs_segmented(utility::string_t(), true, azure::storage::blob_listing_details::none, 1, azure::storage::continuation_token(), options, m_context);
        CHECK_EQUAL(4U, m_context.request_results().size());

        azure::storage::continuation_token token = results.continuation_token();
        options.set_location_mode(azure::storage::location_mode::secondary_only);
        CHECK_THROW(m_container.list_blobs_segmented(utility::string_t(), true, azure::storage::blob_listing_details::none, 1, token, options, m_context), azure::storage::storage_exception);
        CHECK_EQUAL(4U, m_context.request_results().size());

        auto container = m_client.get_container_reference(m_container.name() + _XPLATSTR("-missing"));

        token.set_target_location(azure::storage::storage_location::secondary);
        CHECK_THROW(container.list_blobs_segmented(utility::string_t(), true, azure::storage::blob_listing_details::none, 1, token, options, m_context), azure::storage::storage_exception);
        CHECK_EQUAL(5U, m_context.request_results().size());

        options.set_location_mode(azure::storage::location_mode::primary_only);
        CHECK_THROW(container.list_blobs_segmented(utility::string_t(), true, azure::storage::blob_listing_details::none, 1, token, options, m_context), azure::storage::storage_exception);
        CHECK_EQUAL(5U, m_context.request_results().size());
    }
}
