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

namespace wa { namespace storage {

    /// <summary>
    /// Provides a client-side logical representation of a Windows Azure service. This client is used to configure and execute requests against the service.
    /// </summary>
    /// <remarks>The service client encapsulates the base URI for the service. If the service client will be used for authenticated access, it also encapsulates the credentials for accessing the storage account.</remarks>
    class cloud_client
    {
    public:

        /// <summary>
        /// Gets the base URI for the service client.
        /// </summary>
        /// <returns>The base URI for the service client.</returns>
        storage_uri base_uri() const
        {
            return m_base_uri;
        }

        /// <summary>
        /// Gets the storage account credentials for the service client.
        /// </summary>
        /// <returns>The storage account credentials for the service client.</returns>
        const wa::storage::storage_credentials& credentials() const
        {
            return m_credentials;
        }

        /// <summary>
        /// Gets the authentication scheme to use to sign HTTP requests for the service client.
        /// </summary>
        /// <returns>The authentication scheme to use to sign HTTP requests for the service client.</returns>
        wa::storage::authentication_scheme authentication_scheme() const
        {
            return m_authentication_scheme;
        }

        /// <summary>
        /// Sets the authentication scheme to use to sign HTTP requests.
        /// </summary>
        /// <param name="value">The authentication scheme to use to sign HTTP requests.</param>
        virtual void set_authentication_scheme(wa::storage::authentication_scheme value)
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
        {
        }

        /// <summary>
        /// Initializes a new instance of the service client class using the specified service endpoint.
        /// </summary>
        /// <param name="base_uri">A <see cref="storage_uri" /> object containing the service endpoint for all locations.</param>
        cloud_client(const storage_uri& base_uri)
            : m_base_uri(base_uri)
        {
        }

        /// <summary>
        /// Initializes a new instance of the client class using the specified service endpoint and storage account credentials.
        /// </summary>
        /// <param name="base_uri">A <see cref="storage_uri" /> object containing the service endpoint for all locations.</param>
        /// <param name="credentials">The <see cref="storage_credentials" /> to use.</param>
        cloud_client(const storage_uri& base_uri, const wa::storage::storage_credentials& credentials)
            : m_base_uri(base_uri), m_credentials(credentials)
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

    private:

        storage_uri m_base_uri;
        wa::storage::storage_credentials m_credentials;
        wa::storage::authentication_scheme m_authentication_scheme;
        std::shared_ptr<protocol::authentication_handler> m_authentication_handler;
    };

}} // namespace wa::storage
