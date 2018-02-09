// -----------------------------------------------------------------------------------------
// <copyright file="file.h" company="Microsoft">
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

#include "limits"
#include "service_client.h"

#pragma push_macro("max")
#undef max

namespace azure { namespace storage {

    class cloud_file;
    class cloud_file_directory;
    class cloud_file_share;
    class cloud_file_client;

    class cloud_blob;
    class access_condition;
    class copy_state;

    namespace protocol
    {
        class file_response_parsers;
        class list_shares_reader;
        class get_share_stats_reader;
    }

    typedef result_segment<cloud_file_share> share_result_segment;
    typedef result_iterator<cloud_file_share> share_result_iterator;

    /// <summary>
    /// Represents a set of access conditions to be used for operations against the File service. 
    /// Reserved for future support of file conditional headers.
    /// </summary>
    class file_access_condition
    {
    public:
        file_access_condition()
            : m_valid(false)
        {
        }

        bool is_valid() const
        {
            return m_valid;
        }

    private:

        bool m_valid;
    };

    /// <summary>
    /// Represents a shared access policy, which specifies the start time, expiry time, 
    /// and permissions for a shared access signature for a file or share.
    /// </summary>
    class file_shared_access_policy : public shared_access_policy
    {
    public:

        /// <summary>
        /// An enumeration describing permissions that may be used for a shared access signature.
        /// </summary>
        enum permissions
        {
            /// <summary>
            /// No permissions granted.
            /// </summary>
            none = 0,

            /// <summary>
            /// Permission granted to read the content, properties, metadata for a file or any files in a share. Use file as the source of a copy operation.
            /// </summary>
            read = 1,

            /// <summary>
            /// Permission granted to create or write the content, properties, metadata for a file or any files in a share. Resize the file. Use the file as the destination of a copy operation within the same account.
            /// </summary>
            write = 2,

            /// <summary>
            /// Permission granted to delete a file or any files in a share.
            /// </summary>
            del = 4,

            /// <summary>
            /// Permission granted to list the directories and files in a share.
            /// </summary>
            list = 8,

            /// <summary>
            /// Permission to write a new file, directory, or copy a file to a new file granted.
            /// </summary>
            create = 0x80
        };


        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::file_shared_access_policy" /> class.
        /// </summary>
        file_shared_access_policy()
            : shared_access_policy()
        {
        }


        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::file_shared_access_policy" /> class.
        /// </summary>
        /// <param name="expiry">The expiration date and time of the policy.</param>
        /// <param name="permission">A mask containing the permissions of the policy</param>
        file_shared_access_policy(utility::datetime expiry, uint8_t permission)
            : shared_access_policy(expiry, permission)
        {
        }


        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::file_shared_access_policy" /> class.
        /// </summary>
        /// <param name="start">The start date and time of the policy.</param>
        /// <param name="expiry">The expiration date and time of the policy.</param>
        /// <param name="permission">A mask containing the permissions of the policy</param>
        file_shared_access_policy(utility::datetime start, utility::datetime expiry, uint8_t permission)
            : shared_access_policy(start, expiry, permission)
        {
        }
    };

    /// <summary>
    /// Represents the permissions for a share.
    /// </summary>
    class file_share_permissions : public cloud_permissions<file_shared_access_policy>
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::file_share_permissions" /> class.
        /// </summary>
        file_share_permissions()
            : cloud_permissions()
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+,
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::file_share_permissions" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::file_share_permissions" /> object.</param>
        file_share_permissions(file_share_permissions&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::file_share_permissions" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::file_share_permissions" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::file_share_permissions" /> object with properties set.</returns>
        file_share_permissions& operator=(file_share_permissions&& other)
        {
            if (this != &other)
            {
                cloud_permissions::operator=(other);
            }
            return *this;
        }
#endif
    };

    /// <summary>
    /// Represents the optional headers that can be returned with a file accessed via a shared access signature.
    /// </summary>
    class cloud_file_shared_access_headers
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_shared_access_headers" /> class.
        /// </summary>
        cloud_file_shared_access_headers()
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_shared_access_headers" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_file_shared_access_headers" /> object.</param>
        cloud_file_shared_access_headers(cloud_file_shared_access_headers&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_file_shared_access_headers" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_file_shared_access_headers" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::cloud_file_shared_access_headers" /> object with properties set.</returns>
        cloud_file_shared_access_headers& operator=(cloud_file_shared_access_headers&& other)
        {
            if (this != &other)
            {
                m_cache_control = std::move(other.m_cache_control);
                m_content_disposition = std::move(other.m_content_disposition);
                m_content_encoding = std::move(other.m_content_encoding);
                m_content_language = std::move(other.m_content_language);
                m_content_type = std::move(other.m_content_type);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets the value of the cache-control header returned with the file.
        /// </summary>
        /// <returns>The cache-control value.</returns>
        const utility::string_t& cache_control() const
        {
            return m_cache_control;
        }

        /// <summary>
        /// Sets value of the cache-control header returned with the file.
        /// </summary>
        /// <param name="value">The cache-control value.</param>
        void set_cache_control(utility::string_t value)
        {
            m_cache_control = std::move(value);
        }

        /// <summary>
        /// Gets the value of the content-disposition header returned with the file.
        /// </summary>
        /// <returns>The content-disposition value.</returns>
        const utility::string_t& content_disposition() const
        {
            return m_content_disposition;
        }

        /// <summary>
        /// Sets the value of the content-disposition header returned with the file.
        /// </summary>
        /// <param name="value">The content-disposition value.</param>
        void set_content_disposition(utility::string_t value)
        {
            m_content_disposition = std::move(value);
        }

        /// <summary>
        /// Gets the value of the content-encoding header returned with the file.
        /// </summary>
        /// <returns>The content-encoding value.</returns>
        const utility::string_t& content_encoding() const
        {
            return m_content_encoding;
        }

        /// <summary>
        /// Sets the value of the content-encoding header returned with the file.
        /// </summary>
        /// <param name="value">The content-encoding value.</param>
        void set_content_encoding(utility::string_t value)
        {
            m_content_encoding = std::move(value);
        }

        /// <summary>
        /// Gets the value of the content-language header returned with the file.
        /// </summary>
        /// <returns>The content-language value.</returns>
        const utility::string_t& content_language() const
        {
            return m_content_language;
        }

        /// <summary>
        /// Sets the value of the content-language header returned with the file.
        /// </summary>
        /// <param name="value">The content-language value.</param>
        void set_content_language(utility::string_t value)
        {
            m_content_language = std::move(value);
        }

        /// <summary>
        /// Gets the value of the content-type header returned with the file.
        /// </summary>
        /// <returns>The content-type value.</returns>
        const utility::string_t& content_type() const
        {
            return m_content_type;
        }

        /// <summary>
        /// Sets the value of the content-type header returned with the file.
        /// </summary>
        /// <param name="value">The content-type value.</param>
        void set_content_type(utility::string_t value)
        {
            m_content_type = std::move(value);
        }

    private:

        utility::string_t m_cache_control;
        utility::string_t m_content_disposition;
        utility::string_t m_content_encoding;
        utility::string_t m_content_language;
        utility::string_t m_content_type;
    };

    /// <summary>
    /// Represents the system properties for a share.
    /// </summary>
    class cloud_file_share_properties
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_share_properties" /> class.
        /// </summary>
        cloud_file_share_properties()
            : m_quota(0)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.
        
        cloud_file_share_properties(cloud_file_share_properties&& other)
        {
            *this = std::move(other);
        }

        cloud_file_share_properties& operator=(cloud_file_share_properties&& other)
        {
            if (this != &other)
            {
                m_quota = other.m_quota;
                m_etag = std::move(other.m_etag);
                m_last_modified = std::move(other.m_last_modified);
            }
            return *this;
        }

#endif

        /// <summary>
        /// Gets the size of the share, in bytes.
        /// </summary>
        /// <returns>The share's size in bytes.</returns>
        utility::size64_t quota() const
        {
            return m_quota;
        }

        /// <summary>
        /// Sets the size of the share, in bytes.
        /// </summary>
        /// <param name="quota">The share's size in bytes.</param>
        void set_quota(utility::size64_t quota) 
        {
            m_quota = quota;
        }

        /// <summary>
        /// Gets the share's ETag value.
        /// </summary>
        /// <returns>The share's ETag value.</returns>
        const utility::string_t& etag() const
        {
            return m_etag;
        }

        /// <summary>
        /// Gets the last-modified time for the share, expressed as a UTC value.
        /// </summary>
        /// <returns>The share's last-modified time, in UTC format.</returns>
        utility::datetime last_modified() const
        {
            return m_last_modified;
        }

    private:

        utility::size64_t m_quota;
        utility::string_t m_etag;
        utility::datetime m_last_modified;

        void update_etag_and_last_modified(const cloud_file_share_properties& other);

        friend class cloud_file_share;
        friend class protocol::file_response_parsers;
        friend class protocol::list_shares_reader;
    };

    /// <summary>
    /// Represents a set of timeout and retry policy options that may be specified on a request against the File service.
    /// </summary>
    class file_request_options : public request_options
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::file_request_options" /> class.
        /// </summary>
        file_request_options()
            : request_options(),
            m_use_transactional_md5(false),
            m_disable_content_md5_validation(false),
            m_store_file_content_md5(false),
            m_parallelism_factor(1)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        file_request_options(file_request_options&& other)
        {
            *this = std::move(other);
        }

        file_request_options& operator=(file_request_options&& other)
        {
            if (this != &other)
            {
                request_options::operator=(std::move(other));
                m_use_transactional_md5 = other.m_use_transactional_md5;
                m_disable_content_md5_validation = other.m_disable_content_md5_validation;
                m_store_file_content_md5 = other.m_store_file_content_md5;
                m_parallelism_factor = other.m_parallelism_factor;
            }
            return *this;
        }
#endif

        /// <summary>
        /// Applies the default set of request options.
        /// </summary>
        /// <param name="other">A reference to a set of <see cref="azure::storage::file_request_options" />.</param>
        /// <param name="apply_expiry">Specifies that an expiry time be applied to the
        /// request options. This parameter is used internally.</param>
        void apply_defaults(const file_request_options& other, bool apply_expiry = true)
        {
            request_options::apply_defaults(other, apply_expiry);

            m_use_transactional_md5.merge(other.m_use_transactional_md5);
            m_disable_content_md5_validation.merge(other.m_disable_content_md5_validation);
            m_store_file_content_md5.merge(other.m_store_file_content_md5);
            m_parallelism_factor.merge(other.m_parallelism_factor);
        }

        /// <summary>
        /// Gets a value indicating whether the content-MD5 hash will be calculated and validated for the request.
        /// </summary>
        /// <returns><c>true</c> if the content-MD5 hash will be calculated and validated for the request; otherwise, <c>false</c>.</returns>
        bool use_transactional_md5() const
        {
            return m_use_transactional_md5;
        }

        /// <summary>
        /// Indicates whether to calculate and validate the content-MD5 hash for the request.
        /// </summary>
        /// <param name="value"><c>true</c> to calculate and validate the content-MD5 hash for the request; otherwise, <c>false</c>.</param>
        void set_use_transactional_md5(bool value)
        {
            m_use_transactional_md5 = value;
        }

        /// <summary>
        /// Gets a value indicating whether content-MD5 validation will be disabled when downloading files.
        /// </summary>
        /// <returns><c>true</c> to disable content-MD5 validation; otherwise, <c>false</c>.</returns>
        bool disable_content_md5_validation() const
        {
            return m_disable_content_md5_validation;
        }

        /// <summary>
        /// Indicates whether to disable content-MD5 validation when downloading files.
        /// </summary>
        /// <param name="value"><c>true</c> to disable content-MD5 validation; otherwise, <c>false</c>.</param>
        void set_disable_content_md5_validation(bool value)
        {
            m_disable_content_md5_validation = value;
        }

        /// <summary>
        /// Gets a value indicating whether the content-MD5 hash will be calculated and stored when uploading a file.
        /// </summary>
        /// <returns><c>true</c> to calculate and store the content-MD5 hash when uploading a file; otherwise, <c>false</c>.</returns>
        bool store_file_content_md5() const
        {
            return m_store_file_content_md5;
        }

        /// <summary>
        /// Indicates whether to calculate and store the content-MD5 hash when uploading a file.
        /// </summary>
        /// <param name="value"><c>true</c> to calculate and store the content-MD5 hash when uploading a file; otherwise, <c>false</c>.</param>
        void set_store_file_content_md5(bool value)
        {
            m_store_file_content_md5 = value;
        }

        /// <summary>
        /// Gets the number of ranges that may be simultaneously uploaded or downloaded when uploading or downloading a file that is greater than
        /// the value specified by the <see cref="single_blob_upload_threshold_in_bytes" /> property in size.
        /// </summary>
        /// <returns>The number of parallel range upload or download operations that may proceed.</returns>
        int parallelism_factor() const
        {
            return m_parallelism_factor;
        }

        /// <summary>
        /// Sets the number of ranges that may be simultaneously uploaded or downloaded when uploading or downloading a file that is greater than
        /// the value specified by the <see cref="single_blob_upload_threshold_in_bytes" /> property in size.
        /// </summary>
        /// <param name="value">The number of parallel range upload or download operations that may proceed.</param>
        void set_parallelism_factor(int value)
        {
            m_parallelism_factor = value;
        }

    private:

        option_with_default<bool> m_use_transactional_md5;
        option_with_default<bool> m_disable_content_md5_validation;
        option_with_default<bool> m_store_file_content_md5;
        option_with_default<int> m_parallelism_factor;
    };

