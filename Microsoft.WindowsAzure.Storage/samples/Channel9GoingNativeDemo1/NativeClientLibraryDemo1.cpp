// NativeClientLibraryDemo1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "was/storage_account.h"
#include "was/table.h"
#include "was/queue.h"

int _tmain(int argc, _TCHAR* argv[])
{
    try
    {
        azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(U("DefaultEndpointsProtocol=https;AccountName=ACCOUNT_NAME;AccountKey=ACCOUNT_KEY"));

        azure::storage::cloud_table_client table_client = storage_account.create_cloud_table_client();
        azure::storage::cloud_table table = table_client.get_table_reference(U("blogposts"));
        table.create_if_not_exists();

        azure::storage::cloud_queue_client queue_client = storage_account.create_cloud_queue_client();
        azure::storage::cloud_queue queue = queue_client.get_queue_reference(U("blog-processing"));
        queue.create_if_not_exists();

        while (true)
        {
            utility::string_t name;
            ucin >> name;


            // Table

            azure::storage::table_batch_operation batch_operation;
            for (int i = 0; i < 3; ++i)
            {
                utility::string_t partition_key = U("partition");
                utility::string_t row_key = name + utility::conversions::print_string(i);

                azure::storage::table_entity entity(partition_key, row_key);
                entity.properties()[U("PostId")] = azure::storage::entity_property(rand());
                entity.properties()[U("Content")] = azure::storage::entity_property(utility::string_t(U("some text")));
                entity.properties()[U("Date")] = azure::storage::entity_property(utility::datetime::utc_now());
                batch_operation.insert_entity(entity);
            }

            pplx::task<void> table_task = table.execute_batch_async(batch_operation).then([](std::vector<azure::storage::table_result> results)
            {
                for (auto it = results.cbegin(); it != results.cend(); ++it)
                {
                    ucout << U("Status: ") << it->http_status_code() << std::endl;
                }
            });


            // Queue

            azure::storage::cloud_queue_message queue_message(name);
            std::chrono::seconds time_to_live(100000);
            std::chrono::seconds initial_visibility_timeout(rand() % 30);

            pplx::task<void> queue_task = queue.add_message_async(queue_message, time_to_live, initial_visibility_timeout, azure::storage::queue_request_options(), azure::storage::operation_context());


            queue_task.wait();
            table_task.wait();
        }
    }
    catch (const azure::storage::storage_exception& e)
    {
        ucout << e.what() << std::endl;
    }
}

