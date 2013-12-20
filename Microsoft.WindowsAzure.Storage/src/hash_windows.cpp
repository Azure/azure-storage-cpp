// -----------------------------------------------------------------------------------------
// <copyright file="hash_windows.cpp" company="Microsoft">
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
#include "wascore/hash_windows.h"

#ifdef WIN32

namespace wa { namespace storage { namespace core {

    class hash_hmac_sha256_algorithm
    {
    public:

        static const hash_hmac_sha256_algorithm& instance()
        {
            // This is thread-safe in C++11 per ISO C++ Jan 2012 working draft
            // 6.7 Declaration statement [stmt.dcl] para. 4
            static hash_hmac_sha256_algorithm singleton_instance;
            return singleton_instance;
        }

        ~hash_hmac_sha256_algorithm()
        {
            BCryptCloseAlgorithmProvider(m_algorithm, 0);
        }

        operator BCRYPT_ALG_HANDLE() const
        {
            return m_algorithm;
        }

    private:

        hash_hmac_sha256_algorithm()
        {
            NTSTATUS status = BCryptOpenAlgorithmProvider(&m_algorithm, BCRYPT_SHA256_ALGORITHM, NULL, BCRYPT_ALG_HANDLE_HMAC_FLAG);
            if (status != 0)
            {
                throw utility::details::create_system_error(status);
            }
        }

        BCRYPT_ALG_HANDLE m_algorithm;
    };

    class hash_md5_algorithm
    {
    public:

        static const hash_md5_algorithm& instance()
        {
            // This is thread-safe in C++11 per ISO C++ Jan 2012 working draft
            // 6.7 Declaration statement [stmt.dcl] para. 4
            static hash_md5_algorithm singleton_instance;
            return singleton_instance;
        }

        ~hash_md5_algorithm()
        {
            BCryptCloseAlgorithmProvider(m_algorithm, 0);
        }

        operator BCRYPT_ALG_HANDLE() const
        {
            return m_algorithm;
        }

    private:

        hash_md5_algorithm()
        {
            NTSTATUS status = BCryptOpenAlgorithmProvider(&m_algorithm, BCRYPT_MD5_ALGORITHM, NULL, 0);
            if (status != 0)
            {
                throw utility::details::create_system_error(status);
            }
        }

        BCRYPT_ALG_HANDLE m_algorithm;
    };

    basic_hash_hmac_sha256_streambuf::basic_hash_hmac_sha256_streambuf(const std::vector<unsigned char>& key)
    {
        DWORD hash_object_size = 0;
        DWORD data_length = 0;
        NTSTATUS status = BCryptGetProperty(hash_hmac_sha256_algorithm::instance(), BCRYPT_OBJECT_LENGTH, (PBYTE)&hash_object_size, sizeof(DWORD), &data_length, 0);
        if (status != 0)
        {
            throw utility::details::create_system_error(status);
        }

        m_hash_object.resize(hash_object_size);
        status = BCryptCreateHash(hash_hmac_sha256_algorithm::instance(), &m_handle, (PUCHAR)m_hash_object.data(), (ULONG)m_hash_object.size(), (PUCHAR)key.data(), (ULONG)key.size(), 0);
        if (status != 0)
        {
            throw utility::details::create_system_error(status);
        }
    }

    basic_hash_hmac_sha256_streambuf::~basic_hash_hmac_sha256_streambuf()
    {
        BCryptDestroyHash(m_handle);
    }

    pplx::task<void> basic_hash_hmac_sha256_streambuf::_close_write()
    {
        DWORD hash_length = 0;
        DWORD data_length = 0;
        NTSTATUS status = BCryptGetProperty(m_handle, BCRYPT_HASH_LENGTH, (PBYTE)&hash_length, sizeof(DWORD), &data_length, 0);
        if (status != 0)
        {
            store_and_throw(std::make_exception_ptr(utility::details::create_system_error(status)));
        }

        m_hash.resize(hash_length);
        status = BCryptFinishHash(m_handle, m_hash.data(), (ULONG)m_hash.size(), 0);
        if (status != 0)
        {
            store_and_throw(std::make_exception_ptr(utility::details::create_system_error(status)));
        }

        return basic_hash_streambuf::_close_write();
    }

