// -----------------------------------------------------------------------------------------
// <copyright file="table_test_base.cpp" company="Microsoft">
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
#include "table_test_base.h"
#include "check_macros.h"
#include "wascore/streams.h"

utility::string_t table_service_test_base::table_type_name = utility::string_t(_XPLATSTR("table"));

azure::storage::cloud_table_client table_service_test_base::get_table_client()
{
    return test_config::instance().account().create_cloud_table_client();
}

utility::string_t table_service_test_base::get_table_name()
{
    // table name naming convention: "^[A-Za-z][A-Za-z0-9]{2,62}$"
    // here we construct the table name as:
    //    object_name_prefix + "tableA0" + random_characters

    std::vector<utility::char_t> charset;
    for (utility::char_t c = 'A'; c <= 'Z'; ++c) charset.push_back(c);
    for (utility::char_t c = 'a'; c <= 'z'; ++c) charset.push_back(c);
    for (utility::char_t c = '0'; c <= '9'; ++c) charset.push_back(c);

    utility::string_t table_name;
    table_name.reserve(39U + table_type_name.size());
    table_name.append(object_name_prefix);
    table_name.append(table_type_name);
    table_name.append(1, _XPLATSTR('A'));
    table_name.append(1, _XPLATSTR('0'));
    table_name.append(get_random_string(charset, 10));

    return table_name;
}

azure::storage::cloud_table table_service_test_base::get_table(bool create)
{
    azure::storage::cloud_table_client client = get_table_client();
    utility::string_t table_name = get_table_name();
    azure::storage::cloud_table table = client.get_table_reference(table_name);
    if (create)
    {
        table.create_if_not_exists();
    }
    return table;
}