// -----------------------------------------------------------------------------------------
// <copyright file="queue_request_factory.cpp" company="Microsoft">
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
#include "wascore/protocol_xml.h"
#include "wascore/constants.h"
#include "wascore/resources.h"
#include "was/common.h"
#include "was/queue.h"

namespace azure { namespace storage { namespace protocol {

    web::http::uri generate_queue_uri(const web::http::uri& base_uri, const cloud_queue& queue)
    {
        if (base_uri.is_empty())
        {
            return web::http::uri();
        }

        web::http::uri_builder builder(base_uri);
        builder.append_path(queue.name(), /* do_encoding */ true);
        return builder.to_uri();
    }

    storage_uri generate_queue_uri(const cloud_queue_client& service_client, const cloud_queue& queue)
    {
        web::http::uri primary_uri(generate_queue_uri(service_client.base_uri().primary_uri(), queue));
        web::http::uri secondary_uri(generate_queue_uri(service_client.base_uri().secondary_uri(), queue));

        return storage_uri(std::move(primary_uri), std::move(secondary_uri));
    }

    web::http::uri generate_queue_uri(const web::http::uri& base_uri, const utility::string_t& prefix, bool get_metadata, int max_results, const continuation_token& token)
    {
        if (base_uri.is_empty())
        {
            return web::http::uri();
        }

        web::http::uri_builder builder(base_uri);

        if (!prefix.empty())
        {
            builder.append_query(core::make_query_parameter(uri_query_prefix, prefix));
        }

        if (get_metadata)
        {
            builder.append_query(core::make_query_parameter(uri_query_include, component_metadata, /* do_encoding */ false));
        }

        if (max_results > 0)
        {
            builder.append_query(core::make_query_parameter(uri_query_max_results, max_results, /* do_encoding */ false));
        }

        if (!token.empty())
        {
            builder.append_query(token.next_marker());
        }

        return builder.to_uri();
    }

    storage_uri generate_queue_uri(const cloud_queue_client& service_client, const utility::string_t& prefix, bool get_metadata, int max_results, const continuation_token& token)
    {
        web::http::uri primary_uri(generate_queue_uri(service_client.base_uri().primary_uri(), prefix, get_metadata, max_results, token));
        web::http::uri secondary_uri(generate_queue_uri(service_client.base_uri().secondary_uri(), prefix, get_metadata, max_results, token));

        return storage_uri(std::move(primary_uri), std::move(secondary_uri));
    }

    web::http::uri generate_queue_message_uri(const web::http::uri& base_uri, const cloud_queue& queue, const utility::string_t& message_id)
    {
        if (base_uri.is_empty())
        {
            return web::http::uri();
        }

        web::http::uri_builder builder(base_uri);
        builder.append_path(queue.name(), /* do_encoding */ true);
        builder.append_path(_XPLATSTR("messages"));
        builder.append_path(message_id, /* do_encoding */ true);

        return builder.to_uri();
    }

    web::http::uri generate_queue_message_uri(const web::http::uri& base_uri, const cloud_queue& queue)
    {
        return generate_queue_message_uri(base_uri, queue, utility::string_t());
    }

    storage_uri generate_queue_message_uri(const cloud_queue_client& service_client, const cloud_queue& queue)
    {
        web::http::uri primary_uri(generate_queue_message_uri(service_client.base_uri().primary_uri(), queue));
        web::http::uri secondary_uri(generate_queue_message_uri(service_client.base_uri().secondary_uri(), queue));

        return storage_uri(std::move(primary_uri), std::move(secondary_uri));
    }

    web::http::uri generate_queue_message_uri(const web::http::uri& base_uri, const cloud_queue& queue, const cloud_queue_message& message)
    {
        return generate_queue_message_uri(base_uri, queue, message.id());
    }

    storage_uri generate_queue_message_uri(const cloud_queue_client& service_client, const cloud_queue& queue, const cloud_queue_message& message)
    {
        web::http::uri primary_uri(generate_queue_message_uri(service_client.base_uri().primary_uri(), queue, message));
        web::http::uri secondary_uri(generate_queue_message_uri(service_client.base_uri().secondary_uri(), queue, message));

        return storage_uri(std::move(primary_uri), std::move(secondary_uri));
    }

    web::http::http_request queue_base_request(web::http::method method, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request = base_request(method, uri_builder, timeout, context);

        return request;
    }

