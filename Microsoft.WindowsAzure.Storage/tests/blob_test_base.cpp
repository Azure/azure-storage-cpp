// -----------------------------------------------------------------------------------------
// <copyright file="blob_test_base.cpp" company="Microsoft">
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

#include "wascore/streams.h"

utility::string_t blob_service_test_base::fill_buffer_and_get_md5(std::vector<uint8_t>& buffer)
{
    return fill_buffer_and_get_md5(buffer, 0, buffer.size());
}

utility::string_t blob_service_test_base::fill_buffer_and_get_md5(std::vector<uint8_t>& buffer, size_t offset, size_t count)
{
    std::generate_n(buffer.begin(), buffer.size(), [] () -> uint8_t
    {
        return (uint8_t)(std::rand() % (int)UINT8_MAX);
    });

    azure::storage::core::hash_provider provider = azure::storage::core::hash_provider::create_md5_hash_provider();
    provider.write(buffer.data() + offset, count);
    provider.close();
    return provider.hash();
}

utility::string_t blob_service_test_base::get_random_container_name(size_t length)
{
    utility::string_t name;
    name.resize(length);
    std::generate_n(name.begin(), length, [] () -> utility::char_t
    {
        const utility::char_t possible_chars[] = { _XPLATSTR("abcdefghijklmnopqrstuvwxyz1234567890") };
        return possible_chars[std::rand() % (sizeof(possible_chars) / sizeof(utility::char_t) - 1)];
    });

    return utility::conversions::print_string(utility::datetime::utc_now().to_interval()) + name;
}

void test_base::check_parallelism(const azure::storage::operation_context& context, int expected_parallelism)
{
    typedef std::pair<utility::datetime, bool> request;
    
    std::vector<request> requests;
    for (auto iter = context.request_results().begin(); iter != context.request_results().end(); ++iter)
    {
        requests.push_back(request(iter->start_time(), true));
        requests.push_back(request(iter->end_time(), false));
    }

    std::sort(requests.begin(), requests.end(), [] (const request& a, const request& b) -> bool
    {
        return a.first.to_interval() < b.first.to_interval();
    });

    int current_count = 0;
    int max_count = 0;
    for (auto iter = requests.begin(); iter != requests.end(); ++iter)
    {
        if (iter->second)
        {
            current_count++;
        }
        else
        {
            current_count--;
        }

        CHECK(current_count >= 0);
        if (max_count < current_count)
        {
            max_count = current_count;
        }
    }
    
    // TODO: Investigate why this is only 5 instead of 6
    CHECK_EQUAL(expected_parallelism, max_count);
}

web::http::uri blob_service_test_base::defiddler(const web::http::uri& uri)
{
    if (uri.host() == _XPLATSTR("ipv4.fiddler"))
    {
        web::http::uri_builder builder(uri);
        builder.set_host(_XPLATSTR("127.0.0.1"));
        return builder.to_uri();
    }

    return uri;
}

void blob_service_test_base::check_blob_equal(const azure::storage::cloud_blob& expected, const azure::storage::cloud_blob& actual)
{
    CHECK(expected.type() == actual.type());
    CHECK_UTF8_EQUAL(expected.uri().primary_uri().to_string(), actual.uri().primary_uri().to_string());
    CHECK_UTF8_EQUAL(expected.uri().secondary_uri().to_string(), actual.uri().secondary_uri().to_string());
    CHECK_EQUAL(expected.is_snapshot(), actual.is_snapshot());
    CHECK(expected.snapshot_time() == actual.snapshot_time());
    CHECK_UTF8_EQUAL(expected.snapshot_qualified_uri().primary_uri().to_string(), actual.snapshot_qualified_uri().primary_uri().to_string());
    CHECK_UTF8_EQUAL(expected.snapshot_qualified_uri().secondary_uri().to_string(), actual.snapshot_qualified_uri().secondary_uri().to_string());
    check_blob_copy_state_equal(expected.copy_state(), actual.copy_state());
    check_blob_properties_equal(expected.properties(), actual.properties(), true);
}

void blob_service_test_base::check_blob_copy_state_equal(const azure::storage::copy_state& expected, const azure::storage::copy_state& actual)
{
    CHECK(expected.status() == actual.status());
    CHECK_EQUAL(expected.bytes_copied(), actual.bytes_copied());
    CHECK_EQUAL(expected.total_bytes(), actual.total_bytes());
    CHECK(expected.completion_time() == actual.completion_time());
    CHECK_UTF8_EQUAL(expected.copy_id(), actual.copy_id());
    CHECK_UTF8_EQUAL(expected.status_description(), actual.status_description());
    CHECK_UTF8_EQUAL(expected.source().to_string(), actual.source().to_string());
}

void blob_service_test_base::check_blob_properties_equal(const azure::storage::cloud_blob_properties& expected, const azure::storage::cloud_blob_properties& actual, bool check_settable_only)
{
    CHECK_UTF8_EQUAL(expected.etag(), actual.etag());
    CHECK(expected.last_modified() == actual.last_modified());
    CHECK_UTF8_EQUAL(expected.cache_control(), actual.cache_control());
    CHECK_UTF8_EQUAL(expected.content_disposition(), actual.content_disposition());
    CHECK_UTF8_EQUAL(expected.content_encoding(), actual.content_encoding());
    CHECK_UTF8_EQUAL(expected.content_language(), actual.content_language());
    CHECK_UTF8_EQUAL(expected.content_md5(), actual.content_md5());
    CHECK_UTF8_EQUAL(expected.content_type(), actual.content_type());
    if (!check_settable_only)
    {
        CHECK(expected.server_encrypted() == actual.server_encrypted());
    }
}
