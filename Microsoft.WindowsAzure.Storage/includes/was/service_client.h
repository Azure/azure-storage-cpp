// -----------------------------------------------------------------------------------------
// <copyright file="service_client.h" company="Microsoft">
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

#include "auth.h"
#include "common.h"

namespace azure { namespace storage {

    /// <summary>
    /// Provides a client-side logical representation of a Windows Azure service. This client is used to configure and execute requests against the service.
    /// </summary>
    /// <remarks>The service client encapsulates the base URI for the service. If the service client will be used for authenticated access, it also encapsulates the credentials for accessing the storage account.</remarks>
    class cloud_client
    {
    public:

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_client" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_client" /> object.</param>
        cloud_client(cloud_client&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_client" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_client" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::cloud_client" /> object with properties set.</returns>
        cloud_client& operator=(cloud_client&& other)
        {
            if (this != &other)
            {
                m_base_uri = std::move(other.m_base_uri);
                m_credentials = std::move(other.m_credentials);
                m_authentication_scheme = std::move(other.m_authentication_scheme);
                m_authentication_handler = std::move(other.m_authentication_handler);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets the base URI for the service client.
        /// </summary>
        /// <returns>The base URI for the service client.</returns>
        const storage_uri& base_uri() const
        {
            return m_base_uri;
        }

        /// <summary>
        /// Gets the storage account credentials for the service client.
        /// </summary>
        /// <returns>The storage account credentials for the service client.</returns>
        const azure::storage::storage_credentials& credentials() const
        {
            return m_credentials;
        }

        /// <summary>
        /// Gets the authentication scheme to use to sign HTTP requests for the service client.
        /// </summary>
        /// <returns>The authentication scheme to use to sign HTTP requests for the service client.</returns>
        azure::storage::authentication_scheme authentication_scheme() const
        {
            return m_authentication_scheme;
        }

        /// <summary>
        /// Sets the authentication scheme to use to sign HTTP requests.
        /// </summary>
        /// <param name="value">The authentication scheme to use to sign HTTP requests.</param>
        virtual void set_authentication_scheme(azure::storage::authentication_scheme value)
        {
            m_authentication_scheme = value;
        }

        /// <summary>
        /// Gets the authentication handler to use to sign HTTP requests.
        /// </summary>
        /// <returns>The authentication handler to use to sign HTTP requests.</returns>
        std::shared_ptr<protocol::authentication_handler> authentication_handler() const
        {
            return m_authentication_handler;
        }

    protected:

        /// <summary>
        /// Initializes a new instance of the service client class using the specified service endpoint.
        /// </summary>
        cloud_client()
            : m_authentication_scheme(azure::storage::authentication_scheme::shared_key)
        {
        }

        /// <summary>
        /// Initializes a new instance of the service client class using the specified service endpoint.
        /// </summary>
        /// <param name="base_uri">An <see cref="azure::storage::storage_uri" /> object containing the service endpoint for all locations.</param>
        explicit cloud_client(storage_uri base_uri)
            : m_base_uri(std::move(base_uri)), m_authentication_scheme(azure::storage::authentication_scheme::shared_key)
        {
        }

        /// <summary>
        /// Initializes a new instance of the client class using the specified service endpoint and storage account credentials.
        /// </summary>
        /// <param name="base_uri">An <see cref="azure::storage::storage_uri" /> object containing the service endpoint for all locations.</param>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        cloud_client(storage_uri base_uri, azure::storage::storage_credentials credentials)
            : m_base_uri(std::move(base_uri)), m_credentials(std::move(credentials)), m_authentication_scheme(azure::storage::authentication_scheme::shared_key)
        {
        }

        /// <summary>
        /// Sets the authentication handler to use to sign HTTP requests.
        /// </summary>
        /// <param name="value">The authentication handler to use to sign HTTP requests.</param>
        void set_authentication_handler(std::shared_ptr<protocol::authentication_handler> value)
        {
            m_authentication_handler = value;
        }

        WASTORAGE_API pplx::task<service_properties> download_service_properties_base_async(const request_options& modified_options, operation_context context) const;
        WASTORAGE_API pplx::task<void> upload_service_properties_base_async(const service_properties& properties, const service_properties_includes& includes, const request_options& modified_options, operation_context context) const;
        WASTORAGE_API pplx::task<service_stats> download_service_stats_base_async(const request_options& modified_options, operation_context context) const;

    private:

        storage_uri m_base_uri;
        azure::storage::storage_credentials m_credentials;
        azure::storage::authentication_scheme m_authentication_scheme;
        std::shared_ptr<protocol::authentication_handler> m_authentication_handler;
    };

}} // namespace azure::storage
