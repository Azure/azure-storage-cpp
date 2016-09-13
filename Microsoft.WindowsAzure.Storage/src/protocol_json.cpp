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
        if (type_name.compare(_XPLATSTR("Edm.Binary")) == 0)
        {
            return edm_type::binary;
        }
        else if (type_name.compare(_XPLATSTR("Edm.Boolean")) == 0)
        {
            return edm_type::boolean;
        }
        else if (type_name.compare(_XPLATSTR("Edm.DateTime")) == 0)
        {
            return edm_type::datetime;
        }
        else if (type_name.compare(_XPLATSTR("Edm.Double")) == 0)
        {
            return edm_type::double_floating_point;
        }
        else if (type_name.compare(_XPLATSTR("Edm.Guid")) == 0)
        {
            return edm_type::guid;
        }
        else if (type_name.compare(_XPLATSTR("Edm.Int32")) == 0)
        {
            return edm_type::int32;
        }
        else if (type_name.compare(_XPLATSTR("Edm.Int64")) == 0)
        {
            return edm_type::int64;
        }
        else
        {
            return edm_type::string;
        }
    }

    utility::string_t get_etag_from_timestamp(const utility::string_t& timestampStr)
    {
        utility::string_t value;
        value.append(_XPLATSTR("W/\"datetime'"));
        value.append(web::http::uri::encode_data_string(timestampStr));
        value.append(_XPLATSTR("'\""));
        return value;
    }

    table_entity parse_table_entity(const web::json::value& document)
    {
        table_entity entity;

        if (document.is_object())
        {
            const web::json::object& entity_obj = document.as_object();
            utility::string_t timestamp_str;

            for (web::json::object::const_iterator it = entity_obj.cbegin(); it != entity_obj.cend(); ++it)
            {
                const utility::string_t& property_name = it->first;
                const web::json::value& property_obj = it->second;

                // The property name must be at least 6 characters long in order to have the "odata." prefix and have a non-empty suffix
                if (property_name.size() >= 6 && property_name.compare(0, 6, _XPLATSTR("odata.")) == 0)
                {
                    // The object is a special OData value

                    // TODO: if needed use: odata.type, odata.id, odata.editlink

                    if (property_name.compare(6, property_name.size() - 6, _XPLATSTR("etag")) == 0)
                    {
                        // The object is the ETag
                        if (property_obj.is_string() && entity.etag().empty())
                        {
                            entity.set_etag(property_obj.as_string());
                        }
                    }
                }
                // The property name must be at least 11 characters long in order to have the "@odata.type" suffix
                else if (property_name.size() < 11 || property_name.compare(property_name.size() - 11, 11, _XPLATSTR("@odata.type")) != 0)
                {
                    // The object is not the type of a property

                    if (property_name.compare(_XPLATSTR("PartitionKey")) == 0)
                    {
                        // The object is the Partition Key

                        if (property_obj.is_string() && entity.partition_key().empty())
                        {
                            entity.set_partition_key(property_obj.as_string());
                        }
                    }
                    else if (property_name.compare(_XPLATSTR("RowKey")) == 0)
                    {
                        // The object is the Row Key

                        if (property_obj.is_string() && entity.row_key().empty())
                        {
                            entity.set_row_key(property_obj.as_string());
                        }
                    }
                    else if (property_name.compare(_XPLATSTR("Timestamp")) == 0)
                    {
                        // The object is the Timestamp

                        if (property_obj.is_string())
                        {
                            timestamp_str = property_obj.as_string();

                            if (!entity.timestamp().is_initialized())
                            {
                                utility::datetime timestamp = utility::datetime::from_string(timestamp_str, utility::datetime::ISO_8601);
                                entity.set_timestamp(timestamp);
                            }
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
                            property_type_key.append(_XPLATSTR("@odata.type"));

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

            // Generate the ETag from the Timestamp if it was not in the response header or the response body
            if (entity.etag().empty() && !timestamp_str.empty())
            {
                entity.set_etag(get_etag_from_timestamp(timestamp_str));
            }
        }

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

            web::json::object::const_iterator error_it = result_obj.find(_XPLATSTR("odata.error"));
            if (error_it != result_obj.cend() && error_it->second.is_object())
            {
                const web::json::object& error_obj = error_it->second.as_object();
                for (auto prop_it = error_obj.cbegin(); prop_it != error_obj.cend(); ++prop_it)
                {
                    auto prop_name = prop_it->first;
                    if (prop_name == _XPLATSTR("code") && prop_it->second.is_string())
                    {
                        error_code = prop_it->second.as_string();
                    }
                    else if (prop_name == _XPLATSTR("message") && prop_it->second.is_object())
                    {
                        const web::json::object& message_obj = prop_it->second.as_object();

                        web::json::object::const_iterator message_text_it = message_obj.find(_XPLATSTR("value"));
                        if (message_text_it != message_obj.cend() && message_text_it->second.is_string())
                        {
                            error_message = message_text_it->second.as_string();
                        }
                    }
                    else if (prop_name == _XPLATSTR("innererror"))
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
                    else if (prop_name.find_first_of(_XPLATSTR('.')) != utility::string_t::npos)
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
