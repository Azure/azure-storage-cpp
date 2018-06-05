// -----------------------------------------------------------------------------------------
// <copyright file="table_request_factory.cpp" company="Microsoft">
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
#include "wascore/constants.h"
#include "wascore/resources.h"
#include "was/common.h"
#include "was/table.h"

namespace azure { namespace storage { namespace protocol {

    web::http::uri generate_table_uri(const web::http::uri& base_uri, const cloud_table& table)
    {
        if (base_uri.is_empty())
        {
            return web::http::uri();
        }

        web::http::uri_builder builder(base_uri);
        builder.append_path(table.name(), /* do_encoding */ true);
        return builder.to_uri();
    }

    web::http::uri generate_table_uri(const web::http::uri& base_uri, const cloud_table& table, bool create_table)
    {
        if (base_uri.is_empty())
        {
            return web::http::uri();
        }

        utility::string_t path;
        if (create_table)
        {
            path.append(_XPLATSTR("Tables"));
        }
        else
        {
            utility::string_t modified_table_name = core::single_quote(table.name());

            path.reserve(modified_table_name.size() + 8U);

            path.append(_XPLATSTR("Tables("));
            path.append(modified_table_name);
            path.push_back(_XPLATSTR(')'));
        }

        web::http::uri_builder builder(base_uri);
        builder.append_path(path, /* do_encoding */ true);
        return builder.to_uri();
    }

    web::http::uri generate_table_uri(const web::http::uri& base_uri, const cloud_table& table, const table_operation& operation)
    {
        if (base_uri.is_empty())
        {
            return web::http::uri();
        }

        utility::string_t path;
        if (operation.operation_type() == table_operation_type::insert_operation)
        {
            path.append(table.name());
        }
        else
        {
            utility::string_t modified_partition_key = core::single_quote(operation.entity().partition_key());
            utility::string_t modified_row_key = core::single_quote(operation.entity().row_key());

            path.reserve(table.name().size() + modified_partition_key.size() + modified_row_key.size() + 23U);

            path.append(table.name());
            path.append(_XPLATSTR("(PartitionKey="));
            path.append(modified_partition_key);
            path.append(_XPLATSTR(",RowKey="));
            path.append(modified_row_key);
            path.push_back(_XPLATSTR(')'));
        }

        web::http::uri_builder builder(base_uri);
        builder.append_path(path, /* do_encoding */ true);
        return builder.to_uri();
    }

    web::http::uri generate_table_uri(const web::http::uri& base_uri, const cloud_table& table, const table_batch_operation& operation)
    {
        UNREFERENCED_PARAMETER(table);
        UNREFERENCED_PARAMETER(operation);

        if (base_uri.is_empty())
        {
            return web::http::uri();
        }

        utility::string_t path(_XPLATSTR("$batch"));

        web::http::uri_builder builder(base_uri);
        builder.append_path(path, /* do_encoding */ true);
        return builder.to_uri();
    }

    web::http::uri generate_table_uri(const web::http::uri& base_uri, const cloud_table& table, const table_query& query, const continuation_token& token)
    {
        if (base_uri.is_empty())
        {
            return web::http::uri();
        }

        web::http::uri_builder builder(base_uri);

        builder.append_path(table.name(), /* do_encoding */ true);

        if (!query.filter_string().empty())
        {
            builder.append_query(core::make_query_parameter(_XPLATSTR("$filter"), query.filter_string()));
        }

        if (query.take_count() >= 0)
        {
            builder.append_query(core::make_query_parameter(_XPLATSTR("$top"), query.take_count(), /* do_encoding */ false));
        }

        if (!query.select_columns().empty())
        {
            utility::string_t select_builder;
            select_builder.reserve(128);
            select_builder.append(_XPLATSTR("PartitionKey,RowKey,Timestamp"));

            std::vector<utility::string_t> select_columns = query.select_columns();
            for(std::vector<utility::string_t>::const_iterator itr = select_columns.cbegin(); itr != select_columns.cend(); ++itr)
            {
                if (itr->compare(_XPLATSTR("PartitionKey")) != 0 && itr->compare(_XPLATSTR("RowKey")) != 0 && itr->compare(_XPLATSTR("Timestamp")) != 0)
                {
                    select_builder.append(_XPLATSTR(","));
                    select_builder.append(*itr);
                }
            }

            builder.append_query(core::make_query_parameter(_XPLATSTR("$select"), select_builder));
        }

        if (!token.empty())
        {
            builder.append_query(token.next_marker());
        }

        return builder.to_uri();
    }

