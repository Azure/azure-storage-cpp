// -----------------------------------------------------------------------------------------
// <copyright file="operation_context.cpp" company="Microsoft">
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

namespace azure { namespace storage {

    client_log_level operation_context::m_global_log_level = client_log_level::log_level_off;

    web::web_proxy operation_context::m_global_proxy;

    operation_context::operation_context()
        : m_impl(std::make_shared<_operation_context>())
    {
        set_log_level(default_log_level());
        set_proxy(default_proxy());
        set_client_request_id(utility::uuid_to_string(utility::new_uuid()));
    }

    client_log_level operation_context::default_log_level()
    {
        return m_global_log_level;
    }

    void operation_context::set_default_log_level(client_log_level log_level)
    {
        m_global_log_level = log_level;
    }

    const web::web_proxy &operation_context::default_proxy()
    {
        return m_global_proxy;
    }

    void operation_context::set_default_proxy(web::web_proxy proxy)
    {
        m_global_proxy = std::move(proxy);
    }

}} // namespace azure::storage
