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

#include "test_base.h"
#include "cpprest/json.h"

utility::string_t test_base::object_name_prefix = utility::string_t(_XPLATSTR("nativeclientlibraryunittest"));
bool test_base::is_random_initialized = false;

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

void test_base::initialize_random()
{
    if (!is_random_initialized)
    {
        srand((unsigned int)time(NULL));
        is_random_initialized = true;
    }
}

bool test_base::get_random_boolean()
{
    initialize_random();
    return (rand() & 0x1) == 0;
}

int32_t test_base::get_random_int32()
{
    initialize_random();
    return (int32_t)rand() << 16 | (int32_t)rand();
}

int64_t test_base::get_random_int64()
{
    initialize_random();
    return (int64_t)rand() << 48 | (int64_t)rand() << 32 | (int64_t)rand() << 16 | (int64_t)rand();
}

double test_base::get_random_double()
{
    initialize_random();
    return (double)rand() / RAND_MAX;
}

utility::string_t test_base::get_random_string(const std::vector<utility::char_t> charset, size_t size)
{
    initialize_random();
    utility::string_t result;
    result.reserve(size);
    for (size_t i = 0; i < size; ++i)
    {
        result.push_back(charset[rand() % charset.size()]);
    }

    return result;
}

utility::string_t test_base::get_random_string(size_t size)
{
    initialize_random();
    utility::string_t result;
    result.reserve(size);
    for (size_t i = 0; i < size; ++i)
    {
        result.push_back((utility::char_t) (_XPLATSTR('0') + rand() % 10));
    }
    return result;
}

utility::datetime test_base::get_random_datetime()
{
    initialize_random();
    return utility::datetime::utc_now() + rand();
}

std::vector<uint8_t> test_base::get_random_binary_data()
{
    initialize_random();
    const int SIZE = 100;
    std::vector<uint8_t> result;
    result.reserve(SIZE);
    for (int i = 0; i < SIZE; ++i)
    {
        result.push_back((unsigned char)(rand() % 256));
    }
    return result;
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
