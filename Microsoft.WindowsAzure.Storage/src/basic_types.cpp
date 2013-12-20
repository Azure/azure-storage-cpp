// -----------------------------------------------------------------------------------------
// <copyright file="basic_types.cpp" company="Microsoft">
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
#include "wascore/basic_types.h"

namespace utility {

    utility::uuid __cdecl new_uuid()
    {
#ifdef WIN32
        RPC_STATUS status;

        UUID uuid;
        status = UuidCreate(&uuid);
        if (status != RPC_S_OK && status != RPC_S_UUID_LOCAL_ONLY && status != RPC_S_UUID_NO_ADDRESS)
        {
            throw std::runtime_error("An error occurred creating the UUID.");
        }
#else
        uuid_t uuid;
        uuid_generate_random(uuid);
#endif

        return uuid;
    }

    utility::string_t __cdecl uuid_to_string(const utility::uuid& value)
    {
#ifdef WIN32
        RPC_STATUS status;

        RPC_WSTR rpc_string;
        status = UuidToStringW(&value, &rpc_string);
        if (status != RPC_S_OK)
        {
            throw std::runtime_error("An error occurred serializing the UUID.");
        }

        std::wstring result(reinterpret_cast<wchar_t*>(rpc_string));

        status = RpcStringFree(&rpc_string);
        if (status != RPC_S_OK)
        {
            throw std::runtime_error("An error occurred freeing the UUID string.");
        }
#else
        char uuid_string[37];
        uuid_unparse_upper(value, uuid_string);

        std::string result(&uuid_string);
#endif

        return result;
    }

    utility::uuid __cdecl string_to_uuid(const utility::string_t& value)
    {
#ifdef WIN32
        RPC_STATUS status;

        RPC_WSTR rpc_string = reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(value.c_str()));

        UUID result;
        status = UuidFromStringW(rpc_string, &result);
        if (status != RPC_S_OK)
        {
            throw std::runtime_error("An error occurred parsing the UUID.");
        }
#else
        uuid_t result;
        int status_code = uuid_parse(value.c_str(), result);
        if (status_code != 0)
        {
            throw std::runtime_error("An error occurred parsing the UUID.");
        }
#endif

        return result;
    }

    bool __cdecl uuid_equal(const utility::uuid& value1, const utility::uuid& value2)
    {
#ifdef WIN32
        return value1 == value2;
#else
        return uuid_compare(value1, value2) == 0;
#endif
    }

} // namespace utility
