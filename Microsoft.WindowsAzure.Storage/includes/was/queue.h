// -----------------------------------------------------------------------------------------
// <copyright file="queue.h" company="Microsoft">
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

#include "service_client.h"

namespace wa { namespace storage {

    class cloud_queue_message;
    class cloud_queue;
    class cloud_queue_client;

    /*
    namespace details
    {
        struct message_data
        {
            message_data()
                : m_dequeue_count(0)
            {
            }

            message_data(const std::string& contents)
                : m_contents(contents.begin(), contents.end()), m_dequeue_count(0)
            {
            }

            message_data(std::vector<uint8_t> contents)
                : m_contents(std::move(contents)), m_dequeue_count(0)
            {
            }

            std::vector<uint8_t> m_contents;
            utility::string_t m_id;
            utility::string_t m_pop_receipt;
            utility::datetime m_insert_time;
            utility::datetime m_expire_time;
            utility::datetime m_next_visible_time;
            size_t m_dequeue_count;
        };
    }
    */

    
    /// <summary>
    /// Represents a shared access policy, which specifies the start time, expiry time, 
    /// and permissions for a shared access signature for the Queue service.
    /// </summary>
    class queue_shared_access_policy : public shared_access_policy
    {
    public:

        /// <summary>
        /// An enumeration describing permissions that may be used for a shared access signature.
        /// </summary>
        enum permissions
        {
            none = 0, read = 1, add = 0x10, update = 0x20, process = 0x40
        };

        /// <summary>
        /// Initializes a new instance of the <see cref="wa::storage::queue_shared_access_policy" /> class.
        /// </summary>
        queue_shared_access_policy()
            : shared_access_policy()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="wa::storage::queue_shared_access_policy" /> class.
        /// </summary>
        /// <param name="expiry">The expiration date and time of the policy.</param>
        /// <param name="permission">A mask containing the permissions of the policy</param>
        queue_shared_access_policy(const utility::datetime& expiry, uint8_t permission)
            : shared_access_policy(expiry, permission)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="wa::storage::queue_shared_access_policy" /> class.
        /// </summary>
        /// <param name="start">The start date and time of the policy.</param>
        /// <param name="expiry">The expiration date and time of the policy.</param>
        /// <param name="permission">A mask containing the permissions of the policy</param>
        queue_shared_access_policy(const utility::datetime& start, const utility::datetime& expiry, uint8_t permission)
            : shared_access_policy(start, expiry, permission)
        {
        }
    };

    /*
    /// <summary>
    /// Represents the permissions for a Queue object.
    /// </summary>
    typedef cloud_permissions<queue_shared_access_policy> cloud_queue_permissions;
    */

    /// <summary>
    /// Represents a set of permissions for a queue.
    /// </summary>
    class queue_permissions : public cloud_permissions<queue_shared_access_policy>
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="wa::storage::queue_permissions" /> class.
        /// </summary>
        queue_permissions()
            : cloud_permissions()
        {
        }
    };

    /*/// <summary>
    /// Specifies which additional details of a queue to download when listing the queues in a storage account.
    /// </summary>
    enum class queue_listing_details
    {
        /// <summary>
        /// Specifies no additional details are downloaded.
        /// </summary>
        none = 0x0,

        /// <summary>
        /// Specifies the metadata of each queue is downloaded.
        /// </summary>
        metadata = 0x1
    };

    /// <summary>
    /// Specifies which fields of a queue message to update.
    /// </summary>
    enum class message_update_fields
    {
        /// <summary>
        /// Specifies the queue message is not updated.
        /// </summary>
        none = 0x0,

        /// <summary>
        /// Specifies the visibility timeout of the queue message is updated.
        /// </summary>
        visibility_timeout = 0x1,

        /// <summary>
        /// Specifies the content of the queue message is updated.
        /// </summary>
        content = 0x2
    };
    */

    /// <summary>
    /// Represents a message in a Windows Azure queue.
    /// </summary>
    class cloud_queue_message
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="cloud_queue_message"/> class.
        /// </summary>
        cloud_queue_message()
            : m_dequeue_count(0)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="cloud_queue_message" /> class, with text content.
        /// </summary>
        /// <param name="content">The content of the message.</param>
        cloud_queue_message(const utility::string_t& content)
            : m_content(content), m_dequeue_count(0)
        {
        }

        /*
        /// <summary>
        /// Initializes a new instance of the cloud queue message class with the specified text stream.
        /// </summary>
        /// <param name="content">The content of the message as text.</param>
        cloud_queue_message(const utility::istringstream_t& content)
            : m_dequeue_count(0)
        {
            // TODO: Read content
        }

        /// <summary>
        /// Initializes a new instance of the cloud queue message class with the specified text array.
        /// </summary>
        /// <param name="content">The content of the message as text.</param>
        cloud_queue_message(const utility::char_t* content)
            : m_dequeue_count(0)
        {
            // TODO: Copy content
        }
        */

        /// <summary>
        /// Initializes a new instance of the <see cref="cloud_queue_message"/> class with the specified raw data.
        /// </summary>
        /// <param name="content">The content of the message as raw data.</param>
        cloud_queue_message(const std::vector<uint8_t>& content)
            : m_content(utility::conversions::to_base64(content)), m_dequeue_count(0)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="cloud_queue_message"/> class.
        /// </summary>
        /// <param name="id">The unique ID of the message.</param>
        /// <param name="pop_receipt">The pop receipt token.</param>
        cloud_queue_message(const utility::string_t& id, const utility::string_t& pop_receipt)
            : m_id(id), m_pop_receipt(pop_receipt), m_dequeue_count(0)
        {
        }

