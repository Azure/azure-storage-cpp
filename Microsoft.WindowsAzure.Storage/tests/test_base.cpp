// -----------------------------------------------------------------------------------------
// <copyright file="test_base.cpp" company="Microsoft">
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

#include <ctime>
#include <random>

#include "test_base.h"
#include "cpprest/json.h"

static thread_local std::mt19937_64 random_generator(std::random_device{}());

bool get_random_boolean()
{
    std::uniform_int_distribution<int> distribution(0, 1);
    return distribution(random_generator) == 0;
}

int32_t get_random_int32()
{
    std::uniform_int_distribution<int32_t> distribution(0, std::numeric_limits<int32_t>::max());
    return distribution(random_generator);
}

int64_t get_random_int64()
{
    std::uniform_int_distribution<int64_t> distribution(0LL, std::numeric_limits<int64_t>::max());
    return distribution(random_generator);
}

double get_random_double()
{
    std::uniform_real_distribution<double> distribution(0, 1.0);
    return distribution(random_generator);
}

utility::string_t test_base::object_name_prefix = utility::string_t(_XPLATSTR("nativeclientlibraryunittest"));

test_config::test_config()
{
    utility::ifstream_t config_file;
    config_file.open("test_configurations.json");

    web::json::value config;
    config_file >> config;

    auto target_name = config[_XPLATSTR("target")].as_string();
    auto premium_target_name = config[_XPLATSTR("premium_target")].as_string();
    auto blob_storage_target_name = config[_XPLATSTR("blob_storage_target")].as_string();
    web::json::value& tenants = config[_XPLATSTR("tenants")];

    for (web::json::array::const_iterator it = tenants.as_array().cbegin(); it != tenants.as_array().cend(); ++it)
    {
        const web::json::value& name_obj = it->at(_XPLATSTR("name"));
        if (name_obj.as_string() == target_name)
        {
            if (!it->has_field(_XPLATSTR("connection_string")))
            {
                azure::storage::storage_credentials credentials(it->at(_XPLATSTR("account_name")).as_string(), it->at(_XPLATSTR("account_key")).as_string());
                azure::storage::storage_uri blob_uri(it->at(_XPLATSTR("blob_primary_endpoint")).as_string(), it->at(_XPLATSTR("blob_secondary_endpoint")).as_string());
                azure::storage::storage_uri queue_uri(it->at(_XPLATSTR("queue_primary_endpoint")).as_string(), it->at(_XPLATSTR("queue_secondary_endpoint")).as_string());
                azure::storage::storage_uri table_uri(it->at(_XPLATSTR("table_primary_endpoint")).as_string(), it->at(_XPLATSTR("table_secondary_endpoint")).as_string());
                m_account = azure::storage::cloud_storage_account(credentials, blob_uri, queue_uri, table_uri);
            }
            else
            {
                const web::json::value& connection_string_obj = it->at(_XPLATSTR("connection_string"));
                m_account = azure::storage::cloud_storage_account::parse(connection_string_obj.as_string());
            }
        }
        else if (name_obj.as_string() == premium_target_name)
        {
            if (!it->has_field(_XPLATSTR("connection_string")))
            {
                azure::storage::storage_credentials credentials(it->at(_XPLATSTR("account_name")).as_string(), it->at(_XPLATSTR("account_key")).as_string());
                azure::storage::storage_uri blob_uri(it->at(_XPLATSTR("blob_primary_endpoint")).as_string(), it->at(_XPLATSTR("blob_secondary_endpoint")).as_string());
                azure::storage::storage_uri queue_uri(it->at(_XPLATSTR("queue_primary_endpoint")).as_string(), it->at(_XPLATSTR("queue_secondary_endpoint")).as_string());
                azure::storage::storage_uri table_uri(it->at(_XPLATSTR("table_primary_endpoint")).as_string(), it->at(_XPLATSTR("table_secondary_endpoint")).as_string());
                m_premium_account = azure::storage::cloud_storage_account(credentials, blob_uri, queue_uri, table_uri);
            }
            else
            {
                const web::json::value& connection_string_obj = it->at(_XPLATSTR("connection_string"));
                m_premium_account = azure::storage::cloud_storage_account::parse(connection_string_obj.as_string());
            }
        }
        else if (name_obj.as_string() == blob_storage_target_name)
        {
            if (!it->has_field(_XPLATSTR("connection_string")))
            {
                azure::storage::storage_credentials credentials(it->at(_XPLATSTR("account_name")).as_string(), it->at(_XPLATSTR("account_key")).as_string());
                azure::storage::storage_uri blob_uri(it->at(_XPLATSTR("blob_primary_endpoint")).as_string(), it->at(_XPLATSTR("blob_secondary_endpoint")).as_string());
                azure::storage::storage_uri queue_uri(it->at(_XPLATSTR("queue_primary_endpoint")).as_string(), it->at(_XPLATSTR("queue_secondary_endpoint")).as_string());
                azure::storage::storage_uri table_uri(it->at(_XPLATSTR("table_primary_endpoint")).as_string(), it->at(_XPLATSTR("table_secondary_endpoint")).as_string());
                m_blob_storage_account = azure::storage::cloud_storage_account(credentials, blob_uri, queue_uri, table_uri);
            }
            else
            {
                const web::json::value& connection_string_obj = it->at(_XPLATSTR("connection_string"));
                m_blob_storage_account = azure::storage::cloud_storage_account::parse(connection_string_obj.as_string());
            }
        }
    }

    web::json::value& token_information = config[_XPLATSTR("token_information")];

    m_token_account_name = token_information.at(_XPLATSTR("account_name")).as_string();
    m_token_tenant_id = token_information.at(_XPLATSTR("tenant_id")).as_string();
    m_token_client_id = token_information.at(_XPLATSTR("client_id")).as_string();
    m_token_client_secret = token_information.at(_XPLATSTR("client_secret")).as_string();
    m_token_resource = token_information.at(_XPLATSTR("resource")).as_string();
}

