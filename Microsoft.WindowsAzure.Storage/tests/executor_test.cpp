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
    TEST_FIXTURE(test_base, timeout)
    {
        azure::storage::cloud_blob_client client = test_config::instance().account().create_cloud_blob_client();
        azure::storage::cloud_blob_container container = client.get_container_reference(_XPLATSTR("this-container-does-not-exist"));

        utility::string_t timeout;

        azure::storage::operation_context context = m_context;
        context.set_sending_request([&timeout] (web::http::http_request& request, azure::storage::operation_context context) mutable
        {
            std::map<utility::string_t, utility::string_t> query_parameters = web::http::uri::split_query(request.request_uri().query());
            std::map<utility::string_t, utility::string_t>::iterator timeout_it = query_parameters.find(_XPLATSTR("timeout"));
            if (timeout_it != query_parameters.end())
            {
                timeout = timeout_it->second;
            }
            else
            {
                timeout.clear();
            }
        });

        {
            azure::storage::blob_request_options options;

            container.exists(options, context);

            CHECK(timeout.empty());
        }

        {
            azure::storage::blob_request_options options;
            options.set_server_timeout(std::chrono::seconds(20));

            container.exists(options, context);

            CHECK(!timeout.empty());
            CHECK_UTF8_EQUAL(_XPLATSTR("20"), timeout);
        }
    }

    TEST_FIXTURE(test_base, operation_context)
    {
        auto client = test_config::instance().account().create_cloud_blob_client();

        utility::string_t client_request_id;
        utility::string_t service_request_id;
        utility::string_t test_key;
        auto start_time = utility::datetime::utc_now();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        azure::storage::operation_context context = m_context;
        context.set_client_request_id(_XPLATSTR("aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"));
        context.user_headers().add(_XPLATSTR("x-ms-test-key"), _XPLATSTR("test-value"));
        context.set_sending_request([&client_request_id, &test_key] (web::http::http_request& request, azure::storage::operation_context context) mutable
        {
            client_request_id = request.headers().find(_XPLATSTR("x-ms-client-request-id"))->second;
            test_key = request.headers().find(_XPLATSTR("x-ms-test-key"))->second;
        });
        context.set_response_received([&service_request_id] (web::http::http_request&, const web::http::http_response& response, azure::storage::operation_context context) mutable
        {
            service_request_id = response.headers().find(_XPLATSTR("x-ms-request-id"))->second;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });

        auto container = client.get_container_reference(_XPLATSTR("this-container-does-not-exist"));
        container.exists(azure::storage::blob_request_options(), context);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto end_time = utility::datetime::utc_now();
        
        CHECK_EQUAL(1U, context.request_results().size());
        auto result = context.request_results().front();

        CHECK(result.is_response_available());
        CHECK_UTF8_EQUAL(_XPLATSTR("aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"), client_request_id);
        CHECK_UTF8_EQUAL(service_request_id, result.service_request_id());
        CHECK_EQUAL(web::http::status_codes::NotFound, result.http_status_code());
        CHECK(start_time.to_interval() < result.start_time().to_interval());
        CHECK(end_time.to_interval() > result.end_time().to_interval());
        CHECK(result.end_time().to_interval() > result.start_time().to_interval());
    }

    TEST_FIXTURE(test_base, storage_uri)
    {
        azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1"));
        azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1"), _XPLATSTR("http://www.microsoft.com/test1"));
        azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1"), _XPLATSTR(""));
        azure::storage::storage_uri(_XPLATSTR(""), _XPLATSTR("http://www.microsoft.com/test1"));
        azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1/example1"), _XPLATSTR("http://www.microsoft.com/test1/example1"));

        CHECK_THROW(azure::storage::storage_uri(_XPLATSTR("")), std::invalid_argument);
        CHECK_THROW(azure::storage::storage_uri(_XPLATSTR(""), _XPLATSTR("")), std::invalid_argument);

        CHECK_THROW(azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1"), _XPLATSTR("http://www.microsoft.com/test2")), std::invalid_argument);
        CHECK_THROW(azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1"), _XPLATSTR("http://www.microsoft.com")), std::invalid_argument);
        CHECK_THROW(azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1"), _XPLATSTR("http://www.microsoft.com/test1/")), std::invalid_argument);
        CHECK_THROW(azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1"), _XPLATSTR("http://www.microsoft.com/test")), std::invalid_argument);
        CHECK_THROW(azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1"), _XPLATSTR("http://www.microsoft.com/test11")), std::invalid_argument);
        CHECK_THROW(azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1/example1"), _XPLATSTR("http://www.microsoft.com/test1/example2")), std::invalid_argument);

        azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1?parameter=value1"), _XPLATSTR("http://www.microsoft.com/test1?parameter=value1"));

        CHECK_THROW(azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1?parameter=value1"), _XPLATSTR("http://www.microsoft.com/test1")), std::invalid_argument);
        CHECK_THROW(azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1?parameter=value1"), _XPLATSTR("http://www.microsoft.com/test1?parameter=value2")), std::invalid_argument);
        CHECK_THROW(azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1?parameter=value1"), _XPLATSTR("http://www.microsoft.com/test1?parameter=value")), std::invalid_argument);
        CHECK_THROW(azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1?parameter=value1"), _XPLATSTR("http://www.microsoft.com/test1?parameter=value11")), std::invalid_argument);

        azure::storage::storage_uri(_XPLATSTR("http://127.0.0.1:10000/account/test1"), _XPLATSTR("http://127.0.0.1:10000/account-secondary/test1"));
        azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1"), _XPLATSTR("http://127.0.0.1:10000/account/test1"));
        azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1/example1"), _XPLATSTR("http://127.0.0.1:10000/account/test1/example1"));
        azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1?parameter=value1"), _XPLATSTR("http://127.0.0.1:10000/account/test1?parameter=value1"));
        azure::storage::storage_uri(_XPLATSTR("http://127.0.0.1:10000/account"), _XPLATSTR("http://127.0.0.1:10000"));

        CHECK_THROW(azure::storage::storage_uri(_XPLATSTR("http://127.0.0.1:10000/account-secondary/test1"), _XPLATSTR("http://127.0.0.1:10000/account/test2")), std::invalid_argument);
        CHECK_THROW(azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1"), _XPLATSTR("http://127.0.0.1:10000/account/test2")), std::invalid_argument);
        CHECK_THROW(azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1/example1"), _XPLATSTR("http://127.0.0.1:10000/account/test2/example2")), std::invalid_argument);
        CHECK_THROW(azure::storage::storage_uri(_XPLATSTR("http://www.microsoft.com/test1?parameter=value1"), _XPLATSTR("http://127.0.0.1:10000/account/test1?parameter=value2")), std::invalid_argument);
    }

    TEST_FIXTURE(test_base, storage_exception)
    {
        azure::storage::cloud_blob blob(azure::storage::storage_uri(_XPLATSTR("http://www.nonexistenthost.com/test1")));

        bool caught_storage_exception = false;
        bool caught_http_exception = false;
        try
        {
            blob.exists();
        }
        catch (const azure::storage::storage_exception& ex1)
        {
            caught_storage_exception = true;

            try
            {
                std::rethrow_exception(ex1.inner_exception());
            }
            catch (web::http::http_exception&)
            {
                caught_http_exception = true;
            }
        }

        CHECK_EQUAL(true, caught_storage_exception);
        CHECK_EQUAL(true, caught_http_exception);
    }

#ifdef _WIN32
    class delayed_scheduler : public azure::storage::delayed_scheduler_interface
    {
    public:
        virtual void schedule_after(pplx::TaskProc_t function, void* context, long long delayInMs) override
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(delayInMs));
            function(context);
        }
    };

    TEST_FIXTURE(block_blob_test_base, verify_retry_after_delay)
    {
        azure::storage::set_wastorage_ambient_delayed_scheduler(std::make_shared<delayed_scheduler>());

        const size_t buffer_size = 1024;
        std::vector<uint8_t> buffer;
        buffer.resize(buffer_size);
        auto md5 = fill_buffer_and_get_md5(buffer);
        auto stream = concurrency::streams::bytestream::open_istream(buffer);

        azure::storage::operation_context context;
        static bool throwException = true;
        context.set_response_received([](web::http::http_request&, const web::http::http_response&, azure::storage::operation_context context)
        {
            if (throwException)
            {
                throwException = false;
                throw azure::storage::storage_exception("retry");
            }
        });

        bool failed = false;
        try
        {
            m_blob.upload_block(get_block_id(0), stream, md5, azure::storage::access_condition(), azure::storage::blob_request_options(), context);
        }
        catch (azure::storage::storage_exception&)
        {
            failed = true;
        }

        azure::storage::set_wastorage_ambient_delayed_scheduler(nullptr);
        CHECK_EQUAL(false, failed);
        CHECK_EQUAL(false, throwException);
    }
#endif
}
