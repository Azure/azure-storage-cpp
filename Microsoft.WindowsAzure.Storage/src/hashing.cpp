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
    cryptography_hash_provider_impl::cryptography_hash_provider_impl(BCRYPT_HANDLE algorithm_handle, const std::vector<uint8_t>& key)
    {
        DWORD hash_object_size = 0;
        DWORD data_length = 0;
        NTSTATUS status = BCryptGetProperty(algorithm_handle, BCRYPT_OBJECT_LENGTH, (PBYTE)&hash_object_size, sizeof(DWORD), &data_length, 0);
        if (status != 0)
        {
            throw utility::details::create_system_error(status);
        }

        m_hash_object.resize(hash_object_size);
        status = BCryptCreateHash(algorithm_handle, &m_hash_handle, (PUCHAR)m_hash_object.data(), (ULONG)m_hash_object.size(), (PUCHAR)key.data(), (ULONG)key.size(), 0);
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

    BCRYPT_ALG_HANDLE hmac_sha256_hash_provider_impl::algorithm_handle()
    {
        static const BCRYPT_ALG_HANDLE alg_handle = []() {
            BCRYPT_ALG_HANDLE handle;
            NTSTATUS status = BCryptOpenAlgorithmProvider(&handle, BCRYPT_SHA256_ALGORITHM, NULL, BCRYPT_ALG_HANDLE_HMAC_FLAG);
            if (status != 0)
            {
                throw utility::details::create_system_error(status);
            }
            return handle;
        }();

        return alg_handle;
    }

    hmac_sha256_hash_provider_impl::hmac_sha256_hash_provider_impl(const std::vector<uint8_t>& key) : cryptography_hash_provider_impl(algorithm_handle(), key)
    {
    }

    hmac_sha256_hash_provider_impl::~hmac_sha256_hash_provider_impl()
    {
    }

    void hmac_sha256_hash_provider_impl::write(const uint8_t* data, size_t count)
    {
        cryptography_hash_provider_impl::write(data, count);
    }

    void hmac_sha256_hash_provider_impl::close()
    {
        cryptography_hash_provider_impl::close();
    }

    BCRYPT_ALG_HANDLE md5_hash_provider_impl::algorithm_handle()
    {
        static const BCRYPT_ALG_HANDLE alg_handle = []() {
            BCRYPT_ALG_HANDLE handle;
            NTSTATUS status = BCryptOpenAlgorithmProvider(&handle, BCRYPT_MD5_ALGORITHM, NULL, 0);
            if (status != 0)
            {
                throw utility::details::create_system_error(status);
            }
            return handle;
        }();

        return alg_handle;
    }

    md5_hash_provider_impl::md5_hash_provider_impl() : cryptography_hash_provider_impl(algorithm_handle(), std::vector<uint8_t>())
    {
    }

    md5_hash_provider_impl::~md5_hash_provider_impl()
    {
    }

    void md5_hash_provider_impl::write(const uint8_t* data, size_t count)
    {
        cryptography_hash_provider_impl::write(data, count);
    }

    void md5_hash_provider_impl::close()
    {
        cryptography_hash_provider_impl::close();
    }

#else // Linux

    hmac_sha256_hash_provider_impl::hmac_sha256_hash_provider_impl(const std::vector<uint8_t>& key)
    {
    #if OPENSSL_VERSION_NUMBER < 0x10100000L || defined (LIBRESSL_VERSION_NUMBER)
        m_hash_context = (HMAC_CTX*) OPENSSL_malloc(sizeof(*m_hash_context));
        memset(m_hash_context, 0, sizeof(*m_hash_context));
        HMAC_CTX_init(m_hash_context);       
    #else
        m_hash_context = HMAC_CTX_new();
         HMAC_CTX_reset(m_hash_context);
    #endif
        HMAC_Init_ex(m_hash_context, &key[0], (int) key.size(), EVP_sha256(), NULL);
    }

    hmac_sha256_hash_provider_impl::~hmac_sha256_hash_provider_impl()
    {
        if (m_hash_context != nullptr)
        {
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined (LIBRESSL_VERSION_NUMBER)
            OPENSSL_free(m_hash_context);
#else
            HMAC_CTX_free(m_hash_context);
#endif
        }
    }

    void hmac_sha256_hash_provider_impl::write(const uint8_t* data, size_t count)
    {   
        HMAC_Update(m_hash_context, data, count);
    }

    void hmac_sha256_hash_provider_impl::close()
    {
        unsigned int length = SHA256_DIGEST_LENGTH;
        m_hash.resize(length);
        HMAC_Final(m_hash_context, &m_hash[0], &length);
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined (LIBRESSL_VERSION_NUMBER)
        HMAC_CTX_cleanup(m_hash_context);
#endif

    }

    md5_hash_provider_impl::md5_hash_provider_impl()
    {
        m_hash_context =(MD5_CTX*) OPENSSL_malloc(sizeof(MD5_CTX));
        memset(m_hash_context, 0, sizeof(*m_hash_context));
        MD5_Init(m_hash_context);
    }

    md5_hash_provider_impl::~md5_hash_provider_impl()
    {
        if (m_hash_context != nullptr)
        {
            OPENSSL_free(m_hash_context);
        }
    }

    void md5_hash_provider_impl::write(const uint8_t* data, size_t count)
    {
        MD5_Update(m_hash_context, data, count);
    }

    void md5_hash_provider_impl::close()
    {
        m_hash.resize(MD5_DIGEST_LENGTH);
        MD5_Final(m_hash.data(), m_hash_context);
    }

#endif

}}} // namespace azure::storage::core

