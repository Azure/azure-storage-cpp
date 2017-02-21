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

#include <was/storage_account.h>
#include <was/table.h>

namespace azure { namespace storage { namespace samples {

    void tables_getting_started_sample()
    {
        try
        {
            // Initialize storage account
            azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(storage_connection_string);

            // Create a table
            azure::storage::cloud_table_client table_client = storage_account.create_cloud_table_client();
            azure::storage::cloud_table table = table_client.get_table_reference(_XPLATSTR("MySampleTable"));
            
            // Return value is true if the table did not exist and was successfully created.
            table.create_if_not_exists();

            // Insert a table entity
            azure::storage::table_entity entity(_XPLATSTR("MyPartitionKey"), _XPLATSTR("MyRowKey"));
            azure::storage::table_entity::properties_type& properties = entity.properties();
            properties.reserve(8);
            properties[_XPLATSTR("StringProperty")] = azure::storage::entity_property(utility::string_t(_XPLATSTR("some string")));
            properties[_XPLATSTR("DateTimeProperty")] = azure::storage::entity_property(utility::datetime::utc_now());
            properties[_XPLATSTR("GuidProperty")] = azure::storage::entity_property(utility::new_uuid());
            properties[_XPLATSTR("Int32Property")] = azure::storage::entity_property(1234567890);
            properties[_XPLATSTR("Int64Property")] = azure::storage::entity_property((int64_t) 1234567890123456789LL);
            properties[_XPLATSTR("DoubleProperty")] = azure::storage::entity_property(9.1234567890123456789);
            properties[_XPLATSTR("BooleanProperty")] = azure::storage::entity_property(true);
            properties[_XPLATSTR("BinaryProperty")] = azure::storage::entity_property(std::vector<uint8_t>(10, 'X'));
            azure::storage::table_operation insert_operation = azure::storage::table_operation::insert_entity(entity);
            azure::storage::table_result insert_result = table.execute(insert_operation);

            // Retrieve a table entity
            azure::storage::table_operation retrieve_operation = azure::storage::table_operation::retrieve_entity(_XPLATSTR("MyPartitionKey"), _XPLATSTR("MyRowKey"));
            azure::storage::table_result retrieve_result = table.execute(retrieve_operation);

            // Insert table entities in a batch
            // A batch can contain a single retrieve operation or any mix of other types of operations up to 100 operations.
            // All the operations in a batch must be for entities with the same Partition Key and different Row Keys.
            azure::storage::table_batch_operation batch_operation;
            for (int i = 0; i < 10; ++i)
            {
                utility::string_t row_key = _XPLATSTR("MyRowKey") + utility::conversions::print_string(i);
                azure::storage::table_entity entity2(_XPLATSTR("MyPartitionKey"), row_key);
                azure::storage::table_entity::properties_type& properties2 = entity2.properties();
                properties2.reserve(3);
                properties2[_XPLATSTR("StringProperty")] = azure::storage::entity_property(utility::string_t(_XPLATSTR("another string")));
                properties2[_XPLATSTR("DateTimeProperty")] = azure::storage::entity_property(utility::datetime::utc_now());
                properties2[_XPLATSTR("GuidProperty")] = azure::storage::entity_property(utility::new_uuid());
                batch_operation.insert_entity(entity2);
            }
            std::vector<azure::storage::table_result> results = table.execute_batch(batch_operation);

            // Query for some table entities
            azure::storage::table_query query;
            query.set_filter_string(azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PartitionKey"), azure::storage::query_comparison_operator::equal, _XPLATSTR("MyPartitionKey")), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("RowKey"), azure::storage::query_comparison_operator::greater_than_or_equal, _XPLATSTR("MyRowKey5"))));
            azure::storage::continuation_token token;
            do
            {
                azure::storage::table_query_segment segment = table.execute_query_segmented(query, token);
                for (std::vector<azure::storage::table_entity>::const_iterator it = segment.results().cbegin(); it != segment.results().cend(); ++it)
                {
                    const azure::storage::table_entity::properties_type& properties = it->properties();
                    ucout << _XPLATSTR("PK: ") << it->partition_key() << _XPLATSTR(", RK: ") << it->row_key() << _XPLATSTR(", Prop: ") << utility::uuid_to_string(properties.at(_XPLATSTR("GuidProperty")).guid_value()) << std::endl;
                }

                token = segment.continuation_token();
            } while (!token.empty());

            // Delete the table entity
            azure::storage::table_operation delete_operation = azure::storage::table_operation::delete_entity(retrieve_result.entity());
            azure::storage::table_result delete_result = table.execute(delete_operation);

            // Delete the table
            // Return value is true if the table did exist and was successfully deleted.
            table.delete_table_if_exists();
        }
        catch (const azure::storage::storage_exception& e)
        {
            ucout << _XPLATSTR("Error: ") << e.what() << std::endl;

            azure::storage::request_result result = e.result();
            azure::storage::storage_extended_error extended_error = result.extended_error();
            if (!extended_error.message().empty())
            {
                ucout << extended_error.message() << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            ucout << _XPLATSTR("Error: ") << e.what() << std::endl;
        }
    }

}}} // namespace azure::storage::samples

int main(int argc, const char *argv[])
{
    azure::storage::samples::tables_getting_started_sample();
    return 0;
}

