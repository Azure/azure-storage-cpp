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
#include "was/table.h"

namespace wa { namespace storage { namespace samples {

    void tables_getting_started_sample()
    {
        try
        {
            // Initialize storage account
            wa::storage::cloud_storage_account storage_account = wa::storage::cloud_storage_account::parse(storage_connection_string);

            // Create a table
            wa::storage::cloud_table_client table_client = storage_account.create_cloud_table_client();
            wa::storage::cloud_table table = table_client.get_table_reference(U("AzureNativeClientLibrarySampleTable"));
            bool created = table.create_if_not_exists();

            // Insert a table entity
            wa::storage::table_entity entity(U("MyPartitionKey"), U("MyRowKey"));
            entity.properties().reserve(8);
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyA"), wa::storage::entity_property(utility::string_t(U("some string")))));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyB"), wa::storage::entity_property(utility::datetime::utc_now())));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(utility::new_uuid())));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(1234567890)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(1234567890123456789LL)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyF"), wa::storage::entity_property(9.1234567890123456789)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyG"), wa::storage::entity_property(true)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyH"), wa::storage::entity_property(std::vector<uint8_t>(10, 'X'))));
            wa::storage::table_operation operation1 = wa::storage::table_operation::insert_or_replace_entity(entity);
            wa::storage::table_result insert_result = table.execute(operation1);

            // Retrieve a table entity
            wa::storage::table_operation operation2 = wa::storage::table_operation::retrieve_entity(U("MyPartitionKey"), U("MyRowKey"));
            wa::storage::table_result retrieve_result = table.execute(operation2);

            // Insert table entities in a batch
            wa::storage::table_batch_operation operation3;
            for (int i = 0; i < 10; ++i)
            {
                utility::string_t row_key = U("MyRowKey") + utility::conversions::print_string(i);
                wa::storage::table_entity entity2(U("MyPartitionKey"), row_key);
                entity2.properties().reserve(3);
                entity2.properties().insert(wa::storage::table_entity::property_type(U("PropertyA"), wa::storage::entity_property(utility::string_t(U("another string")))));
                entity2.properties().insert(wa::storage::table_entity::property_type(U("PropertyB"), wa::storage::entity_property(utility::datetime::utc_now())));
                entity2.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(utility::new_uuid())));
                operation3.insert_or_replace_entity(entity2);
            }
            std::vector<wa::storage::table_result> results = table.execute_batch(operation3);

            // Query for some table entities
            wa::storage::table_query query;
            query.set_filter_string(wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::generate_filter_condition(U("PartitionKey"), wa::storage::query_comparison_operator::equal, U("MyPartitionKey")), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("RowKey"), wa::storage::query_comparison_operator::greater_than_or_equal, U("MyRowKey5"))));
            std::vector<wa::storage::table_entity> entities = table.execute_query(query);
            for (std::vector<wa::storage::table_entity>::const_iterator itr = entities.cbegin(); itr != entities.cend(); ++itr)
            {
                wa::storage::table_entity::properties_type properties = itr->properties();
                ucout << U("PK: ") << itr->partition_key() << U(", RK: ") << itr->row_key() << U(", Prop: ") << utility::uuid_to_string(properties[U("PropertyC")].guid_value()) << std::endl;
            }

            // Delete a table entity
            wa::storage::table_operation operation4 = wa::storage::table_operation::delete_entity(retrieve_result.entity());
            wa::storage::table_result delete_result = table.execute(operation4);

            // Delete the table
            bool deleted = table.delete_table_if_exists();
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
    wa::storage::samples::tables_getting_started_sample();
    return 0;
}