    /// <summary>
    /// Provides a client-side logical representation of the Windows Azure File Service. This client is used to configure and execute requests against the File Service.
    /// </summary>
    /// <remarks>The service client encapsulates the base URI for the File service. If the service client will be used for authenticated access, it also encapsulates the credentials for accessing the storage account.</remarks>
    class cloud_file_client : public cloud_client
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_client" /> class.
        /// </summary>
        cloud_file_client()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_client" /> class using the specified File service endpoint
        /// and anonymous credentials.
        /// </summary>
        /// <param name="base_uri">An <see cref="azure::storage::storage_uri" /> object containing the File service endpoint for all locations.</param>
        explicit cloud_file_client(storage_uri base_uri)
            : cloud_client(std::move(base_uri))
        {
            initialize();
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_client" /> class using the specified File service endpoint
        /// and account credentials.
        /// </summary>
        /// <param name="base_uri">An <see cref="azure::storage::storage_uri" /> object containing the File service endpoint for all locations.</param>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        cloud_file_client(storage_uri base_uri, storage_credentials credentials)
            : cloud_client(std::move(base_uri), std::move(credentials))
        {
            initialize();
        }


        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_client" /> class using the specified File service endpoint
        /// and account credentials.
        /// </summary>
        /// <param name="base_uri">An <see cref="azure::storage::storage_uri" /> object containing the File service endpoint for all locations.</param>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        /// <param name="default_request_options">The default <see cref="azure::storage::file_request_options" /> object to use for all requests made with this client object.</param>
        cloud_file_client(storage_uri base_uri, storage_credentials credentials, file_request_options default_request_options)
            : cloud_client(std::move(base_uri), std::move(credentials)), m_default_request_options(std::move(default_request_options))
        {
            initialize();
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        cloud_file_client(cloud_file_client&& other)
        {
            *this = std::move(other);
        }

        cloud_file_client& operator=(cloud_file_client&& other)
        {
            if (this != &other)
            {
                cloud_client::operator=(std::move(other));
                m_default_request_options = std::move(other.m_default_request_options);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Sets the authentication scheme to use to sign HTTP requests.
        /// </summary>
        /// <param name="value">The authentication scheme.</param>
        WASTORAGE_API void set_authentication_scheme(azure::storage::authentication_scheme value) override;

        /// <summary>
        /// Returns an <see cref="azure::storage::share_result_iterator" /> that can be used to to lazily enumerate a collection of shares.
        /// </summary>
        /// <returns>An <see cref="azure::storage::share_result_iterator" /> that can be used to to lazily enumerate a collection of shares.</returns>
        share_result_iterator list_shares()
        {
            return list_shares(utility::string_t(), false, 0, file_request_options(), operation_context());
        }

        /// <summary>
        /// Returns an <see cref="azure::storage::share_result_iterator" /> that can be used to to lazily enumerate a collection of shares.
        /// </summary>
        /// <param name="prefix">The shares name prefix.</param>
        /// <returns>An <see cref="azure::storage::share_result_iterator" /> that can be used to to lazily enumerate a collection of shares.</returns>
        share_result_iterator list_shares(const utility::string_t& prefix)
        {
            return list_shares(prefix, false, 0, file_request_options(), operation_context());
        }


        /// <summary>
        /// Returns an <see cref="azure::storage::share_result_iterator" /> that can be used to to lazily enumerate a collection of shares.
        /// </summary>
        /// <param name="prefix">The share name prefix.</param>
        /// <param name="get_metadata">A flag that specifies whether to retrieve share metadata.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned.
        /// If this value is zero, the maximum possible number of results will be returned.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An <see cref="azure::storage::share_result_iterator" /> that can be used to to lazily enumerate a collection of shares.</returns>
        WASTORAGE_API share_result_iterator list_shares(const utility::string_t& prefix, bool get_metadata, int max_results, const file_request_options& options, operation_context context);

        /// <summary>
        /// Returns a result segment containing a collection of <see cref="azure::storage::cloud_file_share" /> objects.
        /// </summary>
        /// <param name="token">An <see cref="azure::storage::continuation_token" /> returned by a previous listing operation.</param>
        /// <returns>An <see cref="azure::storage::share_result_segment" /> containing a collection of shares.</returns>
        share_result_segment list_shares_segmented(const continuation_token& token)
        {
            return list_shares_segmented_async(token).get();
        }

        /// <summary>
        /// Returns a result segment containing a collection of <see cref="azure::storage::cloud_file_share" /> objects.
        /// </summary>
        /// <param name="prefix">The share name prefix.</param>
        /// <param name="token">An <see cref="azure::storage::continuation_token" /> returned by a previous listing operation.</param>
        /// <returns>An <see cref="azure::storage::share_result_segment" /> containing a collection of shares.</returns>
        share_result_segment list_shares_segmented(const utility::string_t& prefix, const continuation_token& token)
        {
            return list_shares_segmented_async(prefix, token).get();
        }

        /// <summary>
        /// Returns a result segment containing a collection of <see cref="azure::storage::cloud_file_share" /> objects.
        /// </summary>
        /// <param name="prefix">The share name prefix.</param>
        /// <param name="get_metadata">A flag that specifies whether to retrieve share metadata.</param>
        /// <param name="token">An <see cref="azure::storage::continuation_token" /> returned by a previous listing operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An <see cref="azure::storage::share_result_segment" /> containing a collection of shares.</returns>
        share_result_segment list_shares_segmented(const utility::string_t& prefix, bool get_metadata, int max_results, const continuation_token& token, const file_request_options& options, operation_context context)
        {
            return list_shares_segmented_async(prefix, get_metadata, max_results, token, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to return a result segment containing a collection of <see cref="azure::storage::cloud_file_share" /> objects.
        /// </summary>
        /// <param name="token">An <see cref="azure::storage::continuation_token" /> returned by a previous listing operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::share_result_segment" /> that represents the current operation.</returns>
        pplx::task<share_result_segment> list_shares_segmented_async(const continuation_token& token)
        {
            return list_shares_segmented_async(utility::string_t(), token);
        }

        /// <summary>
        /// Intitiates an asynchronous operation to return a result segment containing a collection of <see cref="azure::storage::cloud_file_share" /> objects.
        /// </summary>
        /// <param name="prefix">The share name prefix.</param>
        /// <param name="token">An <see cref="azure::storage::continuation_token" /> returned by a previous listing operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::share_result_segment" /> that represents the current operation.</returns>
        pplx::task<share_result_segment> list_shares_segmented_async(const utility::string_t& prefix, const continuation_token& token)
        {
            return list_shares_segmented_async(prefix, false, 0, token, file_request_options(), operation_context());
        }
        /// <summary>
        /// Intitiates an asynchronous operation to return a result segment containing a collection of <see cref="azure::storage::cloud_file_share" /> objects.
        /// </summary>
        /// <param name="prefix">The share name prefix.</param>
        /// <param name="get_metadata">A flag that specifies whether to retrieve share metadata.</param>
        /// <param name="token">An <see cref="azure::storage::continuation_token" /> returned by a previous listing operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An <see cref="azure::storage::share_result_segment" /> containing a collection of shares.</returns>
        WASTORAGE_API pplx::task<share_result_segment> list_shares_segmented_async(const utility::string_t& prefix, bool get_metadata, int max_results, const continuation_token& token, const file_request_options& options, operation_context context);

        /// <summary>
        /// Gets the service properties for the File service client.
        /// </summary>
        /// <returns>The <see cref="azure::storage::service_properties" /> for the File service client.</returns>
        service_properties download_service_properties() const
        {
            return download_service_properties_async().get();
        }

        /// <summary>
        /// Gets the service properties for the File service client.
        /// </summary>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The <see cref="azure::storage::service_properties" /> for the File service client.</returns>
        service_properties download_service_properties(const file_request_options& options, operation_context context) const
        {
            return download_service_properties_async(options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to get the properties of the service.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::service_properties" /> that represents the current operation.</returns>
        pplx::task<service_properties> download_service_properties_async() const
        {
            return download_service_properties_async(file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to get the properties of the service.
        /// </summary>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::service_properties" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<service_properties> download_service_properties_async(const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Sets the service properties for the File service client.
        /// </summary>
        /// <param name="properties">The <see cref="azure::storage::service_properties" /> for the File service client.</param>
        /// <param name="includes">An <see cref="azure::storage::service_properties_includes" /> enumeration describing which items to include when setting service properties.</param>
        void upload_service_properties(const service_properties& properties, const service_properties_includes& includes) const
        {
            upload_service_properties_async(properties, includes).wait();
        }

        /// <summary>
        /// Sets the service properties for the File service client.
        /// </summary>
        /// <param name="properties">The <see cref="azure::storage::service_properties" /> for the File service client.</param>
        /// <param name="includes">An <see cref="azure::storage::service_properties_includes" /> enumeration describing which items to include when setting service properties.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <param name="includes">An <see cref="azure::storage::service_properties_includes" /> enumeration describing which items to include when setting service properties.</param>
        void upload_service_properties(const service_properties& properties, const service_properties_includes& includes, const file_request_options& options, operation_context context) const
        {
            upload_service_properties_async(properties, includes, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to set the service properties for the File service client.
        /// </summary>
        /// <param name="properties">The <see cref="azure::storage::service_properties" /> for the File service client.</param>
        /// <param name="includes">An <see cref="azure::storage::service_properties_includes" /> enumeration describing which items to include when setting service properties.</param>
        /// <param name="includes">An <see cref="azure::storage::service_properties_includes" /> enumeration describing which items to include when setting service properties.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_service_properties_async(const service_properties& properties, const service_properties_includes& includes) const
        {
            return upload_service_properties_async(properties, includes, file_request_options(), operation_context());
        }
        /// <summary>
        /// Intitiates an asynchronous operation to set the service properties for the File service client.
        /// </summary>
        /// <param name="properties">The <see cref="azure::storage::service_properties" /> for the File service client.</param>
        /// <param name="includes">An <see cref="azure::storage::service_properties_includes" /> enumeration describing which items to include when setting service properties.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <param name="includes">An <see cref="azure::storage::service_properties_includes" /> enumeration describing which items to include when setting service properties.</param>
        WASTORAGE_API pplx::task<void> upload_service_properties_async(const service_properties& properties, const service_properties_includes& includes, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_file_share" /> object with the specified name.
        /// </summary>
        /// <param name="container_name">The name of the share, or an absolute URI to the container.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_file_share" />.</returns>
        WASTORAGE_API cloud_file_share get_share_reference(utility::string_t share_name) const;

        /// <summary>
        /// Returns the default set of request options.
        /// </summary>
        /// <returns>An <see cref="azure::storage::file_request_options" /> object.</returns>
        const file_request_options& default_request_options() const
        {
            return m_default_request_options;
        }

    private:

        void initialize()
        {
            set_authentication_scheme(azure::storage::authentication_scheme::shared_key);
            if (!m_default_request_options.retry_policy().is_valid())
                m_default_request_options.set_retry_policy(exponential_retry_policy());
        }

        file_request_options m_default_request_options;
    };

    /// <summary>
    /// Represents a share in the Windows Azure File service.
    /// </summary>
    class cloud_file_share
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_share" /> class.
        /// </summary>
        cloud_file_share() {}

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_share" /> class.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the share for all locations.</param>
        WASTORAGE_API cloud_file_share(storage_uri uri);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_share" /> class.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the share for all locations.</param>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        WASTORAGE_API cloud_file_share(storage_uri uri, storage_credentials credentials);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_share" /> class.
        /// </summary>
        /// <param name="name">The name of the share.</param>
        /// <param name="client">The File service client.</param>
        WASTORAGE_API cloud_file_share(utility::string_t name, cloud_file_client client);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_share" /> class.
        /// </summary>
        /// <param name="name">The name of the share.</param>
        /// <param name="client">The File service client.</param>
        /// <param name="properties">The properties for the container.</param>
        /// <param name="metadata">The metadata for the container.</param>
        WASTORAGE_API cloud_file_share(utility::string_t name, cloud_file_client client, cloud_file_share_properties properties, cloud_metadata metadata);

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        cloud_file_share(cloud_file_share&& other)
        {
            *this = std::move(other);
        }

        cloud_file_share& operator=(cloud_file_share&& other)
        {
            if (this != &other)
            {
                m_name = std::move(other.m_name);
                m_client = std::move(other.m_client);
                m_uri = std::move(other.m_uri);
                m_metadata = std::move(other.m_metadata);
                m_properties = std::move(other.m_properties);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Creates the share.
        /// </summary>
        void create()
        {
            create_async().wait();
        }

        /// <summary>
        /// Creates the share and specifies the size of the share.
        /// </summary>
        /// <param name="max_size">An <see cref="utility::size64_t" /> value that specifies the size of the share.</param>
        void create(utility::size64_t max_size)
        {
            create_async(max_size).wait();
        }

        /// <summary>
        /// Creates the share.
        /// </summary>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        void create(const file_request_options& options, operation_context context)
        {
            create_async(options, context).wait();
        }

        /// <summary>
        /// Creates the share and specifies the size of the share.
        /// </summary>
        /// <param name="max_size">An <see cref="utility::size64_t" /> value that specifies the size of the share.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        void create(utility::size64_t max_size, const file_request_options& options, operation_context context)
        {
            create_async(max_size, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to create the share.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> create_async()
        {
            return create_async(file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to create the share.
        /// </summary>
        /// <param name="max_size">An <see cref="utility::size64_t" /> value that specifies the size of the share.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> create_async(utility::size64_t max_size)
        {
            return create_async(max_size, file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to create the share.
        /// </summary>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> create_async(const file_request_options& options, operation_context context);

        /// <summary>
        /// Intitiates an asynchronous operation to create the share.
        /// </summary>
        /// <param name="max_size">An <see cref="utility::size64_t" /> value that specifies the size of the share.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> create_async(utility::size64_t max_size, const file_request_options& options, operation_context context);

        /// <summary>
        /// Creates the share if it does not already exist.
        /// </summary>
        /// <returns><c>true</c> if the share did not already exist and was created; otherwise <c>false</c>.</returns>
        bool create_if_not_exists()
        {
            return create_if_not_exists_async().get();
        }

        /// <summary>
        /// Creates the share if it does not already exist.
        /// </summary>
        /// <param name="max_size">An <see cref="utility::size64_t" /> value that specifies the size of the share.</param>
        /// <returns><c>true</c> if the share did not already exist and was created; otherwise <c>false</c>.</returns>
        bool create_if_not_exists(utility::size64_t max_size)
        {
            return create_if_not_exists_async(max_size).get();
        }

        /// <summary>
        /// Creates the share if it does not already exist.
        /// </summary>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns><c>true</c> if the share did not already exist and was created; otherwise <c>false</c>.</returns>
        bool create_if_not_exists(const file_request_options& options, operation_context context)
        {
            return create_if_not_exists_async(options, context).get();
        }

        /// <summary>
        /// Creates the share if it does not already exist.
        /// </summary>
        /// <param name="max_size">An <see cref="utility::size64_t" /> value that specifies the size of the share.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns><c>true</c> if the share did not already exist and was created; otherwise <c>false</c>.</returns>
        bool create_if_not_exists(utility::size64_t max_size, const file_request_options& options, operation_context context)
        {
            return create_if_not_exists_async(max_size, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to create the share.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<bool> create_if_not_exists_async()
        {
            return create_if_not_exists_async(file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to create the share if it does not already exist and specify the size of the share.
        /// </summary>
        /// <param name="max_size">An <see cref="utility::size64_t" /> value that specifies the size of the share.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<bool> create_if_not_exists_async(utility::size64_t max_size)
        {
            return create_if_not_exists_async(max_size, file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to create the share.
        /// </summary>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<bool> create_if_not_exists_async(const file_request_options& options, operation_context context);

        /// <summary>
        /// Intitiates an asynchronous operation to create the share if it does not already exist and specify the size of the share.
        /// </summary>
        /// <param name="max_size">An <see cref="utility::size64_t" /> value that specifies the size of the share.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<bool> create_if_not_exists_async(utility::size64_t max_size, const file_request_options& options, operation_context context);

        /// <summary>
        /// Deletes the share.
        /// </summary>
        void delete_share()
        {
            return delete_share_async().get();
        }

        /// <summary>
        /// Deletes the share.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void delete_share(const file_access_condition& condition, const file_request_options& options, operation_context context)
        {
            return delete_share_async(condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to delete the share.
        /// </summary>
        pplx::task<void> delete_share_async()
        {
            return delete_share_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to delete the share.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        WASTORAGE_API pplx::task<void> delete_share_async(const file_access_condition& condition, const file_request_options& options, operation_context context);

        /// <summary>
        /// Deletes the share if it already exists.
        /// </summary>
        /// <returns><c>true</c> if the share did not already exist and was deleted; otherwise <c>false</c>.</returns>
        bool delete_share_if_exists()
        {
            return delete_share_if_exists_async().get();
        }

        /// <summary>
        /// Deletes the share if it already exists.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns><c>true</c> if the share did not already exist and was deleted; otherwise <c>false</c>.</returns>
        bool delete_share_if_exists(const file_access_condition& condition, const file_request_options& options, operation_context context)
        {
            return delete_share_if_exists_async(condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to delete the share if it already exists.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<bool> delete_share_if_exists_async()
        {
            return delete_share_if_exists_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to delete the share if it already exists.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<bool> delete_share_if_exists_async(const file_access_condition& condition, const file_request_options& options, operation_context context);

        /// <summary>
        /// Checks existence of the share.
        /// </summary>
        /// <returns><c>true</c> if the share exists.</returns>
        bool exists()
        {
            return exists_async().get();
        }

        /// <summary>
        /// Checks existence of the share.
        /// </summary>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns><c>true</c> if the share exists.</returns>
        bool exists(const file_request_options& options, operation_context context)
        {
            return exists_async(options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to check the existence of the share.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<bool> exists_async()
        {
            return exists_async(file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to check the existence of the share.
        /// </summary>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<bool> exists_async(const file_request_options& options, operation_context context)
        {
            return exists_async(false, options, context);
        }

        /// <summary>
        /// Retrieves the share's attributes.
        /// </summary>
        void download_attributes()
        {
            download_attributes_async().wait();
        }

        /// <summary>
        /// Retrieves the share's attributes.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void download_attributes(const file_access_condition& condition, const file_request_options& options, operation_context context)
        {
            download_attributes_async(condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to retrieve the share's attributes.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> download_attributes_async()
        {
            return download_attributes_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to retrieve the share's attributes.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> download_attributes_async(const file_access_condition& condition, const file_request_options& options, operation_context context);

        /// <summary>
        /// Updates the share's metadata.
        /// </summary>
        void upload_metadata() const
        {
            upload_metadata_async().wait();
        }

        /// <summary>
        /// Updates the share's metadata.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_metadata(const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            upload_metadata_async(condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to update the share's metadata.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> upload_metadata_async() const
        {
            return upload_metadata_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to update the share's metadata.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_metadata_async(const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Retrieves the share's statistics.
        /// </summary>
        /// <returns>The size number in gigabyte of used data for this share.</returns>
        int32_t download_share_usage() const
        {
            return download_share_usage_async().get();
        }

        /// <summary>
        /// Retrieves the share's statistics.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The size number in gigabyte of used data for this share.</returns>
        int32_t download_share_usage(const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            return download_share_usage_aysnc(condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to retrieve the share's statistics.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<int32_t> download_share_usage_async() const
        {
            return download_share_usage_aysnc(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to retrieve the share's statistics.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<int32_t> download_share_usage_aysnc(const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Gets permissions settings for the share.
        /// </summary>
        /// <returns>The share's permissions.</returns>
        file_share_permissions download_permissions() const
        {
            return download_permissions_async().get();
        }

        /// <summary>
        /// Gets permissions settings for the share.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The share's permissions.</returns>
        file_share_permissions download_permissions(const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            return download_permissions_async(condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to get permissions settings for the share.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::file_share_permissions" /> that represents the current operation.</returns>
        pplx::task<file_share_permissions> download_permissions_async() const
        {
            return download_permissions_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to get permissions settings for the share.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::file_share_permissions" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<file_share_permissions> download_permissions_async(const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Uploads permissions settings for the share.
        /// </summary>
        /// <param name="permissions">The permissions to apply to the share.</param>
        void upload_permissions(const file_share_permissions& permissions) const
        {
            return upload_permissions_async(permissions).get();
        }

        /// <summary>
        /// Uploads permissions settings for the share.
        /// </summary>
        /// <param name="permissions">The permissions to apply to the share.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_permissions(const file_share_permissions& permissions, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            return upload_permissions_async(permissions, condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to upload permissions settings for the share.
        /// </summary>
        /// <param name="permissions">The permissions to apply to the share.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> upload_permissions_async(const file_share_permissions& permissions) const
        {
            return upload_permissions_async(permissions, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to upload permissions settings for the share.
        /// </summary>
        /// <param name="permissions">The permissions to apply to the share.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_permissions_async(const file_share_permissions& permissions, const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Resize the share.
        /// </summary>
        /// <param name="quota">The size to apply to the share.</param>
        void resize(utility::size64_t quota)
        {
            resize_async(quota).wait();
        }

        /// <summary>
        /// Resize the share.
        /// </summary>
        /// <param name="quota">The size to apply to the share.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void resize(utility::size64_t quota, const file_access_condition& condition, const file_request_options& options, operation_context context)
        {
            resize_async(quota, condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to resize the share.
        /// </summary>
        /// <param name="quota">The size to apply to the share.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> resize_async(utility::size64_t quota)
        {
            return resize_async(quota, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to resize the share.
        /// </summary>
        /// <param name="quota">The size to apply to the share.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> resize_async(utility::size64_t quota, const file_access_condition& condition, const file_request_options& options, operation_context context);

        /// <summary>
        /// Returns a shared access signature for the share.
        /// </summary>
        /// <param name="policy">The access policy for the shared access signature.</param>
        /// <returns>A string containing a shared access signature.</returns>
        utility::string_t get_shared_access_signature(const file_shared_access_policy& policy) const
        {
            return get_shared_access_signature(policy, utility::string_t(), cloud_file_shared_access_headers());
        }

        /// <summary>
        /// Returns a shared access signature for the share.
        /// </summary>
        /// <param name="policy">The access policy for the shared access signature.</param>
        /// <param name="stored_policy_identifier">A share-level access policy.</param>
        /// <returns>A string containing a shared access signature.</returns>
        WASTORAGE_API utility::string_t get_shared_access_signature(const file_shared_access_policy& policy, const utility::string_t& stored_policy_identifier) const
        {
            return get_shared_access_signature(policy, stored_policy_identifier, cloud_file_shared_access_headers());
        }

        /// <summary>
        /// Returns a shared access signature for the share.
        /// </summary>
        /// <param name="policy">The access policy for the shared access signature.</param>
        /// <param name="stored_policy_identifier">A share-level access policy.</param>
        /// <param name="headers">The optional header values to set for a share returned with this SAS.</param>
        /// <returns>A string containing a shared access signature.</returns>
        WASTORAGE_API utility::string_t get_shared_access_signature(const file_shared_access_policy& policy, const utility::string_t& stored_policy_identifier, const cloud_file_shared_access_headers& headers) const;

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_file_directory" /> object.
        /// </summary>
        /// <returns>A reference to the root directory, of type <see cref="azure::storage::cloud_file_directory" />.</returns>
        WASTORAGE_API cloud_file_directory get_root_directory_reference() const;

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_file_directory" /> object with the specified name.
        /// </summary>
        /// <param name="name">The name of the directory.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_file_directory" />.</returns>
        WASTORAGE_API cloud_file_directory get_directory_reference(utility::string_t name) const;

        /// <summary>
        /// Gets the share's name.
        /// </summary>
        /// <returns>The share's name.</returns>
        utility::string_t name() const
        {
            return m_name;
        }

        /// <summary>
        /// Gets the <see cref="azure::storage::cloud_file_client" /> object that represents the File service.
        /// </summary>
        /// <returns>A client object that specifies the File service endpoint.</returns>
        const cloud_file_client& service_client() const
        {
            return m_client;
        }

        /// <summary>
        /// Gets the file URI for all locations.
        /// </summary>
        /// <returns>An <see cref="azure::storage::storage_uri" /> containing the share URI for all locations.</returns>
        const storage_uri& uri() const
        {
            return m_uri;
        }

        /// <summary>
        /// Gets the share's system metadata.
        /// </summary>
        /// <returns>The share's metadata.</returns>
        cloud_metadata& metadata()
        {
            return *m_metadata;
        }

        /// <summary>
        /// Gets the share's system metadata.
        /// </summary>
        /// <returns>The share's metadata.</returns>
        const cloud_metadata& metadata() const
        {
            return *m_metadata;
        }

        /// <summary>
        /// Gets the share's system properties.
        /// </summary>
        /// <returns>The share's properties.</returns>
        cloud_file_share_properties& properties()
        {
            return *m_properties;
        }

        /// <summary>
        /// Gets the share's system properties.
        /// </summary>
        /// <returns>The share's properties.</returns>
        const cloud_file_share_properties& properties() const
        {
            return *m_properties;
        }

        /// <summary>
        /// Indicates whether the <see cref="azure::storage::cloud_file_share" /> object is valid.
        /// </summary>
        /// <returns><c>true</c> if the <see cref="azure::storage::cloud_file_share" /> object is valid; otherwise, <c>false</c>.</returns>
        bool is_valid() const
        {
            return !m_name.empty();
        }

    private:

        WASTORAGE_API pplx::task<bool> exists_async(bool primary_only, const file_request_options& options, operation_context context);

        static cloud_file_client create_service_client(const storage_uri& uri, storage_credentials credentials);
        static utility::string_t read_share_name(const storage_uri& uri);
        static storage_uri create_uri(const storage_uri& uri);

        utility::string_t m_name;
        cloud_file_client m_client;
        storage_uri m_uri;
        std::shared_ptr<cloud_metadata> m_metadata;
        std::shared_ptr<cloud_file_share_properties> m_properties;
    };

    class list_file_and_directory_item;

    typedef result_segment<list_file_and_directory_item> list_file_and_directory_result_segment;
    typedef result_iterator<list_file_and_directory_item> list_file_and_diretory_result_iterator;
    
    class list_file_and_directory_item;

    class cloud_file_directory_properties
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_directory_properties" /> class.
        /// </summary>
        cloud_file_directory_properties()
            : m_server_encrypted(false)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        cloud_file_directory_properties(cloud_file_directory_properties&& other)
        {
            *this = std::move(other);
        }

        cloud_file_directory_properties& operator=(cloud_file_directory_properties&& other)
        {
            if (this != &other)
            {
                m_etag = std::move(other.m_etag);
                m_last_modified = std::move(other.m_last_modified);
            }
            return *this;
        }

#endif

        /// <summary>
        /// Gets the directory's ETag value.
        /// </summary>
        /// <returns>The directory's ETag value.</returns>
        const utility::string_t& etag() const
        {
            return m_etag;
        }

        /// <summary>
        /// Gets the last-modified time for the directory, expressed as a UTC value.
        /// </summary>
        /// <returns>The share's last-modified time, in UTC format.</returns>
        utility::datetime last_modified() const
        {
            return m_last_modified;
        }

        /// <summary>
        /// Gets if the server is encrypted.
        /// </summary>
        /// <returns><c>true</c> if a server is encrypted.</returns>
        bool server_encrypted()
        {
            return m_server_encrypted;
        }

        /// <summary>
        /// Sets if the server is encrypted.
        /// </summary>
        /// <param name="value">If the server is encrypted.</param>
        void set_server_encrypted(bool value)
        {
            m_server_encrypted = value;
        }

    private:

        utility::string_t m_etag;
        utility::datetime m_last_modified;

        void update_etag_and_last_modified(const cloud_file_directory_properties& other);
        void update_etag(const cloud_file_directory_properties& other);
        bool m_server_encrypted;

        friend class cloud_file_directory;
        friend class protocol::file_response_parsers;
    };

    /// <summary>
    /// Represents a directory of files.
    /// </summary>
    /// <remarks>Shares, which are encapsulated as <see cref="azure::storage::cloud_file_share" /> objects, hold directories, and directories hold files. Directories can also contain sub-directories.</remarks>
    class cloud_file_directory
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_directory" /> class.
        /// </summary>
        cloud_file_directory()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_directory" /> class.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the directory for all locations.</param>
        WASTORAGE_API cloud_file_directory(storage_uri uri);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_directory" /> class.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the directory for all locations.</param>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        WASTORAGE_API cloud_file_directory(storage_uri uri, storage_credentials credentials);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_directory" /> class.
        /// </summary>
        /// <param name="name">The name of the directory.</param>
        /// <param name="share">The File share <see ref="azure::storage::cloud_file_share"> it blongs to.</param>
        WASTORAGE_API cloud_file_directory(utility::string_t name, cloud_file_share share);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_directory" /> class.
        /// </summary>
        /// <param name="name">The name of the directory.</param>
        /// <param name="share">The File share <see ref="azure::storage::cloud_file_share"> it blongs to.</param>
        /// <param name="properties">A set of properties <see ref="azure::storage::cloud_file_directory_properties"> for the directory.</param>
        /// <param name="metadata">A collection of name-value pairs defining the metadata of the directory.</param>
        WASTORAGE_API cloud_file_directory(utility::string_t name, cloud_file_share share, cloud_file_directory_properties properties, cloud_metadata metadata);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_directory" /> class.
        /// </summary>
        /// <param name="name">The name of the directory.</param>
        /// <param name="share">The File directory <see ref="azure::storage::cloud_file_directory"> it blongs to.</param>
        WASTORAGE_API cloud_file_directory(utility::string_t name, cloud_file_directory directory);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_directory" /> class.
        /// </summary>
        /// <param name="name">The name of the directory.</param>
        /// <param name="share">The File directory <see ref="azure::storage::cloud_file_directory"> it blongs to.</param>
        /// <param name="properties">A set of properties <see ref="azure::storage::cloud_file_directory_properties"> for the directory.</param>
        /// <param name="metadata">A collection of name-value pairs defining the metadata of the directory.</param>
        WASTORAGE_API cloud_file_directory(utility::string_t name, cloud_file_directory directory, cloud_file_directory_properties properties, cloud_metadata metadata);

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        cloud_file_directory(cloud_file_directory&& other)
        {
            *this = std::move(other);
        }

        cloud_file_directory& operator=(cloud_file_directory&& other)
        {
            if (this != &other)
            {
                m_name = std::move(other.m_name);
                m_share = std::move(other.m_share);
                m_uri = std::move(other.m_uri);
                m_metadata = std::move(other.m_metadata);
                m_properties = std::move(other.m_properties);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Returns an <see cref="azure::storage::list_file_and_diretory_result_iterator" /> that can be used to to lazily enumerate a collection of file or directory items.
        /// </summary>
        /// <returns>An <see cref="azure::storage::list_file_and_diretory_result_iterator" /> that can be used to to lazily enumerate a collection of file or directory items in the the directory.</returns>
        list_file_and_diretory_result_iterator list_files_and_directories() const
        {
            return list_files_and_directories(0);
        }

        /// <summary>
        /// Returns an <see cref="azure::storage::list_file_and_diretory_result_iterator" /> that can be used to to lazily enumerate a collection of file or directory items.
        /// </summary>
        /// <param name="prefix">The file/directory name prefix.</param>
        /// <returns>An <see cref="azure::storage::list_file_and_diretory_result_iterator" /> that can be used to to lazily enumerate a collection of file or directory items in the the directory.</returns>
        list_file_and_diretory_result_iterator list_files_and_directories(const utility::string_t& prefix) const
        {
            return list_files_and_directories(prefix, 0);
        }

        /// <summary>
        /// Returns an <see cref="azure::storage::list_file_and_diretory_result_iterator" /> that can be used to to lazily enumerate a collection of file or directory items.
        /// </summary>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned.
        /// If this value is zero, the maximum possible number of results will be returned.</param>
        /// <returns>An <see cref="azure::storage::list_file_and_diretory_result_iterator" /> that can be used to to lazily enumerate a collection of file or directory items in the the directory.</returns>
        list_file_and_diretory_result_iterator list_files_and_directories(int64_t max_results) const
        {
            return list_files_and_directories(utility::string_t(), max_results);
        }

        /// <summary>
        /// Returns an <see cref="azure::storage::list_file_and_diretory_result_iterator" /> that can be used to to lazily enumerate a collection of file or directory items.
        /// </summary>
        /// <param name="prefix">The file/directory name prefix.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned.
        /// If this value is zero, the maximum possible number of results will be returned.</param>
        /// <returns>An <see cref="azure::storage::list_file_and_diretory_result_iterator" /> that can be used to to lazily enumerate a collection of file or directory items in the the directory.</returns>
        list_file_and_diretory_result_iterator list_files_and_directories(const utility::string_t& prefix, int64_t max_results) const
        {
            return list_files_and_directories(prefix, max_results, file_request_options(), operation_context());
        }

        /// <summary>
        /// Returns an <see cref="azure::storage::list_file_and_diretory_result_iterator" /> that can be used to to lazily enumerate a collection of file or directory items.
        /// </summary>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned.
        /// If this value is zero, the maximum possible number of results will be returned.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An <see cref="azure::storage::list_file_and_diretory_result_iterator" /> that can be used to to lazily enumerate a collection of file or directory items in the the directory.</returns>
        list_file_and_diretory_result_iterator list_files_and_directories(int64_t max_results, const file_request_options& options, operation_context context) const
        {
            return list_files_and_directories(utility::string_t(), max_results, options, context);
        }
        
        /// <summary>
        /// Returns an <see cref="azure::storage::list_file_and_diretory_result_iterator" /> that can be used to to lazily enumerate a collection of file or directory items.
        /// </summary>
        /// <param name="prefix">The file/directory name prefix.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned.
        /// If this value is zero, the maximum possible number of results will be returned.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An <see cref="azure::storage::list_file_and_diretory_result_iterator" /> that can be used to to lazily enumerate a collection of file or directory items in the the directory.</returns>
        WASTORAGE_API list_file_and_diretory_result_iterator list_files_and_directories(const utility::string_t& prefix, int64_t max_results, const file_request_options& options, operation_context context) const;
        
        /// <summary>
        /// Returns a result segment <see cref="azure::storage::list_file_and_directory_result_segment" /> that can be used to to lazily enumerate a collection of file or directory items.
        /// </summary>
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <returns>An <see cref="azure::storage::list_file_and_directory_result_segment" /> that can be used to to lazily enumerate a collection of file or directory items in the the directory.</returns>
        list_file_and_directory_result_segment list_files_and_directories_segmented(const continuation_token& token) const
        {
            return list_files_and_directories_segmented_async(token).get();
        }

        /// <summary>
        /// Returns a result segment <see cref="azure::storage::list_file_and_directory_result_segment" /> that can be used to to lazily enumerate a collection of file or directory items.
        /// </summary>
        /// <param name="prefix">The file/directory name prefix.</param>
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <returns>An <see cref="azure::storage::list_file_and_directory_result_segment" /> that can be used to to lazily enumerate a collection of file or directory items in the the directory.</returns>
        list_file_and_directory_result_segment list_files_and_directories_segmented(const utility::string_t& prefix, const continuation_token& token) const
        {
            return list_files_and_directories_segmented_async(prefix, token).get();
        }

        /// <summary>
        /// Returns a result segment <see cref="azure::storage::list_file_and_directory_result_segment" /> that can be used to to lazily enumerate a collection of file or directory items.
        /// </summary>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned.
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An <see cref="azure::storage::list_file_and_directory_result_segment" /> that can be used to to lazily enumerate a collection of file or directory items in the the directory.</returns>
        list_file_and_directory_result_segment list_files_and_directories_segmented(int64_t max_results, const continuation_token& token, const file_request_options& options, operation_context context) const
        {
            return list_files_and_directories_segmented_async(max_results, token, options, context).get();
        }

        /// <summary>
        /// Returns a result segment <see cref="azure::storage::list_file_and_directory_result_segment" /> that can be used to to lazily enumerate a collection of file or directory items.
        /// </summary>
        /// <param name="prefix">The file/directory name prefix.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned.
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An <see cref="azure::storage::list_file_and_directory_result_segment" /> that can be used to to lazily enumerate a collection of file or directory items in the the directory.</returns>
        list_file_and_directory_result_segment list_files_and_directories_segmented(const utility::string_t& prefix, int64_t max_results, const continuation_token& token, const file_request_options& options, operation_context context) const
        {
            return list_files_and_directories_segmented_async(prefix, max_results, token, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to return a result segment <see cref="azure::storage::list_file_and_directory_result_segment" /> that can be used to to lazily enumerate a collection of file or directory items.
        /// </summary>
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::list_file_and_directory_result_segment" /> that represents the current operation.</returns>
        pplx::task<list_file_and_directory_result_segment> list_files_and_directories_segmented_async(const continuation_token& token) const
        {
            return list_files_and_directories_segmented_async(0, token, file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to return a result segment <see cref="azure::storage::list_file_and_directory_result_segment" /> that can be used to to lazily enumerate a collection of file or directory items.
        /// </summary>
        /// <param name="prefix">The file/directory name prefix.</param>
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::list_file_and_directory_result_segment" /> that represents the current operation.</returns>
        pplx::task<list_file_and_directory_result_segment> list_files_and_directories_segmented_async(const utility::string_t& prefix, const continuation_token& token) const
        {
            return list_files_and_directories_segmented_async(prefix, 0, token, file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to return a result segment <see cref="azure::storage::list_file_and_directory_result_segment" /> that can be used to to lazily enumerate a collection of file or directory items.
        /// </summary>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned.
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::list_file_and_directory_result_segment" /> that represents the current operation.</returns>
        pplx::task<list_file_and_directory_result_segment> list_files_and_directories_segmented_async(int64_t max_results, const continuation_token& token, const file_request_options& options, operation_context context) const
        {
            return list_files_and_directories_segmented_async(utility::string_t(), max_results, token, options, context);
        }

        /// <summary>
        /// Intitiates an asynchronous operation to return a result segment <see cref="azure::storage::list_file_and_directory_result_segment" /> that can be used to to lazily enumerate a collection of file or directory items.
        /// </summary>
        /// <param name="prefix">The file/directory name prefix.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned.
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::list_file_and_directory_result_segment" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<list_file_and_directory_result_segment> list_files_and_directories_segmented_async(const utility::string_t& prefix, int64_t max_results, const continuation_token& token, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Creates the directory.
        /// All parent directories must already be created. 
        /// </summary>
        void create()
        {
            create_async().wait();
        }

        /// <summary>
        /// Creates the directory.
        /// All parent directories must already be created. 
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        void create(const file_access_condition& condition, const file_request_options& options, operation_context context)
        {
            create_async(condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to create the directory.
        /// All parent directories must already be created. 
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> create_async()
        {
            return create_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to create the directory.
        /// All parent directories must already be created. 
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> create_async(const file_access_condition& condition, const file_request_options& options, operation_context context);

        /// <summary>
        /// Creates the directory if it does not already exist.
        /// </summary>
        /// <returns><c>true</c> if the directory did not already exist and was created; otherwise <c>false</c>.</returns>
        bool create_if_not_exists()
        {
            return create_if_not_exists_async().get();
        }

        /// <summary>
        /// Creates the directory if it does not already exist.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns><c>true</c> if the directory did not already exist and was created; otherwise <c>false</c>.</returns>
        bool create_if_not_exists(const file_access_condition& condition, const file_request_options& options, operation_context context)
        {
            return create_if_not_exists_async(condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to create the directory if it does not already exist.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<bool> create_if_not_exists_async()
        {
            return create_if_not_exists_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to create the directory if it does not already exist.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<bool> create_if_not_exists_async(const file_access_condition& condition, const file_request_options& options, operation_context context);

        /// <summary>
        /// Deletes the directory.
        /// </summary>
        void delete_directory()
        {
            delete_directory_async().wait();
        }

        /// <summary>
        /// Deletes the directory.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        void delete_directory(const file_access_condition& condition, const file_request_options& options, operation_context context)
        {
            delete_directory_async(condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to delete the directory.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> delete_directory_async()
        {
            return delete_directory_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to delete the directory.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> delete_directory_async(const file_access_condition& condition, const file_request_options& options, operation_context context);

        /// <summary>
        /// Deletes the directory if it does exist.
        /// </summary>
        /// <returns><c>true</c> if the directory did exist and was deleted; otherwise <c>false</c>.</returns>
        bool delete_directory_if_exists()
        {
            return delete_directory_if_exists_async().get();
        }

        /// <summary>
        /// Deletes the directory if it does exist.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns><c>true</c> if the directory did exist and was deleted; otherwise <c>false</c>.</returns>
        bool delete_directory_if_exists(const file_access_condition& condition, const file_request_options& options, operation_context context)
        {
            return delete_directory_if_exists_async(condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to delete the directory if it does exist.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<bool> delete_directory_if_exists_async()
        {
            return delete_directory_if_exists_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to delete the directory if it does exist.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<bool> delete_directory_if_exists_async(const file_access_condition& condition, const file_request_options& options, operation_context context);

        /// <summary>
        /// Checks existence of the directory.
        /// </summary>
        /// <returns><c>true</c> if the directory exists.</returns>
        bool exists()
        {
            return exists_async().get();
        }

        /// <summary>
        /// Checks existence of the directory.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns><c>true</c> if the directory exists.</returns>
        bool exists(const file_access_condition& condition, const file_request_options& options, operation_context context)
        {
            return exists_async(condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to check existence of the directory.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<bool> exists_async()
        {
            return exists_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to check existence of the directory.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<bool> exists_async(const file_access_condition& condition, const file_request_options& options, operation_context context)
        {
            return exists_async(false, condition, options, context);
        }

        /// <summary>
        /// Retrieves the directory's attributes.
        /// </summary>
        void download_attributes()
        {
            download_attributes_async().wait();
        }

        /// <summary>
        /// Retrieves the directory's attributes.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        void download_attributes(const file_access_condition& condition, const file_request_options& options, operation_context context)
        {
            download_attributes_async(condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to retrieve the directory's attributes.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> download_attributes_async()
        {
            return download_attributes_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to retrieve the directory's attributes.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> download_attributes_async(const file_access_condition& condition, const file_request_options& options, operation_context context);

        /// <summary>
        /// Uploads the directory's metadata.
        /// </summary>
        void upload_metadata() const
        {
            upload_metadata_async().wait();
        }

        /// <summary>
        /// Uploads the directory's metadata.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        void upload_metadata(const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            upload_metadata_async(condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to upload the directory's metadata.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> upload_metadata_async() const
        {
            return upload_metadata_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to upload the directory's metadata.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_metadata_async(const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Gets a reference to a sub directory.
        /// </summary>
        /// <param name="name">The name of the directory.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_file_directory" />.</returns>
        cloud_file_directory get_subdirectory_reference(utility::string_t name) const
        {
            return cloud_file_directory(name, *this);
        }

        /// <summary>
        /// Gets a reference to a file.
        /// </summary>
        /// <param name="name">The name of the file.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_file" />.</returns>
        WASTORAGE_API cloud_file get_file_reference(const utility::string_t& name) const;

        /// <summary>
        /// Gets a reference to parent directory.
        /// </summary>
        /// <returns>A reference to an <see cref="azure::storage::cloud_file_directory" />.</returns>
        WASTORAGE_API cloud_file_directory get_parent_directory_reference() const;

        /// <summary>
        /// Gets a reference to parent share.
        /// </summary>
        /// <returns>A reference to an <see cref="azure::storage::cloud_file_share" />.</returns>
        cloud_file_share get_parent_share_reference() const
        {
            return m_share;
        }

        /// <summary>
        /// Gets the <see cref="azure::storage::cloud_file_client" /> object that represents the File service.
        /// </summary>
        /// <returns>A client object that specifies the File service endpoint.</returns>
        const cloud_file_client& service_client() const
        {
            return m_share.service_client();
        }

        /// <summary>
        /// Gets the directory's name.
        /// </summary>
        /// <returns>The directory's name.</returns>
        const utility::string_t& name() const
        {
            return m_name;
        }

        /// <summary>
        /// Gets the directory URI for all locations.
        /// </summary>
        /// <returns>An <see cref="azure::storage::storage_uri" /> containing the directory URI for all locations.</returns>
        const storage_uri& uri() const
        {
            return m_uri;
        }

        /// <summary>
        /// Gets the directory's system metadata.
        /// </summary>
        /// <returns>The directory's metadata.</returns>
        cloud_metadata& metadata()
        {
            return *m_metadata;
        }

        /// <summary>
        /// Gets the directory's system metadata.
        /// </summary>
        /// <returns>The directory's metadata.</returns>
        const cloud_metadata& metadata() const
        {
            return *m_metadata;
        }

        /// <summary>
        /// Gets the directory's system properties.
        /// </summary>
        /// <returns>The directory's properties.</returns>
        cloud_file_directory_properties& properties()
        {
            return *m_properties;
        }

        /// <summary>
        /// Gets the directory's system properties.
        /// </summary>
        /// <returns>The directory's properties.</returns>
        const cloud_file_directory_properties& properties() const
        {
            return *m_properties;
        }

    private:

        void init(storage_credentials credentials);
        WASTORAGE_API pplx::task<bool> exists_async(bool primary_only, const file_access_condition& condition, const file_request_options& options, operation_context context);

        utility::string_t m_name;
        cloud_file_share m_share;
        storage_uri m_uri;
        std::shared_ptr<cloud_metadata> m_metadata;
        std::shared_ptr<cloud_file_directory_properties> m_properties;
    };

    /// <summary>
    /// Represents the system properties for a file.
    /// </summary>
    class cloud_file_properties
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file_properties" /> class.
        /// </summary>
        cloud_file_properties()
            : m_length(0),
            m_server_encrypted(false)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        cloud_file_properties(cloud_file_properties&& other)
        {
            *this = std::move(other);
        }

        cloud_file_properties& operator=(cloud_file_properties&& other)
        {
            if (this != &other)
            {
                m_length = other.m_length;
                m_etag = std::move(other.m_etag);
                m_last_modified = std::move(other.m_last_modified);

                m_type = std::move(other.m_type);
                m_content_type = std::move(other.m_content_type);
                m_content_encoding = std::move(other.m_content_encoding);
                m_content_language = std::move(other.m_content_language);
                m_cache_control = std::move(other.m_cache_control);
                m_content_md5 = std::move(other.m_content_md5);
                m_content_disposition = std::move(other.m_content_disposition);
            }
            return *this;
        }

#endif

        /// <summary>
        /// Gets the size of the file, in bytes.
        /// </summary>
        /// <returns>The file's size in bytes.</returns>
        utility::size64_t length() const
        {
            return m_length;
        }

        /// <summary>
        /// Gets the size of the file, in bytes.
        /// </summary>
        /// <returns>The file's size in bytes.</returns>
        utility::size64_t size() const
        {
            return m_length;
        }

        /// <summary>
        /// Gets the file's ETag value.
        /// </summary>
        /// <returns>The file's ETag value.</returns>
        const utility::string_t& etag() const
        {
            return m_etag;
        }

        /// <summary>
        /// Gets the last-modified time for the file, expressed as a UTC value.
        /// </summary>
        /// <returns>The file's last-modified time, in UTC format.</returns>
        utility::datetime last_modified() const
        {
            return m_last_modified;
        }

        /// <summary>
        /// Gets the type of the file.
        /// </summary>
        /// <returns>An <see cref="utility::string_t&" /> object that indicates the type of the file.</returns>
        const utility::string_t& type() const
        {
            return m_type;
        }

        /// <summary>
        /// Gets the content-type value stored for the file.
        /// </summary>
        /// <returns>The file's content-type value.</returns>
        const utility::string_t& content_type() const
        {
            return m_content_type;
        }

        /// <summary>
        /// Sets the content-type value stored for the file.
        /// </summary>
        /// <param name="value">The file's content-type value.</param>
        void set_content_type(utility::string_t content_type)
        {
            m_content_type = std::move(content_type);
        }

        /// <summary>
        /// Gets the content-encoding value stored for the file.
        /// </summary>
        /// <returns>The file's content-encoding value.</returns>
        const utility::string_t& content_encoding() const
        {
            return m_content_encoding;
        }

        /// <summary>
        /// Sets the content-encoding value stored for the file.
        /// </summary>
        /// <param name="value">The file's content-encoding value.</param>
        void set_content_encoding(utility::string_t value)
        {
            m_content_encoding = std::move(value);
        }

        /// <summary>
        /// Gets the content-language value stored for the file.
        /// </summary>
        /// <returns>The file's content-language value.</returns>
        const utility::string_t& content_language() const
        {
            return m_content_language;
        }

        /// <summary>
        /// Sets the content-language value stored for the file.
        /// </summary>
        /// <param name="value">The file's content-language value.</param>
        void set_content_language(utility::string_t value)
        {
            m_content_language = std::move(value);
        }

        /// <summary>
        /// Gets the cache-control value stored for the file.
        /// </summary>
        /// <returns>The file's cache-control value.</returns>
        const utility::string_t& cache_control() const
        {
            return m_cache_control;
        }

        /// <summary>
        /// Sets the cache-control value stored for the file.
        /// </summary>
        /// <param name="value">The file's cache-control value.</param>
        void set_cache_control(utility::string_t value)
        {
            m_cache_control = std::move(value);
        }

        /// <summary>
        /// Gets the content-MD5 value stored for the file.
        /// </summary>
        /// <returns>The file's content-MD5 hash.</returns>
        const utility::string_t& content_md5() const
        {
            return m_content_md5;
        }

        /// <summary>
        /// Sets the content-MD5 value stored for the file.
        /// </summary>
        /// <param name="value">The file's content-MD5 hash.</param>
        void set_content_md5(utility::string_t value)
        {
            m_content_md5 = std::move(value);
        }

        /// <summary>
        /// Gets the content-disposition value stored for the file.
        /// </summary>
        /// <returns>The file's content-disposition value.</returns>
        const utility::string_t& content_disposition() const
        {
            return m_content_disposition;
        }

        /// <summary>
        /// Sets the content-disposition value stored for the file.
        /// </summary>
        /// <param name="value">The file's content-disposition value.</param>
        void set_content_disposition(utility::string_t value)
        {
            m_content_disposition = std::move(value);
        }

        /// <summary>
        /// Gets if the server is encrypted.
        /// </summary>
        /// <returns><c>true</c> if a server is encrypted.</returns>
        bool server_encrypted()
        {
            return m_server_encrypted;
        }

        /// <summary>
        /// Sets if the server is encrypted.
        /// </summary>
        /// <param name="value">If the server is encrypted.</param>
        void set_server_encrypted(bool value)
        {
            m_server_encrypted = value;
        }

    private:

        utility::size64_t m_length;
        utility::string_t m_etag;
        utility::datetime m_last_modified;

        utility::string_t m_type;
        utility::string_t m_content_type;
        utility::string_t m_content_encoding;
        utility::string_t m_content_language;
        utility::string_t m_cache_control;
        utility::string_t m_content_md5;
        utility::string_t m_content_disposition;

        bool m_server_encrypted;

        void update_etag_and_last_modified(const cloud_file_properties& other);
        void update_etag(const cloud_file_properties& other);

        friend class cloud_file;
        friend class protocol::file_response_parsers;
    };

    enum class file_range_write
    {
        update = 0x1,
        clear = 0x2
    };

    /// <summary>
    /// Represents a range of file ranges.
    /// </summary>
    class file_range
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::file_range" /> class.
        /// </summary>
        /// <param name="start_offset">The starting offset.</param>
        /// <param name="end_offset">The ending offset.</param>
        file_range(int64_t start_offset, int64_t end_offset)
            : m_start_offset(start_offset), m_end_offset(end_offset)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::file_range" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::file_range" /> object.</param>
        file_range(file_range&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::file_range" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::file_range" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::page_range" /> object with properties set.</returns>
        file_range& operator=(file_range&& other)
        {
            if (this != &other)
            {
                m_start_offset = std::move(other.m_start_offset);
                m_end_offset = std::move(other.m_end_offset);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Returns the content of the file range as a string.
        /// </summary>
        /// <returns>The content of the file range.</returns>
        utility::string_t to_string() const
        {
            utility::ostringstream_t value;
            value << protocol::header_value_range_prefix << m_start_offset << _XPLATSTR('-') << m_end_offset;
            return value.str();
        }

        /// <summary>
        /// Gets the starting offset of the file range.
        /// </summary>
        /// <returns>The starting offset.</returns>
        int64_t start_offset() const
        {
            return m_start_offset;
        }

        /// <summary>
        /// Gets the ending offset of the file range.
        /// </summary>
        /// <returns>The ending offset.</returns>
        int64_t end_offset() const
        {
            return m_end_offset;
        }

    private:

        int64_t m_start_offset;
        int64_t m_end_offset;
    };

    /// <summary>
    /// A class for Windows Azure file.
    /// </summary>
    class cloud_file
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file" /> class.
        /// </summary>
        cloud_file()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file" /> class.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the file for all locations.</param>
        WASTORAGE_API explicit cloud_file(storage_uri uri);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file" /> class.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the file for all locations.</param>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        WASTORAGE_API cloud_file(storage_uri uri, storage_credentials credentials);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file" /> class.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the file for all locations.</param>
        /// <param name="share">The File directory <see ref="azure::storage::cloud_file_directory"> it blongs to.</param>
        WASTORAGE_API cloud_file(utility::string_t name, cloud_file_directory directory);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_file" /> class.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the file for all locations.</param>
        /// <param name="share">The File directory <see ref="azure::storage::cloud_file_directory"> it blongs to.</param>
        /// <param name="properties">A set of properties for the file.</param>
        /// <param name="metadata">User-defined metadata for the file.</param>
        /// <param name="copy_state">the state of the most recent or pending copy operation.</param>
        WASTORAGE_API cloud_file(utility::string_t name, cloud_file_directory directory, cloud_file_properties properties, cloud_metadata metadata, azure::storage::copy_state copy_state);

        /// <summary>
        /// Creates the file and specifies the size of the file.
        /// </summary>
        /// <param name="length">An <see cref="utility::size64_t" /> value that specifies the size of the file.</param>
        void create(int64_t length)
        {
            create_async(length).wait();
        }

        /// <summary>
        /// Creates the file and specifies the size of the file.
        /// </summary>
        /// <param name="length">An <see cref="utility::size64_t" /> value that specifies the size of the file.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        void create(int64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context)
        {
            create_async(length, condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to create the file and specifies the size of the file.
        /// </summary>
        /// <param name="length">An <see cref="utility::size64_t" /> value that specifies the size of the file.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> create_async(int64_t length)
        {
            return create_async(length, azure::storage::file_access_condition(), azure::storage::file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to create the file and specifies the size of the file.
        /// </summary>
        /// <param name="length">An <see cref="utility::size64_t" /> value that specifies the size of the file.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> create_async(int64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context);

        /// <summary>
        /// Creates the file if it does not already exist.
        /// </summary>
        /// <param name="max_size">An <see cref="utility::size64_t" /> value that specifies the size of the file.</param>
        /// <returns><c>true</c> if the file did not already exist and was created; otherwise <c>false</c>.</returns>
        bool create_if_not_exists(int64_t length)
        {
            return create_if_not_exists_async(length).get();
        }

        /// <summary>
        /// Creates the file if it does not already exist.
        /// </summary>
        /// <param name="max_size">An <see cref="utility::size64_t" /> value that specifies the size of the file.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns><c>true</c> if the file did not already exist and was created; otherwise <c>false</c>.</returns>
        bool create_if_not_exists(int64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context)
        {
            return create_if_not_exists_async(length, condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to create the file if it does not already exist and specify the size of the file.
        /// </summary>
        /// <param name="max_size">An <see cref="utility::size64_t" /> value that specifies the size of the file.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<bool> create_if_not_exists_async(int64_t length)
        {
            return create_if_not_exists_async(length, azure::storage::file_access_condition(), azure::storage::file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to create the file if it does not already exist and specify the size of the file.
        /// </summary>
        /// <param name="max_size">An <see cref="utility::size64_t" /> value that specifies the size of the file.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<bool> create_if_not_exists_async(int64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context);

        /// <summary>
        /// Deletes the file.
        /// </summary>
        void delete_file()
        {
            delete_file_async().wait();
        }

        /// <summary>
        /// Deletes the file.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void delete_file(const file_access_condition& condition, const file_request_options& options, operation_context context)
        {
            delete_file_async(condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to delete the file.
        /// </summary>
        pplx::task<void> delete_file_async()
        {
            return delete_file_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to delete the file.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        WASTORAGE_API pplx::task<void> delete_file_async(const file_access_condition& condition, const file_request_options& options, operation_context context);

        /// <summary>
        /// Deletes the file if it already exists.
        /// </summary>
        /// <returns><c>true</c> if the file did not already exist and was deleted; otherwise <c>false</c>.</returns>
        bool delete_file_if_exists()
        {
            return delete_file_if_exists_async().get();
        }

        /// <summary>
        /// Deletes the file if it already exists.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns><c>true</c> if the file did not already exist and was deleted; otherwise <c>false</c>.</returns>
        bool delete_file_if_exists(const file_access_condition& condition, const file_request_options& options, operation_context context)
        {
            return delete_file_if_exists_async(condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to delete the file if it already exists.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<bool> delete_file_if_exists_async()
        {
            return delete_file_if_exists_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to delete the file if it already exists.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<bool> delete_file_if_exists_async(const file_access_condition& condition, const file_request_options& options, operation_context context);

        /// <summary>
        /// Checks existence of the file.
        /// </summary>
        /// <returns><c>true</c> if the file exists.</returns>
        bool exists() const
        {
            return exists_async().get();
        }

        /// <summary>
        /// Checks existence of the file.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns><c>true</c> if the file exists.</returns>
        bool exists(const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            return exists_async(condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to check the existence of the file.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<bool> exists_async() const
        {
            return exists_async(true, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to check the existence of the file.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<bool> exists_async(const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            return exists_async(true, condition, options, context);
        }

        /// <summary>
        /// Retrieves the file's attributes.
        /// </summary>
        void download_attributes()
        {
            download_attributes_async().wait();
        }

        /// <summary>
        /// Retrieves the file's attributes.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void download_attributes(const file_access_condition& condition, const file_request_options& options, operation_context context)
        {
            download_attributes_async(condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to retrieve the file's attributes.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> download_attributes_async()
        {
            return download_attributes_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to retrieve the file's attributes.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> download_attributes_async(const file_access_condition& condition, const file_request_options& options, operation_context context);

        /// <summary>
        /// Updates the file's properties.
        /// </summary>
        void upload_properties() const
        {
            upload_properties_async().wait();
        }

        /// <summary>
        /// Updates the file's properties.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_properties(const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            upload_properties_async(condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to update the file's properties.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> upload_properties_async() const
        {
            return upload_properties_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to update the file's properties.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_properties_async(const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Updates the file's metadata.
        /// </summary>
        void upload_metadata() const
        {
            upload_metadata_async().wait();
        }

        /// <summary>
        /// Updates the file's metadata.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_metadata(const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            upload_metadata_async(condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to update the file's metadata.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> upload_metadata_async() const
        {
            return upload_metadata_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to update the file's metadata.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_metadata_async(const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Begins an operation to copy a file's contents, properties, and metadata to a new file.
        /// </summary>
        /// <param name="source">The URI of a source file.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// This method fetches the file's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_copy(const web::http::uri& source)
        {
            return start_copy_async(source).get();
        }

        /// <summary>
        /// Begins an operation to copy a file's contents, properties, and metadata to a new file.
        /// </summary>
        /// <param name="source">The URI of a source file.</param>
        /// <param name="source_condition">An object that represents the <see cref="azure::storage::file_access_condition" /> for the source file.</param>
        /// <param name="destination_condition">An object that represents the <see cref="azure::storage::file_access_condition" /> for the destination file.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// This method fetches the file's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_copy(const web::http::uri& source, const file_access_condition& source_condition, const file_access_condition& dest_condition, const file_request_options& options, operation_context context)
        {
            return start_copy_async(source, source_condition, dest_condition, options, context).get();
        }

        /// <summary>
        /// Begins an operation to copy a blob's contents, properties, and metadata to a new file.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <param name="source_condition">An object that represents the <see cref="azure::storage::file_access_condition" /> for the source file.</param>
        /// <param name="destination_condition">An object that represents the <see cref="azure::storage::file_access_condition" /> for the destination file.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// This method fetches the file's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_copy(const web::http::uri& source, const access_condition& source_condition, const file_access_condition& dest_condition, const file_request_options& options, operation_context context)
        {
            return start_copy_async(source, source_condition, dest_condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to begin to copy a file's contents, properties, and metadata to a new file.
        /// </summary>
        /// <param name="source">The URI of a source file.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// This method fetches the file's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        pplx::task<utility::string_t> start_copy_async(const web::http::uri& source)
        {
            return start_copy_async(source, file_access_condition(), file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to begin to copy a file's contents, properties, and metadata to a new file.
        /// </summary>
        /// <param name="source">The URI of a source file.</param>
        /// <param name="source_condition">An object that represents the <see cref="azure::storage::file_access_condition" /> for the source file.</param>
        /// <param name="destination_condition">An object that represents the <see cref="azure::storage::file_access_condition" /> for the destination file.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// This method fetches the file's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        WASTORAGE_API pplx::task<utility::string_t> start_copy_async(const web::http::uri& source, const file_access_condition& source_condition, const file_access_condition& dest_condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Intitiates an asynchronous operation to begin to copy a blob's contents, properties, and metadata to a new file.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <param name="source_condition">An object that represents the <see cref="azure::storage::file_access_condition" /> for the source file.</param>
        /// <param name="destination_condition">An object that represents the <see cref="azure::storage::file_access_condition" /> for the destination file.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// This method fetches the file's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        WASTORAGE_API pplx::task<utility::string_t> start_copy_async(const web::http::uri& source, const access_condition& source_condition, const file_access_condition& dest_condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Begins an operation to copy a file's contents, properties, and metadata to a new file.
        /// </summary>
        /// <param name="source">An object that represents the <see cref="azure::storage::cloud_file"> for the source file.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// This method fetches the file's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_copy(const cloud_file& source)
        {
            return start_copy_async(source).get();
        }

        /// <summary>
        /// Begins an operation to copy a file's contents, properties, and metadata to a new file.
        /// </summary>
        /// <param name="source">An object that represents the <see cref="azure::storage::cloud_file"> for the source file.</param>
        /// <param name="source_condition">An object that represents the <see cref="file_access_condition" /> for the source file.</param>
        /// <param name="dest_condition">An object that represents the <see cref="file_access_condition" /> for the destination file.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// This method fetches the file's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_copy(const cloud_file& source, const file_access_condition& source_condition, const file_access_condition& dest_condition, const file_request_options& options, operation_context context)
        {
            return start_copy_async(source, source_condition, dest_condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to begin to copy a file's contents, properties, and metadata to a new file.
        /// </summary>
        /// <param name="source">An object that represents the <see cref="azure::storage::cloud_file"> for the source file.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// This method fetches the file's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        pplx::task<utility::string_t> start_copy_async(const cloud_file& source)
        {
            return start_copy_async(source, file_access_condition(), file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to begin to copy a file's contents, properties, and metadata to a new file.
        /// </summary>
        /// <param name="source">An object that represents the <see cref="azure::storage::cloud_file"> for the source file.</param>
        /// <param name="source_condition">An object that represents the <see cref="azure::storage::file_access_condition" /> for the source file.</param>
        /// <param name="destination_condition">An object that represents the <see cref="azure::storage::file_access_condition" /> for the destination file.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// This method fetches the file's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        WASTORAGE_API pplx::task<utility::string_t> start_copy_async(const cloud_file& source, const file_access_condition& source_condition, const file_access_condition& dest_condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Begins an operation to copy a blob's contents, properties, and metadata to a new file.
        /// </summary>
        /// <param name="source">An object that represents the <see cref="azure::storage::cloud_blob"> for the source blob.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// This method fetches the file's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t  start_copy(const cloud_blob& source)
        {
            return start_copy_async(source).get();
        }

        /// <summary>
        /// Begins an operation to copy a blob's contents, properties, and metadata to a new file.
        /// </summary>
        /// <param name="source">An object that represents the <see cref="azure::storage::cloud_blob"> for the source blob.</param>
        /// <param name="source_condition">An object that represents the <see cref="access_condition" /> for the source blob.</param>
        /// <param name="dest_condition">An object that represents the <see cref="file_access_condition" /> for the destination file.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// This method fetches the file's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_copy(const cloud_blob& source, const access_condition& source_condition, const file_access_condition& dest_condition, const file_request_options& options, operation_context context)
        {
            return start_copy_async(source, source_condition, dest_condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to begin to copy a blob's contents, properties, and metadata to a new file.
        /// </summary>
        /// <param name="source">An object that represents the <see cref="azure::storage::cloud_blob"> for the source blob.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// This method fetches the file's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        WASTORAGE_API pplx::task<utility::string_t> start_copy_async(const cloud_blob& source) const;

        /// <summary>
        /// Intitiates an asynchronous operation to begin to copy a blob's contents, properties, and metadata to a new file.
        /// </summary>
        /// <param name="source">An object that represents the <see cref="azure::storage::cloud_blob"> for the source blob.</param>
        /// <param name="source_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the source blob.</param>
        /// <param name="destination_condition">An object that represents the <see cref="azure::storage::file_access_condition" /> for the destination file.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// This method fetches the file's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        WASTORAGE_API pplx::task<utility::string_t> start_copy_async(const cloud_blob& source, const access_condition& source_condition, const file_access_condition& dest_condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Aborts an ongoing file copy operation.
        /// </summary>
        /// <param name="copy_id">A string identifying the copy operation.</param>
        void abort_copy(const utility::string_t& copy_id) const
        {
            abort_copy_async(copy_id).wait();
        }

        /// <summary>
        /// Aborts an ongoing file copy operation.
        /// </summary>
        /// <param name="copy_id">A string identifying the copy operation.</param>
        /// <param name="condition">An <see cref="azure::storage::file_request_options" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void abort_copy(const utility::string_t& copy_id, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            abort_copy_async(copy_id, condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to abort an ongoing file copy operation.
        /// </summary>
        /// <param name="copy_id">A string identifying the copy operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> abort_copy_async(const utility::string_t& copy_id) const
        {
            return abort_copy_async(copy_id, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to abort an ongoing file copy operation.
        /// </summary>
        /// <param name="copy_id">A string identifying the copy operation.</param>
        /// <param name="condition">An <see cref="azure::storage::file_request_options" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> abort_copy_async(const utility::string_t& copy_id, const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Gets a collection of valid file ranges.
        /// </summary>
        /// <returns>An enumerable collection of file ranges.</returns>
        std::vector<file_range> list_ranges() const
        {
            return list_ranges_async().get();
        }

        /// <summary>
        /// Gets a collection of valid file ranges.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An enumerable collection of file ranges.</returns>
        std::vector<file_range> list_ranges(const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            return list_ranges_async(condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to get a collection of valid file ranges.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="azure::storage::file_range" />, that represents the current operation.</returns>
        pplx::task<std::vector<file_range>> list_ranges_async() const
        {
            return list_ranges_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to get a collection of valid file ranges.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="azure::storage::file_range" />, that represents the current operation.</returns>
        pplx::task<std::vector<file_range>> list_ranges_async(const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            return list_ranges_async(std::numeric_limits<utility::size64_t>::max(), 0, condition, options, context);
        }

        /// <summary>
        /// Gets a collection of valid file ranges and their starting bytes.
        /// </summary>
        /// <param name="offset">The starting offset of the data range over which to list file ranges, in bytes. Must be a multiple of 512.
        /// If the value sets to be max of size64_t, it will attampt to list all ranges.</param>
        /// <returns>An enumerable collection of file ranges.</returns>
        std::vector<file_range> list_ranges(utility::size64_t start_offset) const
        {
            return list_ranges_async(start_offset).get();
        }

        /// <summary>
        /// Gets a collection of valid file ranges and their starting bytes.
        /// </summary>
        /// <param name="offset">The starting offset of the data range over which to list file ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An enumerable collection of file ranges.</returns>
        std::vector<file_range> list_ranges(utility::size64_t start_offset, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            return list_ranges_async(start_offset, condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to get a collection of valid file ranges and their starting bytes.
        /// </summary>
        /// <param name="offset">The starting offset of the data range over which to list file ranges, in bytes. Must be a multiple of 512.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="azure::storage::file_range" />, that represents the current operation.</returns>
        pplx::task<std::vector<file_range>> list_ranges_async(utility::size64_t start_offset) const
        {
            return list_ranges_async(start_offset, 0, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to get a collection of valid file ranges and their starting bytes.
        /// </summary>
        /// <param name="offset">The starting offset of the data range over which to list file ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="azure::storage::file_range" />, that represents the current operation.</returns>
        pplx::task<std::vector<file_range>> list_ranges_async(utility::size64_t start_offset, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            return list_ranges_async(start_offset, 0, condition, options, context);
        }

        /// <summary>
        /// Gets a collection of valid file ranges and their starting and ending bytes.
        /// </summary>
        /// <param name="offset">The starting offset of the data range over which to list file ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="length">The length of the data range over which to list file ranges, in bytes. Must be a multiple of 512.</param>
        /// <returns>An enumerable collection of file ranges.</returns>
        std::vector<file_range> list_ranges(utility::size64_t start_offset, utility::size64_t length) const
        {
            return list_ranges_async(start_offset, length).get();
        }

        /// <summary>
        /// Gets a collection of valid file ranges and their starting and ending bytes.
        /// </summary>
        /// <param name="offset">The starting offset of the data range over which to list file ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="length">The length of the data range over which to list file ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An enumerable collection of file ranges.</returns>
        std::vector<file_range> list_ranges(utility::size64_t start_offset, utility::size64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            return list_ranges_async(start_offset, length, condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to get a collection of valid file ranges and their starting and ending bytes.
        /// </summary>
        /// <param name="offset">The starting offset of the data range over which to list file ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="length">The length of the data range over which to list file ranges, in bytes. Must be a multiple of 512.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="azure::storage::file_range" />, that represents the current operation.</returns>
        pplx::task<std::vector<file_range>> list_ranges_async(utility::size64_t start_offset, utility::size64_t length) const
        {
            return list_ranges_async(start_offset, length, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to get a collection of valid file ranges and their starting and ending bytes.
        /// </summary>
        /// <param name="offset">The starting offset of the data range over which to list file ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="length">The length of the data range over which to list file ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="azure::storage::file_range" />, that represents the current operation.</returns>
        WASTORAGE_API pplx::task<std::vector<file_range>> list_ranges_async(utility::size64_t start_offset, utility::size64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Clears range from a file.
        /// </summary>
        /// <param name="start_offset">The offset at which to begin clearing ranges, in bytes. The offset must be a multiple of 512.</param>
        /// <param name="length">The length of the data range to be cleared, in bytes. The length must be a multiple of 512.</param>
        void clear_range(utility::size64_t start_offset, utility::size64_t length) const
        {
            clear_range_async(start_offset, length).wait();
        }

        /// <summary>
        /// Clears range from a file.
        /// </summary>
        /// <param name="start_offset">The offset at which to begin clearing ranges, in bytes. The offset must be a multiple of 512.</param>
        /// <param name="length">The length of the data range to be cleared, in bytes. The length must be a multiple of 512.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        void clear_range(utility::size64_t start_offset, utility::size64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            clear_range_async(start_offset, length, condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to clear range from a file.
        /// </summary>
        /// <param name="start_offset">The offset at which to begin clearing ranges, in bytes. The offset must be a multiple of 512.</param>
        /// <param name="length">The length of the data range to be cleared, in bytes. The length must be a multiple of 512.</param>
        pplx::task<void> clear_range_async(utility::size64_t start_offset, utility::size64_t length) const
        {
            return clear_range_async(start_offset, length, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to clear range from a file.
        /// </summary>
        /// <param name="start_offset">The offset at which to begin clearing ranges, in bytes. The offset must be a multiple of 512.</param>
        /// <param name="length">The length of the data range to be cleared, in bytes. The length must be a multiple of 512.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> clear_range_async(utility::size64_t start_offset, utility::size64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Writes range to a file.
        /// </summary>
        /// <param name="stream">A stream providing the file range data.</param>
        /// <param name="start_offset">The offset at which to begin writing, in bytes. The offset must be a multiple of 512.</param>
        /// <param name="content_md5">An optional hash value that will be used to set the Content-MD5 property
        /// on the blob. May be an empty string.</param>
        void write_range(Concurrency::streams::istream stream, int64_t start_offset, utility::string_t content_md5) const
        {
            write_range_async(stream, start_offset, content_md5).wait();
        }

        /// <summary>
        /// Writes range to a file.
        /// </summary>
        /// <param name="stream">A stream providing the file range data.</param>
        /// <param name="start_offset">The offset at which to begin writing, in bytes. The offset must be a multiple of 512.</param>
        /// <param name="content_md5">An optional hash value that will be used to set the Content-MD5 property
        /// on the blob. May be an empty string.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void write_range(Concurrency::streams::istream stream, int64_t start_offset, utility::string_t content_md5, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            write_range_async(stream, start_offset, content_md5, condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to write range to a file.
        /// </summary>
        /// <param name="stream">A stream providing the file range data.</param>
        /// <param name="start_offset">The offset at which to begin writing, in bytes. The offset must be a multiple of 512.</param>
        /// <param name="content_md5">An optional hash value that will be used to set the Content-MD5 property
        /// on the blob. May be an empty string.</param>
        pplx::task<void> write_range_async(Concurrency::streams::istream stream, int64_t start_offset, utility::string_t content_md5) const
        {
            return write_range_async(stream, start_offset, content_md5, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to write range to a file.
        /// </summary>
        /// <param name="stream">A stream providing the file range data.</param>
        /// <param name="start_offset">The offset at which to begin writing, in bytes. The offset must be a multiple of 512.</param>
        /// <param name="content_md5">An optional hash value that will be used to set the Content-MD5 property
        /// on the blob. May be an empty string.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        WASTORAGE_API pplx::task<void> write_range_async(Concurrency::streams::istream stream, int64_t start_offset, const utility::string_t& content_md5, const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Downloads the contents of a file to a stream.
        /// </summary>
        /// <param name="target">The target stream.</param>
        void download_to_stream(concurrency::streams::ostream target) const
        {
            download_to_stream_async(target).wait();
        }

        /// <summary>
        /// Downloads the contents of a file to a stream.
        /// </summary>
        /// <param name="target">The target stream.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void download_to_stream(concurrency::streams::ostream target, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            download_to_stream_async(target, condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to download the contents of a file to a stream.
        /// </summary>
        /// <param name="target">The target stream.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> download_to_stream_async(concurrency::streams::ostream target) const
        {
            return download_to_stream_async(target, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to download the contents of a file to a stream.
        /// </summary>
        /// <param name="target">The target stream.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> download_to_stream_async(concurrency::streams::ostream target, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            return download_range_to_stream_async(target, std::numeric_limits<utility::size64_t>::max(), 0, condition, options, context);
        }

        /// <summary>
        /// Downloads a range of bytes in a file to a stream.
        /// </summary>
        /// <param name="target">The target stream.</param>
        /// <param name="offset">The offset at which to begin downloading the file, in bytes.</param>
        /// <param name="length">The length of the data to download from the file, in bytes.</param>
        void download_range_to_stream(concurrency::streams::ostream target, utility::size64_t offset, utility::size64_t length) const
        {
            download_range_to_stream_async(target, offset, length).wait();
        }

        /// <summary>
        /// Downloads a range of bytes in a file to a stream.
        /// </summary>
        /// <param name="target">The target stream.</param>
        /// <param name="offset">The offset at which to begin downloading the file, in bytes.</param>
        /// <param name="length">The length of the data to download from the file, in bytes.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void download_range_to_stream(concurrency::streams::ostream target, utility::size64_t offset, utility::size64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            download_range_to_stream_async(target, offset, length, condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to download a range of bytes in a file to a stream.
        /// </summary>
        /// <param name="target">The target stream.</param>
        /// <param name="offset">The offset at which to begin downloading the file, in bytes.</param>
        /// <param name="length">The length of the data to download from the file, in bytes.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> download_range_to_stream_async(concurrency::streams::ostream target, int64_t offset, int64_t length) const
        {
            return download_range_to_stream_async(target, offset, length, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to download a range of bytes in a file to a stream.
        /// </summary>
        /// <param name="target">The target stream.</param>
        /// <param name="offset">The offset at which to begin downloading the file, in bytes.</param>
        /// <param name="length">The length of the data to download from the file, in bytes.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> download_range_to_stream_async(concurrency::streams::ostream target, utility::size64_t start_offset, utility::size64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Downloads the contents of a file to a file.
        /// </summary>
        /// <param name="path">The target file.</param>
        void download_to_file(const utility::string_t &path) const
        {
            download_to_file_async(path).wait();
        }

        /// <summary>
        /// Downloads the contents of a file to a file.
        /// </summary>
        /// <param name="path">The target file.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void download_to_file(const utility::string_t &path, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            download_to_file_async(path, condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to download the contents of a file to a file.
        /// </summary>
        /// <param name="path">The target file.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> download_to_file_async(const utility::string_t &path) const
        {
            return download_to_file_async(path, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to download the contents of a file to a file.
        /// </summary>
        /// <param name="path">The target file.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> download_to_file_async(const utility::string_t &path, const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Downloads the contents of a file as text.
        /// </summary>
        /// <returns>The contents of the file, as a string.</returns>
        utility::string_t download_text() const
        {
            return download_text_async().get();
        }

        /// <summary>
        /// Downloads the contents of a file as text.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The contents of the file, as a string.</returns>
        utility::string_t download_text(const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            return download_text_async(condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to download the contents of a file as text.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        pplx::task<utility::string_t> download_text_async() const
        {
            return download_text_async(file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to download the contents of a file as text.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<utility::string_t> download_text_async(const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Opens a stream for writing to the file.
        /// </summary>
        /// <returns>A stream to be used for writing to the file.</returns>
        concurrency::streams::ostream open_write() const
        {
            return open_write_async().get();
        }

        /// <summary>
        /// Opens a stream for writing to the file.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A stream to be used for writing to the file.</returns>
        concurrency::streams::ostream open_write(const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            return open_write_async(condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to open a stream for writing to the file.
        /// </summary>
        /// <returns>A stream to be used for writing to the file.</returns>
        pplx::task<concurrency::streams::ostream> open_write_async() const
        {
            return open_write_async(file_access_condition(), file_request_options(), operation_context());
        }
        
        /// <summary>
        /// Intitiates an asynchronous operation to open a stream for writing to the file.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A stream to be used for writing to the file.</returns>
        WASTORAGE_API pplx::task<concurrency::streams::ostream> open_write_async(const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Opens a stream for writing to the file.
        /// </summary>
        /// <param name="length">The length of the file, in bytes.</param>
        /// <returns>A stream to be used for writing to the file.</returns>
        concurrency::streams::ostream open_write(utility::size64_t length) const
        {
            return open_write_async(length).get();
        }

        /// <summary>
        /// Opens a stream for writing to the file.
        /// </summary>
        /// <param name="length">The length of the file, in bytes.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A stream to be used for writing to the file.</returns>
        concurrency::streams::ostream open_write(utility::size64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            return open_write_async(length, condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to open a stream for writing to the file.
        /// </summary>
        /// <param name="length">The length of the file, in bytes.</param>
        /// <returns>A stream to be used for writing to the file.</returns>
        pplx::task<concurrency::streams::ostream> open_write_async(utility::size64_t length) const
        {
            return open_write_async(length, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to open a stream for writing to the file.
        /// </summary>
        /// <param name="length">The length of the file, in bytes.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A stream to be used for writing to the file.</returns>
        WASTORAGE_API pplx::task<concurrency::streams::ostream> open_write_async(utility::size64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Uploads the file from stream.
        /// </summary>
        /// <param name="stream">The stream providing the file content.</param>
        void upload_from_stream(concurrency::streams::istream stream) const
        {
            upload_from_stream_async(stream).wait();
        }

        /// <summary>
        /// Uploads the file from stream.
        /// </summary>
        /// <param name="stream">The stream providing the file content.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_from_stream(concurrency::streams::istream stream, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            upload_from_stream_async(stream, condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to upload the file from stream.
        /// </summary>
        /// <param name="stream">The stream providing the file content.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> upload_from_stream_async(concurrency::streams::istream stream) const
        {
            return upload_from_stream_async(stream, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to upload the file from stream.
        /// </summary>
        /// <param name="stream">The stream providing the file content.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> upload_from_stream_async(concurrency::streams::istream stream, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            return upload_from_stream_async(stream, std::numeric_limits<utility::size64_t>::max(), condition, options, context);
        }

        /// <summary>
        /// Uploads the file from stream.
        /// </summary>
        /// <param name="stream">The stream providing the file content.</param>
        /// <param name="length">The length of the file, in bytes.</param>
        void upload_from_stream(concurrency::streams::istream stream, utility::size64_t length) const
        {
            upload_from_stream_async(stream, length).wait();
        }

        /// <summary>
        /// Uploads the file from stream.
        /// </summary>
        /// <param name="stream">The stream providing the file content.</param>
        /// <param name="length">The length of the file, in bytes.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_from_stream(concurrency::streams::istream stream, utility::size64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            upload_from_stream_async(stream, length, condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to upload the file from stream.
        /// </summary>
        /// <param name="stream">The stream providing the file content.</param>
        /// <param name="length">The length of the file, in bytes.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> upload_from_stream_async(concurrency::streams::istream stream, utility::size64_t length) const
        {
            return upload_from_stream_async(stream, length, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to upload the file from stream.
        /// </summary>
        /// <param name="stream">The stream providing the file content.</param>
        /// <param name="length">The length of the file, in bytes.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_from_stream_async(concurrency::streams::istream stream, utility::size64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Uploads the file from local file.
        /// </summary>
        /// <param name="path">The local file providing the file content.</param>
        void upload_from_file(const utility::string_t& path) const
        {
            upload_from_file_async(path).wait();
        }

        /// <summary>
        /// Uploads the file from local file.
        /// </summary>
        /// <param name="path">The local file providing the file content.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_from_file(const utility::string_t& path, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            upload_from_file_async(path, condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to upload the file from local file.
        /// </summary>
        /// <param name="path">The local file providing the file content.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> upload_from_file_async(const utility::string_t& path) const
        {
            return upload_from_file_async(path, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to upload the file from local file.
        /// </summary>
        /// <param name="path">The local file providing the file content.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_from_file_async(const utility::string_t& path, const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Uploads the file from text.
        /// </summary>
        /// <param name="text">The text providing the file content.</param>
        void upload_text(const utility::string_t& text) const
        {
            upload_text_async(text).wait();
        }

        /// <summary>
        /// Uploads the file from text.
        /// </summary>
        /// <param name="text">The text providing the file content.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_text(const utility::string_t& text, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            upload_text_async(text, condition, options, context).wait();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to upload the file from text.
        /// </summary>
        /// <param name="text">The text providing the file content.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> upload_text_async(const utility::string_t& text) const
        {
            return upload_text_async(text, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to upload the file from text.
        /// </summary>
        /// <param name="text">The text providing the file content.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_text_async(const utility::string_t& text, const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Resize the file.
        /// </summary>
        /// <param name="quota">The size to apply to the file.</param>
        void resize(int64_t length) const
        {
            return resize_async(length).get();
        }

        /// <summary>
        /// Resize the file.
        /// </summary>
        /// <param name="quota">The size to apply to the file.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void resize(int64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context) const
        {
            return resize_async(length, condition, options, context).get();
        }

        /// <summary>
        /// Intitiates an asynchronous operation to resize the file.
        /// </summary>
        /// <param name="quota">The size to apply to the file.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<void> resize_async(int64_t length) const
        {
            return resize_async(length, file_access_condition(), file_request_options(), operation_context());
        }

        /// <summary>
        /// Intitiates an asynchronous operation to resize the file.
        /// </summary>
        /// <param name="quota">The size to apply to the file.</param>
        /// <param name="condition">An <see cref="azure::storage::file_access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::file_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> resize_async(int64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context) const;

        /// <summary>
        /// Returns a shared access signature for the file.
        /// </summary>
        /// <param name="policy">The access policy for the shared access signature.</param>
        /// <returns>A string containing a shared access signature.</returns>
        utility::string_t get_shared_access_signature(const file_shared_access_policy& policy) const
        {
            return get_shared_access_signature(policy, utility::string_t(), cloud_file_shared_access_headers());
        }

        /// <summary>
        /// Returns a shared access signature for the file.
        /// </summary>
        /// <param name="policy">The access policy for the shared access signature.</param>
        /// <param name="stored_policy_identifier">A share-level access policy.</param>
        /// <returns>A string containing a shared access signature.</returns>
        WASTORAGE_API utility::string_t get_shared_access_signature(const file_shared_access_policy& policy, const utility::string_t& stored_policy_identifier)
        {
            return get_shared_access_signature(policy, stored_policy_identifier, cloud_file_shared_access_headers());
        }

        /// <summary>
        /// Returns a shared access signature for the file.
        /// </summary>
        /// <param name="policy">The access policy for the shared access signature.</param>
        /// <param name="stored_policy_identifier">A share-level access policy.</param>
        /// <param name="headers">The optional header values to set for a file returned with this SAS.</param>
        /// <returns>A string containing a shared access signature.</returns>
        WASTORAGE_API utility::string_t get_shared_access_signature(const file_shared_access_policy& policy, const utility::string_t& stored_policy_identifier, const cloud_file_shared_access_headers& headers) const;

        /// <summary>
        /// Gets the <see cref="azure::storage::cloud_file_client" /> object that represents the File service.
        /// </summary>
        /// <returns>A client object that specifies the File service endpoint.</returns>
        const cloud_file_client& service_client() const
        {
            return m_directory.service_client();
        }

        /// <summary>
        /// Gets the <see cref="azure::storage::cloud_file_client" /> object that represents the File service.
        /// </summary>
        /// <returns>A client object that specifies the File service endpoint.</returns>
        cloud_file_share get_parent_share_reference() const
        {
            return m_directory.get_parent_share_reference();
        }

        /// <summary>
        /// Gets the <see cref="azure::storage::cloud_file_client" /> object that represents the File service.
        /// </summary>
        /// <returns>A client object that specifies the File service endpoint.</returns>
        cloud_file_directory get_parent_directory_reference() const
        {
            return m_directory;
        }

        /// <summary>
        /// Gets the file's path.
        /// </summary>
        /// <returns>The file's path.</returns>
        const utility::string_t path() const;

        /// <summary>
        /// Gets the file's name.
        /// </summary>
        /// <returns>The file's name.</returns>
        utility::string_t& name()
        {
            return m_name;
        }

        /// <summary>
        /// Gets the file's name.
        /// </summary>
        /// <returns>The file's name.</returns>
        const utility::string_t& name() const
        {
            return m_name;
        }

        /// <summary>
        /// Gets the file URI for all locations.
        /// </summary>
        /// <returns>An <see cref="azure::storage::storage_uri" /> containing the file URI for all locations.</returns>
        const storage_uri& uri() const
        {
            return m_uri;
        }

        /// <summary>
        /// Gets the file's system metadata.
        /// </summary>
        /// <returns>The file's metadata.</returns>
        const cloud_metadata& metadata() const
        {
            return *m_metadata;
        }

        /// <summary>
        /// Gets the file's system metadata.
        /// </summary>
        /// <returns>The file's metadata.</returns>
        cloud_metadata& metadata()
        {
            return *m_metadata;
        }

        /// <summary>
        /// Gets the file's system properties.
        /// </summary>
        /// <returns>The file's properties.</returns>
        const cloud_file_properties& properties() const
        {
            return *m_properties;
        }

        /// <summary>
        /// Gets the file's system properties.
        /// </summary>
        /// <returns>The file's properties.</returns>
        cloud_file_properties& properties()
        {
            return *m_properties;
        }

        /// <summary>
        /// Gets the file's system properties.
        /// </summary>
        /// <returns>The file's properties.</returns>
        const azure::storage::copy_state& copy_state() const
        {
            return *m_copy_state;
        }

        /// <summary>
        /// Gets the file's system properties.
        /// </summary>
        /// <returns>The file's properties.</returns>
        azure::storage::copy_state& copy_state()
        {
            return *m_copy_state;
        }

    private:

        void init(storage_credentials credentials);
        WASTORAGE_API pplx::task<bool> exists_async(bool primary_only, const file_access_condition& condition, const file_request_options& options, operation_context context) const;
        WASTORAGE_API pplx::task<void> download_single_range_to_stream_async(concurrency::streams::ostream target, utility::size64_t offset, utility::size64_t length, const file_access_condition& condition, const file_request_options& options, operation_context context, bool update_properties = false, bool validate_last_modify = false) const;

        utility::string_t m_name;
        cloud_file_directory m_directory;
        storage_uri m_uri;
        std::shared_ptr<cloud_metadata> m_metadata;
        std::shared_ptr<cloud_file_properties> m_properties;
        std::shared_ptr<azure::storage::copy_state> m_copy_state;
    };

    /// <summary>
    /// Represents an item that may be returned by a directory listing operation.
    /// </summary>
    class list_file_and_directory_item
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::list_file_and_directory_item" /> class that represents a cloud file or directory.
        /// </summary>
        /// <param name="is_file"><c>true</c>if it is a file, <c>false</c> otherwise</param>
        /// <param name="name">The name of the file or directory.</param>
        list_file_and_directory_item(bool is_file, utility::string_t name)
            : m_is_file(is_file), m_name(std::move(name))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::list_file_and_directory_item" /> class that represents a cloud file or directory.
        /// </summary>
        /// <param name="is_file"><c>true</c>if it is a file, <c>false</c> otherwise</param>
        /// <param name="name">The name of the file or directory.</param>
        /// <param name="length">The size of the file.</param>
        list_file_and_directory_item(bool is_file, utility::string_t name, int64_t length)
            : m_is_file(is_file), m_name(std::move(name)), m_length(length)
        {
        }

        /// <summary>
        /// Returns the item as an <see cref="azure::storage::cloud_file_directory" /> object, if and only if it represents a cloud file directory.
        /// </summary>
        /// <returns>An <see cref="azure::storage::cloud_file_directory" /> object.</returns>
        cloud_file_directory as_directory() const
        {
            if (!is_directory())
            {
                throw std::runtime_error("Cannot access a cloud file as cloud file directory ");
            }
            return cloud_file_directory(m_name, m_directory);
        }

        /// <summary>
        /// Returns the item as an <see cref="azure::storage::cloud_file" /> object, if and only if it represents a cloud file.
        /// </summary>
        /// <returns>An <see cref="azure::storage::cloud_file" /> object.</returns>
        cloud_file as_file() const
        {
            if (!is_file())
            {
                throw std::runtime_error("Cannot access a cloud file directory as cloud file");
            }
            return cloud_file(m_name, m_directory);
        }

        /// <summary>
        /// Gets a value indicating whether this <see cref="azure::storage::list_file_and_directory_item" /> represents a cloud file or a cloud file directory.
        /// </summary>
        /// <returns><c>true</c> if this <see cref="azure::storage::list_file_and_directory_item" /> represents a cloud file; otherwise, <c>false</c>.</returns>
        bool is_file() const
        {
            return m_is_file;
        }

        /// <summary>
        /// Gets a value indicating whether this <see cref="azure::storage::list_file_and_directory_item" /> represents a cloud file or a cloud file directory.
        /// </summary>
        /// <returns><c>true</c> if this <see cref="azure::storage::list_file_and_directory_item" /> represents a cloud file directory; otherwise, <c>false</c>.</returns>
        bool is_directory() const
        {
            return !m_is_file;
        }

        /// <summary>
        /// Set the directory it blongs to.
        /// </summary>
        /// <param name="directory">The File directory <see ref="azure::storage::cloud_file_directory"> it blongs to.</param>
        void set_directory(cloud_file_directory directory)
        {
            m_directory = std::move(directory);
        }

        /// <summary>
        /// Get the name of file or directory.
        /// </summary>
        const utility::string_t& name() const
        {
            return m_name;
        }

    private:

        bool m_is_file;
        utility::string_t m_name;
        int64_t m_length;
        cloud_file_directory m_directory;
    };
}} // namespace azure::storage

#pragma pop_macro("max")
