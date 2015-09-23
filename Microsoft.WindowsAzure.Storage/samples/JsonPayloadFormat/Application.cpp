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

namespace azure { namespace storage { namespace samples {

    utility::string_t get_connection_string()
    {
        // Load configuration
        utility::ifstream_t config_file;
        config_file.open("test_configurations.json");

        web::json::value config;
        config_file >> config;

        auto target_name = config[U("target")].as_string();
        web::json::value& tenants = config[U("tenants")];

        for (web::json::array::const_iterator it = tenants.as_array().cbegin(); it != tenants.as_array().cend(); ++it)
        {
            const web::json::value& name_obj = it->at(U("name"));
            if (name_obj.as_string() == target_name)
            {
                return it->at(U("connection_string")).as_string();
            }
        }
    }

    void tables_getting_started_sample()
    {
        try
        {
            // Initialize storage account
  	    azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(get_connection_string());

            // Create a table
            azure::storage::cloud_table_client table_client = storage_account.create_cloud_table_client();
            azure::storage::cloud_table table = table_client.get_table_reference(U("MySampleTable"));

            // Return value is true if the table did not exist and was successfully created.
            table.create_if_not_exists();

            // Insert some table entities
            azure::storage::table_batch_operation batch_operation;
            for (int i = 0; i < 10; ++i)
            {
                utility::string_t row_key = U("MyRowKey") + utility::conversions::print_string(i);
                azure::storage::table_entity entity(U("MyPartitionKey"), row_key);
                azure::storage::table_entity::properties_type& properties = entity.properties();
                properties.reserve(8);
                properties[U("StringProperty")] = azure::storage::entity_property(utility::string_t(U("some string")));
                properties[U("DateTimeProperty")] = azure::storage::entity_property(utility::datetime::utc_now());
                properties[U("GuidProperty")] = azure::storage::entity_property(utility::new_uuid());
                properties[U("Int32Property")] = azure::storage::entity_property(1234567890);
                properties[U("Int64Property")] = azure::storage::entity_property((int64_t) 1234567890123456789LL);
                properties[U("DoubleProperty")] = azure::storage::entity_property(9.1234567890123456789);
                properties[U("BooleanProperty")] = azure::storage::entity_property(true);
                properties[U("BinaryProperty")] = azure::storage::entity_property(std::vector<uint8_t>(10, 'X'));
                batch_operation.insert_or_replace_entity(entity);
            }
            std::vector<azure::storage::table_result> results = table.execute_batch(batch_operation);

            // Set the payload format to reduce the size of the network payload, however, some property types cannot be automatically inferred and need to be set explicitly.
            // For more information about the support for JSON, check the following document: http://blogs.msdn.com/b/windowsazurestorage/archive/2013/12/05/windows-azure-tables-introducing-json.aspx.
            azure::storage::table_request_options options;
            options.set_payload_format(azure::storage::table_payload_format::json_no_metadata);

            // Query for the table entities
            azure::storage::table_query query;
            azure::storage::operation_context context;
            azure::storage::continuation_token token;
            do
            {
                azure::storage::table_query_segment segment = table.execute_query_segmented(query, token);
                for (std::vector<azure::storage::table_entity>::const_iterator it = segment.results().cbegin(); it != segment.results().cend(); ++it)
                {
                    azure::storage::table_entity::properties_type properties = it->properties();

                    // Explictly set the property types for datetime, guid, int64, and binary properties because these cannot be automatically inferred when the "no metadata" option is used
                    properties.at(U("DateTimeProperty")).set_property_type(azure::storage::edm_type::datetime);
                    properties.at(U("GuidProperty")).set_property_type(azure::storage::edm_type::guid);
                    properties.at(U("Int64Property")).set_property_type(azure::storage::edm_type::int64);
                    properties.at(U("BinaryProperty")).set_property_type(azure::storage::edm_type::binary);

                    // Print all property values
                    ucout << U("PK: ") << it->partition_key() << U(", RK: ") << it->row_key() << U(", TS: ") << it->timestamp().to_string(utility::datetime::ISO_8601) << std::endl;
                    ucout << U("StringProperty:   ") << properties.at(U("StringProperty")).string_value() << std::endl;
                    ucout << U("DateTimeProperty: ") << properties.at(U("DateTimeProperty")).datetime_value().to_string(utility::datetime::ISO_8601) << std::endl;
                    ucout << U("GuidProperty:     ") << utility::uuid_to_string(properties.at(U("GuidProperty")).guid_value()) << std::endl;
                    ucout << U("Int32Property:    ") << properties.at(U("Int32Property")).int32_value() << std::endl;
                    ucout << U("Int64Property:    ") << properties.at(U("Int64Property")).int64_value() << std::endl;
                    ucout << U("DoubleProperty:   ") << properties.at(U("DoubleProperty")).double_value() << std::endl;
                    ucout << U("BooleanProperty:  ") << properties.at(U("BooleanProperty")).boolean_value() << std::endl;
                    ucout << U("BinaryProperty:   ") << utility::conversions::to_base64(properties.at(U("BinaryProperty")).binary_value()) << std::endl;
                }

                token = segment.continuation_token();
            } while (!token.empty());

            // Delete the table
            // Return value is true if the table did exist and was successfully deleted.
            table.delete_table_if_exists();
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
    azure::storage::samples::tables_getting_started_sample();
    return 0;
}