        /// <summary>
        /// Gets the content of the message as text.
        /// </summary>
        /// <returns>The content of the message as text.</returns>
        const utility::string_t content_as_string() const
        {
            return m_content;
        }

        /// <summary>
        /// Gets the content of the message as raw data.
        /// </summary>
        /// <returns>The content of the message as raw data.</returns>
        const std::vector<uint8_t> content_as_binary() const
        {
            return std::vector<uint8_t>(utility::conversions::from_base64(m_content));
        }

        /// <summary>
        /// Sets the content of this message.
        /// </summary>
        /// <param name="content">The new message content.</param>
        void set_content(const utility::string_t& value)
        {
            m_content = value;
        }

        /// <summary>
        /// Sets the content of this message.
        /// </summary>
        /// <param name="content">The new message content.</param>
        void set_content(const std::vector<uint8_t>& value)
        {
            m_content = utility::conversions::to_base64(value);
        }

        /// <summary>
        /// Gets the unique ID of the message.
        /// </summary>
        /// <returns>The unique ID of the message.</returns>
        utility::string_t id() const
        {
            return m_id;
        }

        /// <summary>
        /// Gets the message's pop receipt.
        /// </summary>
        /// <returns>The pop receipt value.</returns>
        utility::string_t pop_receipt() const
        {
            return m_pop_receipt;
        }

        /// <summary>
        /// Returns the expiration time for the message, based on its time-to-live.
        /// </summary>
        /// <returns>The expiration time for the message.</returns>
        utility::datetime expiration_time() const
        {
            return m_expiration_time;
        }

        /// <summary>
        /// Returns the time that the message was inserted into the queue.
        /// </summary>
        /// <returns>The time that the message was inserted into the queue.</returns>
        utility::datetime insertion_time() const
        {
            return m_insertion_time;
        }

        /// <summary>
        /// Returns the next time that the message will be visible.
        /// </summary>
        /// <returns>The next time that the message will be visible.</returns>
        utility::datetime next_visibile_time() const
        {
            return m_next_visible_time;
        }

        /// <summary>
        /// Returns the dequeue count indicating the number of times the message has been retrieved from the queue.
        /// </summary>
        /// <returns>The dequeue count.</returns>
        int dequeue_count() const
        {
            return m_dequeue_count;
        }

        /// <summary>
        /// The maximum message size, in bytes.
        /// </summary>
        static const size_t max_message_size = 64U * 1024U;

        /// <summary>
        /// The maximum amount of time a message is kept in the queue, in seconds.
        /// </summary>
        WASTORAGE_API static const std::chrono::seconds max_time_to_live;

        /// <summary>
        /// The maximum number of messages that can be peeked from the queue at a time.
        /// </summary>
        static const int max_number_of_messages_to_peek = 32;

    private:

        cloud_queue_message(const utility::string_t& content, const utility::string_t& id, const utility::string_t& pop_receipt, const utility::datetime& insertion_time, const utility::datetime& expiration_time, const utility::datetime& next_visible_time, int dequeue_count)
            : m_content(content), m_id(id), m_pop_receipt(pop_receipt), m_insertion_time(insertion_time), m_expiration_time(expiration_time), m_next_visible_time(next_visible_time), m_dequeue_count(dequeue_count)
        {
        }

        void set_pop_receipt(const utility::string_t& pop_receipt)
        {
            m_pop_receipt = std::move(pop_receipt);
        }

        void set_next_visible_time(const utility::datetime& next_visible_time)
        {
            m_next_visible_time = std::move(next_visible_time);
        }
        
        /*
        friend class cloud_queue;
        typedef std::shared_ptr<wa::storage::details::message_data> dataptr_t;

        cloud_queue_message(dataptr_t data)
            : m_data(data)
        {
        }

        dataptr_t data() const
        {
            return m_data;
        }

        dataptr_t m_data;
        */

        utility::string_t m_content;
        utility::string_t m_id;
        utility::string_t m_pop_receipt;
        utility::datetime m_insertion_time;
        utility::datetime m_expiration_time;
        utility::datetime m_next_visible_time;
        int m_dequeue_count;

