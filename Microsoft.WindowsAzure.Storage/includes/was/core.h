// -----------------------------------------------------------------------------------------
// <copyright file="core.h" company="Microsoft">
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

#include "cpprest/http_client.h"

#include "wascore/basic_types.h"
#include "wascore/constants.h"

#pragma push_macro("max")
#undef max

namespace azure { namespace storage {

    class operation_context;

    /// <summary>
    /// Specifies the authentication scheme used to sign HTTP requests.
    /// </summary>
    enum class authentication_scheme
    {
        /// <summary>
        /// Signs HTTP requests using the Shared Key Lite authentication scheme.
        /// </summary>
        shared_key_lite,

        /// <summary>
        /// Signs HTTP requests using the Shared Key authentication scheme.
        /// </summary>
        shared_key
    };

    /// <summary>
    /// Represents a storage service location.
    /// </summary>
    enum class storage_location
    {
        unspecified,

        /// <summary>
        /// Primary storage service location.
        /// </summary>
        primary,

        /// <summary>
        /// Secondary storage service location.
        /// </summary>
        secondary
    };

    /// <summary>
    /// Specifies the location mode used to decide which location the request should be sent to.
    /// </summary>
    enum class location_mode
    {
        unspecified,

        /// <summary>
        /// Requests should always be sent to the primary location.
        /// </summary>
        primary_only,

        /// <summary>
        /// Requests should always be sent to the primary location first. If the request fails, it should be sent to the secondary location.
        /// </summary>
        primary_then_secondary,

        /// <summary>
        /// Requests should always be sent to the secondary location.
        /// </summary>
        secondary_only,

        /// <summary>
        /// Requests should always be sent to the secondary location first. If the request fails, it should be sent to the primary location.
        /// </summary>
        secondary_then_primary,
    };

