// -----------------------------------------------------------------------------------------
// <copyright file="resources.h" company="Microsoft">
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

#include "cpprest/asyncrt_utils.h"

#include "wascore/basic_types.h"

namespace azure { namespace storage { namespace protocol {

#define _RESOURCES
#define DAT(a, b) extern const char* a; const size_t a ## _size = sizeof(b) / sizeof(char) - 1;
#include "wascore/constants.dat"
#undef DAT
#undef _RESOURCES

}}} // namespace azure::storage::protocol
