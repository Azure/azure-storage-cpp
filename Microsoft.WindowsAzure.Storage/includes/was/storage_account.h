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
#include "common.h"

namespace azure { namespace storage {

    class cloud_blob_client;
    class cloud_queue_client;
    class cloud_table_client;
    class cloud_file_client;
    class blob_request_options;
    class queue_request_options;
    class table_request_options;
    class file_request_options;
    class account_shared_access_policy;

    /// <summary>
    /// Represents a Windows Azure storage account.
    /// </summary>
    class cloud_storage_account
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_storage_account" /> class.
        /// </summary>
        cloud_storage_account()
            : m_initialized(false), m_default_endpoints(false), m_is_development_storage_account(false)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_storage_account" /> class using the specified
        /// credentials and service endpoints.
        /// </summary>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        /// <param name="blob_endpoint">The Blob service endpoint.</param>
        /// <param name="queue_endpoint">The Queue service endpoint.</param>
        /// <param name="table_endpoint">The Table service endpoint.</param>
        cloud_storage_account(const storage_credentials& credentials, const storage_uri& blob_endpoint, const storage_uri& queue_endpoint, const storage_uri& table_endpoint)
            : m_initialized(true), m_default_endpoints(false), m_is_development_storage_account(false), m_blob_endpoint(blob_endpoint), m_queue_endpoint(queue_endpoint), m_table_endpoint(table_endpoint), m_credentials(credentials)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_storage_account" /> class using the specified
        /// credentials and service endpoints.
        /// </summary>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        /// <param name="blob_endpoint">The Blob service endpoint.</param>
        /// <param name="queue_endpoint">The Queue service endpoint.</param>
        /// <param name="table_endpoint">The Table service endpoint.</param>
        /// <param name="file_endpoint">The File service endpoint.</param>
        cloud_storage_account(const storage_credentials& credentials, const storage_uri& blob_endpoint, const storage_uri& queue_endpoint, const storage_uri& table_endpoint, const storage_uri& file_endpoint)
            : m_initialized(true), m_is_development_storage_account(false), m_credentials(credentials), m_blob_endpoint(blob_endpoint), m_queue_endpoint(queue_endpoint), m_table_endpoint(table_endpoint), m_file_endpoint(file_endpoint), m_default_endpoints(false)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_storage_account" /> class using the specified
        /// credentials and the default service endpoints.
        /// </summary>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        /// <param name="use_https"><c>true</c> to use HTTPS to connect to storage service endpoints; otherwise, <c>false</c>.</param>
        cloud_storage_account(const storage_credentials& credentials, bool use_https)
            : m_initialized(true), m_default_endpoints(true), m_is_development_storage_account(false), m_credentials(credentials)
        {
            initialize_default_endpoints(use_https);
        }
        
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_storage_account" /> class using the specified
        /// credentials and the default service endpoints.
        /// </summary>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        /// <param name="endpoint_suffix">The DNS endpoint suffix for the storage services, e.g., &quot;core.windows.net&quot;.</param>
        /// <param name="use_https"><c>true</c> to use HTTPS to connect to storage service endpoints; otherwise, <c>false</c>.</param>
        cloud_storage_account(const storage_credentials& credentials, const utility::string_t& endpoint_suffix, bool use_https)
            : m_initialized(true), m_default_endpoints(true), m_is_development_storage_account(false), m_credentials(credentials), m_endpoint_suffix(endpoint_suffix)
        {
            initialize_default_endpoints(use_https);
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_storage_account" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_storage_account" /> object.</param>
        cloud_storage_account(cloud_storage_account&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_storage_account" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_storage_account" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::cloud_storage_account" /> object with properties set.</returns>
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
                m_file_endpoint = std::move(other.m_file_endpoint);
                m_credentials = std::move(other.m_credentials);
                m_endpoint_suffix = std::move(other.m_endpoint_suffix);
                m_settings = std::move(other.m_settings);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Parses a connection string and returns an <see cref="azure::storage::cloud_storage_account" /> created
        /// from the connection string.
        /// </summary>
        /// <param name="connection_string">A valid connection string.</param>
        /// <returns>An <see cref="azure::storage::cloud_storage_account" /> object constructed from the values provided in the connection string.</returns>
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
        /// Creates the File service client.
        /// </summary>
        /// <returns>A client object that specifies the Blob service endpoint.</returns>
        WASTORAGE_API cloud_file_client create_cloud_file_client() const;

        /// <summary>
        /// Creates the File service client.
        /// </summary>
        /// <returns>A client object that specifies the Blob service endpoint.</returns>
        WASTORAGE_API cloud_file_client create_cloud_file_client(const file_request_options& default_request_options) const;

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
        /// Gets an <see cref="azure::storage::cloud_storage_account" /> object that references the development storage account.
        /// </summary>
        /// <returns>A reference to the development storage account.</returns>
        WASTORAGE_API static cloud_storage_account development_storage_account();

        /// <summary>
        /// Gets the endpoint for the Blob service for all location.
        /// </summary>
        /// <returns>An <see cref="azure::storage::storage_uri" /> object containing the Blob service endpoint for all locations.</returns>
        const storage_uri& blob_endpoint() const
        {
            return m_blob_endpoint;
        }

        /// <summary>
        /// Gets the endpoint for the Queue service for all location.
        /// </summary>
        /// <returns>An <see cref="azure::storage::storage_uri" /> object containing the Queue service endpoint for all locations.</returns>
        const storage_uri& queue_endpoint() const
        {
            return m_queue_endpoint;
        }

        /// <summary>
        /// Gets the endpoint for the Table service for all location.
        /// </summary>
        /// <returns>An <see cref="azure::storage::storage_uri" /> object containing the Table service endpoint for all locations.</returns>
        const storage_uri& table_endpoint() const
        {
            return m_table_endpoint;
        }

        /// <summary>
        /// Gets the endpoint for the File service for all location.
        /// </summary>
        /// <returns>An <see cref="azure::storage::storage_uri" /> object containing the Table service endpoint for all locations.</returns>
        const storage_uri& file_endpoint() const
        {
            return m_file_endpoint;
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

        /// <summary>
        /// Returns a shared access signature for the account.
        /// </summary>
        /// <param name="policy">The access policy for the shared access signature.</param>
        /// <returns>A string containing a shared access signature.</returns>
        WASTORAGE_API utility::string_t get_shared_access_signature(const account_shared_access_policy& policy) const;

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
        storage_uri m_file_endpoint;
        storage_credentials m_credentials;
        utility::string_t m_endpoint_suffix;
        std::map<utility::string_t, utility::string_t> m_settings;
    };

    /// <summary>
    /// Represents a shared access policy for an account, which specifies the start time, expiry time,
    /// permissions, singed service, signed resource type, signed protocol, and signed IP addresses for a shared access signature.
    /// </summary>
    class account_shared_access_policy : public shared_access_policy
    {
    public:

        /// <summary>
        /// An enumeration describing permissions that may be used for a shared access signature.
        /// </summary>
        enum permissions
        {
            /// <summary>
            /// No shared access granted.
            /// </summary>
            none = 0,

            /// <summary>
            /// Permission to read resources and list queues and tables granted.
            /// </summary>
            read = 1,

            /// <summary>
            /// Permission to write resources granted.
            /// </summary>
            write = 2,

            /// <summary>
            /// Permission to delete resources granted.
            /// </summary>
            del = 4,

            /// <summary>
            /// Permission to list blob containers, blobs, shares, directories, and files granted.
            /// </summary>
            list = 8,

            /// <summary>
            /// Permission to add messages, table entities, blobs, and files granted.
            /// </summary>
            add = 0x10,

            /// <summary>
            /// Permissions to update messages and table entities granted.
            /// </summary>
            update = 0x20,

            /// <summary>
            /// Permission to get and delete messages granted.
            /// </summary>
            process = 0x40,

            /// <summary>
            /// Permission to create containers, blobs, shares, directories, and files granted.
            /// </summary>
            create = 0x80
        };

        /// <summary>
        /// Specifies the set of possible signed services for a shared access account policy.
        /// </summary>
        enum service_types
        {
            /// <summary>
            /// Permission to access blob resources granted.
            /// </summary>
            blob = 0x1,

            /// <summary>
            /// Permission to access queue resources granted.
            /// </summary>
            queue = 0x2,

            /// <summary>
            /// Permission to access table resources granted.
            /// </summary>
            table = 0x4,

            /// <summary>
            /// Permission to access file resources granted.
            /// </summary>
            file = 0x8
        };

        /// <summary>
        /// Get a canonical string representation of the services for a shared access policy.
        /// </summary>
        utility::string_t service_types_to_string() const
        {
            utility::string_t services;
            if (m_service_type & blob)
            {
                services.push_back(_XPLATSTR('b'));
            }

            if (m_service_type & queue)
            {
                services.push_back(_XPLATSTR('q'));
            }

            if (m_service_type & table)
            {
                services.push_back(_XPLATSTR('t'));
            }

            if (m_service_type & file)
            {
                services.push_back(_XPLATSTR('f'));
            }

            return services;
        }

        /// <summary>
        /// Specifies the set of possible signed resource types for a shared access account policy.
        /// </summary>
        enum resource_types
        {
            /// <summary>
            /// Permission to access service level APIs granted.
            /// </summary>
            service = 0x1,

            /// <summary>
            /// Permission to access container level APIs (Blob Containers, Tables, Queues, File Shares) granted.
            /// </summary>
            container = 0x2,

            /// <summary>
            /// Permission to access object level APIs (Blobs, Table Entities, Queue Messages, Files) granted
            /// </summary>
            object = 0x4
        };

        /// <summary>
        /// Get a canonical string representation of the resource types for a shared access policy.
        /// </summary>
        utility::string_t resource_types_to_string() const
        {
            utility::string_t resource_types;
            if (m_resource_type & service)
            {
                resource_types.push_back(_XPLATSTR('s'));
            }

            if (m_resource_type & container)
            {
                resource_types.push_back(_XPLATSTR('c'));
            }

            if (m_resource_type & object)
            {
                resource_types.push_back(_XPLATSTR('o'));
            }

            return resource_types;
        }
        
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::account_shared_access_policy" /> class.
        /// </summary>
        account_shared_access_policy()
            : shared_access_policy()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::account_shared_access_policy" /> class.
        /// </summary>
        /// <param name="service">The services (blob, file, queue, table) for the shared access policy.</param>
        /// <param name="resource">The resource type for the shared access policy.</param>
        account_shared_access_policy(service_types service, resource_types resource)
            : shared_access_policy(), m_service_type(service), m_resource_type(resource)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::account_shared_access_policy" /> class.
        /// </summary>
        /// <param name="expiry">The expiration date and time of the policy.</param>
        /// <param name="permission">A mask containing the permissions of the policy</param>
        /// <param name="service">The services (blob, file, queue, table) for the shared access policy.</param>
        /// <param name="resource">The resource type for the shared access policy.</param>
        account_shared_access_policy(utility::datetime expiry, uint8_t permission, service_types service, resource_types resource)
            : shared_access_policy(expiry, permission), m_service_type(service), m_resource_type(resource)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::account_shared_access_policy" /> class.
        /// </summary>
        /// <param name="start">The start date and time of the policy.</param>
        /// <param name="expiry">The expiration date and time of the policy.</param>
        /// <param name="permission">A mask containing the permissions of the policy</param>
        /// <param name="service">The services (blob, file, queue, table) for the shared access policy.</param>
        /// <param name="resource">The resource type for the shared access policy.</param>
        account_shared_access_policy(utility::datetime start, utility::datetime expiry, uint8_t permission, service_types service, resource_types resource)
            : shared_access_policy(start, expiry, permission), m_service_type(service), m_resource_type(resource)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::account_shared_access_policy" /> class.
        /// </summary>
        /// <param name="start">The start date and time of the policy.</param>
        /// <param name="expiry">The expiration date and time of the policy.</param>
        /// <param name="permission">A mask containing the permissions of the policy</param>
        /// <param name="protocol">The allowed protocols for the shared access policy.</param>
        /// <param name="address">The allowed IP address for a shared access signature associated with this shared access policy.</param>
        /// <param name="service">The services (blob, file, queue, table) for the shared access policy.</param>
        /// <param name="resource">The resource type for the shared access policy.</param>
        account_shared_access_policy(utility::datetime start, utility::datetime expiry, uint8_t permission, protocols protocol, utility::string_t address, service_types service, resource_types resource)
            : shared_access_policy(start, expiry, permission, protocol, std::move(address)), m_service_type(service), m_resource_type(resource)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::account_shared_access_policy" /> class.
        /// </summary>
        /// <param name="start">The start date and time of the policy.</param>
        /// <param name="expiry">The expiration date and time of the policy.</param>
        /// <param name="permission">A mask containing the permissions of the policy</param>
        /// <param name="protocol">The allowed protocols for the shared access policy.</param>
        /// <param name="minimum_address">The minimum allowed address for an IP range for the shared access policy.</param>
        /// <param name="maximum_address">The maximum allowed address for an IP range for the shared access policy.</param>
        /// <param name="service">The services (blob, file, queue, table) for the shared access policy.</param>
        /// <param name="resource">The resource type for the shared access policy.</param>
        account_shared_access_policy(utility::datetime start, utility::datetime expiry, uint8_t permission, protocols protocol, utility::string_t minimum_address, utility::string_t maximum_address, service_types service, resource_types resource)
            : shared_access_policy(start, expiry, permission, protocol, std::move(minimum_address), std::move(maximum_address)), m_service_type(service), m_resource_type(resource)
        {
        }

        /// <summary>
        /// Sets the services (blob, file, queue, table) for a shared access signature associated with this shared access policy.
        /// </summary>
        /// <param name="value">The services for the shared access policy.</param>
        void set_service_type(service_types value)
        {
            m_service_type = value;
        }
        
        /// <summary>
        /// Gets the services (blob, file, queue, table) for a shared access signature associated with this shared access policy.
        /// </summary>
        /// <returns>The services for the shared access policy.</returns>
        service_types service_type()
        {
            return m_service_type;
        }

        /// <summary>
        /// Sets the resource type for a shared access signature associated with this shared access policy.
        /// </summary>
        /// <param name="value">The resource type for the shared access policy.</param>
        void set_resource_type(resource_types value)
        {
            m_resource_type = value;
        }

        /// <summary>
        /// Gets the resource type for a shared access signature associated with this shared access policy.
        /// </summary>
        /// <returns>The resource type for the shared access policy.</returns>
        resource_types resource_type()
        {
            return m_resource_type;
        }

    private:
        service_types m_service_type;
        resource_types m_resource_type;
    };
}} // namespace azure::storage
