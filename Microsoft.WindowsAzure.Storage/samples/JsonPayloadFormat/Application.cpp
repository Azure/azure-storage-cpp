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

            // Insert some table entities
            wa::storage::table_batch_operation batch_operation;
            for (int i = 0; i < 10; ++i)
            {
                utility::string_t row_key = U("MyRowKey") + utility::conversions::print_string(i);
                wa::storage::table_entity entity(U("MyPartitionKey"), row_key);
                entity.properties().reserve(8);
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyA"), wa::storage::entity_property(utility::string_t(U("some string")))));
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyB"), wa::storage::entity_property(utility::datetime::utc_now())));
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(utility::new_uuid())));
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(1234567890)));
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(1234567890123456789LL)));
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyF"), wa::storage::entity_property(9.1234567890123456789)));
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyG"), wa::storage::entity_property(true)));
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyH"), wa::storage::entity_property(std::vector<uint8_t>(10, 'X'))));
                batch_operation.insert_or_replace_entity(entity);
            }
            std::vector<wa::storage::table_result> results = table.execute_batch(batch_operation);

            // Set the payload format to reduce the size of the network payload, however, some property types cannot be automatically inferred and need to be set explicitly.
			// For more information about the support for JSON, check the following document: http://blogs.msdn.com/b/windowsazurestorage/archive/2013/12/05/windows-azure-tables-introducing-json.aspx.
            wa::storage::table_request_options options;
            options.set_payload_format(wa::storage::table_payload_format::json_no_metadata);

            // Query for the table entities
            wa::storage::table_query query;
            wa::storage::operation_context context;
            std::vector<wa::storage::table_entity> entities = table.execute_query(query, options, context);
            for (std::vector<wa::storage::table_entity>::const_iterator itr = entities.cbegin(); itr != entities.cend(); ++itr)
            {
                wa::storage::table_entity::properties_type properties = itr->properties();

                // Explictly set the property types for datetime, guid, int64, and binary properties because these cannot be automatically inferred when the "no metadata" option is used
                properties[U("PropertyB")].set_property_type(wa::storage::edm_type::datetime);
                properties[U("PropertyC")].set_property_type(wa::storage::edm_type::guid);
                properties[U("PropertyE")].set_property_type(wa::storage::edm_type::int64);
                properties[U("PropertyH")].set_property_type(wa::storage::edm_type::binary);

                // Print all property values
                ucout << U("PK: ") << itr->partition_key() << U(", RK: ") << itr->row_key() << U(", TS: ") << itr->timestamp().to_string(utility::datetime::ISO_8601) << std::endl;
                ucout << U("PropertyA: ") << properties[U("PropertyA")].string_value() << std::endl;
                ucout << U("PropertyB: ") << properties[U("PropertyB")].datetime_value().to_string(utility::datetime::ISO_8601) << std::endl;
                ucout << U("PropertyC: ") << utility::uuid_to_string(properties[U("PropertyC")].guid_value()) << std::endl;
                ucout << U("PropertyD: ") << properties[U("PropertyD")].int32_value() << std::endl;
                ucout << U("PropertyE: ") << properties[U("PropertyE")].int64_value() << std::endl;
                ucout << U("PropertyF: ") << properties[U("PropertyF")].double_value() << std::endl;
                ucout << U("PropertyG: ") << properties[U("PropertyG")].boolean_value() << std::endl;
                ucout << U("PropertyH: ") << utility::conversions::to_base64(properties[U("PropertyH")].binary_value()) << std::endl;
            }

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

