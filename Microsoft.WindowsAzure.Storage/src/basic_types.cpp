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
#include "wascore/resources.h"

namespace utility {

    utility::uuid __cdecl new_uuid()
    {
        uuid result;

#ifdef _WIN32
        RPC_STATUS status;

        status = UuidCreate(&result);
        if (status != RPC_S_OK && status != RPC_S_UUID_LOCAL_ONLY && status != RPC_S_UUID_NO_ADDRESS)
        {
            throw std::runtime_error(azure::storage::protocol::error_create_uuid);
        }
#else
        uuid_generate_random(result.data);
#endif

        return result;
    }

    utility::string_t __cdecl uuid_to_string(const utility::uuid& value)
    {
#ifdef _WIN32
        RPC_STATUS status;

        RPC_WSTR rpc_string;
        status = UuidToStringW(&value, &rpc_string);
        if (status != RPC_S_OK)
        {
            throw std::runtime_error(azure::storage::protocol::error_serialize_uuid);
        }

        std::wstring result(reinterpret_cast<wchar_t*>(rpc_string));

        status = RpcStringFree(&rpc_string);
        if (status != RPC_S_OK)
        {
            throw std::runtime_error(azure::storage::protocol::error_free_uuid);
        }
#else
        char uuid_string[37];
        uuid_unparse_upper(value.data, uuid_string);

        std::string result(uuid_string);
#endif

        return result;
    }

    utility::uuid __cdecl string_to_uuid(const utility::string_t& value)
    {
        uuid result;

#ifdef _WIN32
        RPC_STATUS status;

        RPC_WSTR rpc_string = reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(value.c_str()));

        status = UuidFromStringW(rpc_string, &result);
        if (status != RPC_S_OK)
        {
            throw std::runtime_error(azure::storage::protocol::error_parse_uuid);
        }
#else
        int status_code = uuid_parse(value.c_str(), result.data);
        if (status_code != 0)
        {
            throw std::runtime_error(azure::storage::protocol::error_parse_uuid);
        }
#endif

        return result;
    }

    bool __cdecl uuid_equal(const utility::uuid& value1, const utility::uuid& value2)
    {
#ifdef _WIN32
        return value1 == value2;
#else
        return uuid_compare(value1.data, value2.data) == 0;
#endif
    }

} // namespace utility
