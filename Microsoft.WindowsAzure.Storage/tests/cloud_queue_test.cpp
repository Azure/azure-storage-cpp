// -----------------------------------------------------------------------------------------
// <copyright file="cloud_queue_test.cpp" company="Microsoft">
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
#include "queue_test_base.h"
#include "was/queue.h"

SUITE(Queue)
{
    TEST_FIXTURE(queue_service_test_base, Queue_Empty)
    {
        azure::storage::cloud_queue queue;

        CHECK(queue.service_client().base_uri().primary_uri().is_empty());
        CHECK(queue.service_client().base_uri().secondary_uri().is_empty());
        CHECK(queue.service_client().credentials().is_anonymous());
        CHECK(queue.name().empty());
        CHECK(queue.uri().primary_uri().is_empty());
        CHECK(queue.uri().secondary_uri().is_empty());
        CHECK_EQUAL(-1, queue.approximate_message_count());
    }

    TEST_FIXTURE(queue_service_test_base, Queue_Uri)
    {
        azure::storage::storage_uri uri(web::http::uri(_XPLATSTR("https://myaccount.queue.core.windows.net/myqueue")), web::http::uri(_XPLATSTR("https://myaccount-secondary.queue.core.windows.net/myqueue")));

        azure::storage::cloud_queue queue1(uri);

        web::http::uri expected_primary_uri(_XPLATSTR("https://myaccount.queue.core.windows.net"));
        web::http::uri expected_secondary_uri(_XPLATSTR("https://myaccount-secondary.queue.core.windows.net"));

        CHECK(queue1.service_client().base_uri().primary_uri() == expected_primary_uri);
        CHECK(queue1.service_client().base_uri().secondary_uri() == expected_secondary_uri);
        CHECK(queue1.service_client().credentials().is_anonymous());
        CHECK(queue1.name().compare(_XPLATSTR("myqueue")) == 0);
        CHECK(queue1.uri().primary_uri() == uri.primary_uri());
        CHECK(queue1.uri().secondary_uri() == uri.secondary_uri());
        CHECK_EQUAL(-1, queue1.approximate_message_count());

        utility::string_t sas_token(_XPLATSTR("se=2013-05-14T18%3A23%3A15Z&sig=mysignature&sp=raup&st=2013-05-14T17%3A23%3A15Z&sv=2012-02-12"));

        azure::storage::storage_uri sas_uri(web::http::uri(_XPLATSTR("https://myaccount.queue.core.windows.net/myqueue?sp=raup&sv=2012-02-12&se=2013-05-14T18%3A23%3A15Z&st=2013-05-14T17%3A23%3A15Z&sig=mysignature")), web::http::uri(_XPLATSTR("https://myaccount-secondary.queue.core.windows.net/myqueue?sp=raup&sv=2012-02-12&se=2013-05-14T18%3A23%3A15Z&st=2013-05-14T17%3A23%3A15Z&sig=mysignature")));

        azure::storage::cloud_queue queue2(sas_uri);

        CHECK(queue2.service_client().base_uri().primary_uri() == expected_primary_uri);
        CHECK(queue2.service_client().base_uri().secondary_uri() == expected_secondary_uri);
        CHECK(queue2.service_client().credentials().is_sas());
        CHECK(queue2.service_client().credentials().sas_token() == sas_token);
        CHECK(queue2.name().compare(_XPLATSTR("myqueue")) == 0);
        CHECK(queue2.uri().primary_uri() == uri.primary_uri());
        CHECK(queue2.uri().secondary_uri() == uri.secondary_uri());
        CHECK_EQUAL(-1, queue2.approximate_message_count());
    }

    TEST_FIXTURE(queue_service_test_base, Queue_UriAndCredentials)
    {
        azure::storage::storage_uri uri(web::http::uri(_XPLATSTR("https://myaccount.queue.core.windows.net/myqueue")), web::http::uri(_XPLATSTR("https://myaccount-secondary.queue.core.windows.net/myqueue")));
        azure::storage::storage_credentials credentials(_XPLATSTR("devstoreaccount1"), _XPLATSTR("Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="));

        azure::storage::cloud_queue queue1(uri, credentials);

        web::http::uri expected_primary_uri(_XPLATSTR("https://myaccount.queue.core.windows.net"));
        web::http::uri expected_secondary_uri(_XPLATSTR("https://myaccount-secondary.queue.core.windows.net"));

        CHECK(queue1.service_client().base_uri().primary_uri() == expected_primary_uri);
        CHECK(queue1.service_client().base_uri().secondary_uri() == expected_secondary_uri);
        CHECK(queue1.service_client().credentials().is_shared_key());
        CHECK(queue1.name().compare(_XPLATSTR("myqueue")) == 0);
        CHECK(queue1.uri().primary_uri() == uri.primary_uri());
        CHECK(queue1.uri().secondary_uri() == uri.secondary_uri());
        CHECK_EQUAL(-1, queue1.approximate_message_count());

        utility::string_t sas_token(_XPLATSTR("sv=2012-02-12&st=2013-05-14T17%3A23%3A15Z&se=2013-05-14T18%3A23%3A15Z&sp=raup&sig=mysignature"));
        utility::string_t invalid_sas_token(_XPLATSTR("sv=2012-02-12&st=2013-05-14T17%3A23%3A15Z&se=2013-05-14T18%3A23%3A15Z&sp=raup&sig=invalid"));

        azure::storage::storage_uri sas_uri(web::http::uri(_XPLATSTR("https://myaccount.queue.core.windows.net/myqueue?sp=raup&sv=2012-02-12&se=2013-05-14T18%3A23%3A15Z&st=2013-05-14T17%3A23%3A15Z&sig=mysignature")), web::http::uri(_XPLATSTR("https://myaccount-secondary.queue.core.windows.net/myqueue?sp=raup&sv=2012-02-12&se=2013-05-14T18%3A23%3A15Z&st=2013-05-14T17%3A23%3A15Z&sig=mysignature")));
        azure::storage::storage_credentials sas_credentials(sas_token);

        CHECK_THROW(azure::storage::cloud_queue(sas_uri, sas_credentials), std::invalid_argument);

        azure::storage::storage_credentials invalid_sas_credentials(invalid_sas_token);

        CHECK_THROW(azure::storage::cloud_queue(sas_uri, invalid_sas_credentials), std::invalid_argument);

        CHECK_THROW(azure::storage::cloud_queue(sas_uri, credentials), std::invalid_argument);
    }
 
    TEST_FIXTURE(queue_service_test_base, Message_Empty)
    {
        azure::storage::cloud_queue_message message;

        CHECK(message.content_as_string().empty());
        CHECK(message.content_as_binary().empty());
        CHECK(message.id().empty());
        CHECK(message.pop_receipt().empty());
        CHECK(!message.expiration_time().is_initialized());
        CHECK(!message.insertion_time().is_initialized());
        CHECK(!message.next_visibile_time().is_initialized());
        CHECK_EQUAL(0, message.dequeue_count());
    }

    TEST_FIXTURE(queue_service_test_base, Message_String)
    {
        utility::string_t content = get_random_string();

        azure::storage::cloud_queue_message message(content);

        CHECK(content.compare(message.content_as_string()) == 0);
        CHECK_THROW(message.content_as_binary(), std::runtime_error);
        CHECK(message.id().empty());
        CHECK(message.pop_receipt().empty());
        CHECK(!message.expiration_time().is_initialized());
        CHECK(!message.insertion_time().is_initialized());
        CHECK(!message.next_visibile_time().is_initialized());
        CHECK_EQUAL(0, message.dequeue_count());

        content = get_random_string();
        message.set_content(content);

        CHECK(content.compare(message.content_as_string()) == 0);
        CHECK_THROW(message.content_as_binary(), std::runtime_error);
        CHECK(message.id().empty());
        CHECK(message.pop_receipt().empty());
        CHECK(!message.expiration_time().is_initialized());
        CHECK(!message.insertion_time().is_initialized());
        CHECK(!message.next_visibile_time().is_initialized());
        CHECK_EQUAL(0, message.dequeue_count());
    }

    TEST_FIXTURE(queue_service_test_base, Message_Binary)
    {
        std::vector<uint8_t> content = get_random_binary_data();

        azure::storage::cloud_queue_message message(content);

        CHECK(message.content_as_string().size() >= content.size());
        CHECK_ARRAY_EQUAL(content, message.content_as_binary(), (int)content.size());
        CHECK(message.id().empty());
        CHECK(message.pop_receipt().empty());
        CHECK(!message.expiration_time().is_initialized());
        CHECK(!message.insertion_time().is_initialized());
        CHECK(!message.next_visibile_time().is_initialized());
        CHECK_EQUAL(0, message.dequeue_count());

        content = get_random_binary_data();
        message.set_content(content);

        CHECK(message.content_as_string().size() >= content.size());
        CHECK_ARRAY_EQUAL(content, message.content_as_binary(), (int)content.size());
        CHECK(message.id().empty());
        CHECK(message.pop_receipt().empty());
        CHECK(!message.expiration_time().is_initialized());
        CHECK(!message.insertion_time().is_initialized());
        CHECK(!message.next_visibile_time().is_initialized());
        CHECK_EQUAL(0, message.dequeue_count());
    }

    TEST_FIXTURE(queue_service_test_base, Message_IdAndPopReceipt)
    {
        utility::string_t id = get_random_string();
        utility::string_t pop_receipt = get_random_string();

        azure::storage::cloud_queue_message message(id, pop_receipt);

        CHECK(message.content_as_string().empty());
        CHECK(message.content_as_binary().empty());
        CHECK(id.compare(message.id()) == 0);
        CHECK(pop_receipt.compare(message.pop_receipt()) == 0);
        CHECK(!message.expiration_time().is_initialized());
        CHECK(!message.insertion_time().is_initialized());
        CHECK(!message.next_visibile_time().is_initialized());
        CHECK_EQUAL(0, message.dequeue_count());
    }

    TEST_FIXTURE(queue_service_test_base, QueueRequestOptions_Normal)
    {
        azure::storage::queue_request_options options;

        CHECK(options.location_mode() == azure::storage::location_mode::primary_only);
    }

    TEST_FIXTURE(queue_service_test_base, Queue_CreateAndDelete)
    {
        utility::string_t queue_name1 = get_queue_name();
        utility::string_t queue_name2 = get_queue_name();
        azure::storage::cloud_queue_client client = get_queue_client();

        azure::storage::cloud_queue queue1 = client.get_queue_reference(queue_name1);
        azure::storage::cloud_queue queue2 = client.get_queue_reference(queue_name2);

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;

            queue1.create(options, context);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::Created, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            bool exists = queue1.exists(options, context);

            CHECK(exists);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            // It is allowed to create a queue that already exists with the same metadata
            queue1.create(options, context);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::cloud_metadata metadata;
            metadata[_XPLATSTR("MyMetadata1")] = _XPLATSTR("AAA");
            metadata[_XPLATSTR("MyMetadata2")] = _XPLATSTR("BBB");

            queue1.set_metadata(metadata);
            queue1.upload_metadata();

            // It is an error to create a queue that already exists with different metadata
            try
            {
                queue1.create(options, context);
                CHECK(false);
            }
            catch (const azure::storage::storage_exception& e)
            {
                CHECK_EQUAL(web::http::status_codes::Conflict, e.result().http_status_code());
                CHECK(e.result().extended_error().code().compare(_XPLATSTR("QueueAlreadyExists")) == 0);
                CHECK(!e.result().extended_error().message().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::Conflict, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().compare(_XPLATSTR("QueueAlreadyExists")) == 0);
            CHECK(!context.request_results()[0].extended_error().message().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            queue1.delete_queue(options, context);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            bool exists = queue1.exists(options, context);

            CHECK(!exists);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NotFound, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            try
            {
                queue2.delete_queue(options, context);
                CHECK(false);
            }
            catch (const azure::storage::storage_exception& e)
            {
                CHECK_EQUAL(web::http::status_codes::NotFound, e.result().http_status_code());
                CHECK(e.result().extended_error().code().compare(_XPLATSTR("QueueNotFound")) == 0);
                CHECK(!e.result().extended_error().message().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NotFound, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().compare(_XPLATSTR("QueueNotFound")) == 0);
            CHECK(!context.request_results()[0].extended_error().message().empty());
        }
    }

    TEST_FIXTURE(queue_service_test_base, Queue_CreateIfNotExistsAndDeleteIfExists)
    {
        utility::string_t queue_name = get_queue_name();
        azure::storage::cloud_queue_client client = get_queue_client();

        azure::storage::cloud_queue queue = client.get_queue_reference(queue_name);

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            bool created = queue.create_if_not_exists(options, context);

            CHECK(created);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(2U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NotFound, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
            CHECK(context.request_results()[1].is_response_available());
            CHECK(context.request_results()[1].start_time().is_initialized());
            CHECK(context.request_results()[1].end_time().is_initialized());
            CHECK(context.request_results()[1].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::Created, context.request_results()[1].http_status_code());
            CHECK(!context.request_results()[1].service_request_id().empty());
            CHECK(context.request_results()[1].request_date().is_initialized());
            CHECK(context.request_results()[1].content_md5().empty());
            CHECK(context.request_results()[1].etag().empty());
            CHECK(context.request_results()[1].extended_error().code().empty());
            CHECK(context.request_results()[1].extended_error().message().empty());
            CHECK(context.request_results()[1].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            bool exists = queue.exists(options, context);

            CHECK(exists);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            bool created = queue.create_if_not_exists(options, context);

            CHECK(!created);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            bool deleted = queue.delete_queue_if_exists(options, context);

            CHECK(deleted);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(2U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
            CHECK(context.request_results()[1].is_response_available());
            CHECK(context.request_results()[1].start_time().is_initialized());
            CHECK(context.request_results()[1].end_time().is_initialized());
            CHECK(context.request_results()[1].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[1].http_status_code());
            CHECK(!context.request_results()[1].service_request_id().empty());
            CHECK(context.request_results()[1].request_date().is_initialized());
            CHECK(context.request_results()[1].content_md5().empty());
            CHECK(context.request_results()[1].etag().empty());
            CHECK(context.request_results()[1].extended_error().code().empty());
            CHECK(context.request_results()[1].extended_error().message().empty());
            CHECK(context.request_results()[1].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            bool exists = queue.exists(options, context);

            CHECK(!exists);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NotFound, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            bool deleted = queue.delete_queue_if_exists(options, context);

            CHECK(!deleted);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NotFound, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }
    }

    TEST_FIXTURE(queue_service_test_base, Queue_ApproximateMessageCount)
    {
        const int MESSAGE_COUNT = 3;

        azure::storage::cloud_queue queue = get_queue();

        azure::storage::queue_request_options options;
        azure::storage::operation_context context;
        print_client_request_id(context, _XPLATSTR(""));

        CHECK_EQUAL(-1, queue.approximate_message_count());

        queue.download_attributes(options, context);

        CHECK_EQUAL(0, queue.approximate_message_count());

        for (int i = 0; i < MESSAGE_COUNT; ++i)
        {
            azure::storage::cloud_queue_message message(_XPLATSTR("Hello World!"));
            queue.add_message(message);
        }

        CHECK_EQUAL(0, queue.approximate_message_count());

        queue.download_attributes(options, context);

        CHECK_EQUAL(MESSAGE_COUNT, queue.approximate_message_count());

        queue.clear();

        CHECK_EQUAL(MESSAGE_COUNT, queue.approximate_message_count());

        queue.download_attributes(options, context);

        CHECK_EQUAL(0, queue.approximate_message_count());

        queue.delete_queue();
    }

    TEST_FIXTURE(queue_service_test_base, Queue_Metadata)
    {
        azure::storage::cloud_queue_client client = get_queue_client();
        utility::string_t queue_name = get_queue_name();
        azure::storage::cloud_queue queue1 = client.get_queue_reference(queue_name);
        azure::storage::cloud_queue queue2 = client.get_queue_reference(queue_name);
        queue1.create_if_not_exists();

        azure::storage::queue_request_options options;
        azure::storage::operation_context context;
        print_client_request_id(context, _XPLATSTR(""));

        CHECK(queue1.metadata().empty());

        queue1.download_attributes(options, context);

        CHECK(queue1.metadata().empty());

        queue1.metadata().insert(std::make_pair(_XPLATSTR("aaa"), _XPLATSTR("111")));
        queue1.metadata().insert(std::make_pair(_XPLATSTR("bbb"), _XPLATSTR("222")));
        queue1.metadata().insert(std::make_pair(_XPLATSTR("ccc"), _XPLATSTR("333")));

        queue1.upload_metadata(options, context);

        CHECK_EQUAL(3U, queue1.metadata().size());
        CHECK(queue1.metadata()[_XPLATSTR("aaa")].compare(_XPLATSTR("111")) == 0);
        CHECK(queue1.metadata()[_XPLATSTR("bbb")].compare(_XPLATSTR("222")) == 0);
        CHECK(queue1.metadata()[_XPLATSTR("ccc")].compare(_XPLATSTR("333")) == 0);

        queue1.download_attributes(options, context);

        CHECK_EQUAL(3U, queue1.metadata().size());
        CHECK(queue1.metadata()[_XPLATSTR("aaa")].compare(_XPLATSTR("111")) == 0);
        CHECK(queue1.metadata()[_XPLATSTR("bbb")].compare(_XPLATSTR("222")) == 0);
        CHECK(queue1.metadata()[_XPLATSTR("ccc")].compare(_XPLATSTR("333")) == 0);

        queue2.download_attributes(options, context);

        CHECK_EQUAL(3U, queue2.metadata().size());
        CHECK(queue2.metadata()[_XPLATSTR("aaa")].compare(_XPLATSTR("111")) == 0);
        CHECK(queue2.metadata()[_XPLATSTR("bbb")].compare(_XPLATSTR("222")) == 0);
        CHECK(queue2.metadata()[_XPLATSTR("ccc")].compare(_XPLATSTR("333")) == 0);

        queue2.metadata().erase(_XPLATSTR("bbb"));

        CHECK_EQUAL(2U, queue2.metadata().size());
        CHECK(queue2.metadata()[_XPLATSTR("aaa")].compare(_XPLATSTR("111")) == 0);
        CHECK(queue2.metadata()[_XPLATSTR("ccc")].compare(_XPLATSTR("333")) == 0);

        queue2.upload_metadata(options, context);

        CHECK_EQUAL(2U, queue2.metadata().size());
        CHECK(queue2.metadata()[_XPLATSTR("aaa")].compare(_XPLATSTR("111")) == 0);
        CHECK(queue2.metadata()[_XPLATSTR("ccc")].compare(_XPLATSTR("333")) == 0);

        queue1.download_attributes(options, context);

        CHECK_EQUAL(2U, queue1.metadata().size());
        CHECK(queue1.metadata()[_XPLATSTR("aaa")].compare(_XPLATSTR("111")) == 0);
        CHECK(queue1.metadata()[_XPLATSTR("ccc")].compare(_XPLATSTR("333")) == 0);

        queue1.metadata().insert(std::make_pair(_XPLATSTR("ddd"), _XPLATSTR("")));
        CHECK_THROW(queue1.upload_metadata(options, context), std::invalid_argument);
        queue1.metadata().erase(_XPLATSTR("ddd"));

        queue1.metadata().insert(std::make_pair(_XPLATSTR("ddd"), _XPLATSTR(" ")));
        CHECK_THROW(queue1.upload_metadata(options, context), std::invalid_argument);
        queue1.metadata().erase(_XPLATSTR("ddd"));

        queue1.metadata().insert(std::make_pair(_XPLATSTR("ddd"), _XPLATSTR(" \t\r\n ")));
        CHECK_THROW(queue1.upload_metadata(options, context), std::invalid_argument);
        queue1.metadata().erase(_XPLATSTR("ddd"));

        azure::storage::operation_context whitespace_metadata_context;
        print_client_request_id(context, _XPLATSTR(""));
        whitespace_metadata_context.set_sending_request([](web::http::http_request& request, azure::storage::operation_context)
        {
            request.headers().add(_XPLATSTR("x-ms-meta-mywhitespacekey"), _XPLATSTR(""));
        });

        queue1.upload_metadata(options, whitespace_metadata_context);

        queue1.download_attributes(options, context);

        CHECK(!queue1.metadata().empty());
        CHECK(queue1.metadata()[_XPLATSTR("mywhitespacekey")].compare(_XPLATSTR("")) == 0);

        queue1.delete_queue();
    }

    TEST_FIXTURE(queue_service_test_base, Queue_NotFound)
    {
        utility::string_t queue_name = get_queue_name();
        azure::storage::cloud_queue_client client = get_queue_client();

        azure::storage::cloud_queue queue = client.get_queue_reference(queue_name);

        size_t message_count;
        std::chrono::seconds visibility_timeout;
        azure::storage::queue_request_options options;
        azure::storage::operation_context context;
        print_client_request_id(context, _XPLATSTR(""));

        message_count = 2U;
        visibility_timeout = std::chrono::seconds(1);

        try
        {
            queue.get_messages(message_count, visibility_timeout, options, context);
            CHECK(false);
        }
        catch (const azure::storage::storage_exception& e)
        {
            CHECK_EQUAL(web::http::status_codes::NotFound, e.result().http_status_code());
            CHECK(e.result().extended_error().code().compare(_XPLATSTR("QueueNotFound")) == 0);
            CHECK(!e.result().extended_error().message().empty());
        }

        CHECK(!context.client_request_id().empty());
        CHECK(context.start_time().is_initialized());
        CHECK(context.end_time().is_initialized());
        CHECK_EQUAL(1U, context.request_results().size());
        CHECK(context.request_results()[0].is_response_available());
        CHECK(context.request_results()[0].start_time().is_initialized());
        CHECK(context.request_results()[0].end_time().is_initialized());
        CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
        CHECK_EQUAL(web::http::status_codes::NotFound, context.request_results()[0].http_status_code());
        CHECK(!context.request_results()[0].service_request_id().empty());
        CHECK(context.request_results()[0].request_date().is_initialized());
        CHECK(context.request_results()[0].content_md5().empty());
        CHECK(context.request_results()[0].etag().empty());
        CHECK(context.request_results()[0].extended_error().code().compare(_XPLATSTR("QueueNotFound")) == 0);
        CHECK(!context.request_results()[0].extended_error().message().empty());
    }

    TEST_FIXTURE(queue_service_test_base, Queue_UpdateAndDelQueuedMessage)
    {
        azure::storage::cloud_queue queue = get_queue();
        utility::string_t content = get_random_string();

        {
            azure::storage::queue_request_options options;
            azure::storage::cloud_queue_message message;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            message.set_content(content);

            queue.add_message(message, std::chrono::seconds(15 * 60), std::chrono::seconds(0), options, context);

            CHECK(!message.id().empty());
            CHECK(!message.pop_receipt().empty());
            CHECK(message.insertion_time().is_initialized());
            CHECK(message.expiration_time().is_initialized());
            CHECK(message.next_visibile_time().is_initialized());

            utility::string_t old_pop_recepit = message.pop_receipt();
            utility::datetime old_next_visible_time = message.next_visibile_time();
            message.set_content(get_random_string());
            queue.update_message(message, std::chrono::seconds(15 * 60), true, options, context);

            CHECK(old_pop_recepit.compare(message.pop_receipt()) != 0);
            CHECK(old_next_visible_time != message.next_visibile_time());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(2U, context.request_results().size());
            CHECK(context.request_results()[1].is_response_available());
            CHECK(context.request_results()[1].start_time().is_initialized());
            CHECK(context.request_results()[1].end_time().is_initialized());
            CHECK(context.request_results()[1].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[1].http_status_code());
            CHECK(!context.request_results()[1].service_request_id().empty());
            CHECK(context.request_results()[1].request_date().is_initialized());
            CHECK(context.request_results()[1].content_md5().empty());
            CHECK(context.request_results()[1].etag().empty());
            CHECK(context.request_results()[1].extended_error().code().empty());
            CHECK(context.request_results()[1].extended_error().message().empty());
            CHECK(context.request_results()[1].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::cloud_queue_message message;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            message.set_content(content);

            queue.add_message(message, std::chrono::seconds(15 * 60), std::chrono::seconds(0), options, context);
            queue.delete_message(message, options, context);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(2U, context.request_results().size());
            CHECK(context.request_results()[1].is_response_available());
            CHECK(context.request_results()[1].start_time().is_initialized());
            CHECK(context.request_results()[1].end_time().is_initialized());
            CHECK(context.request_results()[1].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[1].http_status_code());
            CHECK(!context.request_results()[1].service_request_id().empty());
            CHECK(context.request_results()[1].request_date().is_initialized());
            CHECK(context.request_results()[1].content_md5().empty());
            CHECK(context.request_results()[1].etag().empty());
            CHECK(context.request_results()[1].extended_error().code().empty());
            CHECK(context.request_results()[1].extended_error().message().empty());
            CHECK(context.request_results()[1].extended_error().details().empty());
        }

        queue.delete_queue();
    }

    TEST_FIXTURE(queue_service_test_base, Queue_Messages)
    {
        azure::storage::cloud_queue queue = get_queue();

        utility::string_t content1 = get_random_string();
        utility::string_t content2 = get_random_string();
        utility::string_t content3 = get_random_string();
        utility::string_t new_content;

        azure::storage::cloud_queue_message message1;
        azure::storage::cloud_queue_message message2;
        azure::storage::cloud_queue_message message3;

        {
            std::chrono::seconds time_to_live;
            std::chrono::seconds initial_visibility_timeout;
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            message1.set_content(content1);
            time_to_live = std::chrono::seconds(15 * 60);
            initial_visibility_timeout = std::chrono::seconds(0);

            queue.add_message(message1, time_to_live, initial_visibility_timeout, options, context);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::Created, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());

            message2.set_content(content2);
            time_to_live = std::chrono::seconds(15 * 60);
            initial_visibility_timeout = std::chrono::seconds(0);

            queue.add_message(message2, time_to_live, initial_visibility_timeout, options, context);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(2U, context.request_results().size());
            CHECK(context.request_results()[1].is_response_available());
            CHECK(context.request_results()[1].start_time().is_initialized());
            CHECK(context.request_results()[1].end_time().is_initialized());
            CHECK(context.request_results()[1].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::Created, context.request_results()[1].http_status_code());
            CHECK(!context.request_results()[1].service_request_id().empty());
            CHECK(context.request_results()[1].request_date().is_initialized());
            CHECK(context.request_results()[1].content_md5().empty());
            CHECK(context.request_results()[1].etag().empty());
            CHECK(context.request_results()[1].extended_error().code().empty());
            CHECK(context.request_results()[1].extended_error().message().empty());
            CHECK(context.request_results()[1].extended_error().details().empty());

            message3.set_content(content3);
            time_to_live = std::chrono::seconds(15 * 60);
            initial_visibility_timeout = std::chrono::seconds(0);

            queue.add_message(message3, time_to_live, initial_visibility_timeout, options, context);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(3U, context.request_results().size());
            CHECK(context.request_results()[2].is_response_available());
            CHECK(context.request_results()[2].start_time().is_initialized());
            CHECK(context.request_results()[2].end_time().is_initialized());
            CHECK(context.request_results()[2].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::Created, context.request_results()[2].http_status_code());
            CHECK(!context.request_results()[2].service_request_id().empty());
            CHECK(context.request_results()[2].request_date().is_initialized());
            CHECK(context.request_results()[2].content_md5().empty());
            CHECK(context.request_results()[2].etag().empty());
            CHECK(context.request_results()[2].extended_error().code().empty());
            CHECK(context.request_results()[2].extended_error().message().empty());
            CHECK(context.request_results()[2].extended_error().details().empty());
        }

        {
            size_t message_count;
            std::chrono::seconds visibility_timeout;
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            message_count = 2U;
            visibility_timeout = std::chrono::seconds(1);

            std::vector<azure::storage::cloud_queue_message> messages = queue.get_messages(message_count, visibility_timeout, options, context);

            CHECK_EQUAL(2U, messages.size());

            message1 = messages[0];
            message2 = messages[1];

            for (std::vector<azure::storage::cloud_queue_message>::const_iterator itr = messages.cbegin(); itr != messages.cend(); ++itr)
            {
                azure::storage::cloud_queue_message message = *itr;

                CHECK(message.content_as_string().compare(content1) == 0 || message.content_as_string().compare(content2) == 0 || message.content_as_string().compare(content3) == 0);
                CHECK(!message.id().empty());
                CHECK(!message.pop_receipt().empty());
                CHECK(message.insertion_time().is_initialized());
                CHECK(message.expiration_time().is_initialized());
                CHECK(message.next_visibile_time().is_initialized());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        std::this_thread::sleep_for(std::chrono::seconds(3LL));

        {
            utility::string_t old_pop_recepit = message1.pop_receipt();
            utility::datetime old_next_visible_time = message1.next_visibile_time();

            std::chrono::seconds visibility_timeout;
            bool update_content;
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            visibility_timeout = std::chrono::seconds(15 * 60);
            update_content = false;

            queue.update_message(message1, visibility_timeout, update_content, options, context);

            CHECK(message1.content_as_string().compare(content1) == 0 || message1.content_as_string().compare(content2) == 0 || message1.content_as_string().compare(content3) == 0);
            CHECK(!message1.id().empty());
            CHECK(!message1.pop_receipt().empty());
            CHECK(message1.insertion_time().is_initialized());
            CHECK(message1.expiration_time().is_initialized());
            CHECK(message1.next_visibile_time().is_initialized());

            CHECK(old_pop_recepit.compare(message1.pop_receipt()) != 0);
            CHECK(old_next_visible_time != message1.next_visibile_time());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            size_t message_count;
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            message_count = 5U;

            std::vector<azure::storage::cloud_queue_message> messages = queue.peek_messages(message_count, options, context);

            CHECK_EQUAL(2U, messages.size());

            for (std::vector<azure::storage::cloud_queue_message>::const_iterator itr = messages.cbegin(); itr != messages.cend(); ++itr)
            {
                azure::storage::cloud_queue_message message = *itr;

                CHECK(message.content_as_string().compare(content1) == 0 || message.content_as_string().compare(content2) == 0 || message.content_as_string().compare(content3) == 0);
                CHECK(!message.id().empty());
                CHECK(message.pop_receipt().empty());
                CHECK(message.insertion_time().is_initialized());
                CHECK(message.expiration_time().is_initialized());
                CHECK(!message.next_visibile_time().is_initialized());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            queue.delete_message(message2, options, context);

            CHECK(message2.content_as_string().compare(content1) == 0 || message2.content_as_string().compare(content2) == 0 || message2.content_as_string().compare(content3) == 0);
            CHECK(!message2.id().empty());
            CHECK(!message2.pop_receipt().empty());
            CHECK(message2.insertion_time().is_initialized());
            CHECK(message2.expiration_time().is_initialized());
            CHECK(message2.next_visibile_time().is_initialized());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::cloud_queue_message message = queue.peek_message(options, context);

            CHECK(message.content_as_string().compare(content1) == 0 || message.content_as_string().compare(content2) == 0 || message.content_as_string().compare(content3) == 0);
            CHECK(!message.id().empty());
            CHECK(message.pop_receipt().empty());
            CHECK(message.insertion_time().is_initialized());
            CHECK(message.expiration_time().is_initialized());
            CHECK(!message.next_visibile_time().is_initialized());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            std::chrono::seconds visibility_timeout;
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            visibility_timeout = std::chrono::seconds(15 * 60);

            message3 = queue.get_message(visibility_timeout, options, context);

            CHECK(message3.content_as_string().compare(content1) == 0 || message3.content_as_string().compare(content2) == 0 || message3.content_as_string().compare(content3) == 0);
            CHECK(!message3.id().empty());
            CHECK(!message3.pop_receipt().empty());
            CHECK(message3.insertion_time().is_initialized());
            CHECK(message3.expiration_time().is_initialized());
            CHECK(message3.next_visibile_time().is_initialized());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::cloud_queue_message message = queue.peek_message(options, context);

            CHECK(message.content_as_string().empty());
            CHECK(message.id().empty());
            CHECK(message.pop_receipt().empty());
            CHECK(!message.insertion_time().is_initialized());
            CHECK(!message.expiration_time().is_initialized());
            CHECK(!message.next_visibile_time().is_initialized());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            std::chrono::seconds visibility_timeout;
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            visibility_timeout = std::chrono::seconds(1);

            azure::storage::cloud_queue_message message = queue.get_message(visibility_timeout, options, context);

            CHECK(message.content_as_string().empty());
            CHECK(message.id().empty());
            CHECK(message.pop_receipt().empty());
            CHECK(!message.insertion_time().is_initialized());
            CHECK(!message.expiration_time().is_initialized());
            CHECK(!message.next_visibile_time().is_initialized());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        std::this_thread::sleep_for(std::chrono::seconds(3LL));

        {
            new_content = get_random_string();
            utility::string_t old_pop_recepit = message3.pop_receipt();
            utility::datetime old_next_visible_time = message3.next_visibile_time();

            std::chrono::seconds visibility_timeout;
            bool update_content;
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            message3.set_content(new_content);
            visibility_timeout = std::chrono::seconds(0);
            update_content = true;

            queue.update_message(message3, visibility_timeout, update_content, options, context);

            CHECK(message3.content_as_string().compare(new_content) == 0);
            CHECK(!message3.id().empty());
            CHECK(!message3.pop_receipt().empty());
            CHECK(message3.insertion_time().is_initialized());
            CHECK(message3.expiration_time().is_initialized());
            CHECK(message3.next_visibile_time().is_initialized());

            CHECK(old_pop_recepit.compare(message3.pop_receipt()) != 0);
            CHECK(old_next_visible_time != message3.next_visibile_time());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::cloud_queue_message message = queue.peek_message(options, context);

            CHECK(message.content_as_string().compare(new_content) == 0);
            CHECK(!message.id().empty());
            CHECK(message.pop_receipt().empty());
            CHECK(message.insertion_time().is_initialized());
            CHECK(message.expiration_time().is_initialized());
            CHECK(!message.next_visibile_time().is_initialized());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            new_content = get_random_string();

            std::chrono::seconds visibility_timeout;
            bool update_content;
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            message3.set_content(new_content);
            visibility_timeout = std::chrono::seconds(0);
            update_content = true;

            queue.update_message(message3, visibility_timeout, update_content, options, context);

            CHECK(message3.content_as_string().compare(new_content) == 0);
            CHECK(!message3.id().empty());
            CHECK(!message3.pop_receipt().empty());
            CHECK(message3.insertion_time().is_initialized());
            CHECK(message3.expiration_time().is_initialized());
            CHECK(message3.next_visibile_time().is_initialized());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::cloud_queue_message message = queue.peek_message(options, context);

            CHECK(message.content_as_string().compare(new_content) == 0);
            CHECK(!message.id().empty());
            CHECK(message.pop_receipt().empty());
            CHECK(message.insertion_time().is_initialized());
            CHECK(message.expiration_time().is_initialized());
            CHECK(!message.next_visibile_time().is_initialized());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            queue.clear(options, context);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::cloud_queue_message message = queue.peek_message(options, context);

            CHECK(message.content_as_string().empty());
            CHECK(message.id().empty());
            CHECK(message.pop_receipt().empty());
            CHECK(!message.insertion_time().is_initialized());
            CHECK(!message.expiration_time().is_initialized());
            CHECK(!message.next_visibile_time().is_initialized());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        queue.delete_queue();
    }

    TEST_FIXTURE(queue_service_test_base, Queue_InvalidMessages)
    {
        azure::storage::cloud_queue queue = get_queue();

        utility::string_t content = get_random_string();

        azure::storage::queue_request_options options;
        azure::storage::operation_context context;
        print_client_request_id(context, _XPLATSTR(""));

        {
            azure::storage::cloud_queue_message message;
            std::chrono::seconds time_to_live;
            std::chrono::seconds initial_visibility_timeout;

            message.set_content(content);
            time_to_live = std::chrono::seconds(-1);
            initial_visibility_timeout = std::chrono::seconds(0);

            CHECK_THROW(queue.add_message(message, time_to_live, initial_visibility_timeout, options, context), std::invalid_argument);
        }

        {
            azure::storage::cloud_queue_message message;
            std::chrono::seconds time_to_live;
            std::chrono::seconds initial_visibility_timeout;

            message.set_content(content);
            time_to_live = std::chrono::seconds(0);
            initial_visibility_timeout = std::chrono::seconds(0);

            CHECK_THROW(queue.add_message(message, time_to_live, initial_visibility_timeout, options, context), std::invalid_argument);
        }

        {
            azure::storage::cloud_queue_message message;
            std::chrono::seconds time_to_live;
            std::chrono::seconds initial_visibility_timeout;

            message.set_content(content);
            time_to_live = std::chrono::seconds(30 * 24 * 60 * 60);
            initial_visibility_timeout = std::chrono::seconds(0);

            CHECK_THROW(queue.add_message(message, time_to_live, initial_visibility_timeout, options, context), std::invalid_argument);
        }

        {
            azure::storage::cloud_queue_message message;
            std::chrono::seconds time_to_live;
            std::chrono::seconds initial_visibility_timeout;

            message.set_content(content);
            time_to_live = std::chrono::seconds(15 * 60);
            initial_visibility_timeout = std::chrono::seconds(-1);

            CHECK_THROW(queue.add_message(message, time_to_live, initial_visibility_timeout, options, context), std::invalid_argument);
        }

        {
            azure::storage::cloud_queue_message message;
            std::chrono::seconds time_to_live;
            std::chrono::seconds initial_visibility_timeout;

            message.set_content(content);
            time_to_live = std::chrono::seconds(15 * 60);
            initial_visibility_timeout = std::chrono::seconds(30 * 24 * 60 * 60);

            CHECK_THROW(queue.add_message(message, time_to_live, initial_visibility_timeout, options, context), std::invalid_argument);
        }

        {
            std::chrono::seconds visibility_timeout;

            visibility_timeout = std::chrono::seconds(-1);

            CHECK_THROW(queue.get_message(visibility_timeout, options, context), std::invalid_argument);
        }

        {
            std::chrono::seconds visibility_timeout;

            visibility_timeout = std::chrono::seconds(30 * 24 * 60 * 60);

            CHECK_THROW(queue.get_message(visibility_timeout, options, context), std::invalid_argument);
        }

        {
            size_t message_count;
            std::chrono::seconds visibility_timeout;

            message_count = 50U;
            visibility_timeout = std::chrono::seconds(1);

            CHECK_THROW(queue.get_messages(message_count, visibility_timeout, options, context), std::invalid_argument);
        }

        {
            size_t message_count;
            std::chrono::seconds visibility_timeout;

            message_count = 3U;
            visibility_timeout = std::chrono::seconds(-1);

            CHECK_THROW(queue.get_messages(message_count, visibility_timeout, options, context), std::invalid_argument);
        }

        {
            size_t message_count;
            std::chrono::seconds visibility_timeout;

            message_count = 3U;
            visibility_timeout = std::chrono::seconds(30 * 24 * 60 * 60);

            CHECK_THROW(queue.get_messages(message_count, visibility_timeout, options, context), std::invalid_argument);
        }

        {
            utility::string_t id = _XPLATSTR("");
            utility::string_t pop_receipt = _XPLATSTR("ABCDEF");

            azure::storage::cloud_queue_message message(id, pop_receipt);
            std::chrono::seconds visibility_timeout;
            bool update_content;

            message.set_content(content);
            visibility_timeout = std::chrono::seconds(15 * 60);
            update_content = true;

            CHECK_THROW(queue.update_message(message, visibility_timeout, update_content, options, context), std::invalid_argument);
        }

        {
            utility::string_t id = _XPLATSTR("12345");
            utility::string_t pop_receipt = _XPLATSTR("");

            azure::storage::cloud_queue_message message(id, pop_receipt);
            std::chrono::seconds visibility_timeout;
            bool update_content;

            message.set_content(content);
            visibility_timeout = std::chrono::seconds(15 * 60);
            update_content = true;

            CHECK_THROW(queue.update_message(message, visibility_timeout, update_content, options, context), std::invalid_argument);
        }

        {
            utility::string_t id = _XPLATSTR("12345");
            utility::string_t pop_receipt = _XPLATSTR("ABCDEF");

            azure::storage::cloud_queue_message message(id, pop_receipt);
            std::chrono::seconds visibility_timeout;
            bool update_content;

            message.set_content(content);
            visibility_timeout = std::chrono::seconds(-1);
            update_content = true;

            CHECK_THROW(queue.update_message(message, visibility_timeout, update_content, options, context), std::invalid_argument);
        }

        {
            utility::string_t id = _XPLATSTR("12345");
            utility::string_t pop_receipt = _XPLATSTR("ABCDEF");

            azure::storage::cloud_queue_message message(id, pop_receipt);
            std::chrono::seconds visibility_timeout;
            bool update_content;

            message.set_content(content);
            visibility_timeout = std::chrono::seconds(30 * 24 * 60 * 60);
            update_content = true;

            CHECK_THROW(queue.update_message(message, visibility_timeout, update_content, options, context), std::invalid_argument);
        }
    }

    TEST_FIXTURE(queue_service_test_base, Queue_Permissions)
    {
        azure::storage::cloud_queue queue = get_queue();

        utility::string_t policy_name1 = _XPLATSTR("policy1");
        utility::string_t policy_name2 = _XPLATSTR("policy2");

        uint8_t permission1 = azure::storage::queue_shared_access_policy::permissions::read | azure::storage::queue_shared_access_policy::permissions::add;
        uint8_t permission2 = azure::storage::queue_shared_access_policy::permissions::read | azure::storage::queue_shared_access_policy::permissions::update;

        azure::storage::queue_shared_access_policy policy1(utility::datetime::utc_now(), utility::datetime::utc_now(), permission1);
        azure::storage::queue_shared_access_policy policy2(utility::datetime::utc_now(), utility::datetime::utc_now(), permission2);

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::queue_permissions permissions = queue.download_permissions(options, context);

            CHECK(permissions.policies().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_permissions permissions;
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::shared_access_policies<azure::storage::queue_shared_access_policy> policies;
            policies.insert(std::make_pair(policy_name1, policy1));
            policies.insert(std::make_pair(policy_name2, policy2));

            permissions.set_policies(policies);
            queue.upload_permissions(permissions, options, context);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::queue_permissions permissions = queue.download_permissions(options, context);

            CHECK_EQUAL(2U, permissions.policies().size());
            CHECK_EQUAL(permission1, permissions.policies()[policy_name1].permission());
            CHECK(permissions.policies()[policy_name1].start().is_initialized());
            CHECK(permissions.policies()[policy_name1].expiry().is_initialized());
            CHECK_EQUAL(permission2, permissions.policies()[policy_name2].permission());
            CHECK(permissions.policies()[policy_name2].start().is_initialized());
            CHECK(permissions.policies()[policy_name2].expiry().is_initialized());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_permissions permissions;
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::shared_access_policies<azure::storage::queue_shared_access_policy> policies;

            permissions.set_policies(policies);
            queue.upload_permissions(permissions, options, context);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::queue_permissions permissions = queue.download_permissions(options, context);

            CHECK(permissions.policies().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }
    }

    TEST_FIXTURE(queue_service_test_base, Queue_SharedAccessSignature)
    {
        azure::storage::cloud_queue queue1 = get_queue();

        utility::string_t content = get_random_string();

        azure::storage::cloud_queue_message message1;
        message1.set_content(content);
        queue1.add_message(message1);

        utility::datetime start_date = utility::datetime::utc_now() - utility::datetime::from_minutes(5U);
        utility::datetime expiry_date = start_date + utility::datetime::from_hours(2U);

        azure::storage::queue_shared_access_policy policy;
        policy.set_permissions(azure::storage::queue_shared_access_policy::permissions::process);
        policy.set_start(start_date);
        policy.set_expiry(expiry_date);

        const azure::storage::storage_uri& uri = queue1.uri();
        utility::string_t sas_token = queue1.get_shared_access_signature(policy, utility::string_t());
        azure::storage::storage_credentials credentials(sas_token);
        azure::storage::cloud_queue queue2(uri, credentials);

        azure::storage::cloud_queue_message message2 = queue2.get_message();

        CHECK(message2.content_as_string().compare(content) == 0);

        CHECK_THROW(queue2.update_message(message2, std::chrono::seconds(0), /* update_content */ false), azure::storage::storage_exception);
    }

    TEST_FIXTURE(queue_service_test_base, Queue_RepeatedAddAndGetAndUpdateMessage)
    {
        azure::storage::cloud_queue queue = get_queue();

        for (int i = 0; i < 100; ++i)
        {
            azure::storage::cloud_queue_message message1;
            utility::string_t content = get_random_string();
            message1.set_content(content);

            queue.add_message(message1);
            azure::storage::cloud_queue_message message2 = queue.get_message();

            CHECK(message2.content_as_string().compare(content) == 0);
            CHECK(!message2.id().empty());
            CHECK(!message2.pop_receipt().empty());
            CHECK(message2.insertion_time().is_initialized());
            CHECK(message2.expiration_time().is_initialized());
            CHECK(message2.next_visibile_time().is_initialized());

            utility::string_t old_pop_recepit = message2.pop_receipt();
            utility::datetime old_next_visible_time = message2.next_visibile_time();

            UNREFERENCED_PARAMETER(old_pop_recepit);
            UNREFERENCED_PARAMETER(old_next_visible_time);

            std::chrono::seconds visibility_timeout;
            bool update_content;
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            visibility_timeout = std::chrono::seconds(3600);
            update_content = true;

            queue.update_message(message2, visibility_timeout, update_content, options, context);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }
    }
}
