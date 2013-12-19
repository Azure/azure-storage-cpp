// -----------------------------------------------------------------------------------------
// <copyright file="test_helper.cpp" company="Microsoft">
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
#include "test_helper.h"
#include "test_base.h"

utility::string_t object_name_prefix = utility::string_t(U("nativeclientlibraryunittest"));
utility::string_t table_type_name = utility::string_t(U("table"));
utility::string_t queue_type_name = utility::string_t(U("queue"));
bool is_random_initialized = false;

/*
utility::string_t get_partition_key(int partition_number)
{
    utility::ostringstream_t result;
    result << partition_number;
    return result.str();
}
*/

utility::string_t get_string(utility::char_t value1, utility::char_t value2)
{
    utility::ostringstream_t result;
    result << value1 << value2;
    return result.str();
}

void initialize_random()
{
    if (!is_random_initialized)
    {
        srand((unsigned int)time(NULL));
        is_random_initialized = true;
    }
}

bool get_random_boolean()
{
    initialize_random();
    return (rand() & 0x1) == 0;
}

int32_t get_random_int32()
{
    initialize_random();
    return (int32_t)rand() << 16 | (int32_t)rand();
}

int64_t get_random_int64()
{
    initialize_random();
    return (int64_t)rand() << 48 | (int64_t)rand() << 32 | (int64_t)rand() << 16 | (int64_t)rand();
}

double get_random_double()
{
    initialize_random();
    return (double)rand() / RAND_MAX;
}

utility::string_t get_random_string()
{
    initialize_random();
    const int SIZE = 10;
    utility::string_t result;
    result.reserve(SIZE);
    for (int i = 0; i < SIZE; ++i)
    {
        result.push_back(U('0') + rand() % 10);
    }
    return result;
}

utility::datetime get_random_datetime()
{
    initialize_random();
    return utility::datetime::utc_now() + rand();
}

std::vector<uint8_t> get_random_binary_data()
{
    initialize_random();
    const int SIZE = 100;
    std::vector<uint8_t> result;
    result.reserve(SIZE);
    for (int i = 0; i < SIZE; ++i)
    {
        result.push_back(rand() % 256);
    }
    return result;
}

utility::uuid get_random_guid()
{
    return utility::new_uuid();
}

utility::string_t get_object_name(const utility::string_t& object_type_name)
{
    utility::string_t object_name;
    object_name.reserve(37U + object_type_name.size());
    object_name.append(object_name_prefix);
    object_name.append(object_type_name);
    object_name.append(get_random_string());
    return object_name;
}

wa::storage::cloud_table_client get_table_client()
{
    return test_config::instance().account().create_cloud_table_client();
}

utility::string_t get_table_name()
{
    return get_object_name(table_type_name);
}

wa::storage::cloud_table get_table(bool create)
{
    wa::storage::cloud_table_client client = get_table_client();
    utility::string_t table_name = get_table_name();
    wa::storage::cloud_table table = client.get_table_reference(table_name);
    if (create)
    {
        table.create_if_not_exists();
    }
    return table;
}

wa::storage::cloud_queue_client get_queue_client()
{
    return test_config::instance().account().create_cloud_queue_client();
}

utility::string_t get_queue_name()
{
    return get_object_name(queue_type_name);
}

wa::storage::cloud_queue get_queue(bool create)
{
    wa::storage::cloud_queue_client client = get_queue_client();
    utility::string_t queue_name = get_queue_name();
    wa::storage::cloud_queue queue = client.get_queue_reference(queue_name);
    if (create)
    {
        queue.create_if_not_exists();
    }
    return queue;
}