    pplx::task<basic_hash_hmac_sha256_streambuf::int_type> basic_hash_hmac_sha256_streambuf::_putc(basic_hash_hmac_sha256_streambuf::char_type ch)
    {
        return putn(&ch, 1).then([ch] (size_t count) -> basic_hash_hmac_sha256_streambuf::int_type
        {
            return count ? (basic_hash_hmac_sha256_streambuf::int_type)ch : traits::eof();
        });
    }

    pplx::task<size_t> basic_hash_hmac_sha256_streambuf::_putn(const basic_hash_hmac_sha256_streambuf::char_type* ptr, size_t count)
    {
        NTSTATUS status = BCryptHashData(m_handle, (PBYTE)ptr, (ULONG)count, 0);
        if (status != 0)
        {
            store_and_throw(std::make_exception_ptr(utility::details::create_system_error(status)));
        }

        return pplx::task_from_result(count);
    }

    basic_hash_md5_streambuf::basic_hash_md5_streambuf()
    {
        DWORD hash_object_size = 0;
        DWORD data_length = 0;
        NTSTATUS status = BCryptGetProperty(hash_md5_algorithm::instance(), BCRYPT_OBJECT_LENGTH, (PBYTE)&hash_object_size, sizeof(DWORD), &data_length, 0);
        if (status != 0)
        {
            throw utility::details::create_system_error(status);
        }

        m_hash_object.resize(hash_object_size);
        status = BCryptCreateHash(hash_md5_algorithm::instance(), &m_handle, (PUCHAR)m_hash_object.data(), (ULONG)m_hash_object.size(), NULL, 0, 0);
        if (status != 0)
        {
            throw utility::details::create_system_error(status);
        }
    }

    basic_hash_md5_streambuf::~basic_hash_md5_streambuf()
    {
        BCryptDestroyHash(m_handle);
    }

    pplx::task<void> basic_hash_md5_streambuf::_close_write()
    {
        DWORD hash_length = 0;
        DWORD data_length = 0;
        NTSTATUS status = BCryptGetProperty(m_handle, BCRYPT_HASH_LENGTH, (PBYTE)&hash_length, sizeof(DWORD), &data_length, 0);
        if (status != 0)
        {
            store_and_throw(std::make_exception_ptr(utility::details::create_system_error(status)));
        }

        m_hash.resize(hash_length);
        status = BCryptFinishHash(m_handle, m_hash.data(), (ULONG)m_hash.size(), 0);
        if (status != 0)
        {
            store_and_throw(std::make_exception_ptr(utility::details::create_system_error(status)));
        }

        return basic_hash_streambuf::_close_write();
    }

    pplx::task<basic_hash_md5_streambuf::int_type> basic_hash_md5_streambuf::_putc(basic_hash_md5_streambuf::char_type ch)
    {
        return putn(&ch, 1).then([ch] (size_t count) -> basic_hash_md5_streambuf::int_type
        {
            return count ? (basic_hash_md5_streambuf::int_type)ch : traits::eof();
        });
    }

    pplx::task<size_t> basic_hash_md5_streambuf::_putn(const basic_hash_md5_streambuf::char_type* ptr, size_t count)
    {
        NTSTATUS status = BCryptHashData(m_handle, (PBYTE)ptr, (ULONG)count, 0);
        if (status != 0)
        {
            store_and_throw(std::make_exception_ptr(utility::details::create_system_error(status)));
        }

        return pplx::task_from_result(count);
    }

}}} // namespace wa::storage::core

#endif // WIN32
