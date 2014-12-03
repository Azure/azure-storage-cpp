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

namespace azure { namespace storage { namespace samples {

    void queues_getting_started_sample()
    {
        try
        {
            // Initialize storage account
            azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(storage_connection_string);

            // Create a queue
            azure::storage::cloud_queue_client queue_client = storage_account.create_cloud_queue_client();
            azure::storage::cloud_queue queue = queue_client.get_queue_reference(U("my-sample-queue"));
            
            // Return value is true if the queue did not exist and was successfully created.
            queue.create_if_not_exists();

            // Insert some queue messages
            azure::storage::cloud_queue_message message1(U("some message"));
            queue.add_message(message1);
            azure::storage::cloud_queue_message message2(U("different message"));
            queue.add_message(message2);
            azure::storage::cloud_queue_message message3(U("another message"));
            queue.add_message(message3);

            // Peek the next queue message
            azure::storage::cloud_queue_message peeked_message = queue.peek_message();
            ucout << U("Peek: ") << peeked_message.content_as_string() << std::endl;

            // Dequeue the next queue message
            azure::storage::cloud_queue_message dequeued_message = queue.get_message();
            ucout << U("Get: ") << dequeued_message.content_as_string() << std::endl;

            // Update a queue message (content and visibility timeout)
            dequeued_message.set_content(U("changed message"));
            queue.update_message(dequeued_message, std::chrono::seconds(30), true);
            ucout << U("Update: ") << dequeued_message.content_as_string() << std::endl;

            // Delete the queue message
            queue.delete_message(dequeued_message);

            // Dequeue some queue messages (maximum 32 at a time) and set their visibility timeout to 5 minutes
            azure::storage::queue_request_options options;
            azure::storage::operation_context context;
            std::vector<azure::storage::cloud_queue_message> messages = queue.get_messages(32, std::chrono::seconds(300), options, context);
            for (std::vector<azure::storage::cloud_queue_message>::const_iterator it = messages.cbegin(); it != messages.cend(); ++it)
            {
                ucout << U("Get: ") << it->content_as_string() << std::endl;
            }

            // Delete the queue messages
            for (std::vector<azure::storage::cloud_queue_message>::iterator it = messages.begin(); it != messages.end(); ++it)
            {
                queue.delete_message(*it);
            }

            // Get the approximate queue size
            queue.download_attributes();
            ucout << U("Approximate message count: ") << queue.approximate_message_count() << std::endl;

            // Delete the queue
            // Return value is true if the queue did exist and was succesfully deleted.
            queue.delete_queue_if_exists();
        }
        catch (const azure::storage::storage_exception& e)
        {
            ucout << U("Error: ") << e.what() << std::endl;

            azure::storage::request_result result = e.result();
            azure::storage::storage_extended_error extended_error = result.extended_error();
            if (!extended_error.message().empty())
            {
                ucout << extended_error.message() << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            ucout << U("Error: ") << e.what() << std::endl;
        }
    }

}}} // namespace azure::storage::samples

int main(int argc, const char *argv[])
{
    azure::storage::samples::queues_getting_started_sample();
    return 0;
}

