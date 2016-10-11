// -----------------------------------------------------------------------------------------
// <copyright file="basic_types.h" company="Microsoft">
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
#pragma warning(disable: 4634)

#include "cpprest/details/basic_types.h"

#ifdef _NO_WASTORAGE_API
    #define WASTORAGE_API
#else
    #ifdef WASTORAGE_DLL
        #define WASTORAGE_API __declspec( dllexport )
    #else
        #define WASTORAGE_API __declspec( dllimport )
    #endif
#endif

#ifdef _WIN32
    #define DEPRECATED(message) __declspec(deprecated(message))
#else
    #define DEPRECATED(message) __attribute__((deprecated(message)))
#endif

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Rpc.h>
#else
extern "C"
{
#include <uuid/uuid.h>
}
#endif

namespace utility {

    //typedef struct { uint8_t data[16]; } uuid;

#ifdef _WIN32
    typedef UUID uuid;
#else
    typedef struct
    {
            uuid_t data;
    } uuid;
#endif

    /// <summary>
    /// Generates a new UUID.
    /// </summary>
    WASTORAGE_API utility::uuid __cdecl new_uuid();

    /// <summary>
    /// Converts an UUID to a string.
    /// </summary>
    WASTORAGE_API utility::string_t __cdecl uuid_to_string(const utility::uuid& value);

    /// <summary>
    /// Converts a string to a UUID.
    /// </summary>
    WASTORAGE_API utility::uuid __cdecl string_to_uuid(const utility::string_t& value);

    /// <summary>
    /// Compares two UUIDs for equality.
    /// </summary>
    WASTORAGE_API bool __cdecl uuid_equal(const utility::uuid& value1, const utility::uuid& value2);

    template<typename T>
    void assert_in_bounds(const utility::string_t& param_name, const T& value, const T& min)
    {
        if (value < min)
        {
            throw std::invalid_argument(utility::conversions::to_utf8string(param_name));
        }
    }

    template<typename T>
    void assert_in_bounds(const utility::string_t& param_name, const T& value, const T& min, const T& max)
    {
        assert_in_bounds(param_name, value, min);

        if (value > max)
        {
            throw std::invalid_argument(utility::conversions::to_utf8string(param_name));
        }
    }

} // namespace utility
