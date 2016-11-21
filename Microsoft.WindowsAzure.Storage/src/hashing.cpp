// -----------------------------------------------------------------------------------------
// <copyright file="hashing.cpp" company="Microsoft">
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
#include "wascore/hashing.h"

namespace azure { namespace storage { namespace core {

#ifdef _WIN32

    cryptography_hash_algorithm::cryptography_hash_algorithm(LPCWSTR algorithm_id, ULONG flags)
    {
        NTSTATUS status = BCryptOpenAlgorithmProvider(&m_algorithm_handle, algorithm_id, NULL, flags);
        if (status != 0)
        {
            throw utility::details::create_system_error(status);
        }
    }

    cryptography_hash_algorithm::~cryptography_hash_algorithm()
    {
        BCryptCloseAlgorithmProvider(m_algorithm_handle, 0);
    }

    hmac_sha256_hash_algorithm hmac_sha256_hash_algorithm::m_instance;

    md5_hash_algorithm md5_hash_algorithm::m_instance;

    cryptography_hash_provider_impl::cryptography_hash_provider_impl(const cryptography_hash_algorithm& algorithm, const std::vector<uint8_t>& key)
    {
        DWORD hash_object_size = 0;
        DWORD data_length = 0;
        NTSTATUS status = BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, (PBYTE)&hash_object_size, sizeof(DWORD), &data_length, 0);
        if (status != 0)
        {
            throw utility::details::create_system_error(status);
        }

        m_hash_object.resize(hash_object_size);
        status = BCryptCreateHash(algorithm, &m_hash_handle, (PUCHAR)m_hash_object.data(), (ULONG)m_hash_object.size(), (PUCHAR)key.data(), (ULONG)key.size(), 0);
        if (status != 0)
        {
            throw utility::details::create_system_error(status);
        }
    }

    cryptography_hash_provider_impl::~cryptography_hash_provider_impl()
    {
        BCryptDestroyHash(m_hash_handle);
    }

    void cryptography_hash_provider_impl::write(const uint8_t* data, size_t count)
    {
        NTSTATUS status = BCryptHashData(m_hash_handle, (PBYTE)data, (ULONG)count, 0);
        if (status != 0)
        {
            throw utility::details::create_system_error(status);
        }
    }

    void cryptography_hash_provider_impl::close()
    {
        DWORD hash_length = 0;
        DWORD data_length = 0;
        NTSTATUS status = BCryptGetProperty(m_hash_handle, BCRYPT_HASH_LENGTH, (PBYTE)&hash_length, sizeof(DWORD), &data_length, 0);
        if (status != 0)
        {
            throw utility::details::create_system_error(status);
        }

        m_hash.resize(hash_length);

        status = BCryptFinishHash(m_hash_handle, m_hash.data(), (ULONG)m_hash.size(), 0);

        // Don't throw an exception if status is success (0), or if the hash was already closed (0x c000 0008)
        if ((status != 0) && (status != 0xc0000008))
        {
            throw utility::details::create_system_error(status);
        }
    }

    hmac_sha256_hash_provider_impl::hmac_sha256_hash_provider_impl(const std::vector<uint8_t>& key)
        : cryptography_hash_provider_impl(hmac_sha256_hash_algorithm::instance(), key)
    {
    }

    md5_hash_provider_impl::md5_hash_provider_impl()
        : cryptography_hash_provider_impl(md5_hash_algorithm::instance(), std::vector<uint8_t>())
    {
    }

#else // Linux

    hmac_sha256_hash_provider_impl::hmac_sha256_hash_provider_impl(const std::vector<uint8_t>& key)
    {
        HMAC_CTX_init(&m_hash_context);
        HMAC_Init_ex(&m_hash_context, &key[0], (int) key.size(), EVP_sha256(), NULL);
    }

    void hmac_sha256_hash_provider_impl::write(const uint8_t* data, size_t count)
    {
        HMAC_Update(&m_hash_context, data, count);
    }

    void hmac_sha256_hash_provider_impl::close()
    {
        unsigned int length = SHA256_DIGEST_LENGTH;
        m_hash.resize(length);
        HMAC_Final(&m_hash_context, &m_hash[0], &length);
        HMAC_CTX_cleanup(&m_hash_context);
    }

    md5_hash_provider_impl::md5_hash_provider_impl()
    {
        MD5_Init(&m_hash_context);
    }

    void md5_hash_provider_impl::write(const uint8_t* data, size_t count)
    {
        MD5_Update(&m_hash_context, data, count);
    }

    void md5_hash_provider_impl::close()
    {
        m_hash.resize(MD5_DIGEST_LENGTH);
        MD5_Final(m_hash.data(), &m_hash_context);
    }

#endif

}}} // namespace azure::storage::core

