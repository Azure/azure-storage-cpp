// -----------------------------------------------------------------------------------------
// <copyright file="executor_test.cpp" company="Microsoft">
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

SUITE(Core)
{
    TEST(operation_context)
    {
        auto client = test_config::instance().account().create_cloud_blob_client();

        utility::string_t client_request_id;
        utility::string_t service_request_id;
        utility::string_t test_key;
        auto start_time = utility::datetime::utc_now();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        wa::storage::operation_context context;
        context.set_client_request_id(U("aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"));
        context.user_headers().add(U("x-ms-test-key"), U("test-value"));
        context.set_sending_request([&client_request_id, &test_key] (web::http::http_request& request, wa::storage::operation_context context) mutable
        {
            client_request_id = request.headers().find(U("x-ms-client-request-id"))->second;
            test_key = request.headers().find(U("x-ms-test-key"))->second;
        });
        context.set_response_received([&service_request_id] (web::http::http_request& request, const web::http::http_response& response, wa::storage::operation_context context) mutable
        {
            service_request_id = response.headers().find(U("x-ms-request-id"))->second;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });

        auto container = client.get_container_reference(U("this-container-does-not-exist"));
        container.exists(wa::storage::blob_request_options(), context);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto end_time = utility::datetime::utc_now();
        
        CHECK_EQUAL(1, context.request_results().size());
        auto result = context.request_results().front();

        CHECK(result.is_response_available());
        CHECK_UTF8_EQUAL(U("aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"), client_request_id);
        CHECK_UTF8_EQUAL(service_request_id, result.service_request_id());
        CHECK_EQUAL(web::http::status_codes::NotFound, result.http_status_code());
        CHECK(start_time.to_interval() < result.start_time().to_interval());
        CHECK(end_time.to_interval() > result.end_time().to_interval());
        CHECK(result.end_time().to_interval() > result.start_time().to_interval());
    }

    TEST(storage_uri)
    {
        CHECK_THROW(wa::storage::storage_uri(U("http://www.microsoft.com/test1"), U("http://www.microsoft.com/test2")), std::invalid_argument);
    }
}
