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

test_config::test_config()
{
    utility::ifstream_t config_file;
    config_file.open("test_configurations.json");

    web::json::value config;
    config_file >> config;

    auto target_name = config[U("target")].as_string();
    auto tenants = config[U("tenants")];

    for (auto iter = tenants.cbegin(); iter != tenants.cend(); ++iter)
    {
        if (iter->second[U("name")].as_string() == target_name)
        {
            if (!iter->second.has_field(U("connection_string")))
            {
                wa::storage::storage_credentials credentials(iter->second[U("account_name")].as_string(), iter->second[U("account_key")].as_string());
                wa::storage::storage_uri blob_uri(iter->second[U("blob_primary_endpoint")].as_string(), iter->second[U("blob_secondary_endpoint")].as_string());
                wa::storage::storage_uri queue_uri(iter->second[U("queue_primary_endpoint")].as_string(), iter->second[U("queue_secondary_endpoint")].as_string());
                wa::storage::storage_uri table_uri(iter->second[U("table_primary_endpoint")].as_string(), iter->second[U("table_secondary_endpoint")].as_string());
                m_account = wa::storage::cloud_storage_account(credentials, blob_uri, queue_uri, table_uri);
            }
            else
            {
                auto connection_string = iter->second[U("connection_string")];
                m_account = wa::storage::cloud_storage_account::parse(connection_string.as_string());
            }

            break;
        }
    }
}

void test_base::print_client_request_id(const wa::storage::operation_context& context, const utility::string_t& purpose)
{
    std::string suite_name(UnitTest::CurrentTest::Details()->suiteName);
    std::string test_name(UnitTest::CurrentTest::Details()->testName);
    ucout << utility::conversions::to_string_t(suite_name) << U("::") << utility::conversions::to_string_t(test_name) << U(": ") << purpose << U(" client request ID: ") << context.client_request_id() << std::endl;
}
