// -----------------------------------------------------------------------------------------
// <copyright file="cloud_client.cpp" company="Microsoft">
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
#include "was/service_client.h"
#include "wascore/protocol.h"
#include "wascore/protocol_xml.h"

namespace azure { namespace storage {

    pplx::task<service_properties> cloud_client::download_service_properties_base_async(const request_options& modified_options, operation_context context) const
    {
        auto command = std::make_shared<core::storage_command<service_properties>>(base_uri());
        command->set_build_request(std::bind(protocol::get_service_properties, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary);
        command->set_preprocess_response(std::bind(protocol::preprocess_response<service_properties>, service_properties(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_postprocess_response([] (const web::http::http_response& response, const request_result&, const core::ostream_descriptor&, operation_context context) -> pplx::task<service_properties>
        {
            protocol::service_properties_reader reader(response.body());
            return pplx::task_from_result<service_properties>(reader.move_properties());
        });
        return core::executor<service_properties>::execute_async(command, modified_options, context);
    }

    pplx::task<void> cloud_client::upload_service_properties_base_async(const service_properties& properties, const service_properties_includes& includes, const request_options& modified_options, operation_context context) const
    {
        protocol::service_properties_writer writer;
        concurrency::streams::istream stream(concurrency::streams::bytestream::open_istream(writer.write(properties, includes)));

        auto command = std::make_shared<core::storage_command<void>>(base_uri());
        command->set_build_request(std::bind(protocol::set_service_properties, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(authentication_handler());
        command->set_preprocess_response(std::bind(protocol::preprocess_response_void, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        return core::istream_descriptor::create(stream).then([command, context, modified_options] (core::istream_descriptor request_body) -> pplx::task<void>
        {
            command->set_request_body(request_body);
            return core::executor<void>::execute_async(command, modified_options, context);
        });
    }

    pplx::task<service_stats> cloud_client::download_service_stats_base_async(const request_options& modified_options, operation_context context) const
    {
        if (modified_options.location_mode() == location_mode::primary_only)
        {
            throw storage_exception("download_service_stats cannot be run with a 'primary_only' location mode.");
        }

        auto command = std::make_shared<core::storage_command<service_stats>>(base_uri());
        command->set_build_request(std::bind(protocol::get_service_stats, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary);
        command->set_preprocess_response(std::bind(protocol::preprocess_response<service_stats>, service_stats(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_postprocess_response([] (const web::http::http_response& response, const request_result&, const core::ostream_descriptor&, operation_context context) -> pplx::task<service_stats>
        {
            protocol::service_stats_reader reader(response.body());
            return pplx::task_from_result<service_stats>(reader.move_stats());
        });
        return core::executor<service_stats>::execute_async(command, modified_options, context);
    }

}
} // namespace azure::storage
