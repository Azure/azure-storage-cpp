// -----------------------------------------------------------------------------------------
// <copyright file="test_base.h" company="Microsoft">
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

#include "was/common.h"
#include "was/storage_account.h"

class test_config
{
public:

    static const test_config& instance()
    {
        static test_config singleton_instance;
        return singleton_instance;
    }

    const azure::storage::cloud_storage_account& account() const
    {
        return m_account;
    }

private:

    test_config();

    azure::storage::cloud_storage_account m_account;
};

class test_base
{
public:

    test_base()
    {
        print_client_request_id(m_context, U("test fixture"));
    }

    ~test_base()
    {
    }

protected:

    static void print_client_request_id(const azure::storage::operation_context& context, const utility::string_t& purpose);
  
    azure::storage::operation_context m_context;
    
    static utility::string_t object_name_prefix;
    static bool is_random_initialized;

public:

    static utility::string_t get_string(utility::char_t value1, utility::char_t value2);
    static void initialize_random();
    static bool get_random_boolean();
    static int32_t get_random_int32();
    static int64_t get_random_int64();
    static double get_random_double();
    static utility::string_t get_random_string(const std::vector<utility::char_t> charset, size_t size);
    static utility::string_t get_random_string(size_t size = 10);
    static utility::datetime get_random_datetime();
    static std::vector<uint8_t> get_random_binary_data();
    static utility::uuid get_random_guid();
    static utility::string_t get_object_name(const utility::string_t& object_type_name);
};
