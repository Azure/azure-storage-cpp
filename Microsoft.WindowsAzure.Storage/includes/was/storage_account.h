// -----------------------------------------------------------------------------------------
// <copyright file="storage_account.h" company="Microsoft">
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

#include "core.h"

namespace azure { namespace storage {

    class cloud_blob_client;
    class cloud_queue_client;
    class cloud_table_client;
    class blob_request_options;
    class queue_request_options;
    class table_request_options;

    /// <summary>
    /// Represents a Windows Azure storage account.
    /// </summary>
    class cloud_storage_account
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="cloud_storage_account" /> class.
        /// </summary>
        cloud_storage_account()
            : m_initialized(false), m_is_development_storage_account(false), m_default_endpoints(false)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_storage_account" /> class using the specified
        /// credentials and service endpoints.
        /// </summary>
        /// <param name="credentials">The <see cref="storage_credentials" /> to use.</param>
        /// <param name="blob_endpoint">The Blob service endpoint.</param>
        /// <param name="queue_endpoint">The Queue service endpoint.</param>
        /// <param name="table_endpoint">The Table service endpoint.</param>
        cloud_storage_account(const storage_credentials& credentials, const storage_uri& blob_endpoint, const storage_uri& queue_endpoint, const storage_uri& table_endpoint)
            : m_initialized(true), m_is_development_storage_account(false), m_credentials(credentials), m_blob_endpoint(blob_endpoint), m_queue_endpoint(queue_endpoint), m_table_endpoint(table_endpoint), m_default_endpoints(false)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_storage_account" /> class using the specified
        /// credentials and the default service endpoints.
        /// </summary>
        /// <param name="credentials">The <see cref="storage_credentials" /> to use.</param>
        /// <param name="use_https"><c>true</c> to use HTTPS to connect to storage service endpoints; otherwise, <c>false</c>.</param>
        cloud_storage_account(const storage_credentials& credentials, bool use_https)
            : m_initialized(true), m_is_development_storage_account(false), m_credentials(credentials), m_default_endpoints(true)
        {
            initialize_default_endpoints(use_https);
        }
        
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_storage_account" /> class using the specified
        /// credentials and the default service endpoints.
        /// </summary>
        /// <param name="credentials">The <see cref="storage_credentials" /> to use.</param>
        /// <param name="endpoint_suffix">The DNS endpoint suffix for the storage services, e.g., &quot;core.windows.net&quot;.</param>
        /// <param name="use_https"><c>true</c> to use HTTPS to connect to storage service endpoints; otherwise, <c>false</c>.</param>
        cloud_storage_account(const storage_credentials& credentials, const utility::string_t& endpoint_suffix, bool use_https)
            : m_initialized(true), m_is_development_storage_account(false), m_credentials(credentials), m_default_endpoints(true), m_endpoint_suffix(endpoint_suffix)
        {
            initialize_default_endpoints(use_https);
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="cloud_storage_account"/> class.
        /// </summary>
        /// <param name="other">A reference to a set of <see cref="cloud_storage_account" /> on which to base the new instance.</param>
        cloud_storage_account(cloud_storage_account&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to a <see cref="cloud_storage_account" /> object.
        /// </summary>
        /// <param name="other">A reference to a set of <see cref="cloud_storage_account" /> to use to set properties.</param>
        /// <returns>A <see cref="cloud_storage_account" /> object with properties set.</returns>
        cloud_storage_account& operator=(cloud_storage_account&& other)
        {
            if (this != &other)
            {
                m_initialized = std::move(other.m_initialized);
                m_default_endpoints = std::move(other.m_default_endpoints);
                m_is_development_storage_account = std::move(other.m_is_development_storage_account);
                m_blob_endpoint = std::move(other.m_blob_endpoint);
                m_queue_endpoint = std::move(other.m_queue_endpoint);
                m_table_endpoint = std::move(other.m_table_endpoint);
                m_credentials = std::move(other.m_credentials);
                m_endpoint_suffix = std::move(other.m_endpoint_suffix);
                m_settings = std::move(other.m_settings);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Parses a connection string and returns a <see cref="azure::storage::cloud_storage_account" /> created
        /// from the connection string.
        /// </summary>
        /// <param name="connection_string">A valid connection string.</param>
        /// <returns>A <see cref="azure::storage::cloud_storage_account" /> object constructed from the values provided in the connection string.</returns>
        WASTORAGE_API static cloud_storage_account parse(const utility::string_t& connection_string);
        
        /// <summary>
        /// Creates the Blob service client.
        /// </summary>
        /// <returns>A client object that specifies the Blob service endpoint.</returns>
        WASTORAGE_API cloud_blob_client create_cloud_blob_client() const;

        /// <summary>
        /// Creates the Blob service client.
        /// </summary>
        /// <returns>A client object that specifies the Blob service endpoint.</returns>
        WASTORAGE_API cloud_blob_client create_cloud_blob_client(const blob_request_options& default_request_options) const;

        /// <summary>
        /// Creates the Queue service client.
        /// </summary>
        /// <returns>A client object that specifies the Queue service endpoint.</returns>
        WASTORAGE_API cloud_queue_client create_cloud_queue_client() const;

        /// <summary>
        /// Creates the Queue service client.
        /// </summary>
        /// <returns>A client object that specifies the Queue service endpoint.</returns>
        WASTORAGE_API cloud_queue_client create_cloud_queue_client(const queue_request_options& default_request_options) const;

        /// <summary>
        /// Creates the Table service client.
        /// </summary>
        /// <returns>A client object that specifies the Table service endpoint.</returns>
        WASTORAGE_API cloud_table_client create_cloud_table_client() const;

        /// <summary>
        /// Creates the Table service client.
        /// </summary>
        /// <returns>A client object that specifies the Table service endpoint.</returns>
        WASTORAGE_API cloud_table_client create_cloud_table_client(const table_request_options& default_request_options) const;

        /// <summary>
        /// Returns a connection string for this storage account, without sensitive data.
        /// </summary>
        /// <returns>A connection string.</returns>
        utility::string_t to_string()
        {
            return to_string(false);
        }

        /// <summary>
        /// Returns a connection string for the storage account, optionally with sensitive data.
        /// </summary>
        /// <param name="export_secrets"><c>true</c> to include sensitive data in the string; otherwise, <c>false</c>.</param>
        /// <returns>A connection string.</returns>
        WASTORAGE_API utility::string_t to_string(bool export_secrets);

        /// <summary>
        /// Gets a <see cref="azure::storage::cloud_storage_account" /> object that references the development storage account.
        /// </summary>
        /// <returns>A reference to the development storage account.</returns>
        WASTORAGE_API static cloud_storage_account development_storage_account();

        /// <summary>
        /// Gets the endpoint for the Blob service for all location.
        /// </summary>
        /// <returns>A <see cref="storage_uri" /> object containing the Blob service endpoint for all locations.</returns>
        const storage_uri& blob_endpoint() const
        {
            return m_blob_endpoint;
        }

        /// <summary>
        /// Gets the endpoint for the Queue service for all location.
        /// </summary>
        /// <returns>A <see cref="storage_uri" /> object containing the Queue service endpoint for all locations.</returns>
        const storage_uri& queue_endpoint() const
        {
            return m_queue_endpoint;
        }

        /// <summary>
        /// Gets the endpoint for the Table service for all location.
        /// </summary>
        /// <returns>A <see cref="storage_uri" /> object containing the Table service endpoint for all locations.</returns>
        const storage_uri& table_endpoint() const
        {
            return m_table_endpoint;
        }

        /// <summary>
        /// Gets the credentials used to create this <see cref="azure::storage::cloud_storage_account" /> object.
        /// </summary>
        /// <returns>The credentials used to create the <see cref="azure::storage::cloud_storage_account" /> object.</returns>
        const storage_credentials& credentials() const
        {
            return m_credentials;
        }

        /// <summary>
        /// Indicates whether the storage account has been initialized.
        /// </summary>
        /// <returns><c>true</c> if the storage account has been initialized.</returns>
        bool is_initialized() const
        {
            return m_initialized;
        }

    private:

        WASTORAGE_API void initialize_default_endpoints(bool use_https);
        static cloud_storage_account get_development_storage_account(const web::http::uri& proxy_uri);
        static cloud_storage_account parse_devstore_settings(std::map<utility::string_t, utility::string_t> settings);
        static cloud_storage_account parse_defaults_settings(std::map<utility::string_t, utility::string_t> settings);
        static cloud_storage_account parse_explicit_settings(std::map<utility::string_t, utility::string_t> settings);

        bool m_initialized;
        bool m_default_endpoints;
        bool m_is_development_storage_account;
        storage_uri m_blob_endpoint;
        storage_uri m_queue_endpoint;
        storage_uri m_table_endpoint;
        storage_credentials m_credentials;
        utility::string_t m_endpoint_suffix;
        std::map<utility::string_t, utility::string_t> m_settings;
    };

}} // namespace azure::storage
