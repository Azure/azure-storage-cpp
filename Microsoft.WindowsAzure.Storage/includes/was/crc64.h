// -----------------------------------------------------------------------------------------
// <copyright file="hashing.h" company="Microsoft">
//    Copyright 2019 Microsoft Corporation
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


namespace azure { namespace storage {

    constexpr uint64_t INITIAL_CRC64 = 0ULL;

    WASTORAGE_API uint64_t update_crc64(const uint8_t* data, size_t size, uint64_t crc);
    WASTORAGE_API void set_crc64_func(std::function<uint64_t(const uint8_t*, size_t, uint64_t)> func);

    inline uint64_t crc64(const uint8_t* data, size_t size)
    {
        return update_crc64(data, size, INITIAL_CRC64);
    }

}}  // namespace azure::storage