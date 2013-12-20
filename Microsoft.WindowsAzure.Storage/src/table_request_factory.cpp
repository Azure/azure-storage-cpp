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

namespace wa { namespace storage { namespace protocol {

    /*
    utility::string_t generate_table_query_string(const table_query& query, const continuation_token& continuation_token)
    {
        continuation_token.next_marker()
    }
    */

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
            path.append(U("Tables"));
        }
        else
        {
            utility::string_t modified_table_name = core::single_quote(table.name());

            path.reserve(modified_table_name.size() + 8U);

            path.append(U("Tables("));
            path.append(modified_table_name);
            path.push_back(U(')'));
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
            path.append(U("(PartitionKey="));
            path.append(modified_partition_key);
            path.append(U(",RowKey="));
            path.append(modified_row_key);
            path.push_back(U(')'));
        }

        web::http::uri_builder builder(base_uri);
        builder.append_path(path, /* do_encoding */ true);
        return builder.to_uri();
    }

    web::http::uri generate_table_uri(const web::http::uri& base_uri, const cloud_table& table, const table_batch_operation& operation)
    {
        if (base_uri.is_empty())
        {
            return web::http::uri();
        }

        utility::string_t path(U("$batch"));

        web::http::uri_builder builder(base_uri);
        builder.append_path(path, /* do_encoding */ true);
        return builder.to_uri();
    }

    web::http::uri generate_table_uri(const web::http::uri& base_uri, const cloud_table& table, const table_query& query, const continuation_token& continuation_token)
    {
        if (base_uri.is_empty())
        {
            return web::http::uri();
        }

        web::http::uri_builder builder(base_uri);

        builder.append_path(table.name(), /* do_encoding */ true);

        if (!query.filter_string().empty())
        {
            builder.append_query(U("$filter"), query.filter_string());
        }

        if (query.take_count() >= 0)
        {
            builder.append_query(U("$top"), query.take_count());
        }

        if (!query.select_columns().empty())
        {
            utility::ostringstream_t select_builder;

            // TODO: Consider calculating the final size of the string and pre-allocating it

            select_builder << U("PartitionKey,RowKey,Timestamp");

            std::vector<utility::string_t> select_columns = query.select_columns();
            for(std::vector<utility::string_t>::const_iterator itr = select_columns.cbegin(); itr != select_columns.cend(); ++itr)
            {
                if (itr->compare(U("PartitionKey")) != 0 && itr->compare(U("RowKey")) != 0 && itr->compare(U("Timestamp")) != 0)
                {
                    select_builder << U(',') << *itr;
                }
            }

            builder.append_query(U("$select"), select_builder.str());
        }

        if (!continuation_token.empty())
        {
            builder.append_query(continuation_token.next_marker());
        }

        return builder.to_uri();
    }

    storage_uri generate_table_uri(const cloud_table_client& service_client, const cloud_table& table)
    {
        web::http::uri primary_uri = generate_table_uri(service_client.base_uri().primary_uri(), table);
        web::http::uri secondary_uri = generate_table_uri(service_client.base_uri().secondary_uri(), table);

        return storage_uri(primary_uri, secondary_uri);
    }

    storage_uri generate_table_uri(const cloud_table_client& service_client, const cloud_table& table, bool create_table)
    {
        web::http::uri primary_uri = generate_table_uri(service_client.base_uri().primary_uri(), table, create_table);
        web::http::uri secondary_uri = generate_table_uri(service_client.base_uri().secondary_uri(), table, create_table);

        return storage_uri(primary_uri, secondary_uri);
    }

    storage_uri generate_table_uri(const cloud_table_client& service_client, const cloud_table& table, const table_operation& operation)
    {
        web::http::uri primary_uri = generate_table_uri(service_client.base_uri().primary_uri(), table, operation);
        web::http::uri secondary_uri = generate_table_uri(service_client.base_uri().secondary_uri(), table, operation);

        return storage_uri(primary_uri, secondary_uri);
    }

