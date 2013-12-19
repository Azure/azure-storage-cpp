// -----------------------------------------------------------------------------------------
// <copyright file="cloud_queue_client_test.cpp" company="Microsoft">
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
#include "was/queue.h"

SUITE(QueueClient)
{
    TEST(QueueClient_Empty)
    {
        wa::storage::cloud_queue_client client;

        CHECK(client.base_uri().primary_uri().is_empty());
        CHECK(client.base_uri().secondary_uri().is_empty());
        CHECK(client.credentials().is_anonymous());
    }

    TEST(QueueClient_BaseUri)
    {
        wa::storage::storage_uri base_uri(web::http::uri(U("https://myaccount.queue.core.windows.net")), web::http::uri(U("https://myaccount-secondary.queue.core.windows.net")));

        wa::storage::cloud_queue_client client(base_uri);

        CHECK(client.base_uri().primary_uri() == base_uri.primary_uri());
        CHECK(client.base_uri().secondary_uri() == base_uri.secondary_uri());
        CHECK(client.credentials().is_anonymous());
    }

    TEST(QueueClient_BaseUriAndCredentials)
    {
        wa::storage::storage_uri base_uri(web::http::uri(U("https://myaccount.queue.core.windows.net")), web::http::uri(U("https://myaccount-secondary.queue.core.windows.net")));
        wa::storage::storage_credentials credentials(U("devstoreaccount1"), U("Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="));
        wa::storage::queue_request_options default_request_options;

        wa::storage::cloud_queue_client client(base_uri, credentials);

        CHECK(client.base_uri().primary_uri() == base_uri.primary_uri());
        CHECK(client.base_uri().secondary_uri() == base_uri.secondary_uri());
        CHECK(client.credentials().is_shared_key());
    }

    TEST(QueueClient_BaseUriAndCredentialsAndDefaultRequestOptions)
    {
        wa::storage::storage_uri base_uri(web::http::uri(U("https://myaccount.queue.core.windows.net")), web::http::uri(U("https://myaccount-secondary.queue.core.windows.net")));
        wa::storage::storage_credentials credentials(U("devstoreaccount1"), U("Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="));
        wa::storage::queue_request_options default_request_options;

        wa::storage::cloud_queue_client client(base_uri, credentials, default_request_options);

        CHECK(client.base_uri().primary_uri() == base_uri.primary_uri());
        CHECK(client.base_uri().secondary_uri() == base_uri.secondary_uri());
        CHECK(client.credentials().is_shared_key());

        default_request_options.set_location_mode(wa::storage::location_mode::secondary_only);

        client = wa::storage::cloud_queue_client(base_uri, credentials, default_request_options);

        CHECK(client.default_request_options().location_mode() == wa::storage::location_mode::secondary_only);
    }

    TEST(ListQueues_Normal)
    {
        const int QUEUE_COUNT = 5;

        wa::storage::cloud_queue queues[QUEUE_COUNT];
        bool is_found[QUEUE_COUNT];
        for (int i = 0; i < QUEUE_COUNT; ++i)
        {
            wa::storage::cloud_queue queue = get_queue();

            queue.metadata().insert(std::pair<utility::string_t, utility::string_t>(U("aaa"), U("111")));
            queue.metadata().insert(std::pair<utility::string_t, utility::string_t>(U("bbb"), U("222")));
            queue.upload_metadata();

            queues[i] = queue;
            is_found[i] = false;
        }

        wa::storage::cloud_queue_client client = get_queue_client();

        utility::string_t prefix;
        bool get_metadata;
        wa::storage::queue_request_options options;
        wa::storage::operation_context context;

        {
            prefix = object_name_prefix;
            get_metadata = false;

            std::vector<wa::storage::cloud_queue> results = client.list_queues(prefix, get_metadata, options, context);

            CHECK(results.size() >= QUEUE_COUNT);

            for (std::vector<wa::storage::cloud_queue>::const_iterator itr = results.cbegin(); itr != results.cend(); ++itr)
            {
                wa::storage::cloud_queue queue = *itr;

                for (int i = 0; i < QUEUE_COUNT; ++i)
                {
                    if (queue.name().compare(queues[i].name()) == 0)
                    {
                        CHECK(!queue.service_client().base_uri().primary_uri().is_empty());
                        CHECK(queue.service_client().credentials().is_shared_key());
                        CHECK(!queue.name().empty());
                        CHECK(!queue.uri().primary_uri().is_empty());

                        CHECK(queue.metadata().empty());

                        is_found[i] = true;
                    }
                }
            }
        }

        {
            prefix = object_name_prefix;
            get_metadata = true;

            std::vector<wa::storage::cloud_queue> results = client.list_queues(prefix, get_metadata, options, context);

            CHECK(results.size() >= QUEUE_COUNT);

            for (std::vector<wa::storage::cloud_queue>::const_iterator itr = results.cbegin(); itr != results.cend(); ++itr)
            {
                wa::storage::cloud_queue queue = *itr;

                for (int i = 0; i < QUEUE_COUNT; ++i)
                {
                    if (queue.name().compare(queues[i].name()) == 0)
                    {
                        CHECK(!queue.service_client().base_uri().primary_uri().is_empty());
                        CHECK(queue.service_client().credentials().is_shared_key());
                        CHECK(!queue.name().empty());
                        CHECK(!queue.uri().primary_uri().is_empty());

                        CHECK_EQUAL(2U, queue.metadata().size());
                        CHECK(queue.metadata()[U("aaa")].compare(U("111")) == 0);
                        CHECK(queue.metadata()[U("bbb")].compare(U("222")) == 0);

                        is_found[i] = true;
                    }
                }
            }
        }

        for (int i = 0; i < QUEUE_COUNT; ++i)
        {
            queues[i].delete_queue();
        }
    }

    TEST(ListQueues_Segmented)
    {
        const int QUEUE_COUNT = 5;

        wa::storage::cloud_queue queues[QUEUE_COUNT];
        bool is_found[QUEUE_COUNT];
        for (int i = 0; i < QUEUE_COUNT; ++i)
        {
            queues[i] = get_queue();
            is_found[i] = false;
        }

        wa::storage::cloud_queue_client client = get_queue_client();

        utility::string_t prefix;
        bool get_metadata;
        int max_results;
        wa::storage::continuation_token continuation_token;
        wa::storage::queue_request_options options;
        wa::storage::operation_context context;

        prefix = object_name_prefix;
        get_metadata = false;
        max_results = 3;

        int segment_count = 0;
        wa::storage::queue_result_segment result_segment;
        do
        {
            result_segment = client.list_queues_segmented(prefix, get_metadata, max_results, continuation_token, options, context);
            std::vector<wa::storage::cloud_queue> results = result_segment.results();

            CHECK(results.size() >= 0);
            CHECK((int)results.size() <= max_results);

            for (std::vector<wa::storage::cloud_queue>::const_iterator itr = results.cbegin(); itr != results.cend(); ++itr)
            {
                wa::storage::cloud_queue queue = *itr;

                for (int i = 0; i < QUEUE_COUNT; ++i)
                {
                    if (queue.name().compare(queues[i].name()) == 0)
                    {
                        CHECK(!queue.service_client().base_uri().primary_uri().is_empty());
                        CHECK(queue.service_client().credentials().is_shared_key());
                        CHECK(!queue.name().empty());
                        CHECK(!queue.uri().primary_uri().is_empty());

                        is_found[i] = true;
                    }
                }
            }

            ++segment_count;
            continuation_token = result_segment.continuation_token();
        }
        while (!continuation_token.empty());

        CHECK(segment_count > 1);

        for (int i = 0; i < QUEUE_COUNT; ++i)
        {
            queues[i].delete_queue();
        }
    }
}
