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
#include "test_helper.h"
#include "was/queue.h"

SUITE(Queue)
{
    TEST(Queue_Empty)
    {
        wa::storage::cloud_queue queue;

        CHECK(queue.service_client().base_uri().primary_uri().is_empty());
        CHECK(queue.service_client().base_uri().secondary_uri().is_empty());
        CHECK(queue.service_client().credentials().is_anonymous());
        CHECK(queue.name().empty());
        CHECK(queue.uri().primary_uri().is_empty());
        CHECK(queue.uri().secondary_uri().is_empty());
        CHECK_EQUAL(-1, queue.approximate_message_count());
    }

    TEST(Queue_Uri)
    {
        wa::storage::storage_uri uri(web::http::uri(U("https://myaccount.queue.core.windows.net/myqueue")), web::http::uri(U("https://myaccount-secondary.queue.core.windows.net/myqueue")));

        wa::storage::cloud_queue queue(uri);

        web::http::uri expected_primary_uri(U("https://myaccount.queue.core.windows.net"));
        web::http::uri expected_secondary_uri(U("https://myaccount-secondary.queue.core.windows.net"));

        CHECK(queue.service_client().base_uri().primary_uri() == expected_primary_uri);
        CHECK(queue.service_client().base_uri().secondary_uri() == expected_secondary_uri);
        CHECK(queue.service_client().credentials().is_anonymous());
        CHECK(queue.name().compare(U("myqueue")) == 0);
        CHECK(queue.uri().primary_uri() == uri.primary_uri());
        CHECK(queue.uri().secondary_uri() == uri.secondary_uri());
        CHECK_EQUAL(-1, queue.approximate_message_count());
    }

    TEST(Queue_UriAndCredentials)
    {
        wa::storage::storage_uri uri(web::http::uri(U("https://myaccount.queue.core.windows.net/myqueue")), web::http::uri(U("https://myaccount-secondary.queue.core.windows.net/myqueue")));
        wa::storage::storage_credentials credentials(U("devstoreaccount1"), U("Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="));

        wa::storage::cloud_queue queue(uri, credentials);

        web::http::uri expected_primary_uri(U("https://myaccount.queue.core.windows.net"));
        web::http::uri expected_secondary_uri(U("https://myaccount-secondary.queue.core.windows.net"));

        CHECK(queue.service_client().base_uri().primary_uri() == expected_primary_uri);
        CHECK(queue.service_client().base_uri().secondary_uri() == expected_secondary_uri);
        CHECK(queue.service_client().credentials().is_shared_key());
        CHECK(queue.name().compare(U("myqueue")) == 0);
        CHECK(queue.uri().primary_uri() == uri.primary_uri());
        CHECK(queue.uri().secondary_uri() == uri.secondary_uri());
        CHECK_EQUAL(-1, queue.approximate_message_count());
    }

 
    TEST(Message_Empty)
    {
        wa::storage::cloud_queue_message message;

        CHECK(message.content_as_string().empty());
        CHECK(message.content_as_binary().empty());
        CHECK(message.id().empty());
        CHECK(message.pop_receipt().empty());
        CHECK(!message.expiration_time().is_initialized());
        CHECK(!message.insertion_time().is_initialized());
        CHECK(!message.next_visibile_time().is_initialized());
        CHECK_EQUAL(0, message.dequeue_count());
    }

    TEST(Message_String)
    {
        utility::string_t content = get_random_string();

        wa::storage::cloud_queue_message message(content);

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

    TEST(Message_Binary)
    {
        std::vector<uint8_t> content = get_random_binary_data();

        wa::storage::cloud_queue_message message(content);

        CHECK(message.content_as_string().size() >= content.size());
        CHECK_ARRAY_EQUAL(content, message.content_as_binary(), content.size());
        CHECK(message.id().empty());
        CHECK(message.pop_receipt().empty());
        CHECK(!message.expiration_time().is_initialized());
        CHECK(!message.insertion_time().is_initialized());
        CHECK(!message.next_visibile_time().is_initialized());
        CHECK_EQUAL(0, message.dequeue_count());

        content = get_random_binary_data();
        message.set_content(content);

        CHECK(message.content_as_string().size() >= content.size());
        CHECK_ARRAY_EQUAL(content, message.content_as_binary(), content.size());
        CHECK(message.id().empty());
        CHECK(message.pop_receipt().empty());
        CHECK(!message.expiration_time().is_initialized());
        CHECK(!message.insertion_time().is_initialized());
        CHECK(!message.next_visibile_time().is_initialized());
        CHECK_EQUAL(0, message.dequeue_count());
    }

    TEST(Message_IdAndPopReceipt)
    {
        utility::string_t id = get_random_string();
        utility::string_t pop_receipt = get_random_string();

        wa::storage::cloud_queue_message message(id, pop_receipt);

        CHECK(message.content_as_string().empty());
        CHECK(message.content_as_binary().empty());
        CHECK(id.compare(message.id()) == 0);
        CHECK(pop_receipt.compare(message.pop_receipt()) == 0);
        CHECK(!message.expiration_time().is_initialized());
        CHECK(!message.insertion_time().is_initialized());
        CHECK(!message.next_visibile_time().is_initialized());
        CHECK_EQUAL(0, message.dequeue_count());
    }

    TEST(QueueRequestOptions_Normal)
    {
        wa::storage::queue_request_options options;

        CHECK(options.location_mode() == wa::storage::location_mode::primary_only);
    }

    TEST(Queue_CreateAndDelete)
    {
        utility::string_t queue_name = get_queue_name();
        wa::storage::cloud_queue_client client = get_queue_client();

        wa::storage::cloud_queue queue = client.get_queue_reference(queue_name);

        {
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            queue.create(options, context);
        }

        {
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            bool exists = queue.exists(options, context);

            CHECK(exists);
        }

        {
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            CHECK_THROW(queue.create(options, context), wa::storage::storage_exception);
        }

        {
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            queue.delete_queue(options, context);
        }

        {
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            bool exists = queue.exists(options, context);

            CHECK(!exists);
        }

        std::this_thread::sleep_for(std::chrono::seconds(3LL));

        {
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            //CHECK_THROW(queue.delete_queue(options, context), wa::storage::storage_exception);
        }
    }

    TEST(Queue_CreateIfNotExistsAndDeleteIfExists)
    {
        utility::string_t queue_name = get_queue_name();
        wa::storage::cloud_queue_client client = get_queue_client();

        wa::storage::cloud_queue queue = client.get_queue_reference(queue_name);

        {
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            bool created = queue.create_if_not_exists(options, context);

            CHECK(created);
        }

        {
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            bool exists = queue.exists(options, context);

            CHECK(exists);
        }

        {
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            bool created = queue.create_if_not_exists(options, context);

            CHECK(!created);
        }

        {
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            bool deleted = queue.delete_queue_if_exists(options, context);

            CHECK(deleted);
        }

        {
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            bool exists = queue.exists(options, context);

            CHECK(!exists);
        }

        {
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            bool deleted = queue.delete_queue_if_exists(options, context);

            CHECK(!deleted);
        }
    }

    TEST(Queue_ApproximateMessageCount)
    {
        const int MESSAGE_COUNT = 3;

        wa::storage::cloud_queue queue = get_queue();

        wa::storage::queue_request_options options;
        wa::storage::operation_context context;

        CHECK_EQUAL(-1, queue.approximate_message_count());

        queue.download_attributes(options, context);

        CHECK_EQUAL(0, queue.approximate_message_count());

        for (int i = 0; i < MESSAGE_COUNT; ++i)
        {
            wa::storage::cloud_queue_message message(U("Hello World!"));
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

    TEST(Queue_Metadata)
    {
        wa::storage::cloud_queue_client client = get_queue_client();
        utility::string_t queue_name = get_queue_name();
        wa::storage::cloud_queue queue1 = client.get_queue_reference(queue_name);
        wa::storage::cloud_queue queue2 = client.get_queue_reference(queue_name);
        queue1.create_if_not_exists();

        wa::storage::queue_request_options options;
        wa::storage::operation_context context;

        CHECK(queue1.metadata().empty());

        queue1.download_attributes(options, context);

        CHECK(queue1.metadata().empty());

        queue1.metadata().insert(std::pair<utility::string_t, utility::string_t>(U("aaa"), U("111")));
        queue1.metadata().insert(std::pair<utility::string_t, utility::string_t>(U("bbb"), U("222")));
        queue1.metadata().insert(std::pair<utility::string_t, utility::string_t>(U("ccc"), U("333")));

        queue1.upload_metadata(options, context);

        CHECK_EQUAL(3, queue1.metadata().size());
        CHECK(queue1.metadata()[U("aaa")].compare(U("111")) == 0);
        CHECK(queue1.metadata()[U("bbb")].compare(U("222")) == 0);
        CHECK(queue1.metadata()[U("ccc")].compare(U("333")) == 0);

        queue1.download_attributes(options, context);

        CHECK_EQUAL(3, queue1.metadata().size());
        CHECK(queue1.metadata()[U("aaa")].compare(U("111")) == 0);
        CHECK(queue1.metadata()[U("bbb")].compare(U("222")) == 0);
        CHECK(queue1.metadata()[U("ccc")].compare(U("333")) == 0);

        queue2.download_attributes(options, context);

        CHECK_EQUAL(3, queue2.metadata().size());
        CHECK(queue2.metadata()[U("aaa")].compare(U("111")) == 0);
        CHECK(queue2.metadata()[U("bbb")].compare(U("222")) == 0);
        CHECK(queue2.metadata()[U("ccc")].compare(U("333")) == 0);

        queue2.metadata().erase(U("bbb"));

        CHECK_EQUAL(2, queue2.metadata().size());
        CHECK(queue2.metadata()[U("aaa")].compare(U("111")) == 0);
        CHECK(queue2.metadata()[U("ccc")].compare(U("333")) == 0);

        queue2.upload_metadata(options, context);

        CHECK_EQUAL(2, queue2.metadata().size());
        CHECK(queue2.metadata()[U("aaa")].compare(U("111")) == 0);
        CHECK(queue2.metadata()[U("ccc")].compare(U("333")) == 0);

        queue1.download_attributes(options, context);

        CHECK_EQUAL(2, queue1.metadata().size());
        CHECK(queue1.metadata()[U("aaa")].compare(U("111")) == 0);
        CHECK(queue1.metadata()[U("ccc")].compare(U("333")) == 0);

        queue1.delete_queue();
    }

    TEST(Queue_Messages)
    {
        wa::storage::cloud_queue queue = get_queue();

        utility::string_t content1 = get_random_string();
        utility::string_t content2 = get_random_string();
        utility::string_t content3 = get_random_string();
        utility::string_t new_content;

        wa::storage::cloud_queue_message message1;
        wa::storage::cloud_queue_message message2;
        wa::storage::cloud_queue_message message3;

        wa::storage::queue_request_options options;
        wa::storage::operation_context context;

        {
            std::chrono::seconds time_to_live;
            std::chrono::seconds initial_visibility_timeout;

            message1.set_content(content1);
            time_to_live = std::chrono::seconds(15 * 60);
            initial_visibility_timeout = std::chrono::seconds(0);

            queue.add_message(message1, time_to_live, initial_visibility_timeout, options, context);

            message2.set_content(content2);
            time_to_live = std::chrono::seconds(15 * 60);
            initial_visibility_timeout = std::chrono::seconds(0);

            queue.add_message(message2, time_to_live, initial_visibility_timeout, options, context);

            message3.set_content(content3);
            time_to_live = std::chrono::seconds(15 * 60);
            initial_visibility_timeout = std::chrono::seconds(0);

            queue.add_message(message3, time_to_live, initial_visibility_timeout, options, context);
        }

        {
            size_t message_count;
            std::chrono::seconds visibility_timeout;

            message_count = 2U;
            visibility_timeout = std::chrono::seconds(1);

            std::vector<wa::storage::cloud_queue_message> messages = queue.get_messages(message_count, visibility_timeout, options, context);

            CHECK_EQUAL(2U, messages.size());

            message1 = messages[0];
            message2 = messages[1];

            for (std::vector<wa::storage::cloud_queue_message>::const_iterator itr = messages.cbegin(); itr != messages.cend(); ++itr)
            {
                wa::storage::cloud_queue_message message = *itr;

                CHECK(message.content_as_string().compare(content1) == 0 || message.content_as_string().compare(content2) == 0 || message.content_as_string().compare(content3) == 0);
                CHECK(!message.id().empty());
                CHECK(!message.pop_receipt().empty());
                CHECK(message.insertion_time().is_initialized());
                CHECK(message.expiration_time().is_initialized());
                CHECK(message.next_visibile_time().is_initialized());
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(3LL));

        {
            utility::string_t old_pop_recepit = message1.pop_receipt();
            utility::datetime old_next_visible_time = message1.next_visibile_time();

            std::chrono::seconds visibility_timeout;
            bool update_content;

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
        }

        {
            size_t message_count;

            message_count = 5U;

            std::vector<wa::storage::cloud_queue_message> messages = queue.peek_messages(message_count, options, context);

            CHECK_EQUAL(2U, messages.size());

            for (std::vector<wa::storage::cloud_queue_message>::const_iterator itr = messages.cbegin(); itr != messages.cend(); ++itr)
            {
                wa::storage::cloud_queue_message message = *itr;

                CHECK(message.content_as_string().compare(content1) == 0 || message.content_as_string().compare(content2) == 0 || message.content_as_string().compare(content3) == 0);
                CHECK(!message.id().empty());
                CHECK(message.pop_receipt().empty());
                CHECK(message.insertion_time().is_initialized());
                CHECK(message.expiration_time().is_initialized());
                CHECK(!message.next_visibile_time().is_initialized());
            }
        }

        {
            queue.delete_message(message2, options, context);

            CHECK(message2.content_as_string().compare(content1) == 0 || message2.content_as_string().compare(content2) == 0 || message2.content_as_string().compare(content3) == 0);
            CHECK(!message2.id().empty());
            CHECK(!message2.pop_receipt().empty());
            CHECK(message2.insertion_time().is_initialized());
            CHECK(message2.expiration_time().is_initialized());
            CHECK(message2.next_visibile_time().is_initialized());
        }

        {
            wa::storage::cloud_queue_message message = queue.peek_message(options, context);

            CHECK(message.content_as_string().compare(content1) == 0 || message.content_as_string().compare(content2) == 0 || message.content_as_string().compare(content3) == 0);
            CHECK(!message.id().empty());
            CHECK(message.pop_receipt().empty());
            CHECK(message.insertion_time().is_initialized());
            CHECK(message.expiration_time().is_initialized());
            CHECK(!message.next_visibile_time().is_initialized());
        }

        {
            std::chrono::seconds visibility_timeout;

            visibility_timeout = std::chrono::seconds(15 * 60);

            message3 = queue.get_message(visibility_timeout, options, context);

            CHECK(message3.content_as_string().compare(content1) == 0 || message3.content_as_string().compare(content2) == 0 || message3.content_as_string().compare(content3) == 0);
            CHECK(!message3.id().empty());
            CHECK(!message3.pop_receipt().empty());
            CHECK(message3.insertion_time().is_initialized());
            CHECK(message3.expiration_time().is_initialized());
            CHECK(message3.next_visibile_time().is_initialized());
        }

        {
            wa::storage::cloud_queue_message message = queue.peek_message(options, context);

            CHECK(message.content_as_string().empty());
            CHECK(message.id().empty());
            CHECK(message.pop_receipt().empty());
            CHECK(!message.insertion_time().is_initialized());
            CHECK(!message.expiration_time().is_initialized());
            CHECK(!message.next_visibile_time().is_initialized());
        }

        {
            std::chrono::seconds visibility_timeout;

            visibility_timeout = std::chrono::seconds(1);

            wa::storage::cloud_queue_message message = queue.get_message(visibility_timeout, options, context);

            CHECK(message.content_as_string().empty());
            CHECK(message.id().empty());
            CHECK(message.pop_receipt().empty());
            CHECK(!message.insertion_time().is_initialized());
            CHECK(!message.expiration_time().is_initialized());
            CHECK(!message.next_visibile_time().is_initialized());
        }

        std::this_thread::sleep_for(std::chrono::seconds(3LL));

        {
            new_content = get_random_string();
            utility::string_t old_pop_recepit = message3.pop_receipt();
            utility::datetime old_next_visible_time = message3.next_visibile_time();

            std::chrono::seconds visibility_timeout;
            bool update_content;

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
        }

        {
            wa::storage::cloud_queue_message message = queue.peek_message(options, context);

            CHECK(message.content_as_string().compare(new_content) == 0);
            CHECK(!message.id().empty());
            CHECK(message.pop_receipt().empty());
            CHECK(message.insertion_time().is_initialized());
            CHECK(message.expiration_time().is_initialized());
            CHECK(!message.next_visibile_time().is_initialized());
        }

        {
            new_content = get_random_string();

            std::chrono::seconds visibility_timeout;
            bool update_content;

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
        }

        {
            wa::storage::cloud_queue_message message = queue.peek_message(options, context);

            CHECK(message.content_as_string().compare(new_content) == 0);
            CHECK(!message.id().empty());
            CHECK(message.pop_receipt().empty());
            CHECK(message.insertion_time().is_initialized());
            CHECK(message.expiration_time().is_initialized());
            CHECK(!message.next_visibile_time().is_initialized());
        }

        {
            queue.clear(options, context);
        }

        {
            wa::storage::cloud_queue_message message = queue.peek_message(options, context);

            CHECK(message.content_as_string().empty());
            CHECK(message.id().empty());
            CHECK(message.pop_receipt().empty());
            CHECK(!message.insertion_time().is_initialized());
            CHECK(!message.expiration_time().is_initialized());
            CHECK(!message.next_visibile_time().is_initialized());
        }

        queue.delete_queue();
    }

    TEST(Queue_Permissions)
    {
        wa::storage::cloud_queue queue = get_queue();

        utility::string_t policy_name1 = U("policy1");
        utility::string_t policy_name2 = U("policy2");

        uint8_t permission1 = wa::storage::queue_shared_access_policy::permissions::read | wa::storage::queue_shared_access_policy::permissions::add;
        uint8_t permission2 = wa::storage::queue_shared_access_policy::permissions::read | wa::storage::queue_shared_access_policy::permissions::update;

        wa::storage::queue_shared_access_policy policy1(utility::datetime::utc_now(), utility::datetime::utc_now(), permission1);
        wa::storage::queue_shared_access_policy policy2(utility::datetime::utc_now(), utility::datetime::utc_now(), permission2);

        {
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            wa::storage::queue_permissions permissions = queue.download_permissions(options, context);

            CHECK(permissions.policies().empty());
        }

        {
            wa::storage::queue_permissions permissions;
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            wa::storage::shared_access_policies<wa::storage::queue_shared_access_policy> policies;
            policies.insert(std::pair<utility::string_t, wa::storage::queue_shared_access_policy>(policy_name1, policy1));
            policies.insert(std::pair<utility::string_t, wa::storage::queue_shared_access_policy>(policy_name2, policy2));

            permissions.set_policies(policies);
            queue.upload_permissions(permissions, options, context);
        }

        {
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            wa::storage::queue_permissions permissions = queue.download_permissions(options, context);

            CHECK_EQUAL(2U, permissions.policies().size());
            CHECK_EQUAL(permission1, permissions.policies()[policy_name1].permission());
            CHECK(permissions.policies()[policy_name1].start().is_initialized());
            CHECK(permissions.policies()[policy_name1].expiry().is_initialized());
            CHECK_EQUAL(permission2, permissions.policies()[policy_name2].permission());
            CHECK(permissions.policies()[policy_name2].start().is_initialized());
            CHECK(permissions.policies()[policy_name2].expiry().is_initialized());
        }

        {
            wa::storage::queue_permissions permissions;
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            wa::storage::shared_access_policies<wa::storage::queue_shared_access_policy> policies;

            permissions.set_policies(policies);
            queue.upload_permissions(permissions, options, context);
        }

        {
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;

            wa::storage::queue_permissions permissions = queue.download_permissions(options, context);

            CHECK(permissions.policies().empty());
        }
    }
}