const utility::string_t& test_config::get_oauth_account_name() const
{
    return m_token_account_name;
}

utility::string_t test_config::get_oauth_token() const
{
    utility::string_t uri = _XPLATSTR("https://login.microsoftonline.com/") + m_token_tenant_id + _XPLATSTR("/oauth2/token");

    utility::string_t body;
    body += _XPLATSTR("grant_type=client_credentials&");
    body += _XPLATSTR("client_id=") + m_token_client_id + _XPLATSTR("&");
    body += _XPLATSTR("client_secret=") + m_token_client_secret + _XPLATSTR("&");
    body += _XPLATSTR("resource=") + m_token_resource;

    web::http::http_request request;
    request.set_method(web::http::methods::POST);
    request.set_body(body, _XPLATSTR("application/x-www-form-urlencoded"));

    utility::string_t access_token;

    web::http::client::http_client client(uri);
    auto get_token_task = client.request(request).then([](web::http::http_response response)
    {
        if (response.status_code() == web::http::status_codes::OK)
        {
            return response.extract_json();
        }
        return pplx::task_from_result(web::json::value());
    }).then([&access_token](pplx::task<web::json::value> json_task)
    {
        const auto& json_value = json_task.get();
        access_token = json_value.at(_XPLATSTR("access_token")).as_string();
    });

    try
    {
        get_token_task.wait();
    }
    catch (std::exception& e)
    {
        std::cout << "Cannot get OAuth access token: " << e.what() << std::endl;
    }

    return access_token;
}

utility::datetime test_base::parse_datetime(const utility::string_t& value, utility::datetime::date_format format)
{
    if (!value.empty())
    {
        return utility::datetime::from_string(value, format);
    }
    else
    {
        return utility::datetime();
    }
}

void test_base::print_client_request_id(const azure::storage::operation_context& context, const utility::string_t& purpose)
{
    std::string suite_name(UnitTest::CurrentTest::Details()->suiteName);
    std::string test_name(UnitTest::CurrentTest::Details()->testName);
    ucout << utility::conversions::to_string_t(suite_name) << _XPLATSTR(":") << utility::conversions::to_string_t(test_name) << _XPLATSTR(": ") << purpose << _XPLATSTR(" client request ID: ") << context.client_request_id() << std::endl;
}

utility::string_t test_base::get_string(utility::char_t value1, utility::char_t value2)
{
    utility::ostringstream_t result;
    result << value1 << value2;
    return result.str();
}

utility::string_t test_base::get_random_string(const std::vector<utility::char_t>& charset, size_t size)
{
    utility::string_t result;
    result.reserve(size);
    std::uniform_int_distribution<size_t> distribution(0, charset.size() - 1);
    for (size_t i = 0; i < size; ++i)
    {
        result.push_back(charset[distribution(random_generator)]);
    }
    return result;
}

utility::string_t test_base::get_random_string(size_t size)
{
    const static std::vector<utility::char_t> charset {
        _XPLATSTR('0'), _XPLATSTR('1'), _XPLATSTR('2'), _XPLATSTR('3'), _XPLATSTR('4'),
        _XPLATSTR('5'), _XPLATSTR('6'), _XPLATSTR('7'), _XPLATSTR('8'), _XPLATSTR('9'),
    };
    return get_random_string(charset, size);
}

utility::datetime test_base::get_random_datetime()
{
    return utility::datetime::utc_now() + get_random_int32();
}

std::vector<uint8_t> test_base::get_random_binary_data()
{
    const int SIZE = 100;
    std::vector<uint8_t> result;
    result.reserve(SIZE);
    std::uniform_int_distribution<int> distribution(std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max());
    for (int i = 0; i < SIZE; ++i)
    {
        result.push_back(uint8_t(distribution(random_generator)));
    }
    return result;
}

void test_base::fill_buffer(std::vector<uint8_t>& buffer)
{
    fill_buffer(buffer, 0, buffer.size());
}

void test_base::fill_buffer(std::vector<uint8_t>& buffer, size_t offset, size_t count)
{
    using rand_int_type = uint64_t;
    const size_t rand_int_size = sizeof(rand_int_type);

    uint8_t* start_addr = buffer.data() + offset;
    uint8_t* end_addr = start_addr + count;

    auto random_char_generator = []() -> uint8_t
    {
        return uint8_t(get_random_int32());
    };

    while (uintptr_t(start_addr) % rand_int_size != 0 && start_addr < end_addr)
    {
        *(start_addr++) = random_char_generator();
    }

    std::uniform_int_distribution<rand_int_type> distribution(0LL, std::numeric_limits<rand_int_type>::max());
    while (start_addr + rand_int_size <= end_addr)
    {
        *reinterpret_cast<rand_int_type*>(start_addr) = distribution(random_generator);
        start_addr += rand_int_size;
    }

    std::generate(start_addr, end_addr, random_char_generator);
}

utility::uuid test_base::get_random_guid()
{
    return utility::new_uuid();
}

utility::string_t test_base::get_object_name(const utility::string_t& object_type_name)
{
    utility::string_t object_name;
    object_name.reserve(37U + object_type_name.size());
    object_name.append(object_name_prefix);
    object_name.append(object_type_name);
    object_name.append(get_random_string());
    return object_name;
}