    storage_uri generate_table_uri(const cloud_table_client& service_client, const cloud_table& table, const table_batch_operation& operation)
    {
        web::http::uri primary_uri = generate_table_uri(service_client.base_uri().primary_uri(), table, operation);
        web::http::uri secondary_uri = generate_table_uri(service_client.base_uri().secondary_uri(), table, operation);

        return storage_uri(primary_uri, secondary_uri);
    }

    storage_uri generate_table_uri(const cloud_table_client& service_client, const cloud_table& table, const table_query& query, const continuation_token& continuation_token)
    {
        web::http::uri primary_uri = generate_table_uri(service_client.base_uri().primary_uri(), table, query, continuation_token);
        web::http::uri secondary_uri = generate_table_uri(service_client.base_uri().secondary_uri(), table, query, continuation_token);

        return storage_uri(primary_uri, secondary_uri);
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

    const utility::string_t& get_accept_header(table_payload_format payload_format)
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

    void populate_http_headers(web::http::http_headers& headers)
    {
        headers.add(header_max_data_service_version, header_value_data_service_version);
    }

    void populate_http_headers(web::http::http_headers& headers, const utility::string_t& boundary_name)
    {
        headers.add(web::http::header_names::content_type, get_multipart_content_type(boundary_name));
        populate_http_headers(headers);
    }

    void populate_http_headers(web::http::http_headers& headers, table_payload_format payload_format, const utility::string_t& boundary_name)
    {
        headers.add(web::http::header_names::accept, get_accept_header(payload_format));
        populate_http_headers(headers, boundary_name);
    }

    void populate_http_headers(web::http::http_headers& headers, table_operation_type operation_type, table_payload_format payload_format)
    {
        if (operation_type == table_operation_type::retrieve_operation || 
            operation_type == table_operation_type::insert_operation)
        {
            headers.add(web::http::header_names::accept, get_accept_header(payload_format));
        }

        if (operation_type == table_operation_type::insert_operation || 
            operation_type == table_operation_type::insert_or_merge_operation || 
            operation_type == table_operation_type::insert_or_replace_operation || 
            operation_type == table_operation_type::merge_operation || 
            operation_type == table_operation_type::replace_operation)
        {
            if (operation_type == table_operation_type::insert_operation)
            {
                // TODO: Make the "Prefer: return-no-content" header configurable if needed
                headers.add(header_prefer, U("return-no-content"));
            }

            /*
            if (operation.operation_type() == table_operation_type::insert_or_merge_operation || 
                operation.operation_type() == table_operation_type::merge_operation)
            {
                headers.add(header_http_method, U("MERGE"));
            }
            */

            headers.add(web::http::header_names::content_type, header_value_content_type_json);
        }

        populate_http_headers(headers);
    }

    void populate_http_headers(web::http::http_headers& headers, const table_operation& operation, table_payload_format payload_format)
    {
        table_operation_type operation_type = operation.operation_type();

        if (operation_type == table_operation_type::delete_operation || 
            operation_type == table_operation_type::merge_operation || 
            operation_type == table_operation_type::replace_operation)
        {
            utility::string_t etag;
            if (operation.entity().etag().empty())
            {
                // Default to update/merge/delete any present entity
                etag = U("*");
            }
            else
            {
                etag = operation.entity().etag();
            }

            headers.add(web::http::header_names::if_match, etag);
        }

        populate_http_headers(headers, operation_type, payload_format);
    }

    web::json::value generate_json_object(const table_operation& operation)
    {
        if (operation.operation_type() == table_operation_type::insert_operation || 
            operation.operation_type() == table_operation_type::insert_or_merge_operation || 
            operation.operation_type() == table_operation_type::insert_or_replace_operation || 
            operation.operation_type() == table_operation_type::merge_operation || 
            operation.operation_type() == table_operation_type::replace_operation)
        {
            const std::unordered_map<utility::string_t, entity_property>& properties = operation.entity().properties();
            web::json::value::field_map fields;
            fields.reserve(properties.size() * 2U + 2U);

            web::json::value partition_key_name(U("PartitionKey"));
            web::json::value partition_key_value(operation.entity().partition_key());
            fields.push_back(std::pair<web::json::value, web::json::value>(partition_key_name, partition_key_value));

            web::json::value row_key_name(U("RowKey"));
            web::json::value row_key_value(operation.entity().row_key());
            fields.push_back(std::pair<web::json::value, web::json::value>(row_key_name, row_key_value));

            // TODO: Confirm that there is no need to write the ETag in the message body

            for (std::unordered_map<utility::string_t, entity_property>::const_iterator itr = properties.cbegin(); itr != properties.cend(); ++itr)
            {
                bool requires_type;
                web::json::value property_name(itr->first);

                web::json::value property_value;
                if (itr->second.property_type() == edm_type::boolean)
                {
                    requires_type = false;
                    property_value = web::json::value(itr->second.boolean_value());
                }
                else if (itr->second.property_type() == edm_type::int32)
                {
                    requires_type = false;
                    property_value = web::json::value(itr->second.int32_value());
                }
                else if (itr->second.property_type() == edm_type::double_floating_point)
                {
                    // TODO: Test writing special double values: NaN, Inf, -Inf

                    /*
                    double double_value = itr->second.double_value();
                    if (core::is_finite(double_value))
                    {
                        requires_type = false;
                        property_value = web::json::value(double_value);
                    }
                    else
                    {
                        // Serialize special double values as strings
                        requires_type = true;
                        property_value = web::json::value(itr->second.str());
                    }
                    */

                    // TODO: Remove this temporary workaround and use the above code after Casablanca fixes truncated double values
                    requires_type = true;
                    property_value = web::json::value(itr->second.str());
                }
                else
                {
                    requires_type = true;
                    property_value = web::json::value(itr->second.str());
                }

                if (requires_type)
                {
                    utility::string_t property_name_for_type;
                    property_name_for_type.reserve(itr->first.size() + 11U);
                    property_name_for_type.append(itr->first);
                    property_name_for_type.append(U("@odata.type"));

                    utility::string_t value_for_type = get_property_type_name(itr->second.property_type());

                    web::json::value type_name(property_name_for_type);
                    web::json::value type_value(value_for_type);

                    fields.push_back(std::pair<web::json::value, web::json::value>(type_name, type_value));
                }

                fields.push_back(std::pair<web::json::value, web::json::value>(property_name, property_value));
            }

            return web::json::value::object(fields);
        }

        return web::json::value::null();
    }

    web::http::http_request table_base_request(web::http::method method, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request = base_request(method, uri_builder, timeout, context);

        web::http::http_headers& headers = request.headers();
        headers.add(web::http::header_names::accept_charset, header_value_charset_utf8);

        return request;
    }

    /*
    http_request query_table_base_request(table_payload_format format, method method, web::http::uri_builder web::http::uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        http_request request = table_base_request(method, web::http::uri_builder, timeout, context);
        return request;
    }
    */

    web::http::http_request execute_table_operation(const cloud_table& table, table_operation_type operation_type, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::method method = get_http_method(operation_type);
        web::http::http_request request = table_base_request(method, uri_builder, timeout, context);

        if (operation_type == table_operation_type::retrieve_operation || 
            operation_type == table_operation_type::insert_operation)
        {
            web::http::http_headers& headers = request.headers();

            // This operation is processed internally and it does not need metadata because all property types are known
            populate_http_headers(headers, operation_type, table_payload_format::json_no_metadata);

            if (operation_type == table_operation_type::insert_operation)
            {
                web::json::value property_name(U("TableName"));
                web::json::value property_value(table.name());

                web::json::value::field_map fields;
                fields.push_back(std::pair<web::json::value, web::json::value>(property_name, property_value));

                web::json::value obj = web::json::value::object(fields);
                request.set_body(obj);
            }
        }

        return request;
    }

    /*
    web::http::http_request delete_table(const cloud_table& table, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request = table_base_request(web::http::methods::DEL, uri_builder, timeout, context);

        web::http::http_headers& headers = request.headers();
        headers.add(web::http::header_names::content_type, header_value_content_type_json);

        return request;
    }
    */

    web::http::http_request execute_operation(const table_operation& operation, table_payload_format payload_format, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
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

    web::http::http_request execute_batch_operation(Concurrency::streams::stringstreambuf& response_buffer, const cloud_table& table, const table_batch_operation& operation, table_payload_format payload_format, bool is_query, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        utility::string_t batch_boundary_name = core::generate_boundary_name(U("batch"));
        utility::string_t changeset_boundary_name = core::generate_boundary_name(U("changeset"));
        
        web::http::http_request request = table_base_request(web::http::methods::POST, uri_builder, timeout, context);
        request.set_response_stream(Concurrency::streams::ostream(response_buffer));

        web::http::http_headers& batch_headers = request.headers();
        populate_http_headers(batch_headers, payload_format, batch_boundary_name);

        table_batch_operation::operations_type operations = operation.operations();
        //bool is_query = operations.size() == 1 && operations[0].operation_type() == table_operation_type::retrieve_operation;

        web::http::uri base_uri = table.service_client().base_uri().primary_uri();
        utility::string_t body_text;

        core::write_boundary(body_text, batch_boundary_name);

        if (!is_query)
        {
            web::http::http_headers changeset_headers;
            populate_http_headers(changeset_headers, changeset_boundary_name);

            core::write_request_headers(body_text, changeset_headers);
        }

        if (operations.size() > 0U)
        {
            int content_id = 0;
            for (table_batch_operation::operations_type::const_iterator itr = operations.cbegin(); itr != operations.cend(); ++itr)
            {
                table_operation operation = *itr;
                web::http::method method = get_http_method(operation.operation_type());
                web::http::uri uri = generate_table_uri(base_uri, table, operation);

                web::http::http_headers operation_headers;
                populate_http_headers(operation_headers, operation, payload_format);

                if (!is_query)
                {
                    operation_headers.add(header_content_id, core::convert_to_string(content_id));

                    core::write_boundary(body_text, changeset_boundary_name);
                }

                core::write_mime_multipart_headers(body_text);
                core::write_request_line(body_text, method, uri);
                //core::write_content_id_request_header(body_text, content_id);
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

    web::http::http_request execute_query(table_payload_format payload_format, web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request = table_base_request(web::http::methods::GET, uri_builder, timeout, context);

        web::http::http_headers& headers = request.headers();
        populate_http_headers(headers, table_operation_type::retrieve_operation, payload_format);

        return request;
    }

    web::http::http_request get_table_acl(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(uri_query_component, component_acl);
        web::http::http_request request = table_base_request(web::http::methods::GET, uri_builder, timeout, context);
        return request;
    }

    web::http::http_request set_table_acl(web::http::uri_builder uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(uri_query_component, component_acl);
        web::http::http_request request = table_base_request(web::http::methods::PUT, uri_builder, timeout, context);
        return request;
    }

    utility::string_t get_property_type_name(edm_type property_type)
    {
        switch (property_type)
        {
        case edm_type::binary:
            return U("Edm.Binary");

        case edm_type::boolean:
            return U("Edm.Boolean");

        case edm_type::datetime:
            return U("Edm.DateTime");

        case edm_type::double_floating_point:
            return U("Edm.Double");

        case edm_type::guid:
            return U("Edm.Guid");

        case edm_type::int32:
            return U("Edm.Int32");

        case edm_type::int64:
            return U("Edm.Int64");

        default: // edm_type::string
            return U("Edm.String");
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

}}} // namespace wa::storage::protocol
