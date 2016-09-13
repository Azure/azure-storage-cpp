// -----------------------------------------------------------------------------------------
// <copyright file="auth.h" company="Microsoft">
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

#include "common.h"

namespace azure { namespace storage {

    class cloud_blob_shared_access_headers;
    class cloud_file_shared_access_headers;
    class account_shared_access_policy;

}} // namespace azure::storage

namespace azure { namespace storage { namespace protocol {

    utility::string_t calculate_hmac_sha256_hash(const utility::string_t& string_to_hash, const storage_credentials& credentials);

    const utility::string_t auth_name_shared_key(_XPLATSTR("SharedKey"));
    const utility::string_t auth_name_shared_key_lite(_XPLATSTR("SharedKeyLite"));

#pragma region Canonicalization

    /// <summary>
    /// A helper class for constructing the canonicalization strings required to authenticate a request.
    /// </summary>
    class canonicalizer_helper
    {
    public:
        
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::protocol::canonicalizer_helper" /> class.
        /// </summary>
        /// <param name="request">The request to be authenticated.</param>
        /// <param name="account_name">The storage account name.</param>
        canonicalizer_helper(const web::http::http_request& request, const utility::string_t& account_name)
            : m_request(request), m_account_name(account_name)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        
        // Prevents the compiler from generating default assignment operator.
        canonicalizer_helper& operator=(canonicalizer_helper& other) = delete;

#endif
        /// <summary>
        /// Returns the canonicalized string.
        /// </summary>
        /// <returns>The canonicalized string.</returns>
        utility::string_t str() const
        {
            return m_result;
        }

        /// <summary>
        /// Appends a value to the canonicalization string.
        /// </summary>
        /// <param name="value">The value.</param>
        void append(const utility::string_t& value)
        {
            m_result.append(value);
            m_result.append(_XPLATSTR("\n"));
        }

        /// <summary>
        /// Appends a Windows Azure Storage resource to the canonicalization string.
        /// </summary>
        /// <param name="only_comp"><c>true</c> to include only the comp parameters in the canonicalization string, as
        /// for Shared Key Lite; <c>false</c> to include all URI parameters in the string.</param>
        void append_resource(bool only_comp);

        /// <summary>
        /// Appends a header to the canonicalization string.
        /// </summary>
        /// <param name="header_name">The header name.</param>
        void append_header(const utility::string_t& header_name);

        /// <summary>
        /// Appends Content-Length header to the canonicalization string.
        /// </summary>
        void append_content_length_header();

        /// <summary>
        /// Appends the Date header to the canonicalization string if it exists on the request. Optionally appends
        /// the x_ms_date header to the canonicalization string.
        /// </summary>
        /// <param name="allow_x_ms_date"><c>true</c> to append the x_ms_date header; <c>false</c> to append 
        /// an empty string.</param>
        void append_date_header(bool allow_x_ms_date);

        /// <summary>
        /// Appends the x-ms- headers to the canonicalization string.
        /// </summary>
        void append_x_ms_headers();

    private:
        
        const web::http::http_request& m_request;
        const utility::string_t& m_account_name;
        utility::string_t m_result;
    };

    /// <summary>
    /// Represents a canonicalizer that converts HTTP request data into a standard form for signing.
    /// </summary>
    /// <remarks>
    /// <para>For detailed information on how to authenticate a request, 
    /// see <a href="http://msdn.microsoft.com/en-us/library/windowsazure/dd179428.aspx">Authentication for the Windows Azure Storage Services</a>.</para>
    /// </remarks>
    class canonicalizer
    {
    public:
        
        /// <summary>
        /// Converts the specified HTTP request data into a standard form for signing.
        /// </summary>
        /// <param name="request">The HTTP request to be signed.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The canonicalized string containing the HTTP request data in a standard form for signing.</returns>
        /// <remarks>
        /// <para>For detailed information on how to authenticate a request, 
        /// see <a href="http://msdn.microsoft.com/en-us/library/windowsazure/dd179428.aspx">Authentication for the Windows Azure Storage Services</a>.</para>
        /// </remarks>
        virtual utility::string_t canonicalize(const web::http::http_request& request, operation_context context) const = 0;

