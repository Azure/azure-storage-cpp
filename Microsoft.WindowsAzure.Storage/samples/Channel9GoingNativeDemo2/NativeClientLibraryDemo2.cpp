// NativeClientLibraryDemo2.cpp : Defines the entry point for the console application.
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
            azure::storage::cloud_queue_message message = queue.get_message();
            if (!message.id().empty())
            {
                utility::string_t partition_key(U("partition"));
                utility::string_t start_row_key = message.content_as_string();
                utility::string_t end_row_key = message.content_as_string() + U(":");

                azure::storage::table_query query;
                query.set_filter_string(azure::storage::table_query::combine_filter_conditions(
                    azure::storage::table_query::combine_filter_conditions(
                    azure::storage::table_query::generate_filter_condition(U("PartitionKey"), azure::storage::query_comparison_operator::equal, partition_key),
                    azure::storage::query_logical_operator::op_and,
                    azure::storage::table_query::generate_filter_condition(U("RowKey"), azure::storage::query_comparison_operator::greater_than, start_row_key)),
                    azure::storage::query_logical_operator::op_and,
                    azure::storage::table_query::generate_filter_condition(U("RowKey"), azure::storage::query_comparison_operator::less_than, end_row_key))
                    );

                azure::storage::table_query_iterator it = table.execute_query(query);
                azure::storage::table_query_iterator end_of_results;
                for (; it != end_of_results; ++it)
                {
                    ucout << U("Entity: ") << it->row_key() << U(" ");
                    ucout << it->properties().at(U("PostId")).int32_value() << U(" ");
                    ucout << it->properties().at(U("Content")).string_value() << std::endl;
                }

                queue.delete_message(message);
            }

            Sleep(1000);
        }

        table.delete_table_if_exists();
        queue.delete_queue_if_exists();
    }
    catch (const azure::storage::storage_exception& e)
    {
        ucout << e.what() << std::endl;
    }
}

