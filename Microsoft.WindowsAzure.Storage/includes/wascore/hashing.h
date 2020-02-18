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
#include "was/crc64.h"

#ifdef _WIN32
#include <BCrypt.h>
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
        virtual checksum hash() const = 0;
    };

    class null_hash_provider_impl : public hash_provider_impl
    {
    public:
        ~null_hash_provider_impl() override
        {
        }

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

        checksum hash() const override
        {
            return checksum(checksum_none);
        }
    };

    class cryptography_hash_provider_impl : public hash_provider_impl
    {
    public:
#ifdef _WIN32
        cryptography_hash_provider_impl(BCRYPT_HANDLE algorithm_handle, const std::vector<uint8_t>& key);
        ~cryptography_hash_provider_impl() override;

        void write(const uint8_t* data, size_t count) override;
        void close() override;
#endif

    protected:
        std::vector<uint8_t> m_hash;

#ifdef _WIN32
    private:
        std::vector<uint8_t> m_hash_object;
        BCRYPT_HASH_HANDLE m_hash_handle;
#endif
    };

    class hmac_sha256_hash_provider_impl : public cryptography_hash_provider_impl
    {
    public:
        explicit hmac_sha256_hash_provider_impl(const std::vector<uint8_t>& key);
        ~hmac_sha256_hash_provider_impl() override;

        bool is_enabled() const override
        {
            return true;
        }

        void write(const uint8_t* data, size_t count) override;
        void close() override;

        checksum hash() const override
        {
            return checksum(checksum_hmac_sha256, utility::conversions::to_base64(m_hash));
        }

    private:
#ifdef _WIN32
        static BCRYPT_ALG_HANDLE algorithm_handle();
#else // Linux
        HMAC_CTX* m_hash_context = nullptr;
#endif
    };

    class md5_hash_provider_impl : public cryptography_hash_provider_impl
    {
    public:
        md5_hash_provider_impl();
        ~md5_hash_provider_impl() override;

        bool is_enabled() const override
        {
            return true;
        }

        void write(const uint8_t* data, size_t count) override;
        void close() override;

        checksum hash() const override
        {
            return checksum(checksum_md5, utility::conversions::to_base64(m_hash));
        }

    private:
#ifdef _WIN32
        static BCRYPT_ALG_HANDLE algorithm_handle();
#else // Linux
        MD5_CTX* m_hash_context = nullptr;
#endif
    };

    class sha256_hash_provider_impl : public cryptography_hash_provider_impl
    {
    public:
        sha256_hash_provider_impl();
        ~sha256_hash_provider_impl() override;

        bool is_enabled() const override
        {
            return true;
        }

        void write(const uint8_t* data, size_t count) override;
        void close() override;

        checksum hash() const override
        {
            return checksum(checksum_sha256, utility::conversions::to_base64(m_hash));
        }

    private:
#ifdef _WIN32
        static BCRYPT_ALG_HANDLE algorithm_handle();
#else // Linux
        SHA256_CTX* m_hash_context = nullptr;
#endif
    };

    class crc64_hash_provider_impl : public hash_provider_impl
    {
    public:
        bool is_enabled() const override
        {
            return true;
        }

        void write(const uint8_t* data, size_t count) override
        {
            m_crc = update_crc64(data, count, m_crc);
        }

        void close() override
        {
        }

        checksum hash() const override
        {
            return checksum(checksum_crc64, m_crc);
        }

    private:
        uint64_t m_crc = INITIAL_CRC64;
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

        checksum hash() const
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

        static hash_provider create_sha256_hash_provider()
        {
            return hash_provider(std::make_shared<sha256_hash_provider_impl>());
        }

        static hash_provider create_crc64_hash_provider()
        {
            return hash_provider(std::make_shared<crc64_hash_provider_impl>());
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