        /// <summary>
        /// Gets the authentication scheme used for canonicalization.
        /// </summary>
        /// <returns>The authentication scheme used for canonicalization.</returns>
        /// <remarks>
        /// <para>For detailed information on how to authenticate a request, 
        /// see <a href="http://msdn.microsoft.com/en-us/library/windowsazure/dd179428.aspx">Authentication for the Windows Azure Storage Services</a>.</para>
        /// </remarks>
        virtual utility::string_t authentication_scheme() const = 0;

    protected:
        
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::protocol::canonicalizer" /> class.
        /// </summary>
        /// <param name="account_name">The storage account name.</param>
        explicit canonicalizer(utility::string_t account_name)
            : m_account_name(std::move(account_name))
        {
        }

        /// <summary>
        /// The storage account name.
        /// </summary>
        utility::string_t m_account_name;
    };

    /// <summary>
    /// Represents a canonicalizer that converts HTTP request data into a standard form for signing via 
    /// the Shared Key authentication scheme for the Blob or Queue service.
    /// </summary>
    /// <remarks>
    /// <para>For detailed information on how to authenticate a request, 
    /// see <a href="http://msdn.microsoft.com/en-us/library/windowsazure/dd179428.aspx">Authentication for the Windows Azure Storage Services</a>.</para>
    /// </remarks>
    class shared_key_blob_queue_canonicalizer : public canonicalizer
    {
    public:
        
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::protocol::shared_key_blob_queue_canonicalizer" /> class.
        /// </summary>
        /// <param name="account_name">The storage account name.</param>
        explicit shared_key_blob_queue_canonicalizer(utility::string_t account_name)
            : canonicalizer(std::move(account_name))
        {
        }

        /// <summary>
        /// Converts the specified HTTP request data into a standard form for signing.
        /// </summary>
        /// <param name="request">The HTTP request to be signed.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The canonicalized string containing the HTTP request data in a standard form for signing.</returns>
        /// <remarks>
        /// <para>For detailed information on how to authenticate a request, 
        /// see <a href="http://msdn.microsoft.com/en-us/library/windowsazure/dd179428.aspx">Authentication for the Windows Azure Storage Services</a>.</para>
        /// </remarks>
        WASTORAGE_API utility::string_t canonicalize(const web::http::http_request& request, operation_context context) const override;

        /// <summary>
        /// Gets the authentication scheme used for canonicalization.
        /// </summary>
        /// <returns>The authentication scheme used for canonicalization.</returns>
        /// <remarks>
        /// <para>For detailed information on how to authenticate a request, 
        /// see <a href="http://msdn.microsoft.com/en-us/library/windowsazure/dd179428.aspx">Authentication for the Windows Azure Storage Services</a>.</para>
        /// </remarks>
        utility::string_t authentication_scheme() const override
        {
            return auth_name_shared_key;
        }
    };

    /// <summary>
    /// Represents a canonicalizer that converts HTTP request data into a standard form for signing via 
    /// the Shared Key Lite authentication scheme for the Blob or Queue service.
    /// </summary>
    /// <remarks>
    /// <para>For detailed information on how to authenticate a request, 
    /// see <a href="http://msdn.microsoft.com/en-us/library/windowsazure/dd179428.aspx">Authentication for the Windows Azure Storage Services</a>.</para>
    /// </remarks>
    class shared_key_lite_blob_queue_canonicalizer : public canonicalizer
    {
    public:
        
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::protocol::shared_key_lite_blob_queue_canonicalizer" /> class.
        /// </summary>
        /// <param name="account_name">The storage account name.</param>
        explicit shared_key_lite_blob_queue_canonicalizer(utility::string_t account_name)
            : canonicalizer(std::move(account_name))
        {
        }

        /// <summary>
        /// Converts the specified HTTP request data into a standard form for signing.
        /// </summary>
        /// <param name="request">The HTTP request to be signed.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The canonicalized string containing the HTTP request data in a standard form for signing.</returns>
        /// <remarks>
        /// <para>For detailed information on how to authenticate a request, 
        /// see <a href="http://msdn.microsoft.com/en-us/library/windowsazure/dd179428.aspx">Authentication for the Windows Azure Storage Services</a>.</para>
        /// </remarks>
        WASTORAGE_API utility::string_t canonicalize(const web::http::http_request& request, operation_context context) const override;

