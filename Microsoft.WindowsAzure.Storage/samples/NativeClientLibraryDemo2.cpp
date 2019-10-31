// -----------------------------------------------------------------------------------------
// <copyright file="NativeClientLibraryDemo2.cpp" company="Microsoft">
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

#include <chrono>
#include <thread>

#include <was/storage_account.h>
#include <was/table.h>
#include <was/queue.h>


namespace azure { namespace storage { namespace samples {

    SAMPLE(Channel9GoingNativeDemo2, channel9_going_native_demo2)
    void channel9_going_native_demo2() {
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
                azure::storage::cloud_queue_message message = queue.get_message();
                if (!message.id().empty())
                {
                    utility::string_t partition_key(_XPLATSTR("partition"));
                    utility::string_t start_row_key = message.content_as_string();
                    utility::string_t end_row_key = message.content_as_string() + _XPLATSTR(":");

                    azure::storage::table_query query;
                    query.set_filter_string(azure::storage::table_query::combine_filter_conditions(
                        azure::storage::table_query::combine_filter_conditions(
                            azure::storage::table_query::generate_filter_condition(_XPLATSTR("PartitionKey"), azure::storage::query_comparison_operator::equal, partition_key),
                            azure::storage::query_logical_operator::op_and,
                            azure::storage::table_query::generate_filter_condition(_XPLATSTR("RowKey"), azure::storage::query_comparison_operator::greater_than, start_row_key)),
                        azure::storage::query_logical_operator::op_and,
                        azure::storage::table_query::generate_filter_condition(_XPLATSTR("RowKey"), azure::storage::query_comparison_operator::less_than, end_row_key))
                    );

                    azure::storage::table_query_iterator it = table.execute_query(query);
                    azure::storage::table_query_iterator end_of_results;
                    for (; it != end_of_results; ++it)
                    {
                        ucout << _XPLATSTR("Entity: ") << it->row_key() << _XPLATSTR(" ");
                        ucout << it->properties().at(_XPLATSTR("PostId")).int32_value() << _XPLATSTR(" ");
                        ucout << it->properties().at(_XPLATSTR("Content")).string_value() << std::endl;
                    }

                    queue.delete_message(message);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }

            table.delete_table_if_exists();
            queue.delete_queue_if_exists();
        }
        catch (const azure::storage::storage_exception& e)
        {
            ucout << e.what() << std::endl;
        }
    }
}}}  // namespace azure::storage::samples
