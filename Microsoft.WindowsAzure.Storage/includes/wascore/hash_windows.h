// -----------------------------------------------------------------------------------------
// <copyright file="hash_windows.h" company="Microsoft">
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

#include "basic_types.h"
#include "streambuf.h"

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif

#define NOMINMAX
#include <Windows.h>
#include <BCrypt.h>
#include <winsock2.h>

namespace azure { namespace storage { namespace core {

    class basic_hash_hmac_sha256_streambuf : public basic_hash_streambuf
    {
    public:

        explicit basic_hash_hmac_sha256_streambuf(const std::vector<unsigned char>& key);
        ~basic_hash_hmac_sha256_streambuf();

        pplx::task<void> _close_write();
        pplx::task<int_type> _putc(char_type ch);
        pplx::task<size_t> _putn(const char_type* ptr, size_t count);

    private:

        std::vector<unsigned char> m_hash_object;
        BCRYPT_HASH_HANDLE m_handle;
    };

    class basic_hash_md5_streambuf : public basic_hash_streambuf
    {
    public:

        basic_hash_md5_streambuf();
        ~basic_hash_md5_streambuf();

        pplx::task<void> _close_write();
        pplx::task<int_type> _putc(char_type ch);
        pplx::task<size_t> _putn(const char_type* ptr, size_t count);

    private:

        std::vector<unsigned char> m_hash_object;
        BCRYPT_HASH_HANDLE m_handle;
    };

}}} // namespace azure::storage::core

#endif