        /// <summary>
        /// Gets the authentication scheme used for canonicalization.
        /// </summary>
        /// <returns>The authentication scheme used for canonicalization.</returns>
        /// <remarks>
        /// <para>For detailed information on how to authenticate a request, 
        /// see <a href="http://msdn.microsoft.com/en-us/library/windowsazure/dd179428.aspx">Authentication for the Windows Azure Storage Services</a>.</para>
        /// </remarks>
        utility::string_t authentication_scheme() const override
        {
            return auth_name_shared_key_lite;
        }
    };

    /// <summary>
    /// Represents a canonicalizer that converts HTTP request data into a standard form for signing via 
    /// the Shared Key authentication scheme for the Table service.
    /// </summary>
    /// <remarks>
    /// <para>For detailed information on how to authenticate a request, 
    /// see <a href="http://msdn.microsoft.com/en-us/library/windowsazure/dd179428.aspx">Authentication for the Windows Azure Storage Services</a>.</para>
    /// </remarks>
    class shared_key_table_canonicalizer : public canonicalizer
    {
    public:
        
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::protocol::shared_key_table_canonicalizer" /> class.
        /// </summary>
        /// <param name="account_name">The storage account name.</param>
        explicit shared_key_table_canonicalizer(utility::string_t account_name)
            : canonicalizer(std::move(account_name))
        {
        }

        /// <summary>
        /// Converts the specified HTTP request data into a standard form for signing.
        /// </summary>
        /// <param name="request">The HTTP request to be signed.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The canonicalized string containing the HTTP request data in a standard form for signing.</returns>
        /// <remarks>
        /// <para>For detailed information on how to authenticate a request, 
        /// see <a href="http://msdn.microsoft.com/en-us/library/windowsazure/dd179428.aspx">Authentication for the Windows Azure Storage Services</a>.</para>
        /// </remarks>
        WASTORAGE_API utility::string_t canonicalize(const web::http::http_request& request, operation_context context) const override;

        /// <summary>
        /// Gets the authentication scheme used for canonicalization.
        /// </summary>
        /// <returns>The authentication scheme used for canonicalization.</returns>
        /// <remarks>
        /// <para>For detailed information on how to authenticate a request, 
        /// see <a href="http://msdn.microsoft.com/en-us/library/windowsazure/dd179428.aspx">Authentication for the Windows Azure Storage Services</a>.</para>
        /// </remarks>
        utility::string_t authentication_scheme() const override
        {
            return auth_name_shared_key;
        }
    };

    /// <summary>
    /// Represents a canonicalizer that converts HTTP request data into a standard form for signing via 
    /// the Shared Key Lite authentication scheme for the Table service.
    /// </summary>
    /// <remarks>
    /// <para>For detailed information on how to authenticate a request, 
    /// see <a href="http://msdn.microsoft.com/en-us/library/windowsazure/dd179428.aspx">Authentication for the Windows Azure Storage Services</a>.</para>
    /// </remarks>
    class shared_key_lite_table_canonicalizer : public canonicalizer
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::protocol::shared_key_lite_table_canonicalizer" /> class.
        /// </summary>
        /// <param name="account_name">The storage account name.</param>
        explicit shared_key_lite_table_canonicalizer(utility::string_t account_name)
            : canonicalizer(std::move(account_name))
        {
        }

        /// <summary>
        /// Converts the specified HTTP request data into a standard form for signing.
        /// </summary>
        /// <param name="request">The HTTP request to be signed.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The canonicalized string containing the HTTP request data in a standard form for signing.</returns>
        /// <remarks>
        /// <para>For detailed information on how to authenticate a request, 
        /// see <a href="http://msdn.microsoft.com/en-us/library/windowsazure/dd179428.aspx">Authentication for the Windows Azure Storage Services</a>.</para>
        /// </remarks>
        WASTORAGE_API utility::string_t canonicalize(const web::http::http_request& request, operation_context context) const override;

