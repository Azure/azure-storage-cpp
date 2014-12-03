// -----------------------------------------------------------------------------------------
// <copyright file="cloud_common.cpp" company="Microsoft">
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

#include "was/common.h"

namespace azure { namespace storage {

    WASTORAGE_API request_options::request_options()
        : m_location_mode(azure::storage::location_mode::primary_only), m_retry_policy(exponential_retry_policy()), m_http_buffer_size(protocol::default_buffer_size),\
          m_maximum_execution_time(protocol::default_maximum_execution_time), m_server_timeout(protocol::default_server_timeout)
    {
    }

}} // namespace azure::storage