    storage_uri generate_table_uri(const cloud_table_client& service_client, const cloud_table& table)
    {
        web::http::uri primary_uri(generate_table_uri(service_client.base_uri().primary_uri(), table));
        web::http::uri secondary_uri(generate_table_uri(service_client.base_uri().secondary_uri(), table));

        return storage_uri(std::move(primary_uri), std::move(secondary_uri));
    }

    storage_uri generate_table_uri(const cloud_table_client& service_client, const cloud_table& table, bool create_table)
    {
        web::http::uri primary_uri(generate_table_uri(service_client.base_uri().primary_uri(), table, create_table));
        web::http::uri secondary_uri(generate_table_uri(service_client.base_uri().secondary_uri(), table, create_table));

        return storage_uri(std::move(primary_uri), std::move(secondary_uri));
    }

    storage_uri generate_table_uri(const cloud_table_client& service_client, const cloud_table& table, const table_operation& operation)
    {
        web::http::uri primary_uri(generate_table_uri(service_client.base_uri().primary_uri(), table, operation));
        web::http::uri secondary_uri(generate_table_uri(service_client.base_uri().secondary_uri(), table, operation));

        return storage_uri(std::move(primary_uri), std::move(secondary_uri));
    }

    storage_uri generate_table_uri(const cloud_table_client& service_client, const cloud_table& table, const table_batch_operation& operation)
    {
        web::http::uri primary_uri(generate_table_uri(service_client.base_uri().primary_uri(), table, operation));
        web::http::uri secondary_uri(generate_table_uri(service_client.base_uri().secondary_uri(), table, operation));

        return storage_uri(std::move(primary_uri), std::move(secondary_uri));
    }

    storage_uri generate_table_uri(const cloud_table_client& service_client, const cloud_table& table, const table_query& query, const continuation_token& token)
    {
        web::http::uri primary_uri(generate_table_uri(service_client.base_uri().primary_uri(), table, query, token));
        web::http::uri secondary_uri(generate_table_uri(service_client.base_uri().secondary_uri(), table, query, token));

        return storage_uri(std::move(primary_uri), std::move(secondary_uri));
    }

    const utility::string_t get_http_method(table_operation_type operation_type)
    {
        switch (operation_type)
        {
        case table_operation_type::retrieve_operation:
            return web::http::methods::GET;

        case table_operation_type::insert_or_merge_operation:
        case table_operation_type::merge_operation:
            return web::http::methods::MERGE;

        case table_operation_type::insert_or_replace_operation:
        case table_operation_type::replace_operation:
            return web::http::methods::PUT;

        case table_operation_type::delete_operation:
            return web::http::methods::DEL;

        default: // insert_operation
            return web::http::methods::POST;
        }
    }

    const utility::char_t* get_accept_header(table_payload_format payload_format)
    {
        switch (payload_format)
        {
        case table_payload_format::json_full_metadata:
            return header_value_accept_application_json_full_metadata;
        case table_payload_format::json_no_metadata:
            return header_value_accept_application_json_no_metadata;
        default: // table_payload_format::json
            return header_value_accept_application_json_minimal_metadata;
        }
    }

    void populate_http_headers(web::http::http_headers& headers, const utility::string_t& boundary_name)
    {
        headers.add(web::http::header_names::content_type, get_multipart_content_type(boundary_name));
    }

    void populate_http_headers(web::http::http_headers& headers, table_operation_type operation_type, table_payload_format payload_format)
    {
        headers.add(web::http::header_names::accept, get_accept_header(payload_format));
        headers.add(web::http::header_names::accept_charset, header_value_charset_utf8);

        if (operation_type == table_operation_type::insert_operation || 
            operation_type == table_operation_type::insert_or_merge_operation || 
            operation_type == table_operation_type::insert_or_replace_operation || 
            operation_type == table_operation_type::merge_operation || 
            operation_type == table_operation_type::replace_operation)
        {
            if (operation_type == table_operation_type::insert_operation)
            {
                headers.add(header_prefer, _XPLATSTR("return-no-content"));
            }

            headers.add(web::http::header_names::content_type, header_value_content_type_json);
        }
    }

    void populate_http_headers(web::http::http_headers& headers, const table_operation& operation, table_payload_format payload_format)
    {
        table_operation_type operation_type = operation.operation_type();
        populate_http_headers(headers, operation_type, payload_format);

        if (operation_type == table_operation_type::delete_operation || 
            operation_type == table_operation_type::merge_operation || 
            operation_type == table_operation_type::replace_operation)
        {
            utility::string_t etag;
            if (operation.entity().etag().empty())
            {
                // Default to update/merge/delete any present entity
                etag = _XPLATSTR("*");
            }
            else
            {
                etag = operation.entity().etag();
            }

            headers.add(web::http::header_names::if_match, etag);
        }
    }

