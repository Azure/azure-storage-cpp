// -----------------------------------------------------------------------------------------
// <copyright file="NativeClientLibraryDemo1.cpp" company="Microsoft">
//    Copyright 2019 Microsoft Corporation
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

#include "samples_common.h"

#include <was/storage_account.h>
#include <was/table.h>
#include <was/queue.h>


namespace azure { namespace storage { namespace samples {

    SAMPLE(Channel9GoingNativeDemo1, channel9_going_native_demo1)
    void channel9_going_native_demo1() {
        try
        {
            azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(storage_connection_string);

            azure::storage::cloud_table_client table_client = storage_account.create_cloud_table_client();
            azure::storage::cloud_table table = table_client.get_table_reference(_XPLATSTR("blogposts"));
            table.create_if_not_exists();

            azure::storage::cloud_queue_client queue_client = storage_account.create_cloud_queue_client();
            azure::storage::cloud_queue queue = queue_client.get_queue_reference(_XPLATSTR("blog-processing"));
            queue.create_if_not_exists();

            while (true)
            {
                utility::string_t name;
                ucin >> name;


                // Table

                azure::storage::table_batch_operation batch_operation;
                for (int i = 0; i < 3; ++i)
                {
                    utility::string_t partition_key = _XPLATSTR("partition");
                    utility::string_t row_key = name + utility::conversions::to_string_t(std::to_string(i));

                    azure::storage::table_entity entity(partition_key, row_key);
                    entity.properties()[_XPLATSTR("PostId")] = azure::storage::entity_property(rand());
                    entity.properties()[_XPLATSTR("Content")] = azure::storage::entity_property(utility::string_t(_XPLATSTR("some text")));
                    entity.properties()[_XPLATSTR("Date")] = azure::storage::entity_property(utility::datetime::utc_now());
                    batch_operation.insert_entity(entity);
                }

                pplx::task<void> table_task = table.execute_batch_async(batch_operation).then([](std::vector<azure::storage::table_result> results)
                {
                    for (auto it = results.cbegin(); it != results.cend(); ++it)
                    {
                        ucout << _XPLATSTR("Status: ") << it->http_status_code() << std::endl;
                    }
                });


                // Queue

                azure::storage::cloud_queue_message queue_message(name);
                std::chrono::seconds time_to_live(100000);
                std::chrono::seconds initial_visibility_timeout(rand() % 30);
                azure::storage::queue_request_options options;

                pplx::task<void> queue_task = queue.add_message_async(queue_message, time_to_live, initial_visibility_timeout, options, azure::storage::operation_context());


                queue_task.wait();
                table_task.wait();
            }
        }
        catch (const azure::storage::storage_exception& e)
        {
            ucout << e.what() << std::endl;
        }
    }
}}}  // namespace azure::storage::samples