        friend class cloud_queue;
    };

    /// <summary>
    /// Represents a set of options that may be specified for a request against the Queue service, 
    /// including timeout and retry policy options.
    /// </summary>
    class queue_request_options : public request_options
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="wa::storage::queue_request_options" /> class.
        /// </summary>
        queue_request_options()
            : request_options()
        {
        }

        /// <summary>
        /// Applies the default set of request options.
        /// </summary>
        /// <param name="other">A reference to a set of <see cref="queue_request_options" />.</param>
        void apply_defaults(const queue_request_options& other)
        {
            request_options::apply_defaults(other, true);
        }
    };

    /// <summary>
    /// Represents a segment of <see cref="wa::storage::cloud_queue" /> results 
    /// and provides a continuation token for retrieving additional results.
    /// </summary>
    class queue_result_segment
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="wa::storage::queue_result_segment"/> class.
        /// </summary>
        queue_result_segment()
        {
        }

        /// <summary>
        /// Gets an enumerable collection of <see cref="wa::storage::cloud_queue" /> results.
        /// </summary>
        /// <returns>An enumerable collection of results.</returns>
        std::vector<wa::storage::cloud_queue> results() const
        {
            return m_results;
        }

        /// <summary>
        /// Gets the continuation token used to retrieve the next segment of <see cref="wa::storage::cloud_queue" /> results.
        /// </summary>
        /// <returns>The continuation token.</returns>
        wa::storage::continuation_token continuation_token() const
        {
            return m_continuation_token;
        }

    private:

        void set_results(const std::vector<wa::storage::cloud_queue>& results)
        {
            m_results = std::move(results);
        }

        void set_continuation_token(wa::storage::continuation_token continuation_token)
        {
            m_continuation_token = std::move(continuation_token);
        }

        std::vector<wa::storage::cloud_queue> m_results;
        wa::storage::continuation_token m_continuation_token;

        friend class cloud_queue_client;
    };

    /// <summary>
    /// Provides a client-side logical representation of the Windows Azure Queue service. This client is used to configure and execute requests against the Queue service.
    /// </summary>
    /// <remarks>The service client encapsulates the base URI for the Queue service. If the service client will be used for authenticated access, it also encapsulates the credentials for accessing the storage account.</remarks>
    class cloud_queue_client : public cloud_client
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="wa::storage::cloud_queue_client" /> class.
        /// </summary>
        cloud_queue_client()
            : cloud_client()
        {
            initialize();
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="cloud_queue_client"/> class.
        /// </summary>
        /// <param name="base_uri">A <see cref="storage_uri" /> object containing the Queue service endpoint for all locations.</param>
        cloud_queue_client(const storage_uri& base_uri)
            : cloud_client(base_uri)
        {
            initialize();
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="cloud_queue_client"/> class.
        /// </summary>
        /// <param name="base_uri">A <see cref="storage_uri" /> object containing the Queue service endpoint for all locations.</param>
        /// <param name="credentials">The <see cref="storage_credentials" /> to use.</param>
        cloud_queue_client(const storage_uri& base_uri, wa::storage::storage_credentials credentials)
            : cloud_client(base_uri, credentials)
        {
            initialize();
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="cloud_queue_client" /> class using the specified Queue 
        /// service endpoint and account credentials.
        /// </summary>
        /// <param name="base_uri">A <see cref="storage_uri" /> object containing the Queue service endpoint for all locations.</param>
        /// <param name="credentials">The <see cref="storage_credentials" /> to use.</param>
        /// <param name="default_request_options">The default <see cref="queue_request_options" /> object to use for all requests made with this client object.</param>
        cloud_queue_client(const storage_uri& base_uri, wa::storage::storage_credentials credentials, const queue_request_options& default_request_options)
            : cloud_client(base_uri, credentials), m_default_request_options(default_request_options)
        {
            initialize();
        }

        /// <summary>
        /// Returns a result segment containing a collection of queues in the storage account.
        /// </summary>
        /// <returns>An enumerable collection of <see cref="cloud_queue" /> objects.</returns>
        std::vector<cloud_queue> list_queues() const
        {
            return list_queues_async(utility::string_t(), false, queue_request_options(), operation_context()).get();
        }

        /// <summary>
        /// Returns a result segment containing a collection of queues in the storage account.
        /// </summary>
        /// <param name="prefix">The queue name prefix.</param>
        /// <returns>An enumerable collection of <see cref="cloud_queue" /> objects.</returns>
        std::vector<cloud_queue> list_queues(const utility::string_t& prefix) const
        {
            return list_queues_async(prefix, false, queue_request_options(), operation_context()).get();
        }

        /// <summary>
        /// Returns a result segment containing a collection of queues in the storage account.
        /// </summary>
        /// <param name="prefix">The queue name prefix.</param>
        /// <param name="get_metadata">A enumeration describing which items to include in the listing.</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An enumerable collection of <see cref="cloud_queue" /> objects.</returns>
        std::vector<cloud_queue> list_queues(const utility::string_t& prefix, bool get_metadata, const queue_request_options& options, operation_context context) const
        {
            return list_queues_async(prefix, get_metadata, options, context).get();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to return a result segment containing a collection of queue items.
        /// </summary>
        /// <param name="continuation_token">A continuation token returned by a previous listing operation.</param> 
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="cloud_queue" />, that represents the current operation.</returns>
        pplx::task<std::vector<cloud_queue>> list_queues_async() const
        {
            return list_queues_async(utility::string_t(), false, queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to return a result segment containing a collection of queue items.
        /// </summary>
        /// <param name="continuation_token">A continuation token returned by a previous listing operation.</param> 
        /// <param name="prefix">The queue name prefix.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="cloud_queue" />, that represents the current operation.</returns>
        pplx::task<std::vector<cloud_queue>> list_queues_async(const utility::string_t& prefix) const
        {
            return list_queues_async(prefix, false, queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to return a result segment containing a collection of queue items.
        /// </summary>
        /// <param name="continuation_token">A continuation token returned by a previous listing operation.</param> 
        /// <param name="prefix">The queue name prefix.</param>
        /// <param name="get_metadata">A flag that specifies whether to retrieve queue metadata.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned at a time, up to the 
        /// per-operation limit of 5000. If this value is 0, the maximum possible number of results will be returned, up to 5000.</param>         
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="cloud_queue" />, that represents the current operation.</returns>
        WASTORAGE_API pplx::task<std::vector<cloud_queue>> list_queues_async(const utility::string_t& prefix, bool get_metadata, const queue_request_options& options, operation_context context) const;

        /// <summary>
        /// Returns a result segment containing a collection of queues in the storage account.
        /// </summary>
        /// <param name="continuation_token">A continuation token returned by a previous listing operation.</param> 
        /// <returns>A result segment containing a collection of queues.</returns>
        queue_result_segment list_queues_segmented(const continuation_token& continuation_token) const
        {
            return list_queues_segmented_async(utility::string_t(), false, -1, continuation_token, queue_request_options(), operation_context()).get();
        }

        /// <summary>
        /// Returns a result segment containing a collection of queues in the storage account.
        /// </summary>
        /// <param name="continuation_token">A continuation token returned by a previous listing operation.</param> 
        /// <param name="prefix">The queue name prefix.</param>
        /// <returns>A result segment containing a collection of queues.</returns>
        queue_result_segment list_queues_segmented(const utility::string_t& prefix, const continuation_token& continuation_token) const
        {
            return list_queues_segmented_async(prefix, false, -1, continuation_token, queue_request_options(), operation_context()).get();
        }

        /// <summary>
        /// Returns a result segment containing a collection of queues in the storage account.
        /// </summary>
        /// <param name="continuation_token">A continuation token returned by a previous listing operation.</param> 
        /// <param name="prefix">The queue name prefix.</param>
        /// <param name="get_metadata">A enumeration describing which items to include in the listing.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned at a time, up to the 
        /// per-operation limit of 5000. If this value is 0, the maximum possible number of results will be returned, up to 5000.</param>         
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A result segment containing a collection of queues.</returns>
        queue_result_segment list_queues_segmented(const utility::string_t& prefix, bool get_metadata, int max_results, const continuation_token& continuation_token, const queue_request_options& options, operation_context context) const
        {
            return list_queues_segmented_async(prefix, get_metadata, max_results, continuation_token, options, context).get();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to return a result segment containing a collection of queue items.
        /// </summary>
        /// <param name="continuation_token">A continuation token returned by a previous listing operation.</param> 
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="queue_result_segment" /> that represents the current operation.</returns>
        pplx::task<queue_result_segment> list_queues_segmented_async(const continuation_token& continuation_token) const
        {
            return list_queues_segmented_async(utility::string_t(), false, -1, continuation_token, queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to return a result segment containing a collection of queue items.
        /// </summary>
        /// <param name="continuation_token">A continuation token returned by a previous listing operation.</param> 
        /// <param name="prefix">The queue name prefix.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="queue_result_segment" /> that represents the current operation.</returns>
        pplx::task<queue_result_segment> list_queues_segmented_async(const utility::string_t& prefix, const continuation_token& continuation_token) const
        {
            return list_queues_segmented_async(prefix, false, -1, continuation_token, queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a result segment containing a collection of queues in the storage account.
        /// </summary>
        /// <param name="continuation_token">A continuation token returned by a previous listing operation.</param> 
        /// <param name="prefix">The queue name prefix.</param>
        /// <param name="get_metadata">A flag that specifies whether to retrieve queue metadata.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned at a time, up to the 
        /// per-operation limit of 5000. If this value is 0, the maximum possible number of results will be returned, up to 5000.</param>         
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="queue_result_segment" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<queue_result_segment> list_queues_segmented_async(const utility::string_t& prefix, bool get_metadata, int max_results, const continuation_token& continuation_token, const queue_request_options& options, operation_context context) const;

        /// <summary>
        /// Gets the service properties for the service client.
        /// </summary>
        /// <returns>The <see cref="service_properties" /> for the service client.</returns>
        service_properties download_service_properties() const
        {
            return download_service_properties_async().get();
        }

        /// <summary>
        /// Gets the service properties for the service client.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The <see cref="service_properties" /> for the service client.</returns>
        service_properties download_service_properties(const queue_request_options& options, operation_context context) const
        {
            return download_service_properties_async(options, context).get();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to get the properties of the service.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="service_properties" /> that represents the current operation.</returns>
        pplx::task<service_properties> download_service_properties_async() const
        {
            return download_service_properties_async(queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to get the properties of the service.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="service_properties" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<service_properties> download_service_properties_async(const queue_request_options& options, operation_context context) const;

        /// <summary>
        /// Sets the service properties for the service client.
        /// </summary>
        /// <param name="properties">The <see cref="service_properties" /> for the service client.</param>
        /// <param name="includes">A <see cref="wa::storage::service_properties_includes"/> enumeration describing which items to include when setting service properties.</param>
        void upload_service_properties(const service_properties& properties, const service_properties_includes& includes) const
        {
            upload_service_properties_async(properties, includes).wait();
        }

        /// <summary>
        /// Sets the service properties for the service client.
        /// </summary>
        /// <param name="properties">The <see cref="service_properties" /> for the service client.</param>
        /// <param name="includes">A <see cref="wa::storage::service_properties_includes"/> enumeration describing which items to include when setting service properties.</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_service_properties(const service_properties& properties, const service_properties_includes& includes, const queue_request_options& options, operation_context context) const
        {
            upload_service_properties_async(properties, includes, options, context).wait();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to set the service properties for the service client.
        /// </summary>
        /// <param name="properties">The <see cref="service_properties" /> for the service client.</param>
        /// <param name="includes">A <see cref="wa::storage::service_properties_includes"/> enumeration describing which items to include when setting service properties.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_service_properties_async(const service_properties& properties, const service_properties_includes& includes) const
        {
            return upload_service_properties_async(properties, includes, queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to set the service properties for the service client.
        /// </summary>
        /// <param name="properties">The <see cref="service_properties" /> for the service client.</param>
        /// <param name="includes">A <see cref="wa::storage::service_properties_includes"/> enumeration describing which items to include when setting service properties.</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_service_properties_async(const service_properties& properties, const service_properties_includes& includes, const queue_request_options& options, operation_context context) const;

        /// <summary>
        /// Returns a reference to a queue object with the specified name.
        /// </summary>
        /// <param name="queue_name">The name of the queue, or an absolute URI to the queue.</param>
        /// <returns>The queue.</returns>
        WASTORAGE_API cloud_queue get_queue_reference(const utility::string_t& queue_name) const;

        const queue_request_options& default_request_options() const
        {
            return m_default_request_options;
        }

    protected:

        /// <summary>
        /// Sets the authentication scheme to use to sign HTTP requests.
        /// </summary>
        /// <param name="value">The authentication scheme.</param>
        WASTORAGE_API void set_authentication_scheme(wa::storage::authentication_scheme value) override;

    private:

        void initialize()
        {
            set_authentication_scheme(authentication_scheme::shared_key);
        }

        queue_request_options get_modified_options(const queue_request_options& options) const;

        queue_request_options m_default_request_options;
    };

    /// <summary>
    /// Represents a queue in the Windows Azure Queue service.
    /// </summary>
    class cloud_queue
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="cloud_queue"/> class.
        /// </summary>
        cloud_queue()
            : m_approximate_message_count(-1)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="cloud_queue"/> class.
        /// </summary>
        /// <param name="uri">A <see cref="storage_uri" /> object containing the absolute URI to the queue for all locations.</param>
        WASTORAGE_API cloud_queue(const storage_uri& uri);

        /// <summary>
        /// Initializes a new instance of the <see cref="cloud_queue"/> class.
        /// </summary>
        /// <param name="uri">A <see cref="storage_uri" /> object containing the absolute URI to the queue for all locations.</param>
        /// <param name="credentials">The <see cref="storage_credentials" /> to use.</param>
        WASTORAGE_API cloud_queue(const storage_uri& uri, const storage_credentials& credentials);

        // TODO: Consider using the name create_queue for consistency with delete

        /// <summary>
        /// Creates the queue.
        /// </summary>
        void create()
        {
            create_async().wait();
        }

        /// <summary>
        /// Creates the queue.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        void create(const queue_request_options& options, operation_context context)
        {
            create_async(options, context).wait();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to create the queue.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> create_async()
        {
            return create_async(queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to create the queue.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> create_async(const queue_request_options& options, operation_context context);

        /// <summary>
        /// Creates the queue if it does not already exist.
        /// </summary>
        bool create_if_not_exists()
        {
            return create_if_not_exists_async().get();
        }

        /// <summary>
        /// Creates the queue if it does not already exist.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        bool create_if_not_exists(const queue_request_options & options, operation_context context)
        {
            return create_if_not_exists_async(options, context).get();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to create the queue if it does not already exist.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<bool> create_if_not_exists_async()
        {
            return create_if_not_exists_async(queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to create the queue if it does not already exist.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<bool> create_if_not_exists_async(const queue_request_options& options, operation_context context);

        /// <summary>
        /// Deletes the queue.
        /// </summary>
        void delete_queue()
        {
            delete_queue_async().wait();
        }

        /// <summary>
        /// Deletes the queue.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        void delete_queue(const queue_request_options& options, operation_context context)
        {
            delete_queue_async(options, context).wait();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to delete the queue.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> delete_queue_async()
        {
            return delete_queue_async(queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to delete the queue.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> delete_queue_async(const queue_request_options& options, operation_context context);

        /// <summary>
        /// Deletes the queue if it exists.
        /// </summary>
        bool delete_queue_if_exists()
        {
            return delete_queue_if_exists_async().get();
        }

        /// <summary>
        /// Deletes the queue if it exists.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        bool delete_queue_if_exists(const queue_request_options& options, operation_context context)
        {
            return delete_queue_if_exists_async(options, context).get();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to delete the queue if it exists.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<bool> delete_queue_if_exists_async()
        {
            return delete_queue_if_exists_async(queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to delete the queue if it exists.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<bool> delete_queue_if_exists_async(const queue_request_options& options, operation_context context);

        /// <summary>
        /// Checks for the existence of the queue.
        /// </summary>
        bool exists() const
        {
            return exists_async().get();
        }

        /// <summary>
        /// Checks for the existence of the queue.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        bool exists(const queue_request_options& options, operation_context context) const
        {
            return exists_async(options, context).get();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to check for the existence of the queue.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<bool> exists_async() const
        {
            return exists_async(queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to check for the existence of the queue.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<bool> exists_async(const queue_request_options& options, operation_context context) const;

        /// <summary>
        /// Adds a message to the queue.
        /// </summary>
        /// <param name="message">The message to add to the queue.</param>
        void add_message(cloud_queue_message& message)
        {
            add_message_async(message).get();
        }

        /// <summary>
        /// Adds a message to the queue.
        /// </summary>
        /// <param name="message">The message to add to the queue.</param>
        /// <param name="time_to_live">The maximum time to allow the message to be in the queue.</param>
        /// <param name="initial_visibility_timeout">The length of time from now during which the message will be invisible.</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        void add_message(cloud_queue_message& message, std::chrono::seconds time_to_live, std::chrono::seconds initial_visibility_timeout, queue_request_options& options, operation_context context)
        {
            add_message_async(message, time_to_live, initial_visibility_timeout, options, context).get();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to add a message to the queue.
        /// </summary>
        /// <param name="message">The message to add to the queue.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> add_message_async(cloud_queue_message& message)
        {
            return add_message_async(message, std::chrono::seconds(604800LL), std::chrono::seconds(0LL), queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation to add a message to the queue.
        /// </summary>
        /// <param name="message">The message to add to the queue.</param>
        /// <param name="time_to_live">The maximum time to allow the message to be in the queue.</param>
        /// <param name="initial_visibility_timeout">The length of time from now during which the message will be invisible.</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> add_message_async(cloud_queue_message& message, std::chrono::seconds time_to_live, std::chrono::seconds initial_visibility_timeout, queue_request_options& options, operation_context context);

        /// <summary>
        /// Retrieves a message from the front of the queue
        /// </summary>
        /// <returns>A <see cref="cloud_queue_message" /> object.</returns>
        cloud_queue_message get_message()
        {
            return get_message_async().get();
        }

        /// <summary>
        /// Retrieves a message from the front of the queue
        /// </summary>
        /// <param name="visibility_timeout">The length of time from now during which the message will be invisible.</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="cloud_queue_message" /> object.</returns>
        cloud_queue_message get_message(std::chrono::seconds visibility_timeout, queue_request_options& options, operation_context context)
        {
            return get_message_async(visibility_timeout, options, context).get();
        }

        /*
        {
            auto getMessages = get_messages(1, options);
            return getMessages.then([] (const std::vector<cloud_queue_message>& v) -> cloud_queue_message
            {
                if (v.empty())
                {
                    throw storage_exception(web::http::status_codes::NotFound, "Queue is empty");
                }

                return v[0];
            });
        }
        */

        /// <summary>
        /// Returns a task that performs an asynchronous operation that retrieves a message from the front of the queue
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="cloud_queue_message" /> that represents the current operation.</returns>
        pplx::task<cloud_queue_message> get_message_async()
        {
            return get_message_async(std::chrono::seconds(0LL), queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that retrieves a message from the front of the queue
        /// </summary>
        /// <param name="visibility_timeout">The length of time from now during which the message will be invisible.</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="cloud_queue_message" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<cloud_queue_message> get_message_async(std::chrono::seconds visibility_timeout, queue_request_options& options, operation_context context);

        /// <summary>
        /// Retrieves the specified number of messages from the front of the queue.
        /// </summary>
        /// <param name="message_count">The number of messages to retrieve.</param>
        /// <returns>An enumerable collection of <see cref="cloud_queue_message" /> objects.</returns>
        std::vector<cloud_queue_message> get_messages(size_t message_count)
        {
            return get_messages_async(message_count).get();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that retrieves the specified number of messages from the front of the queue.
        /// </summary>
        /// <param name="message_count">The number of messages to retrieve.</param>
        /// <param name="visibility_timeout">The length of time from now during which the message will be invisible.</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An enumerable collection of <see cref="cloud_queue_message" /> objects.</returns>
        std::vector<cloud_queue_message> get_messages(size_t message_count, std::chrono::seconds visibility_timeout, queue_request_options& options, operation_context context)
        {
            return get_messages_async(message_count, visibility_timeout, options, context).get();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that retrieves the specified number of messages from the front of the queue.
        /// </summary>
        /// <param name="message_count">The number of messages to retrieve.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="cloud_queue_message" />, that represents the current operation.</returns>
        pplx::task<std::vector<cloud_queue_message>> get_messages_async(size_t message_count)
        {
            return get_messages_async(message_count, std::chrono::seconds(0LL), queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that retrieves the specified number of messages from the front of the queue.
        /// </summary>
        /// <param name="message_count">The number of messages to retrieve.</param>
        /// <param name="visibility_timeout">The length of time from now during which the message will be invisible.</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="cloud_queue_message" />, that represents the current operation.</returns>
        WASTORAGE_API pplx::task<std::vector<cloud_queue_message>> get_messages_async(size_t message_count, std::chrono::seconds visibility_timeout, queue_request_options& options, operation_context context);

        /// <summary>
        /// Peeks a message from the front of the queue
        /// </summary>
        /// <returns>A <see cref="cloud_queue_message"/> object.</returns>
        cloud_queue_message peek_message() const
        {
            return peek_message_async().get();
        }

        /// <summary>
        /// Peeks a message from the front of the queue
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="cloud_queue_message"/> object.</returns>
        cloud_queue_message peek_message(const queue_request_options& options, operation_context context) const
        {
            return peek_message_async(options, context).get();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that peeks a message from the front of the queue
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="cloud_queue_message" /> that represents the current operation.</returns>
        pplx::task<cloud_queue_message> peek_message_async() const
        {
            return peek_message_async(queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that peeks a message from the front of the queue
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="cloud_queue_message" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<cloud_queue_message> peek_message_async(const queue_request_options& options, operation_context context) const;

        /// <summary>
        /// Retrieves the specified number of messages from the front of the queue, without affecting
        /// message visibility.
        /// </summary>
        /// <param name="message_count">The number of messages to be retrieved.</param>
        /// <returns>An enumerable collection of <see cref="cloud_queue_message" /> objects.</returns>
        std::vector<cloud_queue_message> peek_messages(size_t message_count) const
        {
            return peek_messages_async(message_count).get();
        }

        /// <summary>
        /// Retrieves the specified number of messages from the front of the queue, without affecting
        /// message visibility.
        /// </summary>
        /// <param name="message_count">The number of messages to be retrieved.</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An enumerable collection of <see cref="cloud_queue_message" /> objects.</returns>
        std::vector<cloud_queue_message> peek_messages(size_t message_count, const queue_request_options& options, operation_context context) const
        {
            return peek_messages_async(message_count, options, context).get();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that retrieves the specified number of messages from the front of the queue, without affecting
        /// message visibility.
        /// </summary>
        /// <param name="message_count">The number of messages to be retrieved.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="cloud_queue_message" /> that represents the current operation.</returns>
        pplx::task<std::vector<cloud_queue_message>> peek_messages_async(size_t message_count) const
        {
            return peek_messages_async(message_count, queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that retrieves the specified number of messages from the front of the queue, without affecting
        /// message visibility.
        /// </summary>
        /// <param name="message_count">The number of messages to be retrieved.</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="cloud_queue_message" />, that represents the current operation.</returns>
        WASTORAGE_API pplx::task<std::vector<cloud_queue_message>> peek_messages_async(size_t message_count, const queue_request_options& options, operation_context context) const;

        /// <summary>
        /// Updates the visibility timeout and the contents of the specified message in the queue.
        /// </summary>
        /// <param name="message">The message to update.</param>
        /// <param name="visibility_timeout">The time interval, in seconds, after which the message becomes visible again, unless it has been deleted.</param>
        /// <param name="update_content"><c>true</c> to update the content of the message.</param>
        void update_message(cloud_queue_message& message, std::chrono::seconds visibility_timeout, bool update_content)
        {
            update_message_async(message, visibility_timeout, update_content).get();
        }

        /// <summary>
        /// Updates the visibility timeout and the contents of the specified message in the queue.
        /// </summary>
        /// <param name="message">The message to update.</param>
        /// <param name="visibility_timeout">The time interval, in seconds, after which the message becomes visible again, unless it has been deleted.</param>
        /// <param name="update_content"><c>true</c> to update the content of the message.</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        void update_message(cloud_queue_message& message, std::chrono::seconds visibility_timeout, bool update_content, queue_request_options& options, operation_context context)
        {
            update_message_async(message, visibility_timeout, update_content, options, context).get();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that updates the visibility timeout and the contents of the specified message in the queue.
        /// </summary>
        /// <param name="message">The message to update.</param>
        /// <param name="visibility_timeout">The time interval, in seconds, after which the message becomes visible again, unless it has been deleted.</param>
        /// <param name="update_content"><c>true</c> to update the content of the message.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> update_message_async(cloud_queue_message& message, std::chrono::seconds visibility_timeout, bool update_content)
        {
            return update_message_async(message, visibility_timeout, update_content, queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that updates the visibility timeout and the contents of the specified message in the queue.
        /// </summary>
        /// <param name="message">The message to update.</param>
        /// <param name="visibility_timeout">The time interval, in seconds, after which the message becomes visible again, unless it has been deleted.</param>
        /// <param name="update_content"><c>true</c> to update the content of the message.</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> update_message_async(cloud_queue_message& message, std::chrono::seconds visibility_timeout, bool update_content, queue_request_options& options, operation_context context);

        /// <summary>
        /// Deletes the given message from the queue.
        /// </summary>
        /// <param name="message">The message to delete.</param>
        void delete_message(cloud_queue_message& message)
        {
            delete_message_async(message).get();
        }

        /// <summary>
        /// Deletes the given message from the queue.
        /// </summary>
        /// <param name="message">The message to delete.</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        void delete_message(cloud_queue_message& message, queue_request_options& options, operation_context context)
        {
            delete_message_async(message, options, context).get();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that deletes the given message from the queue.
        /// </summary>
        /// <param name="message">The message to delete.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> delete_message_async(cloud_queue_message& message)
        {
            return delete_message_async(message, queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that deletes the given message from the queue.
        /// </summary>
        /// <param name="message">The message to delete.</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> delete_message_async(cloud_queue_message& message, queue_request_options& options, operation_context context);

        /// <summary>
        /// Deletes all the messages in the queue.
        /// </summary>
        void clear()
        {
            return clear_async().get();
        }

        /// <summary>
        /// Deletes all the messages in the queue.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        void clear(const queue_request_options& options, operation_context context)
        {
            return clear_async(options, context).get();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that deletes all the messages in the queue.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> clear_async()
        {
            return clear_async(queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that deletes all the messages in the queue.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> clear_async(const queue_request_options& options, operation_context context);

        /// <summary>
        /// Retrieves the user-defined metadata for the queue.
        /// </summary>
        void download_attributes()
        {
            download_attributes_async().wait();
        }

        /// <summary>
        /// Retrieves the user-defined metadata for the queue.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        void download_attributes(const queue_request_options& options, operation_context context)
        {
            download_attributes_async(options, context).wait();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that retrieves the user-defined metadata for the queue.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> download_attributes_async()
        {
            return download_attributes_async(queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that retrieves the user-defined metadata for the queue.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> download_attributes_async(const queue_request_options& options, operation_context context);

        /// <summary>
        /// Sets the user-defined metadata for the queue.
        /// </summary>
        void upload_metadata()
        {
            upload_metadata_async().wait();
        }

        /// <summary>
        /// Sets the user-defined metadata for the queue.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_metadata(const queue_request_options& options, operation_context context)
        {
            upload_metadata_async(options, context).wait();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that sets the user-defined metadata for the queue.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_metadata_async()
        {
            return upload_metadata_async(queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that sets the user-defined metadata for the queue.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_metadata_async(const queue_request_options& options, operation_context context);

        /// <summary>
        /// Retrieves the shared access policies for the queue.
        /// </summary>
        queue_permissions download_permissions() const
        {
            return download_permissions_async().get();
        }

        /// <summary>
        /// Retrieves the shared access policies for the queue.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        queue_permissions download_permissions(const queue_request_options& options, operation_context context) const
        {
            return download_permissions_async(options, context).get();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that retrieves the shared access policies for the queue.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="queue_permissions" /> that represents the current operation.</returns>
        pplx::task<queue_permissions> download_permissions_async() const
        {
            return download_permissions_async(queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that retrieves the shared access policies for the queue.
        /// </summary>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="queue_permissions" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<queue_permissions> download_permissions_async(const queue_request_options& options, operation_context context) const;

        /// <summary>
        /// Sets permissions for the queue.
        /// </summary>
        /// <param name="permissions">The access control list to associate with this queue</param>
        void upload_permissions(const queue_permissions& permissions)
        {
            upload_permissions_async(permissions).wait();
        }

        /// <summary>
        /// Sets permissions for the queue.
        /// </summary>
        /// <param name="permissions">The access control list to associate with this queue</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_permissions(const queue_permissions& permissions, const queue_request_options& options, operation_context context)
        {
            upload_permissions_async(permissions, options, context).wait();
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that sets permissions for the queue.
        /// </summary>
        /// <param name="permissions">The access control list to associate with this queue</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_permissions_async(const queue_permissions& permissions)
        {
            return upload_permissions_async(permissions, queue_request_options(), operation_context());
        }

        /// <summary>
        /// Returns a task that performs an asynchronous operation that sets permissions for the queue.
        /// </summary>
        /// <param name="permissions">The access control list to associate with this queue</param>
        /// <param name="options">A <see cref="wa::storage::queue_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="wa::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_permissions_async(const queue_permissions& permissions, const queue_request_options& options, operation_context context);

        /// <summary>
        /// Gets a shared access signature for the queue.
        /// </summary>
        /// <param name="policy">The access policy for the shared access signature.</param>
        /// <returns>A string containing a shared access signature.</returns>
        utility::string_t get_shared_access_signature(const queue_shared_access_policy& policy) const
        {
            return get_shared_access_signature(policy, utility::string_t());
        }

        /// <summary>
        /// Gets a shared access signature for the queue.
        /// </summary>
        /// <param name="policy">The access policy for the shared access signature.</param>
        /// <param name="stored_policy_identifier">A queue-level access policy.</param>
        /// <returns>A string containing a shared access signature.</returns>
        WASTORAGE_API utility::string_t get_shared_access_signature(const queue_shared_access_policy& policy, const utility::string_t& stored_policy_identifier) const;

        /// <summary>
        /// Gets the Queue service client that specifies the endpoint for the queue service.
        /// </summary>
        /// <returns>The <see cref="cloud_queue_client" /> object that specifies the endpoint for the queue service.</returns>
        const cloud_queue_client& service_client() const 
        { 
            return m_client;
        }

        /*
        /// <summary>
        /// Gets the account credentials used to create the service client.
        /// </summary>
        /// <returns>The <see cref="storage_credentials" /> to use.</returns>
        std::shared_ptr<storage_credentials> credentials() const;
        */

        /// <summary>
        /// Gets the name of the queue.
        /// </summary>
        /// <returns>The name of the queue.</returns>
        const utility::string_t& name() const
        {
            return m_name;
        }

        /// <summary>
        /// Gets the queue URI for all locations.
        /// </summary>
        /// <returns>A <see cref="storage_uri" /> containing the queue URI for all locations.</returns>
        const storage_uri& uri() const
        {
            return m_uri;
            //return uri_builder(m_client.uri()).append_path(m_name, true).to_uri();
        }

        /// <summary>
        /// Gets the approximate number of messages in the queue or -1 if unknown.
        /// </summary>
        /// <returns>The approximate number of messages in the queue or -1 if unknown.</returns>
        /// <remarks>Call the download_attributes function to retrieve this data from the queue service.</remarks>
        int approximate_message_count() const
        {
            return m_approximate_message_count;
        }

        /// <summary>
        /// Gets an unordered map containing user-defined key/value pairs associated with the queue.
        /// </summary>
        /// <returns>An unordered map containing user-defined key/value pairs associated with the queue.</returns>
        /// <remarks>Call the download_attributes function to retrieve this data from the queue service.</remarks>
        cloud_metadata& metadata()
        {
            return m_metadata;
        }

        /// <summary>
        /// Gets an unordered map containing user-defined key/value pairs associated with the queue.
        /// </summary>
        /// <returns>An unordered map containing user-defined key/value pairs associated with the queue.</returns>
        /// <remarks>Call the download_attributes function to retrieve this data from the queue service.</remarks>
        const cloud_metadata& metadata() const
        {
            return m_metadata;
        }

        /// <summary>
        /// Sets an unordered map containing user-defined key/value pairs associated with the queue.
        /// </summary>
        /// <param name="value">An unordered map containing user-defined key/value pairs associated with the queue.</param>
        /// <remarks>Call the upload_attributes function to save this data to the queue service.</remarks>
        void set_metadata(const cloud_metadata& value)
        {
            m_metadata = std::move(value);
        }

    private:

        WASTORAGE_API cloud_queue(const cloud_queue_client& client, const utility::string_t& name);

        /*
        cloud_queue(const cloud_queue_client& client, const utility::string_t& name, const storage_uri& uri)
            : m_client(client), m_name(name), m_uri(uri), m_approximate_message_count(-1)
        {
        }
        */

        static cloud_queue_client create_client(const storage_uri& uri, const storage_credentials& credentials);
        static utility::string_t read_queue_name(const storage_uri& uri);
        static storage_uri create_uri(const cloud_queue_client& client, const utility::string_t& queue_name);
        queue_request_options get_modified_options(const queue_request_options& options) const;
        pplx::task<bool> create_async_impl(const queue_request_options& options, operation_context context, bool allow_conflict);
        pplx::task<bool> delete_async_impl(const queue_request_options& options, operation_context context, bool allow_not_found);
        pplx::task<bool> exists_async_impl(const queue_request_options& options, operation_context context, bool allow_secondary) const;

        cloud_queue_client m_client;
        utility::string_t m_name;
        storage_uri m_uri;
        int m_approximate_message_count;
        cloud_metadata m_metadata;

        friend class cloud_queue_client;
    };


}} // namespace wa::storage