    web::json::value generate_json_object(const table_operation& operation)
    {
        if (operation.operation_type() == table_operation_type::insert_operation || 
            operation.operation_type() == table_operation_type::insert_or_merge_operation || 
            operation.operation_type() == table_operation_type::insert_or_replace_operation || 
            operation.operation_type() == table_operation_type::merge_operation || 
            operation.operation_type() == table_operation_type::replace_operation)
        {
            const table_entity::properties_type& properties = operation.entity().properties();
            std::vector<std::pair<utility::string_t, web::json::value>> fields;

            // The PartitionKey and RowKey each need a field, and every property can have up to 2 fields for a value and type
            fields.reserve(properties.size() * 2 + 2);

            web::json::value partition_key_value(operation.entity().partition_key());
            fields.push_back(std::make_pair(_XPLATSTR("PartitionKey"), std::move(partition_key_value)));

            web::json::value row_key_value(operation.entity().row_key());
            fields.push_back(std::make_pair(_XPLATSTR("RowKey"), std::move(row_key_value)));

            for (table_entity::properties_type::const_iterator it = properties.cbegin(); it != properties.cend(); ++it)
            {
                const utility::string_t& property_name = it->first;
                const entity_property& entity_property = it->second;

                bool requires_type;
                web::json::value property_value;

                if (entity_property.property_type() == edm_type::boolean)
                {
                    requires_type = false;
                    property_value = web::json::value(entity_property.boolean_value());
                }
                else if (entity_property.property_type() == edm_type::int32)
                {
                    requires_type = false;
                    property_value = web::json::value(entity_property.int32_value());
                }
                else if (entity_property.property_type() == edm_type::double_floating_point)
                {
                    double double_value = entity_property.double_value();
                    if (core::is_finite(double_value))
                    {
                        const utility::string_t& string_value = entity_property.str();
                        if (core::is_integral(string_value))
                        {
                            // TODO: Remove this temporary workaround after Casablanca starts serializing whole number doubles with a decimal point

                            // The value consists entirely of an optional negative sign followed by digits, so add a decimal point to make it clear it is not an int32
                            utility::string_t modified_string_value;
                            modified_string_value.reserve(string_value.size() + 2);
                            modified_string_value.append(string_value);
                            modified_string_value.append(_XPLATSTR(".0"));

                            requires_type = true;
                            property_value = web::json::value(modified_string_value);
                        }
                        else
                        {
                            requires_type = false;
                            property_value = web::json::value(double_value);
                        }
                    }
                    else
                    {
                        // Serialize special double values as strings
                        requires_type = true;
                        property_value = web::json::value(entity_property.str());
                    }
                }
                else if (entity_property.property_type() == edm_type::string)
                {
                    requires_type = false;
                    property_value = web::json::value(entity_property.str());
                }
                else
                {
                    requires_type = true;
                    property_value = web::json::value(entity_property.str());
                }

                if (requires_type)
                {
                    // The type name length is the length of the property name plus 11 characters for the @odata.type suffix
                    utility::string_t type_name;
                    type_name.reserve(property_name.size() + 11);
                    type_name.append(property_name);
                    type_name.append(_XPLATSTR("@odata.type"));

                    web::json::value type_value(get_property_type_name(entity_property.property_type()));

                    fields.push_back(std::make_pair(std::move(type_name), std::move(type_value)));
                }

                fields.push_back(std::make_pair(property_name, std::move(property_value)));
            }

            return web::json::value::object(fields);
        }

        return web::json::value::null();
    }

    web::http::http_request table_base_request(web::http::method method, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request = base_request(method, uri_builder, timeout, context);

        web::http::http_headers& headers = request.headers();
        headers.add(header_max_data_service_version, header_value_data_service_version);

        return request;
    }

    web::http::http_request execute_table_operation(const cloud_table& table, table_operation_type operation_type, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::method method = get_http_method(operation_type);
        web::http::http_request request = table_base_request(method, uri_builder, timeout, context);

        web::http::http_headers& headers = request.headers();

        // This operation is processed internally and it does not need metadata because all property types are known
        populate_http_headers(headers, operation_type, table_payload_format::json_no_metadata);

        if (operation_type == table_operation_type::insert_operation)
        {
            web::json::value property_value(table.name());

            std::vector<std::pair<utility::string_t, web::json::value>> fields;
            fields.reserve(1U);
            fields.push_back(std::make_pair(_XPLATSTR("TableName"), std::move(property_value)));

            web::json::value document = web::json::value::object(fields);
            request.set_body(document);
        }

        return request;
    }