    /// <summary>
    /// Contains the URIs for both the primary and secondary locations of a Windows Azure Storage resource.
    /// </summary>
    class storage_uri
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_uri" /> class.
        /// </summary>
        storage_uri()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_uri" /> class using the primary endpoint.
        /// </summary>
        /// <param name="primary_uri">The endpoint for the primary location.</param>
        WASTORAGE_API storage_uri(web::http::uri primary_uri);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_uri" /> class using the primary endpoint.
        /// </summary>
        /// <param name="primary_uri">The endpoint for the primary location.</param>
        /// <param name="secondary_uri">The endpoint for the secondary location.</param>
        WASTORAGE_API storage_uri(web::http::uri primary_uri, web::http::uri secondary_uri);

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+,
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_uri" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::storage_uri" /> object.</param>
        storage_uri(storage_uri&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::storage_uri" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::storage_uri" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::storage_uri" /> object with properties set.</returns>
        storage_uri& operator=(storage_uri&& other)
        {
            if (this != &other)
            {
                m_primary_uri = std::move(other.m_primary_uri);
                m_secondary_uri = std::move(other.m_secondary_uri);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets the primary endpoint.
        /// </summary>
        /// <returns>The primary endpoint.</returns>
        const web::http::uri& primary_uri() const
        {
            return m_primary_uri;
        }

        /// <summary>
        /// Gets the secondary endpoint.
        /// </summary>
        /// <returns>The secondary endpoint.</returns>
        const web::http::uri& secondary_uri() const
        {
            return m_secondary_uri;
        }

        /// <summary>
        /// Gets the path portion of the primary endpoint.
        /// </summary>
        /// <returns>A string containing the path portion of the primary endpoint.</returns>
        const utility::string_t& path() const
        {
            return m_primary_uri.path();
        }

        /// <summary>
        /// Gets the endpoint for a specified location.
        /// </summary>
        /// <param name="location">An <see cref="azure::storage::storage_location" /> object.</param>
        /// <returns>The location endpoint.</returns>
        const web::http::uri& get_location_uri(storage_location location)
        {
            switch (location)
            {
            case storage_location::primary:
                return m_primary_uri;

            case storage_location::secondary:
                return m_secondary_uri;

            default:
                throw std::invalid_argument("location");
            }
        }

    private:

        web::http::uri m_primary_uri;
        web::http::uri m_secondary_uri;
    };

    /// <summary>
    /// Represents a set of credentials used to authenticate access to a Windows Azure storage account.
    /// </summary>
    class storage_credentials
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_credentials" /> class.
        /// </summary>
        storage_credentials()
        {
        }

        class account_key_credential
        {
        public:
            account_key_credential(std::vector<uint8_t> account_key = std::vector<uint8_t>()) : m_account_key(std::move(account_key))
            {
            }

        public:
            std::vector<uint8_t> m_account_key;

        private:
            pplx::extensibility::reader_writer_lock_t m_mutex;

            friend class storage_credentials;
        };

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_credentials" /> class with the specified account name and key value.
        /// </summary>
        /// <param name="account_name">A string containing the name of the storage account.</param>
        /// <param name="account_key">A string containing the Base64-encoded account access key.</param>
        storage_credentials(utility::string_t account_name, const utility::string_t& account_key) : m_account_name(std::move(account_name)), m_account_key_credential(std::make_shared<account_key_credential>(utility::conversions::from_base64(account_key)))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_credentials" /> class with the specified account name and key value.
        /// </summary>
        /// <param name="account_name">A string containing the name of the storage account.</param>
        /// <param name="account_key">An array of bytes that represent the account access key.</param>
        storage_credentials(utility::string_t account_name, std::vector<uint8_t> account_key) : m_account_name(std::move(account_name)), m_account_key_credential(std::make_shared<account_key_credential>(std::move(account_key)))
        {
        }

        class sas_credential
        {
        public:
            explicit sas_credential(utility::string_t sas_token) : m_sas_token(std::move(sas_token))
            {
            }

        private:
            utility::string_t m_sas_token;

            friend class storage_credentials;
        };

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_credentials" /> class with the specified shared access signature token.
        /// </summary>
        /// <param name="sas_token">A string containing the shared access signature token.</param>
        explicit storage_credentials(utility::string_t sas_token)
            : storage_credentials(utility::string_t(), sas_credential{sas_token})
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_credentials" /> class with the specified account name and shared access signature token.
        /// </summary>
        /// <param name="account_name">A string containing the name of the storage account.</param>
        /// <param name="sas_token">An <see cref="azure::storage::sas_credential" /> containing shared access signature token.</param>
        storage_credentials(utility::string_t account_name, sas_credential sas_token)
            : m_account_name(std::move(account_name)), m_sas_token(std::move(sas_token.m_sas_token))
        {
            if (m_sas_token.size() >= 1 && m_sas_token.at(0) == _XPLATSTR('?'))
            {
                m_sas_token = m_sas_token.substr(1);
            }

            auto splitted_query = web::uri::split_query(m_sas_token);
            if (!splitted_query.empty())
            {
                splitted_query[protocol::uri_query_sas_api_version] = protocol::header_value_storage_version;
                web::uri_builder builder;
                for (const auto& kv : splitted_query)
                {
                    builder.append_query(kv.first, kv.second, false);
                }
                m_sas_token_with_api_version = builder.query();
            }
        }

        class bearer_token_credential
        {
        public:
            explicit bearer_token_credential(utility::string_t bearer_token = utility::string_t()) : m_bearer_token(std::move(bearer_token))
            {
            }

        public:
            utility::string_t m_bearer_token;

        private:
            pplx::extensibility::reader_writer_lock_t m_mutex;

            friend class storage_credentials;
        };

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_credentials" /> class with the specified bearer token.
        /// </summary>
        /// <param name="token">A <see cref="azure::storage::storage_credentials::bearer_token_credential" /> class containing bearer token.</param>
        template<class T, typename std::enable_if<std::is_same<typename std::decay<T>::type, bearer_token_credential>::value>::type* = nullptr>
        explicit storage_credentials(T&& token) : storage_credentials(utility::string_t(), std::forward<T>(token))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_credentials" /> class with the specified account name and bearer token.
        /// </summary>
        /// <param name="account_name">A string containing the name of the storage account.</param>
        /// <param name="token">A <see cref="azure::storage::storage_credentials::bearer_token_credential" /> class containing bearer token.</param>
        template<class T, typename std::enable_if<std::is_same<typename std::decay<T>::type, bearer_token_credential>::value>::type* = nullptr>
        storage_credentials(utility::string_t account_name, T&& token) : m_account_name(std::move(account_name)), m_bearer_token_credential(std::make_shared<bearer_token_credential>())
        {
            m_bearer_token_credential->m_bearer_token = std::forward<T>(token).m_bearer_token;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_credentials" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::storage_credentials" /> object.</param>
        template<class T, typename std::enable_if<std::is_same<typename std::decay<T>::type, storage_credentials>::value>::type* = nullptr>
        storage_credentials(T&& other)
        {
            *this = std::forward<T>(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::storage_credentials" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::storage_credentials" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::storage_credentials" /> object with properties set.</returns>
        template<class T, typename std::enable_if<std::is_same<typename std::decay<T>::type, storage_credentials>::value>::type* = nullptr>
        storage_credentials& operator=(T&& other)
        {
            if (this != &other)
            {
                m_sas_token = std::forward<T>(other).m_sas_token;
                m_sas_token_with_api_version = std::forward<T>(other).m_sas_token_with_api_version;
                m_account_name = std::forward<T>(other).m_account_name;
                std::atomic_store_explicit(&m_account_key_credential, std::atomic_load_explicit(&other.m_account_key_credential, std::memory_order_acquire), std::memory_order_release);
                auto key_ptr = std::forward<T>(other).m_account_key_credential;
                std::atomic_store_explicit(&m_bearer_token_credential, std::atomic_load_explicit(&other.m_bearer_token_credential, std::memory_order_acquire), std::memory_order_release);
                auto token_ptr = std::forward<T>(other).m_bearer_token_credential;
            }
            return *this;
        }

        /// <summary>
        /// Transforms a resource URI into a shared access signature URI, by appending a shared access token.
        /// </summary>
        /// <param name="resource_uri">A <see cref="web::http::uri" /> object that represents the resource URI to be transformed.</param>
        /// <returns>A <see cref="web::http::uri" /> object that represents the signature, including the resource URI and the shared access token.</returns>
        web::http::uri transform_uri(const web::http::uri& resource_uri) const
        {
            // TODO: Consider modifying the parameter instead of returning a new URI so a copy doesn't need to be made

            if (is_sas() && !resource_uri.is_empty())
            {
                return web::http::uri_builder(resource_uri).append_query(m_sas_token_with_api_version).to_uri();
            }

            return resource_uri;
        }

        /// <summary>
        /// Gets the shared access signature token associated with the credentials.
        /// </summary>
        /// <returns>The shared access signature token.</returns>
        const utility::string_t& sas_token() const
        {
            return m_sas_token;
        }

        /// <summary>
        /// Gets the associated storage account name for the credentials.
        /// </summary>
        /// <returns>The storage account name.</returns>
        const utility::string_t& account_name() const
        {
            return m_account_name;
        }

        /// <summary>
        /// Returns the key for the credentials.
        /// </summary>
        /// <returns>An array of bytes that contains the key.</returns>
        const std::vector<uint8_t>& account_key() const
        {
            auto account_key_ptr = std::atomic_load_explicit(&m_account_key_credential, std::memory_order_acquire);
            pplx::extensibility::scoped_read_lock_t guard(account_key_ptr->m_mutex);
            return account_key_ptr->m_account_key;
        }

        /// <summary>
        /// Sets the account key for the credentials.
        /// </summary>
        /// <param name="account_key">A string containing the Base64-encoded account access key.</param>
        void set_account_key(const utility::string_t& account_key)
        {
            set_account_key(utility::conversions::from_base64(account_key));
        }

        /// <summary>
        /// Sets the account key for the credentials.
        /// </summary>
        /// <param name="account_key">An array of bytes that represent the account access key.</param>
        void set_account_key(std::vector<uint8_t> account_key)
        {
            auto account_key_ptr = std::atomic_load_explicit(&m_account_key_credential, std::memory_order_acquire);
            if (!account_key_ptr)
            {
                auto new_credential = std::make_shared<account_key_credential>();
                new_credential->m_account_key = std::move(account_key);
                /* Compares m_account_key_credential and account_key_ptr(nullptr).
                 * If they are equivalent, assigns new_credential into m_account_key_credential and returns true.
                 * If they are not equivalent, assigns m_account_key_credential into m_account_key and returns false.
                 */
                bool set = std::atomic_compare_exchange_strong_explicit(&m_account_key_credential, &account_key_ptr, new_credential, std::memory_order_release, std::memory_order_acquire);
                if (set) {
                    return;
                }
                account_key = std::move(new_credential->m_account_key);
            }
            pplx::extensibility::scoped_rw_lock_t guard(account_key_ptr->m_mutex);
            account_key_ptr->m_account_key = std::move(account_key);
        }

        /// <summary>
        /// Gets the bearer token for the credentials.
        /// </summary>
        /// <returns>The bearer token</returns>
        utility::string_t bearer_token() const
        {
            auto token_ptr = std::atomic_load_explicit(&m_bearer_token_credential, std::memory_order_acquire);
            pplx::extensibility::scoped_read_lock_t guard(token_ptr->m_mutex);
            return token_ptr->m_bearer_token;
        }

        /// <summary>
        /// Sets the bearer token for the credentials.
        /// </summary>
        /// <param name="bearer_token">A string that contains bearer token.</param>
        void set_bearer_token(utility::string_t bearer_token)
        {
            auto token_ptr = std::atomic_load_explicit(&m_bearer_token_credential, std::memory_order_acquire);
            if (!token_ptr)
            {
                auto new_credential = std::make_shared<bearer_token_credential>();
                new_credential->m_bearer_token = std::move(bearer_token);
                /* Compares m_bearer_token_credential and token_ptr(nullptr).
                 * If they are equivalent, assigns new_credential into m_bearer_token_credential and returns true.
                 * If they are not equivalent, assigns m_bearer_token_credential into token_ptr and returns false.
                 */
                bool set = std::atomic_compare_exchange_strong_explicit(&m_bearer_token_credential, &token_ptr, new_credential, std::memory_order_release, std::memory_order_acquire);
                if (set) {
                    return;
                }
                bearer_token = std::move(new_credential->m_bearer_token);
            }
            pplx::extensibility::scoped_rw_lock_t guard(token_ptr->m_mutex);
            token_ptr->m_bearer_token = std::move(bearer_token);
        }

        /// <summary>
        /// Indicates whether the credentials are for anonymous access.
        /// </summary>
        /// <returns><c>true</c> if the credentials are for anonymous access; otherwise, <c>false</c>.</returns>
        bool is_anonymous() const
        {
            return m_sas_token.empty() && !is_account_key() && !is_bearer_token();
        }

        /// <summary>
        /// Indicates whether the credentials are a shared access signature token.
        /// </summary>
        /// <returns><c>true</c> if the credentials are a shared access signature token; otherwise, <c>false</c>.</returns>
        bool is_sas() const
        {
            return !m_sas_token.empty() && !is_account_key() && !is_bearer_token();
        }

        /// <summary>
        /// Indicates whether the credentials are a shared key.
        /// </summary>
        /// <returns><c>true</c> if the credentials are a shared key; otherwise, <c>false</c>.</returns>
        bool is_shared_key() const
        {
            return m_sas_token.empty() && is_account_key() && !is_bearer_token();
        }

        /// <summary>
        /// Indicates whether the credentials are a bearer token.
        /// </summary>
        /// <returns><c>true</c> if the credentials are a bearer token; otherwise <c>false</c>.</returns>
        bool is_bearer_token() const {
            auto token_ptr = std::atomic_load_explicit(&m_bearer_token_credential, std::memory_order_acquire);
            if (!token_ptr)
            {
                return false;
            }
            pplx::extensibility::scoped_read_lock_t guard(token_ptr->m_mutex);
            return !token_ptr->m_bearer_token.empty();
        }

        /// <summary>
        /// Indicates whether the credentials are an account key.
        /// </summary>
        /// <returns><c>true</c> if the credentials are an account key; otherwise <c>false</c>.</returns>
        bool is_account_key() const {
            auto account_key_ptr = std::atomic_load_explicit(&m_account_key_credential, std::memory_order_acquire);
            if (!account_key_ptr)
            {
                return false;
            }
            pplx::extensibility::scoped_read_lock_t guard(account_key_ptr->m_mutex);
            return !account_key_ptr->m_account_key.empty();
        }

    private:

        utility::string_t m_sas_token;
        utility::string_t m_sas_token_with_api_version;
        utility::string_t m_account_name;
        // We use std::atomic_{load/store/...} functions specialized for std::shared_ptr<T> to access this member, since std::atomic<std::shared_ptr<T>> is not available until C++20.
        // These become deprecated since C++20, but still compile.
        std::shared_ptr<account_key_credential> m_account_key_credential;
        std::shared_ptr<bearer_token_credential> m_bearer_token_credential;
    };

    enum class checksum_type
    {
        none,
        md5,
        sha256,
        crc64,
        hmac_sha256,
    };

    using checksum_none_t = std::integral_constant<checksum_type, checksum_type::none>;
    using checksum_md5_t = std::integral_constant<checksum_type, checksum_type::md5>;
    using checksum_sha256_t = std::integral_constant<checksum_type, checksum_type::sha256>;
    using checksum_crc64_t = std::integral_constant<checksum_type, checksum_type::crc64>;
    using checksum_hmac_sha256_t = std::integral_constant<checksum_type, checksum_type::hmac_sha256>;

    constexpr auto checksum_none = checksum_none_t();
    constexpr auto checksum_md5 = checksum_md5_t();
    constexpr auto checksum_sha256 = checksum_sha256_t();
    constexpr auto checksum_crc64 = checksum_crc64_t();
    constexpr auto checksum_hmac_sha256 = checksum_hmac_sha256_t();

    /// <summary>
    /// Represents a checksum to verify the integrity of data during transport.
    /// </summary>
    class checksum
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::checksum" /> class without specifying any checksum method.
        /// </summary>
        checksum() : checksum(checksum_none)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::checksum" /> class with MD5 hash value.
        /// </summary>
        /// <param name="md5">A string containing base64-encoded MD5.</param>
        /// <remarks>
        /// If the provided string is empty, this class is initialized as if checksum method isn't specified.
        /// </remarks>
        checksum(utility::string_t md5) : m_type(checksum_type::md5), m_str_hash(std::move(md5))
        {
            if (m_str_hash.empty())
            {
                m_type = checksum_type::none;
            }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::checksum" /> class with MD5 hash value.
        /// </summary>
        /// <param name="md5">A string containing base64-encoded MD5.</param>
        /// <remarks>
        /// If the provided string is empty, this class is initialized as if checksum method isn't specified.
        /// </remarks>
        checksum(const utility::char_t* md5) : checksum(utility::string_t(md5))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::checksum" /> class with CRC64 error-detecting code.
        /// </summary>
        /// <param name="crc64">An integer containing CRC64 code.</param>
        checksum(uint64_t crc64) : m_type(checksum_type::crc64), m_crc64(crc64)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::checksum" /> class without specifying any checksum method.
        /// </summary>
        checksum(checksum_none_t type) : m_type(type.value)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::checksum" /> class with MD5 hash value.
        /// </summary>
        /// <param name="type">Explicitly specified checksum type, must be <see cref="azure::storage::checksum_md5" />.</param>
        /// <param name="val">A string containing base64-encoded MD5.</param>
        checksum(checksum_md5_t type, utility::string_t val) : m_type(type.value), m_str_hash(std::move(val))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::checksum" /> class with SHA-256 hash value.
        /// </summary>
        /// <param name="type">Explicitly specified checksum type, must be <see cref="azure::storage::checksum_sha256" />.</param>
        /// <param name="val">A string containing base64-encoded SHA-256.</param>
        checksum(checksum_sha256_t type, utility::string_t val) : m_type(type.value), m_str_hash(std::move(val))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::checksum" /> class with CRC64 error-detecting code.
        /// </summary>
        /// <param name="type">Explicitly specified checksum type, must be <see cref="azure::storage::checksum_crc64" />.</param>
        /// <param name="val">An integer containing CRC64 code.</param>
        checksum(checksum_crc64_t type, uint64_t val) : m_type(type.value), m_crc64(val)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::checksum" /> class with HMAC-SHA256 authentication code.
        /// </summary>
        /// <param name="type">Explicitly specified checksum type, must be <see cref="azure::storage::checksum_hmac_sha256" />.</param>
        /// <param name="val">A string containing base64-encoded HMAC-SHA256 authentication code.</param>
        checksum(checksum_hmac_sha256_t type, utility::string_t val) : m_type(type.value), m_str_hash(std::move(val))
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+,
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::checksum" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::checksum" /> object.</param>
        checksum(checksum&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::checksum" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::checksum" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::checksum" /> object with properties set.</returns>
        checksum& operator=(checksum&& other)
        {
            if (this != &other)
            {
                m_type = std::move(other.m_type);
                m_str_hash = std::move(other.m_str_hash);
                m_crc64 = std::move(other.m_crc64);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Indicates whether this is an MD5 checksum.
        /// </summary>
        /// <returns><c>true</c> if this is an MD5 checksum; otherwise, <c>false</c>.</returns>
        bool is_md5() const
        {
            return m_type == checksum_type::md5;
        }

        /// <summary>
        /// Indicates whether this is an SHA-256 checksum.
        /// </summary>
        /// <returns><c>true</c> if this is an SHA-256 checksum; otherwise, <c>false</c>.</returns>
        bool is_sha256() const
        {
            return m_type == checksum_type::sha256;
        }

        /// <summary>
        /// Indicates whether this is an HMAC-SHA256 authentication code.
        /// </summary>
        /// <returns><c>true</c> if this is an HMAC-SHA256 authentication code; otherwise, <c>false</c>.</returns>
        bool is_hmac_sha256() const
        {
            return m_type == checksum_type::hmac_sha256;
        }

        /// <summary>
        /// Indicates whether this is a CRC64 checksum.
        /// </summary>
        /// <returns><c>true</c> if this is a CRC64 checksum; otherwise, <c>false</c>.</returns>
        bool is_crc64() const
        {
            return m_type == checksum_type::crc64;
        }

        /// <summary>
        /// Indicates whether any checksum method is used.
        /// </summary>
        /// <returns><c>true</c> if no checksum method is used; otherwise, <c>false</c>.</returns>
        bool empty() const
        {
            return m_type == checksum_type::none;
        }

        /// <summary>
        /// Gets the MD5 checksum.
        /// </summary>
        /// <returns>A string containing base64-encoded MD5.</returns>
        const utility::string_t& md5() const
        {
            return m_str_hash;
        }

        /// <summary>
        /// Gets the SHA-256 checksum.
        /// </summary>
        /// <returns>A string containing base64-encoded SHA-256.</returns>
        const utility::string_t& sha256() const
        {
            return m_str_hash;
        }

        /// <summary>
        /// Gets the HMAC-256 authentication code.
        /// </summary>
        /// <returns>A string containing base64-encoded HMAC-256 authentiction code.</returns>
        const utility::string_t& hmac_sha256() const
        {
            return m_str_hash;
        }

        /// <summary>
        /// Gets the CRC64 error-detecting code.
        /// </summary>
        /// <returns>A string containing base64-encoded CRC64 error-detecting code.</returns>
        utility::string_t crc64() const
        {
            std::vector<uint8_t> crc64_str(sizeof(m_crc64) / sizeof(uint8_t));
            memcpy(crc64_str.data(), &m_crc64, sizeof(m_crc64));
            return utility::conversions::to_base64(crc64_str);
        }

    private:
        checksum_type m_type;
        utility::string_t m_str_hash;
        uint64_t m_crc64;
    };

    /// <summary>
    /// Represents an option on a request.
    /// </summary>
    template<typename T>
    class option_with_default
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::option_with_default" /> class.
        /// </summary>
        option_with_default()
            : m_has_value(false)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::option_with_default" /> class.
        /// </summary>
        /// <param name="default_value">The default_value.</param>
        option_with_default(const T& default_value)
            : m_value(default_value), m_has_value(false)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+,
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::option_with_default" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::option_with_default" /> object.</param>
        option_with_default(option_with_default&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::option_with_default" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::option_with_default" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::option_with_default" /> object with properties set.</returns>
        option_with_default& operator=(option_with_default&& other)
        {
            if (this != &other)
            {
                m_value = std::move(other.m_value);
                m_has_value = std::move(other.m_has_value);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Returns a reference to a request option set to the specified value.
        /// </summary>
        /// <param name="value">The option value.</param>
        /// <returns>An <see cref="azure::storage::option_with_default" /> object.</returns>
        option_with_default<T>& operator=(const T& value)
        {
            m_value = value;
            m_has_value = true;
            return *this;
        }

        /// <summary>
        /// Implements the operator.
        /// </summary>
        /// <returns>The result of the operator.</returns>
        operator const T&() const
        {
            return m_value;
        }

        /// <summary>
        /// Merges the specified value.
        /// </summary>
        /// <param name="value">The value.</param>
        void merge(const T& value)
        {
            if (!m_has_value)
            {
                *this = value;
            }
        }

        /// <summary>
        /// Merges the specified value.
        /// </summary>
        /// <param name="value">The value.</param>
        void merge(const option_with_default<T>& value)
        {
            if (!m_has_value)
            {
                *this = value;
                this->m_has_value = value.has_value();
            }
        }

        /// <summary>
        /// Merges the specified value.
        /// </summary>
        /// <param name="value">The value.</param>
        /// <param name="fallback_value">The fallback value.</param>
        void merge(const option_with_default<T>& value, const T& fallback_value)
        {
            merge(value.m_has_value ? (const T&)value : fallback_value);
        }

        /// <summary>
        /// Indicates whether a specified value is set.
        /// </summary>
        /// <returns>A boolean indicating whether a specified value is set.</retruns>
        bool has_value() const
        {
            return m_has_value;
        }

    private:

        T m_value;
        bool m_has_value;
    };

    /// <summary>
    /// Provides extended error information for a storage services error.
    /// </summary>
    class storage_extended_error
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_extended_error" /> class.
        /// </summary>
        storage_extended_error()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_extended_error" /> class.
        /// </summary>
        /// <param name="code">The error code.</param>
        /// <param name="message">The error message.</param>
        /// <param name="details">The error details.</param>
        storage_extended_error(utility::string_t code, utility::string_t message, std::unordered_map<utility::string_t, utility::string_t> details)
            : m_code(std::move(code)),
            m_message(std::move(message)),
            m_details(std::move(details))
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+,
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_extended_error" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::storage_extended_error" /> object.</param>
        storage_extended_error(storage_extended_error&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::storage_extended_error" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::storage_extended_error" /> object  to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::storage_extended_error" /> object with properties set.</returns>
        storage_extended_error& operator=(storage_extended_error&& other)
        {
            if (this != &other)
            {
                m_code = std::move(other.m_code);
                m_message = std::move(other.m_message);
                m_details = std::move(other.m_details);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets the error code.
        /// </summary>
        /// <returns>A string containing the error code.</returns>
        const utility::string_t& code() const
        {
            return m_code;
        }

        /// <summary>
        /// Gets the error message.
        /// </summary>
        /// <returns>A string containing the error message.</returns>
        const utility::string_t& message() const
        {
            return m_message;
        }

        /// <summary>
        /// Gets the error details.
        /// </summary>
        /// <returns>A string containing the error details.</returns>
        const std::unordered_map<utility::string_t, utility::string_t>& details() const
        {
            return m_details;
        }

    private:

        utility::string_t m_code;
        utility::string_t m_message;
        std::unordered_map<utility::string_t, utility::string_t> m_details;
    };

    /// <summary>
    /// Represents a result returned by a request.
    /// </summary>
    class request_result
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::request_result" /> class.
        /// </summary>
        request_result()
            : m_is_response_available(false),
            m_target_location(storage_location::unspecified),
            m_http_status_code(0),
            m_content_length(std::numeric_limits<utility::size64_t>::max()),
            m_request_server_encrypted(false)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::request_result" /> class.
        /// </summary>
        /// <param name="start_time">The start time of the request.</param>
        /// <param name="target_location">The target location for the request.</param>
        request_result(utility::datetime start_time, storage_location target_location)
            : m_is_response_available(false),
            m_start_time(start_time),
            m_target_location(target_location),
            m_end_time(utility::datetime::utc_now()),
            m_http_status_code(0),
            m_content_length(std::numeric_limits<utility::size64_t>::max()),
            m_request_server_encrypted(false)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::request_result" /> class.
        /// </summary>
        /// <param name="start_time">The start time of the request.</param>
        /// <param name="target_location">The target location for the request.</param>
        /// <param name="response">The HTTP response to read.</param>
        /// <param name="parse_body_as_error">A flag that indicates whether to parse error data from the response body.</param>
        WASTORAGE_API request_result(utility::datetime start_time, storage_location target_location, const web::http::http_response& response, bool parse_body_as_error);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::request_result" /> class.
        /// </summary>
        /// <param name="start_time">The start time of the request.</param>
        /// <param name="target_location">The target location for the request.</param>
        /// <param name="response">The HTTP response to read.</param>
        /// <param name="http_status_code">The HTTP status code for the request.</param>
        /// <param name="extended_error">The extended error information for the request.</param>
        WASTORAGE_API request_result(utility::datetime start_time, storage_location target_location, const web::http::http_response& response, web::http::status_code http_status_code, storage_extended_error extended_error);

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 r-value reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+,
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::request_result" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::request_result" /> object.</param>
        request_result(request_result&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::request_result" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::request_result" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::request_result" /> object with properties set.</returns>
        request_result& operator=(request_result&& other)
        {
            if (this != &other)
            {
                m_is_response_available = std::move(other.m_is_response_available);
                m_start_time = std::move(other.m_start_time);
                m_target_location = std::move(other.m_target_location);
                m_end_time = std::move(other.m_end_time);
                m_http_status_code = std::move(other.m_http_status_code);
                m_service_request_id = std::move(other.m_service_request_id);
                m_request_date = std::move(other.m_request_date);
                m_content_length = std::move(other.m_content_length);
                m_content_md5 = std::move(other.m_content_md5);
                m_content_crc64 = std::move(other.m_content_crc64);
                m_etag = std::move(other.m_etag);
                m_request_server_encrypted = other.m_request_server_encrypted;
                m_extended_error = std::move(other.m_extended_error);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Indicates whether a response is available for the request.
        /// </summary>
        /// <returns><c>true</c> if a response is available.</returns>
        bool is_response_available() const
        {
            return m_is_response_available;
        }

        /// <summary>
        /// Gets the start time of the request.
        /// </summary>
        /// <returns>The start time of the request.</returns>
        utility::datetime start_time() const
        {
            return m_start_time;
        }

        /// <summary>
        /// Gets the target location for the request.
        /// </summary>
        /// <returns>The target location for the request.</returns>
        storage_location target_location() const
        {
            return m_target_location;
        }

        /// <summary>
        /// Gets the end time of the request.
        /// </summary>
        /// <returns>The end time of the request.</returns>
        utility::datetime end_time() const
        {
            return m_end_time;
        }

        /// <summary>
        /// Gets the HTTP status code for the request.
        /// </summary>
        /// <returns>The HTTP status code for the request.</returns>
        web::http::status_code http_status_code() const
        {
            return m_http_status_code;
        }

        /// <summary>
        /// Sets the HTTP status code for the request.
        /// </summary>
        /// <param name="value">The HTTP status code for the request.</param>
        void set_http_status_code(web::http::status_code value)
        {
            m_http_status_code = value;
        }

        /// <summary>
        /// Gets the service request ID.
        /// </summary>
        /// <returns>The service request ID.</returns>
        const utility::string_t& service_request_id() const
        {
            return m_service_request_id;
        }

        /// <summary>
        /// Gets the service request date.
        /// </summary>
        /// <returns>The service request date.</returns>
        utility::datetime request_date() const
        {
            return m_request_date;
        }

        /// <summary>
        /// Gets the content length for the request.
        /// </summary>
        /// <returns>The content length for the request.</returns>
        utility::size64_t content_length() const
        {
            return m_content_length;
        }

        /// <summary>
        /// Gets the content-MD5 hash for the request.
        /// </summary>
        /// <returns>A string containing the content-MD5 hash for the request.</returns>
        const utility::string_t& content_md5() const
        {
            return m_content_md5;
        }

        /// <summary>
        /// Gets the content-CRC64 hash for the request.
        /// </summary>
        /// <returns>A string containing the content-CRC64 hash for the request.</returns>
        const utility::string_t& content_crc64() const
        {
            return m_content_crc64;
        }

        /// <summary>
        /// Gets the ETag for the request.
        /// </summary>
        /// <returns>The ETag for the request.</returns>
        const utility::string_t& etag() const
        {
            return m_etag;
        }

        /// <summary>
        /// Gets extended error information for the request.
        /// </summary>
        /// <returns>The extended error information for the request.</returns>
        const storage_extended_error& extended_error() const
        {
            return m_extended_error;
        }

        /// <summary>
        /// Sets extended error information for the request.
        /// </summary>
        /// <param name="value">The extended error information for the request.</param>
        void set_extended_error(storage_extended_error value)
        {
            m_extended_error = std::move(value);
        }

    private:

        void parse_headers(const web::http::http_headers& headers);
        void parse_body(const web::http::http_response& response);

        bool m_is_response_available;
        utility::datetime m_start_time;
        storage_location m_target_location;
        utility::datetime m_end_time;
        web::http::status_code m_http_status_code;
        utility::string_t m_service_request_id;
        utility::datetime m_request_date;
        utility::size64_t m_content_length;
        utility::string_t m_content_md5;
        utility::string_t m_content_crc64;
        utility::string_t m_etag;
        bool m_request_server_encrypted;
        storage_extended_error m_extended_error;
    };

    /// <summary>
    /// Represents a Windows Azure Storage exception.
    /// </summary>
    class storage_exception : public std::runtime_error
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_exception" /> class.
        /// </summary>
        /// <param name="message">The error message.</param>
        /// <param name="retryable">Indicates whether the request is retryable.</param>
        storage_exception(const std::string& message, bool retryable = true)
            : std::runtime_error(message), m_retryable(retryable)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_exception" /> class.
        /// </summary>
        /// <param name="message">The error message.</param>
        /// <param name="inner_exception">A <see cref="std::exception_ptr" /> object containing the inner exception.</param>
        /// <param name="retryable">Indicates whether the request is retryable.</param>
        storage_exception(const std::string& message, std::exception_ptr inner_exception, bool retryable = true)
            : std::runtime_error(message), m_retryable(retryable), m_inner_exception(inner_exception)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_exception" /> class.
        /// </summary>
        /// <param name="message">The error message.</param>
        /// <param name="result">The request result.</param>
        /// <param name="retryable">Indicates whether the request is retryable.</param>
        storage_exception(const std::string& message, request_result result, bool retryable = true)
            : std::runtime_error(message), m_result(std::move(result)), m_retryable(retryable)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::storage_exception" /> class.
        /// </summary>
        /// <param name="message">The error message.</param>
        /// <param name="result">The request result.</param>
        /// <param name="inner_exception">A <see cref="std::exception_ptr" /> object containing the inner exception.</param>
        /// <param name="retryable">Indicates whether the request is retryable.</param>
        storage_exception(const std::string& message, request_result result, std::exception_ptr inner_exception, bool retryable = true)
            : std::runtime_error(message), m_result(std::move(result)), m_retryable(retryable), m_inner_exception(inner_exception)
        {
        }

        /// <summary>
        /// Gets the request result.
        /// </summary>
        /// <returns>The request result.</returns>
        const request_result& result() const
        {
            return m_result;
        }

        /// <summary>
        /// Indicates whether the request is retryable.
        /// </summary>
        /// <returns><c>true</c> if the request is retryable.</returns>
        bool retryable() const
        {
            return m_retryable;
        }

        /// <summary>
        /// Gets the inner exception object that is the cause for the current <see cref="azure::storage::storage_exception" />.
        /// </summary>
        /// <returns>
        /// A <see cref="std::exception_ptr" /> object that points to the inner exception associated with the current
        /// <see cref="azure::storage::storage_exception" />. A null exception_ptr is returned if there is no inner exception
        /// object.
        /// </returns>
        std::exception_ptr inner_exception() const
        {
            return m_inner_exception;
        }

    private:

        request_result m_result;
        bool m_retryable;
        std::exception_ptr m_inner_exception;
    };

    /// <summary>
    /// Represents the context for one or more retries of a request made against the Windows Azure storage services,
    /// including the number of retries made for the request, the results of the last request, and the storage location and location mode for subsequent retries.
    /// </summary>
    class retry_context
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::retry_context" /> class.
        /// </summary>
        /// <param name="current_retry_count">The current retry count.</param>
        /// <param name="last_request_result">The last request result.</param>
        /// <param name="next_location">The next location to retry.</param>
        /// <param name="current_location_mode">The current location mode.</param>
        /// <param name="nonstorage_exception">Exception Ptr of any exception other than storage_exception</param>
        retry_context(int current_retry_count, request_result last_request_result, storage_location next_location, location_mode current_location_mode, const std::exception_ptr& nonstorage_exception = nullptr)
            : m_current_retry_count(current_retry_count), m_last_request_result(std::move(last_request_result)), m_next_location(next_location), m_current_location_mode(current_location_mode), m_nonstorage_exception(nonstorage_exception)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+,
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::retry_context" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::retry_context" /> object.</param>
        retry_context(retry_context&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::retry_context" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::retry_context" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::retry_context" /> object with properties set.</returns>
        retry_context& operator=(retry_context&& other)
        {
            if (this != &other)
            {
                m_current_retry_count = std::move(other.m_current_retry_count);
                m_last_request_result = std::move(other.m_last_request_result);
                m_next_location = std::move(other.m_next_location);
                m_current_location_mode = std::move(other.m_current_location_mode);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets the number of retries for the given operation.
        /// </summary>
        /// <returns>The number of retries for the given operation.</returns>
        int current_retry_count() const
        {
            return m_current_retry_count;
        }

        /// <summary>
        /// Gets the results of the last request.
        /// </summary>
        /// <returns>An <see cref="azure::storage::request_result" /> object that represents the results of the last request.</returns>
        const request_result& last_request_result() const
        {
            return m_last_request_result;
        }

        /// <summary>
        /// Gets the target location for the next retry.
        /// </summary>
        /// <returns>The <see cref="azure::storage::storage_location" /> for the next retry.</returns>
        storage_location next_location() const
        {
            return m_next_location;
        }

        /// <summary>
        /// Gets the location mode for subsequent retries.
        /// </summary>
        /// <returns>The <see cref="azure::storage::location_mode" /> for subsequent retries.</returns>
        location_mode current_location_mode() const
        {
            return m_current_location_mode;
        }

        /// <summary>
        /// Gets the exception_ptr of any unhandled nonstorage exception during the request.
        /// Example: WinHttp exceptions for timeout (12002)
        /// </summary>
        /// <returns>An <see cref="std::exception_ptr" /> object that represents the nonstorage exception thrown while sending request</returns>
        const std::exception_ptr& nonstorage_exception() const {
            return m_nonstorage_exception;
        }

    private:

        int m_current_retry_count;
        request_result m_last_request_result;
        storage_location m_next_location;
        location_mode m_current_location_mode;
        std::exception_ptr m_nonstorage_exception;
    };

    /// <summary>
    /// Specifies parameters for the next retry of a request to be made against the Windows Azure storage services,
    /// including the target location and location mode for the next retry and the interval until the next retry.
    /// </summary>
    class retry_info
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::retry_info" /> class.
        /// </summary>
        retry_info()
            : m_should_retry(false), m_target_location(storage_location::unspecified),
            m_updated_location_mode(location_mode::unspecified), m_retry_interval()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::retry_info" /> class.
        /// </summary>
        /// <param name="context">The <see cref="azure::storage::retry_context" /> object that was passed in to the retry policy.</param>
        explicit retry_info(const retry_context& context)
            : m_should_retry(true), m_target_location(context.next_location()),
            m_updated_location_mode(context.current_location_mode()), m_retry_interval(protocol::default_retry_interval)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+,
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::retry_info" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::retry_info" /> object.</param>
        retry_info(retry_info&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::retry_info" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::retry_info" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::retry_info" /> object with properties set.</returns>
        retry_info& operator=(retry_info&& other)
        {
            if (this != &other)
            {
                m_should_retry = std::move(other.m_should_retry);
                m_target_location = std::move(other.m_target_location);
                m_updated_location_mode = std::move(other.m_updated_location_mode);
                m_retry_interval = std::move(other.m_retry_interval);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Indicates that the request should be retried.
        /// </summary>
        /// <returns><c>true</c> if the request should be retried.</returns>
        bool should_retry() const
        {
            return m_should_retry;
        }

        /// <summary>
        /// Gets the target location for the next retry.
        /// </summary>
        /// <returns>The target location for the next retry.</returns>
        storage_location target_location() const
        {
            return m_target_location;
        }

        /// <summary>
        /// Sets the target location for the next retry.
        /// </summary>
        /// <param name="value">The target location for the next retry.</param>
        void set_target_location(storage_location value)
        {
            m_target_location = value;
        }

        /// <summary>
        /// Gets the location mode for subsequent retries.
        /// </summary>
        /// <returns>The location mode for subsequent retries.</returns>
        location_mode updated_location_mode() const
        {
            return m_updated_location_mode;
        }

        /// <summary>
        /// Sets the location mode for subsequent retries.
        /// </summary>
        /// <param name="value">The location mode for subsequent retries.</param>
        void set_updated_location_mode(location_mode value)
        {
            m_updated_location_mode = value;
        }

        /// <summary>
        /// Gets the interval until the next retry.
        /// </summary>
        /// <returns>The interval until the next retry.</returns>
        std::chrono::milliseconds retry_interval() const
        {
            return m_retry_interval;
        }

        /// <summary>
        /// Sets the interval until the next retry.
        /// </summary>
        /// <param name="value">The interval until the next retry.</param>
        void set_retry_interval(std::chrono::milliseconds value)
        {
            m_retry_interval = value;
        }

    private:

        bool m_should_retry;
        storage_location m_target_location;
        location_mode m_updated_location_mode;
        std::chrono::milliseconds m_retry_interval;
    };

    class retry_policy;

    /// <summary>
    /// Represents a retry policy.
    /// </summary>
    class basic_retry_policy
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::basic_retry_policy" /> class.
        /// </summary>
        basic_retry_policy()
        {
        }

        virtual retry_info evaluate(const retry_context& retry_context, operation_context context) = 0;
        virtual retry_policy clone() const = 0;
    };

    /// <summary>
    /// Represents a retry policy.
    /// </summary>
    class retry_policy : public basic_retry_policy
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::retry_policy" /> class.
        /// </summary>
        retry_policy()
            : m_policy(nullptr)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::retry_policy" /> class.
        /// </summary>
        /// <param name="ptr">The PTR.</param>
        explicit retry_policy(std::shared_ptr<basic_retry_policy> ptr)
            : m_policy(ptr)
        {
        }

        WASTORAGE_API retry_info evaluate(const retry_context& retry_context, operation_context context) override;

        /// <summary>
        /// Indicates whether the <see cref="azure::storage::retry_policy" /> object is valid.
        /// </summary>
        /// <returns><c>true</c> if the <see cref="azure::storage::retry_policy" /> object is valid; otherwise, <c>false</c>.</returns>
        bool is_valid() const
        {
            return m_policy != nullptr;
        }

        /// <summary>
        /// Clones the retry policy.
        /// </summary>
        /// <returns>A cloned <see cref="azure::storage::retry_policy" />.</returns>
        retry_policy clone() const override
        {
            if (m_policy != nullptr)
            {
                return m_policy->clone();
            }

            return retry_policy();
        }

    private:

        std::shared_ptr<basic_retry_policy> m_policy;
    };

    /// <summary>
    /// The lease state of a resource.
    /// </summary>
    enum class lease_state
    {
        /// <summary>
        /// The lease state is not specified.
        /// </summary>
        unspecified,

        /// <summary>
        /// The lease is in the Available state.
        /// </summary>
        available,

        /// <summary>
        /// The lease is in the Leased state.
        /// </summary>
        leased,

        /// <summary>
        /// The lease is in the Expired state.
        /// </summary>
        expired,

        /// <summary>
        /// The lease is in the Breaking state.
        /// </summary>
        breaking,

        /// <summary>
        /// The lease is in the Broken state.
        /// </summary>
        broken,
    };

    /// <summary>
    /// The lease status of a resource.
    /// </summary>
    enum class lease_status
    {
        /// <summary>
        /// The lease status is not specified.
        /// </summary>
        unspecified,

        /// <summary>
        /// The resource is locked.
        /// </summary>
        locked,

        /// <summary>
        /// The resource is available to be locked.
        /// </summary>
        unlocked
    };

    /// <summary>
    /// Specifies the proposed duration of seconds that the lease should continue before it is broken.
    /// </summary>
    class lease_break_period
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::lease_break_period" /> class that breaks
        /// a fixed-duration lease after the remaining lease period elapses, or breaks an infinite lease immediately.
        /// </summary>
        lease_break_period()
            : m_seconds(std::chrono::seconds::max())
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::lease_break_period" /> class that breaks
        /// a lease after the proposed duration.
        /// </summary>
        /// <param name="seconds">The proposed duration, in seconds, for the lease before it is broken. Value may
        /// be between 0 and 60 seconds.</param>
        lease_break_period(const std::chrono::seconds& seconds)
            : m_seconds(seconds)
        {
            if (seconds != std::chrono::seconds::max())
            {
                utility::assert_in_bounds(_XPLATSTR("seconds"), seconds, protocol::minimum_lease_break_period, protocol::maximum_lease_break_period);
            }
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+,
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::lease_break_period" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::lease_break_period" /> object.</param>
        lease_break_period(lease_break_period&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::lease_break_period" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::lease_break_period" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::lease_break_period" /> object with properties set.</returns>
        lease_break_period& operator=(lease_break_period&& other)
        {
            if (this != &other)
            {
                m_seconds = std::move(other.m_seconds);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Indicates whether the <see cref="azure::storage::lease_break_period" /> object is valid.
        /// </summary>
        /// <returns><c>true</c> if the <see cref="azure::storage::lease_break_period" /> object is valid; otherwise, <c>false</c>.</returns>
        bool is_valid() const
        {
            return m_seconds < std::chrono::seconds::max();
        }

        /// <summary>
        /// Gets the proposed duration for the lease before it is broken.
        /// </summary>
        /// <returns>The proposed proposed duration for the lease before it is broken, in seconds.</returns>
        const std::chrono::seconds& seconds() const
        {
            return m_seconds;
        }

    private:

        std::chrono::seconds m_seconds;
    };

    /// <summary>
    /// The lease duration for a Blob service resource.
    /// </summary>
    enum class lease_duration
    {
        /// <summary>
        /// The lease duration is not specified.
        /// </summary>
        unspecified,

        /// <summary>
        /// The lease duration is finite.
        /// </summary>
        fixed,

        /// <summary>
        /// The lease duration is infinite.
        /// </summary>
        infinite,
    };

    /// <summary>
    /// Specifies the duration of the lease.
    /// </summary>
    class lease_time
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::lease_time" /> class that never expires.
        /// </summary>
        lease_time()
            : m_seconds(-1)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::lease_time" /> class that expires after the
        /// specified duration.
        /// </summary>
        /// <param name="seconds">The duration of the lease in seconds. For a non-infinite lease, this value can be
        /// between 15 and 60 seconds.</param>
        lease_time(const std::chrono::seconds& seconds)
            : m_seconds(seconds)
        {
            if (seconds.count() != -1)
            {
                utility::assert_in_bounds(_XPLATSTR("seconds"), seconds, protocol::minimum_fixed_lease_duration, protocol::maximum_fixed_lease_duration);
            }
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+,
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::lease_time" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::lease_time" /> object.</param>
        lease_time(lease_time&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::lease_time" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::lease_time" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::lease_time" /> object with properties set.</returns>
        lease_time& operator=(lease_time&& other)
        {
            if (this != &other)
            {
                m_seconds = std::move(other.m_seconds);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets the duration of the lease in seconds for a non-infinite lease.
        /// </summary>
        /// <returns>The duration of the lease.</returns>
        const std::chrono::seconds& seconds() const
        {
            return m_seconds;
        }

    private:

        std::chrono::seconds m_seconds;
    };

#ifdef _WIN32
    /// <summary>
    /// Interface for scheduling tasks that start after a provided delay in milliseconds
    /// </summary>
    struct __declspec(novtable) delayed_scheduler_interface
    {
        virtual void schedule_after(pplx::TaskProc_t function, void* context, long long delayInMs) = 0;
    };

    /// <summary>
    /// Sets the ambient scheduler to be used by the PPL constructs. Note this is not thread safe.
    /// </summary>
    WASTORAGE_API void __cdecl set_wastorage_ambient_scheduler(const std::shared_ptr<pplx::scheduler_interface>& scheduler);

    /// <summary>
    /// Gets the ambient scheduler to be used by the PPL constructs. Note this is not thread safe.
    /// </summary>
    WASTORAGE_API const std::shared_ptr<pplx::scheduler_interface> __cdecl get_wastorage_ambient_scheduler();

    /// <summary>
    /// Sets the ambient scheduler to be used for scheduling delayed tasks. Note this is not thread safe.
    /// </summary>
    WASTORAGE_API void __cdecl set_wastorage_ambient_delayed_scheduler(const std::shared_ptr<delayed_scheduler_interface>& scheduler);

    /// <summary>
    /// Gets the ambient scheduler to be used for scheduling delayed tasks. Note this is not thread safe.
    /// </summary>
    WASTORAGE_API const std::shared_ptr<delayed_scheduler_interface>& __cdecl get_wastorage_ambient_delayed_scheduler();
#endif

}} // namespace azure::storage

#ifndef _WIN32
#define UNREFERENCED_PARAMETER(P) (P)
#endif

#pragma pop_macro("max")
