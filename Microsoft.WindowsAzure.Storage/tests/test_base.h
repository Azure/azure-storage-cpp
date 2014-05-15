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
};