    web::http::http_request execute_operation(const table_operation& operation, table_payload_format payload_format, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::method method = get_http_method(operation.operation_type());
        web::http::http_request request = table_base_request(method, uri_builder, timeout, context);

        web::http::http_headers& headers = request.headers();
        populate_http_headers(headers, operation, payload_format);

        web::json::value json_object = generate_json_object(operation);
        if (!json_object.is_null())
        {
            request.set_body(json_object);
        }

        return request;
    }

    web::http::http_request execute_batch_operation(Concurrency::streams::stringstreambuf& response_buffer, const cloud_table& table, const table_batch_operation& batch_operation, table_payload_format payload_format, bool is_query, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        utility::string_t batch_boundary_name = core::generate_boundary_name(_XPLATSTR("batch"));
        utility::string_t changeset_boundary_name = core::generate_boundary_name(_XPLATSTR("changeset"));
        
        web::http::http_request request = table_base_request(web::http::methods::POST, uri_builder, timeout, context);
        // Need to reset the response buffer before each batch operation is executed.
        response_buffer.collection().resize(0);
        response_buffer.seekpos(0, std::ios_base::out);
        request.set_response_stream(Concurrency::streams::ostream(response_buffer));

        web::http::http_headers& request_headers = request.headers();
        request_headers.add(web::http::header_names::accept_charset, header_value_charset_utf8);
        populate_http_headers(request_headers, batch_boundary_name);

        table_batch_operation::operations_type operations = batch_operation.operations();

        web::http::uri base_uri = table.service_client().base_uri().primary_uri();
        utility::string_t body_text;

        core::write_boundary(body_text, batch_boundary_name);

        // Write batch headers
        if (!is_query)
        {
            web::http::http_headers changeset_headers;
            populate_http_headers(changeset_headers, changeset_boundary_name);

            core::write_request_headers(body_text, changeset_headers);
        }

        if (operations.size() > 0U)
        {
            int content_id = 0;
            for (table_batch_operation::operations_type::const_iterator it = operations.cbegin(); it != operations.cend(); ++it)
            {
                const table_operation& operation = *it;
                web::http::method method = get_http_method(operation.operation_type());
                web::http::uri uri = generate_table_uri(base_uri, table, operation);

                web::http::http_headers operation_headers;
                populate_http_headers(operation_headers, operation, payload_format);

                if (!is_query)
                {
                    core::write_boundary(body_text, changeset_boundary_name);
                }

                core::write_mime_changeset_headers(body_text);
                core::write_request_line(body_text, method, uri);
                core::write_request_headers(body_text, operation_headers);

                web::json::value json_object = generate_json_object(operation);
                core::write_request_payload(body_text, json_object);

                ++content_id;
            }
        }
        else
        {
            core::write_boundary(body_text, changeset_boundary_name);
        }

        if (!is_query)
        {
            core::write_boundary(body_text, changeset_boundary_name, /* is_closure */ true);
        }

        core::write_boundary(body_text, batch_boundary_name, /* is_closure */ true);

        request.set_body(body_text);

        return request;
    }

    web::http::http_request execute_query(table_payload_format payload_format, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request = table_base_request(web::http::methods::GET, uri_builder, timeout, context);

        web::http::http_headers& headers = request.headers();
        populate_http_headers(headers, table_operation_type::retrieve_operation, payload_format);

        return request;
    }

    web::http::http_request get_table_acl(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_acl, /* do_encoding */ false));
        web::http::http_request request = base_request(web::http::methods::GET, uri_builder, timeout, context);
        return request;
    }

    web::http::http_request set_table_acl(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_acl, /* do_encoding */ false));
        web::http::http_request request = base_request(web::http::methods::PUT, uri_builder, timeout, context);
        return request;
    }

    utility::string_t get_property_type_name(edm_type property_type)
    {
        switch (property_type)
        {
        case edm_type::binary:
            return _XPLATSTR("Edm.Binary");

        case edm_type::boolean:
            return _XPLATSTR("Edm.Boolean");

        case edm_type::datetime:
            return _XPLATSTR("Edm.DateTime");

        case edm_type::double_floating_point:
            return _XPLATSTR("Edm.Double");

        case edm_type::guid:
            return _XPLATSTR("Edm.Guid");

        case edm_type::int32:
            return _XPLATSTR("Edm.Int32");

        case edm_type::int64:
            return _XPLATSTR("Edm.Int64");

        default: // edm_type::string
            return _XPLATSTR("Edm.String");
        }
    }

    utility::string_t get_multipart_content_type(const utility::string_t& boundary_name)
    {
        utility::string_t content_type;
        content_type.reserve(boundary_name.size() + 26U);
        content_type.append(header_value_content_type_mime_multipart_prefix);
        content_type.append(boundary_name);
        return content_type;
    }

}}} // namespace azure::storage::protocol