        /// <summary>
        /// Gets the authentication scheme used for canonicalization.
        /// </summary>
        /// <returns>The authentication scheme used for canonicalization.</returns>
        /// <remarks>
        /// <para>For detailed information on how to authenticate a request, 
        /// see <a href="http://msdn.microsoft.com/en-us/library/windowsazure/dd179428.aspx">Authentication for the Windows Azure Storage Services</a>.</para>
        /// </remarks>
        utility::string_t authentication_scheme() const override
        {
            return auth_name_shared_key_lite;
        }
    };

#pragma endregion

#pragma region Authentication Handlers

    /// <summary>
    /// A helper class for signing a request for the desired authentication scheme.
    /// </summary>
    class authentication_handler
    {
    public:
        
        /// <summary>
        /// Sign the specified request for authentication.
        /// </summary>
        /// <param name="request">The request to be signed.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        virtual void sign_request(web::http::http_request& request, operation_context context) const
        {
            UNREFERENCED_PARAMETER(request);
            UNREFERENCED_PARAMETER(context);
        }
    };

    /// <summary>
    /// A helper class for signing a request with credentials supplied by a shared access signature.
    /// </summary>
    class sas_authentication_handler : public authentication_handler
    {
    public:
        
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::protocol::sas_authentication_handler" /> class.
        /// </summary>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use to sign the request.</param>
        explicit sas_authentication_handler(storage_credentials credentials)
            : m_credentials(std::move(credentials))
        {
        }

        /// <summary>
        /// Sign the specified request for authentication.
        /// </summary>
        /// <param name="request">The request to be signed.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        WASTORAGE_API void sign_request(web::http::http_request& request, operation_context context) const override;

    private:
        
        storage_credentials m_credentials;
    };

    /// <summary>
    /// A helper class for signing a request for Shared Key authentication.
    /// </summary>
    class shared_key_authentication_handler : public authentication_handler
    {
    public:
        
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::protocol::shared_key_authentication_handler" /> class.
        /// </summary>
        /// <param name="canonicalizer">The canonicalizer to use to sign the request.</param>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use to sign the request.</param>
        shared_key_authentication_handler(std::shared_ptr<canonicalizer> canonicalizer, storage_credentials credentials)
            : m_canonicalizer(canonicalizer), m_credentials(std::move(credentials))
        {
        }

        /// <summary>
        /// Sign the specified request for authentication via Shared Key.
        /// </summary>
        /// <param name="request">The request to be signed.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        WASTORAGE_API void sign_request(web::http::http_request& request, operation_context context) const override;

    private:
        
        std::shared_ptr<canonicalizer> m_canonicalizer;
        storage_credentials m_credentials;
    };

#pragma endregion

#pragma region Shared Access Signatures

    utility::string_t get_account_sas_token(const utility::string_t& identifier, const account_shared_access_policy& policy, const storage_credentials& credentials);
    utility::string_t get_blob_sas_token(const utility::string_t& identifier, const shared_access_policy& policy, const cloud_blob_shared_access_headers& headers, const utility::string_t& resource_type, const utility::string_t& resource, const storage_credentials& credentials);
    utility::string_t get_queue_sas_token(const utility::string_t& identifier, const shared_access_policy& policy, const utility::string_t& resource, const storage_credentials& credentials);
    utility::string_t get_table_sas_token(const utility::string_t& identifier, const shared_access_policy& policy, const utility::string_t& table_name, const utility::string_t& start_partition_key, const utility::string_t& start_row_key, const utility::string_t& end_partition_key, const utility::string_t& end_row_key, const utility::string_t& resource, const storage_credentials& credentials);
    utility::string_t get_file_sas_token(const utility::string_t& identifier, const shared_access_policy& policy, const cloud_file_shared_access_headers& headers, const utility::string_t& resource_type, const utility::string_t& resource, const storage_credentials& credentials);
    storage_credentials parse_query(const web::http::uri& uri, bool require_signed_resource);

#pragma endregion

}}} // namespace azure::storage::protocol
