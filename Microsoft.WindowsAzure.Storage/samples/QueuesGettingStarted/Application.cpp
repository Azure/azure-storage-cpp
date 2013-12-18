// -----------------------------------------------------------------------------------------
// <copyright file="Application.cpp" company="Microsoft">
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
#include "samples_common.h"

#include "was/storage_account.h"
#include "was/queue.h"

namespace wa { namespace storage { namespace samples {

    void queues_getting_started_sample()
    {
        try
        {
            // Initialize storage account
            wa::storage::cloud_storage_account storage_account = wa::storage::cloud_storage_account::parse(storage_connection_string);

            // Create a queue
            wa::storage::cloud_queue_client queue_client = storage_account.create_cloud_queue_client();
            wa::storage::cloud_queue queue = queue_client.get_queue_reference(U("azure-native-client-library-sample-queue"));
            bool created = queue.create_if_not_exists();

            // Insert some queue messages
            wa::storage::cloud_queue_message message1(U("some message"));
            queue.add_message(message1);
            wa::storage::cloud_queue_message message2(U("different message"));
            queue.add_message(message2);
            wa::storage::cloud_queue_message message3(U("another message"));
            queue.add_message(message3);

            // Peek the next queue message
            wa::storage::cloud_queue_message message4 = queue.peek_message();

            // Dequeue the next queue message
            wa::storage::cloud_queue_message message5 = queue.get_message();

            // Update a queue message (content and visibility timeout)
            message5.set_content(U("changed message"));
            queue.update_message(message5, std::chrono::seconds(30), true);

            // Delete a queue message
            queue.delete_message(message5);

            // Dequeue some queue messages (maximum 32 at a time) and set their visibility timeout to 5 minutes
            wa::storage::queue_request_options options;
            wa::storage::operation_context context;
            std::vector<wa::storage::cloud_queue_message> messages = queue.get_messages(32, std::chrono::seconds(300), options, context);

            // Get the approximate queue size
            queue.download_attributes();
            int message_count = queue.approximate_message_count();

            // Delete the queue
            bool deleted = queue.delete_queue_if_exists();
        }
        catch (wa::storage::storage_exception& e)
        {
            ucout << U("Error: ") << e.what() << std::endl;

            wa::storage::request_result result = e.result();
            wa::storage::storage_extended_error extended_error = result.extended_error();
            if (!extended_error.message().empty())
            {
                ucout << extended_error.message() << std::endl;
            }
        }
        catch (std::exception& e)
        {
            ucout << U("Error: ") << e.what() << std::endl;
        }
    }

}}} // namespace wa::storage::samples

int _tmain(int argc, _TCHAR *argv[])
{
    wa::storage::samples::queues_getting_started_sample();
    return 0;
}

