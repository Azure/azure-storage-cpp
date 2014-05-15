// -----------------------------------------------------------------------------------------
// <copyright file="protocol_json.h" company="Microsoft">
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

#pragma once

#include "wascore/basic_types.h"
#include "was/table.h"

namespace azure { namespace storage { namespace protocol {

    table_entity parse_table_entity(const web::json::value& document);
    storage_extended_error parse_table_error(const web::json::value& document);

}}} // namespace azure::storage::protocol
