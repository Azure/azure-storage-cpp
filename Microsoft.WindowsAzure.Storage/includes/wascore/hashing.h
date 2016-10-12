// -----------------------------------------------------------------------------------------
// <copyright file="hashing.h" company="Microsoft">
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

#include "cpprest/streams.h"

#include "wascore/basic_types.h"
#include "was/core.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif
#define NOMINMAX
#include <Windows.h>
#include <BCrypt.h>
#include <winsock2.h>
#else
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/hmac.h>
#endif

namespace azure { namespace storage { namespace core {

    class hash_provider_impl
    {
    public:
        virtual ~hash_provider_impl()
        {
        }

        virtual bool is_enabled() const = 0;
        virtual void write(const uint8_t* data, size_t count) = 0;
        virtual void close() = 0;
        virtual utility::string_t hash() const = 0;
    };

#ifdef _WIN32

    class cryptography_hash_algorithm
    {
    public:
        ~cryptography_hash_algorithm();

        operator BCRYPT_ALG_HANDLE() const
        {
            return m_algorithm_handle;
        }

    protected:
        cryptography_hash_algorithm(LPCWSTR algorithm_id, ULONG flags);

    private:
        BCRYPT_ALG_HANDLE m_algorithm_handle;
    };

    class hmac_sha256_hash_algorithm : public cryptography_hash_algorithm
    {
    public:
        static const hmac_sha256_hash_algorithm& instance()
        {
            return m_instance;
        }

    private:
        hmac_sha256_hash_algorithm()
            : cryptography_hash_algorithm(BCRYPT_SHA256_ALGORITHM, BCRYPT_ALG_HANDLE_HMAC_FLAG)
        {
        }

        static hmac_sha256_hash_algorithm m_instance;
    };

    class md5_hash_algorithm : public cryptography_hash_algorithm
    {
    public:
        static const md5_hash_algorithm& instance()
        {
            return m_instance;
        }

    private:
        md5_hash_algorithm()
            : cryptography_hash_algorithm(BCRYPT_MD5_ALGORITHM, 0)
        {
        }

        static md5_hash_algorithm m_instance;
    };

    class cryptography_hash_provider_impl : public hash_provider_impl
    {
    public:
        cryptography_hash_provider_impl(const cryptography_hash_algorithm& algorithm, const std::vector<uint8_t>& key);
        ~cryptography_hash_provider_impl() override;

        bool is_enabled() const override
        {
            return true;
        }

        void write(const uint8_t* data, size_t count) override;
        void close() override;

        utility::string_t hash() const override
        {
            return utility::conversions::to_base64(m_hash);
        }

    private:
        std::vector<uint8_t> m_hash_object;
        BCRYPT_HASH_HANDLE m_hash_handle;
        std::vector<uint8_t> m_hash;
    };

    class hmac_sha256_hash_provider_impl : public cryptography_hash_provider_impl
    {
    public:
        hmac_sha256_hash_provider_impl(const std::vector<uint8_t>& key);
    };

    class md5_hash_provider_impl : public cryptography_hash_provider_impl
    {
    public:
        md5_hash_provider_impl();
    };

#else // Linux

    class cryptography_hash_provider_impl : public hash_provider_impl
    {
    public:
        ~cryptography_hash_provider_impl() override
        {
        }

        bool is_enabled() const override
        {
            return true;
        }

        utility::string_t hash() const override
        {
            return utility::conversions::to_base64(m_hash);
        }

    protected:
        std::vector<uint8_t> m_hash;
    };

    class hmac_sha256_hash_provider_impl : public cryptography_hash_provider_impl
    {
    public:
        hmac_sha256_hash_provider_impl(const std::vector<uint8_t>& key);

        void write(const uint8_t* data, size_t count) override;
        void close() override;

    private:
        HMAC_CTX m_hash_context;
    };

    class md5_hash_provider_impl : public cryptography_hash_provider_impl
    {
    public:
        md5_hash_provider_impl();

        void write(const uint8_t* data, size_t count) override;
        void close() override;

    private:
        MD5_CTX m_hash_context;
    };

#endif

    class null_hash_provider_impl : public hash_provider_impl
    {
    public:
        bool is_enabled() const override
        {
            return false;
        }

        void write(const uint8_t* data, size_t count) override
        {
            // no-op
            UNREFERENCED_PARAMETER(data);
            UNREFERENCED_PARAMETER(count);
        }

        void close() override
        {
            // no-op
        }

        utility::string_t hash() const override
        {
            return utility::string_t();
        }
    };

    class hash_provider
    {
    public:
        hash_provider()
            : m_implementation(std::make_shared<null_hash_provider_impl>())
        {
        }

        bool is_enabled() const
        {
            return m_implementation->is_enabled();
        }

        void write(const uint8_t* data, size_t count)
        {
            m_implementation->write(data, count);
        }

        void close()
        {
            m_implementation->close();
        }

        utility::string_t hash() const
        {
            return m_implementation->hash();
        }

        static hash_provider create_hmac_sha256_hash_provider(const std::vector<uint8_t>& key)
        {
            return hash_provider(std::make_shared<hmac_sha256_hash_provider_impl>(key));
        }

        static hash_provider create_md5_hash_provider()
        {
            return hash_provider(std::make_shared<md5_hash_provider_impl>());
        }

    private:
        explicit hash_provider(std::shared_ptr<hash_provider_impl> implementation)
            : m_implementation(implementation)
        {
        }

        // The hash provider implementation is in a separate object so the hash_provider object can be copied yet maintain the
        // same shared state for the hashing progress, and so different implementations can store different state on the heap.
        std::shared_ptr<hash_provider_impl> m_implementation;
    };

}}} // namespace azure::storage::core
