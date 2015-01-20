// -----------------------------------------------------------------------------------------
// <copyright file="table_test_base.h" company="Microsoft">
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

#include <iomanip>
#include "CurrentTest.h"

#include "test_base.h"
#include "was/table.h"

class table_service_test_base : public test_base
{
public:

    table_service_test_base()
    {
    }

    ~table_service_test_base()
    {
    }

protected:
    static utility::string_t table_type_name;

    static azure::storage::cloud_table_client get_table_client();
    static utility::string_t get_table_name();
    static azure::storage::cloud_table get_table(bool create = true);
};
