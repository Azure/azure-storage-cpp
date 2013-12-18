// -----------------------------------------------------------------------------------------
// <copyright file="cloud_core.cpp" company="Microsoft">
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

#include "wascore/resources.h"
#include "was/core.h"

namespace wa { namespace storage {

    storage_uri::storage_uri(const web::http::uri& primary_uri, const web::http::uri& secondary_uri)
        : m_primary_uri(primary_uri), m_secondary_uri(secondary_uri)
    {
        if (primary_uri.is_empty())
        {
            throw std::invalid_argument("primary_uri");
        }

        if (!secondary_uri.is_empty() &&
            (primary_uri.resource() != secondary_uri.resource()))
        {
            throw std::invalid_argument(utility::conversions::to_utf8string(protocol::error_storage_uri_mismatch));
        }
    }

}} // namespace wa::storage