    web::http::http_request list_queues(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_list, /* do_encoding */ false));

        web::http::http_request request = queue_base_request(web::http::methods::GET, uri_builder, timeout, context);
        return request;
    }

    web::http::http_request create_queue(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request = queue_base_request(web::http::methods::PUT, uri_builder, timeout, context);
        return request;
    }

    web::http::http_request delete_queue(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request = queue_base_request(web::http::methods::DEL, uri_builder, timeout, context);
        return request;
    }

    web::http::http_request add_message(const cloud_queue_message& message, std::chrono::seconds time_to_live, std::chrono::seconds initial_visibility_timeout, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        if (time_to_live.count() >= 0LL && time_to_live.count() != 604800LL)
        {
            uri_builder.append_query(core::make_query_parameter(_XPLATSTR("messagettl"), time_to_live.count(), /* do_encoding */ false));
        }

        if (initial_visibility_timeout.count() > 0LL)
        {
            uri_builder.append_query(core::make_query_parameter(_XPLATSTR("visibilitytimeout"), initial_visibility_timeout.count(), /* do_encoding */ false));
        }

        web::http::http_request request = queue_base_request(web::http::methods::POST, uri_builder, timeout, context);

        protocol::message_writer writer;
        std::string content = writer.write(message);
        request.set_body(content);

        return request;
    }

    web::http::http_request get_messages(size_t message_count, std::chrono::seconds visibility_timeout, bool is_peek, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        if (is_peek)
        {
            uri_builder.append_query(_XPLATSTR("peekonly=true"));
        }

        if (message_count > 1U)
        {
            // The service uses the default value 1
            uri_builder.append_query(core::make_query_parameter(_XPLATSTR("numofmessages"), message_count, /* do_encoding */ false));
        }

        if (!is_peek && visibility_timeout.count() > 0LL)
        {
            uri_builder.append_query(core::make_query_parameter(_XPLATSTR("visibilitytimeout"), visibility_timeout.count(), /* do_encoding */ false));
        }

        web::http::http_request request = queue_base_request(web::http::methods::GET, uri_builder, timeout, context);
        return request;
    }

    web::http::http_request delete_message(const cloud_queue_message& message, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(_XPLATSTR("popreceipt"), message.pop_receipt()));

        web::http::http_request request = queue_base_request(web::http::methods::DEL, uri_builder, timeout, context);
        return request;
    }

    web::http::http_request update_message(const cloud_queue_message& message, std::chrono::seconds visibility_timeout, bool update_contents, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(_XPLATSTR("popreceipt"), message.pop_receipt()));
        uri_builder.append_query(core::make_query_parameter(_XPLATSTR("visibilitytimeout"), visibility_timeout.count(), /* do_encoding */ false));

        web::http::http_request request = queue_base_request(web::http::methods::PUT, uri_builder, timeout, context);

        if (update_contents)
        {
            protocol::message_writer writer;
            std::string content = writer.write(message);
            request.set_body(content);
        }

        return request;
    }

    web::http::http_request clear_messages(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        web::http::http_request request = queue_base_request(web::http::methods::DEL, uri_builder, timeout, context);
        return request;
    }

    web::http::http_request download_queue_metadata(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_metadata, /* do_encoding */ false));
        web::http::http_request request = queue_base_request(web::http::methods::HEAD, uri_builder, timeout, context);
        return request;
    }

    web::http::http_request upload_queue_metadata(const cloud_metadata& metadata, web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        // TODO: Make a copy of needed data so it is OK for the main object class to be destructed mid-operation
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_metadata, /* do_encoding */ false));
        web::http::http_request request = queue_base_request(web::http::methods::PUT, uri_builder, timeout, context);
        add_metadata(request, metadata);
        return request;
    }

    web::http::http_request get_queue_acl(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_acl, /* do_encoding */ false));
        web::http::http_request request = queue_base_request(web::http::methods::GET, uri_builder, timeout, context);
        return request;
    }

    web::http::http_request set_queue_acl(web::http::uri_builder& uri_builder, const std::chrono::seconds& timeout, operation_context context)
    {
        uri_builder.append_query(core::make_query_parameter(uri_query_component, component_acl, /* do_encoding */ false));
        web::http::http_request request = queue_base_request(web::http::methods::PUT, uri_builder, timeout, context);
        return request;
    }

}}} // namespace azure::storage::protocol
