// -----------------------------------------------------------------------------------------
// <copyright file="protocol_json.cpp" company="Microsoft">
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
#include "wascore/protocol.h"
#include "wascore/protocol_json.h"

namespace azure { namespace storage { namespace protocol {

    edm_type get_property_type(const utility::string_t& type_name)
    {
        if (type_name.compare(U("Edm.Binary")) == 0)
        {
            return edm_type::binary;
        }
        else if (type_name.compare(U("Edm.Boolean")) == 0)
        {
            return edm_type::boolean;
        }
        else if (type_name.compare(U("Edm.DateTime")) == 0)
        {
            return edm_type::datetime;
        }
        else if (type_name.compare(U("Edm.Double")) == 0)
        {
            return edm_type::double_floating_point;
        }
        else if (type_name.compare(U("Edm.Guid")) == 0)
        {
            return edm_type::guid;
        }
        else if (type_name.compare(U("Edm.Int32")) == 0)
        {
            return edm_type::int32;
        }
        else if (type_name.compare(U("Edm.Int64")) == 0)
        {
            return edm_type::int64;
        }
        else
        {
            return edm_type::string;
        }
    }

    table_entity parse_table_entity(const web::json::value& document)
    {
        table_entity entity;

        // TODO: Choose a smart estimate of the number of properties to avoid rehashes
        /*
        if (obj.size() > 3)
        {
        entity.properties().reserve((obj.size() - 2U) / 2U);
        }
        */

        if (document.is_object())
        {
            const web::json::object& entity_obj = document.as_object();

            for (web::json::object::const_iterator it = entity_obj.cbegin(); it != entity_obj.cend(); ++it)
            {
                const utility::string_t& property_name = it->first;
                const web::json::value& property_obj = it->second;

                // The property name must be at least 6 characters long in order to have the "odata." prefix and have a non-empty suffix
                if (property_name.size() >= 6 && property_name.compare(0, 6, U("odata.")) == 0)
                {
                    // The object is a special OData value

                    // TODO: if needed use: odata.type, odata.id, odata.editlink

                    if (property_name.compare(6, property_name.size() - 6, U("etag")) == 0)
                    {
                        // The object is the ETag
                        if (property_obj.is_string() && entity.etag().empty())
                        {
                            entity.set_etag(property_obj.as_string());
                        }
                    }
                }
                // The property name must be at least 11 characters long in order to have the "@odata.type" suffix
                else if (property_name.size() < 11 || property_name.compare(property_name.size() - 11, 11, U("@odata.type")) != 0)
                {
                    // The object is not the type of a property

                    if (property_name.compare(U("PartitionKey")) == 0)
                    {
                        // The object is the Partition Key

                        if (property_obj.is_string() && entity.partition_key().empty())
                        {
                            entity.set_partition_key(property_obj.as_string());
                        }
                    }
                    else if (property_name.compare(U("RowKey")) == 0)
                    {
                        // The object is the Row Key

                        if (property_obj.is_string() && entity.row_key().empty())
                        {
                            entity.set_row_key(property_obj.as_string());
                        }
                    }
                    else if (property_name.compare(U("Timestamp")) == 0)
                    {
                        // The object is the Timestamp

                        if (property_obj.is_string() && !entity.timestamp().is_initialized())
                        {
                            utility::datetime timestamp = utility::datetime::from_string(property_obj.as_string(), utility::datetime::ISO_8601);
                            entity.set_timestamp(timestamp);
                        }
                    }
                    else
                    {
                        // The object is a regular property

                        entity_property entity_property;

                        // The type is set to String for consistency unless a specific EDM type was specified
                        if (property_obj.is_boolean())
                        {
                            entity_property.set_value(property_obj.as_bool());
                        }
                        else if (property_obj.is_integer())
                        {
                            entity_property.set_value(property_obj.as_integer());
                        }
                        else if (property_obj.is_double())
                        {
                            entity_property.set_value(property_obj.as_double());
                        }
                        else if (property_obj.is_string())
                        {
                            entity_property.set_value(property_obj.as_string());

                            utility::string_t property_type_key;
                            property_type_key.reserve(property_name.size() + 11);
                            property_type_key.append(property_name);
                            property_type_key.append(U("@odata.type"));

                            web::json::object::const_iterator property_type_it = entity_obj.find(property_type_key);
                            if (property_type_it != entity_obj.cend())
                            {
                                const web::json::value& property_type_obj = property_type_it->second;
                                if (property_type_obj.is_string())
                                {
                                    utility::string_t property_type_name = property_type_obj.as_string();
                                    edm_type property_type = get_property_type(property_type_name);
                                    entity_property.set_property_type(property_type);
                                }
                            }
                        }

                        entity.properties().insert(table_entity::property_type(property_name, std::move(entity_property)));
                    }
                }
            }
        }

        // TODO: Generate the ETag from the Timestamp if it was not in the response header or the response body

        return entity;
    }

    storage_extended_error parse_table_error(const web::json::value& document)
    {
        utility::string_t error_code;
        utility::string_t error_message;
        std::unordered_map<utility::string_t, utility::string_t> details;

        if (document.is_object())
        {
            const web::json::object& result_obj = document.as_object();

            web::json::object::const_iterator error_it = result_obj.find(U("odata.error"));
            if (error_it != result_obj.cend() && error_it->second.is_object())
            {
                const web::json::object& error_obj = error_it->second.as_object();
                for (auto prop_it = error_obj.cbegin(); prop_it != error_obj.cend(); ++prop_it)
                {
                    auto prop_name = prop_it->first;
                    if (prop_name == U("code") && prop_it->second.is_string())
                    {
                        error_code = prop_it->second.as_string();
                    }
                    else if (prop_name == U("message") && prop_it->second.is_object())
                    {
                        const web::json::object& message_obj = prop_it->second.as_object();

                        web::json::object::const_iterator message_text_it = message_obj.find(U("value"));
                        if (message_text_it != message_obj.cend() && message_text_it->second.is_string())
                        {
                            error_message = message_text_it->second.as_string();
                        }
                    }
                    else if (prop_name == U("innererror"))
                    {
                        const web::json::object& inner_error_obj = prop_it->second.as_object();
                        for (auto details_it = inner_error_obj.cbegin(); details_it != inner_error_obj.cend(); ++details_it)
                        {
                            if (details_it->second.is_string())
                            {
                                details.insert(std::make_pair(details_it->first, details_it->second.as_string()));
                            }
                        }
                    }
                    else if (prop_name.find_first_of(U('.')) != utility::string_t::npos)
                    {
                        // annotation property
                        // TODO: parse annotation property and add it to details
                    }
                }
            }
        }

        return storage_extended_error(std::move(error_code), std::move(error_message), std::move(details));
    }

}}} // namespace azure::storage::protocol
