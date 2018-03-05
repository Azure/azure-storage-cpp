// -----------------------------------------------------------------------------------------
// <copyright file="blob.h" company="Microsoft">
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

    class cloud_blob;
    class cloud_block_blob;
    class cloud_page_blob;
    class cloud_append_blob;
    class cloud_blob_directory;
    class cloud_blob_container;
    class cloud_blob_client;

    class cloud_file;
    class file_access_condition;

    namespace protocol
    {
        class blob_response_parsers;
        class block_list_reader;
        class list_containers_reader;
        class list_blobs_reader;
    }

    namespace core
    {
        class cloud_append_blob_ostreambuf;
    }

    /// <summary>
    /// The type of a blob.
    /// </summary>
    enum class blob_type
    {
        /// <summary>
        /// Not specified.
        /// </summary>
        unspecified,

        /// <summary>
        /// A page blob.
        /// </summary>
        page_blob,

        /// <summary>
        /// A block blob.
        /// </summary>
        block_blob,

        /// <summary>
        /// An append blob.
        /// </summary>
        append_blob,
    };

    /// <summary>
    /// Specifies the level of public access that is allowed on the container.
    /// </summary>
    enum class blob_container_public_access_type
    {
        /// <summary>
        /// No public access. Only the account owner can read resources in this container.
        /// </summary>
        off,

        /// <summary>
        /// Container-level public access. Anonymous clients can read container and blob data.
        /// </summary>
        container,

        /// <summary>
        /// Blob-level public access. Anonymous clients can read only blob data within this container.
        /// </summary>
        blob
    };

    /// <summary>
    /// Represents a shared access policy, which specifies the start time, expiry time, 
    /// and permissions for a shared access signature for a blob or container.
    /// </summary>
    class blob_shared_access_policy : public shared_access_policy
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
            /// Permission granted to read the content, properties, metadata, and block list for a blob or any blob in a container. Use blob as the source of a copy operation.
            /// </summary>
            read = 1,

            /// <summary>
            /// Permission granted to create or write the content, properties, metadata, and block list for a blob or any blob in a container. Snapshot or lease the blob. Resize the blob (page blob only). Use the blob as the destination of a copy operation within the same account.
            /// </summary>
            write = 2,

            /// <summary>
            /// Permission granted to delete a blob or any blob in a container.
            /// </summary>
            del = 4,

            /// <summary>
            /// Permission granted to list the blobs in a container.
            /// </summary>
            list = 8,

            /// <summary>
            /// Permission to add a block to an append blob granted.
            /// </summary>
            add = 0x10,

            /// <summary>
            /// Permission to write a new blob, snapshot a blob, or copy a blob to a new blob granted.
            /// </summary>
            create = 0x80
        };

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::blob_shared_access_policy" /> class.
        /// </summary>
        blob_shared_access_policy()
            : shared_access_policy()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::blob_shared_access_policy" /> class.
        /// </summary>
        /// <param name="expiry">The expiration date and time of the policy.</param>
        /// <param name="permission">A mask containing the permissions of the policy</param>
        blob_shared_access_policy(utility::datetime expiry, uint8_t permission)
            : shared_access_policy(expiry, permission)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::blob_shared_access_policy" /> class.
        /// </summary>
        /// <param name="start">The start date and time of the policy.</param>
        /// <param name="expiry">The expiration date and time of the policy.</param>
        /// <param name="permission">A mask containing the permissions of the policy</param>
        blob_shared_access_policy(utility::datetime start, utility::datetime expiry, uint8_t permission)
            : shared_access_policy(start, expiry, permission)
        {
        }
    };

    /// <summary>
    /// Specifies which items to include when listing a set of blobs.
    /// </summary>
    struct blob_listing_details
    {
        enum values
        {
            /// <summary>
            /// List only committed blobs, and do not return blob metadata.
            /// </summary>
            none = 0,

            /// <summary>
            /// List committed blobs and blob snapshots.
            /// </summary>
            snapshots = 1 << 0,

            /// <summary>
            /// Retrieve blob metadata for each blob returned in the listing.
            /// </summary>
            metadata = 1 << 1,

            /// <summary>
            /// List committed and uncommitted blobs.
            /// </summary>
            uncommitted_blobs = 1 << 2,

            /// <summary>
            /// Include copy properties in the listing.
            /// </summary>
            copy = 1 << 3,

            /// <summary>
            /// List all available committed blobs, uncommitted blobs, and snapshots, and return all metadata and copy status for those blobs.
            /// </summary>
            all = snapshots | metadata | uncommitted_blobs | copy
        };
    };

    /// <summary>
    /// Specifies which details to include when listing the containers in this storage account.
    /// </summary>
    struct container_listing_details
    {
        enum values
        {
            /// <summary>
            /// No additional details.
            /// </summary>
            none = 0x0,

            /// <summary>
            /// Retrieve container metadata.
            /// </summary>
            metadata = 1 << 0,

            /// <summary>
            /// Retrieve all available details.
            /// </summary>
            all = metadata
        };
    };

    /// <summary>
    /// Indicates whether to list only committed blocks, only uncommitted blocks, or all blocks.
    /// </summary>
    enum class block_listing_filter
    {
        /// <summary>
        /// Committed blocks.
        /// </summary>
        committed,

        /// <summary>
        /// Uncommitted blocks.
        /// </summary>
        uncommitted,

        /// <summary>
        /// Both committed and uncommitted blocks.
        /// </summary>
        all
    };

    /// <summary>
    /// Describes actions that may be used for writing to a page blob or clearing a set of pages.
    /// </summary>
    enum class page_write
    {
        /// <summary>
        /// Update the page with new data.
        /// </summary>
        update,

        /// <summary>
        /// Clear the page.
        /// </summary>
        clear
    };

    /// <summary>
    /// Describes the set of options for deleting snapshots as part of a delete blob operation.
    /// </summary>
    enum class delete_snapshots_option
    {
        /// <summary>
        /// Delete the blob only. If the blob has snapshots, this option will result in an error from the service.
        /// </summary>
        none,

        /// <summary>
        /// Delete the blob and its snapshots.
        /// </summary>
        include_snapshots,

        /// <summary>
        /// Delete the blob's snapshots only.
        /// </summary>
        delete_snapshots_only
    };

    /// <summary>
    /// Represents a set of access conditions to be used for operations against the Blob service. 
    /// </summary>
    class access_condition
    {
    public:

        /// <summary>
        /// Describes the set of operators for comparing the blob's sequence number with the specified value.
        /// </summary>
        enum class sequence_number_operators
        {
            /// <summary>
            /// The blob's sequence number will not be used as an access condition.
            /// </summary>
            none,

            /// <summary>
            /// If the blob's sequence number is less than the specified value, the request proceeds.
            /// </summary>
            le,

            /// <summary>
            /// If the blob's sequence number is less than or equal to the specified value, the request proceeds.
            /// </summary>
            lt,

            /// <summary>
            /// If the blob's sequence number is equal to the specified value, the request proceeds.
            /// </summary>
            eq,
        };

        /// <summary>
        /// Constructs an empty access condition.
        /// </summary>
        access_condition()
            : m_sequence_number(0), m_sequence_number_operator(sequence_number_operators::none),
            m_max_size(-1), m_append_position(-1)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="access_condition" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="access_condition" /> object.</param>
        access_condition(access_condition&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="access_condition" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="access_condition" /> object to use to set properties.</param>
        /// <returns>An <see cref="access_condition" /> object with properties set.</returns>
        access_condition& operator=(access_condition&& other)
        {
            if (this != &other)
            {
                m_if_match_etag = std::move(other.m_if_match_etag);
                m_if_none_match_etag = std::move(other.m_if_none_match_etag);
                m_if_modified_since_time = std::move(other.m_if_modified_since_time);
                m_if_not_modified_since_time = std::move(other.m_if_not_modified_since_time);
                m_lease_id = std::move(other.m_lease_id);
                m_sequence_number = std::move(other.m_sequence_number);
                m_sequence_number_operator = std::move(other.m_sequence_number_operator);
                m_max_size = std::move(other.m_max_size);
                m_append_position = std::move(other.m_append_position);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Generates an empty access condition.
        /// </summary>
        /// <returns>An empty <see cref="azure::storage::access_condition" /> object.</returns>
        static access_condition generate_empty_condition()
        {
            return access_condition();
        }

        /// <summary>
        /// Generates an access condition such that an operation will be performed only if the resource does not exist on the service.
        /// </summary>
        /// <returns>An <see cref="azure::storage::access_condition"/> object that represents the If-Not-Exists condition.</returns>
        /// <remarks>Setting this access condition modifies the request to include the HTTP <i>If-None-Match</i> conditional header.</remarks>
        static access_condition generate_if_not_exists_condition()
        {
            access_condition condition;
            condition.set_if_none_match_etag(_XPLATSTR("*"));
            return condition;
        }

        /// <summary>
        /// Generates an access condition such that an operation will be performed only if the resource exists on the service.
        /// </summary>
        /// <returns>An <see cref="azure::storage::access_condition"/> object that represents the If-Exists condition.</returns>
        /// <remarks>Setting this access condition modifies the request to include the HTTP <i>If-Match</i> conditional header.</remarks>
        static access_condition generate_if_exists_condition()
        {
            access_condition condition;
            condition.set_if_match_etag(_XPLATSTR("*"));
            return condition;
        }

        /// <summary>
        /// Generates an access condition such that an operation will be performed only if the resource's ETag value
        /// matches the specified ETag value.
        /// </summary>
        /// <param name="etag">The ETag value that must be matched.</param>
        /// <returns>An <see cref="azure::storage::access_condition" /> object that represents the If-Match condition.</returns>
        static access_condition generate_if_match_condition(utility::string_t etag)
        {
            access_condition condition;
            condition.set_if_match_etag(std::move(etag));
            return condition;
        }

        /// <summary>
        /// Generates an access condition such that an operation will be performed only if the resource has been
        /// modified since the specified time.
        /// </summary>
        /// <param name="modified_time">The time since which the resource must have been modified in order for the operation to proceed.</param>
        /// <returns>An <see cref="azure::storage::access_condition" /> object that represents the If-Modified-Since condition.</returns>
        static access_condition generate_if_modified_since_condition(utility::datetime modified_time)
        {
            access_condition condition;
            condition.set_if_modified_since_time(modified_time);
            return condition;
        }

        /// <summary>
        /// Generates an access condition such that an operation will be performed only if the resource's ETag value
        /// does not match the specified ETag value.
        /// </summary>
        /// <param name="etag">The ETag value that must be matched, or <c>"*"</c>.</param>
        /// <returns>An <see cref="azure::storage::access_condition" /> object that represents the If-None-Match condition.</returns>
        /// <remarks>
        /// If <c>"*"</c> is specified as the parameter then this condition requires that the resource does not exist.
        /// </remarks>
        static access_condition generate_if_none_match_condition(utility::string_t etag)
        {
            access_condition condition;
            condition.set_if_none_match_etag(std::move(etag));
            return condition;
        }

        /// <summary>
        /// Generates an access condition such that an operation will be performed only if the resource has not been
        /// modified since the specified time.
        /// </summary>
        /// <param name="modified_time">The time since which the resource must not have been modified in order for the operation to proceed.</param>
        /// <returns>An <see cref="azure::storage::access_condition" /> object that represents the If-Unmodified-Since condition.</returns>
        static access_condition generate_if_not_modified_since_condition(utility::datetime modified_time)
        {
            access_condition condition;
            condition.set_if_not_modified_since_time(modified_time);
            return condition;
        }

        /// <summary>
        /// Generates an access condition such that an operation will be performed only if resource's current sequence
        /// number is less than or equal to the specified value.
        /// </summary>
        /// <param name="sequence_number">The value that the current sequence number of the resource must be less than or equal to.</param>
        /// <returns>An <see cref="azure::storage::access_condition" /> object that represents the If-Sequence-Number-LE condition.</returns>
        static access_condition generate_if_sequence_number_less_than_or_equal_condition(int64_t sequence_number)
        {
            access_condition condition;
            condition.set_if_sequence_number_less_than_or_equal(sequence_number);
            return condition;
        }

        /// <summary>
        /// Generates an access condition such that an operation will be performed only if resource's current sequence
        /// number is less than the specified value.
        /// </summary>
        /// <param name="sequence_number">The value that the current sequence number of the resource must be less than.</param>
        /// <returns>An <see cref="azure::storage::access_condition" /> object that represents the If-Sequence-Number-LT condition.</returns>
        static access_condition generate_if_sequence_number_less_than_condition(int64_t sequence_number)
        {
            access_condition condition;
            condition.set_if_sequence_number_less_than(sequence_number);
            return condition;
        }

        /// <summary>
        /// Generates an access condition such that an operation will be performed only if resource's current sequence
        /// number is equal to the specified value.
        /// </summary>
        /// <param name="sequence_number">The value that the current sequence number of the resource must be equal to.</param>
        /// <returns>An <see cref="azure::storage::access_condition" /> object that represents the If-Sequence-Number-EQ condition.</returns>
        static access_condition generate_if_sequence_number_equal_condition(int64_t sequence_number)
        {
            access_condition condition;
            condition.set_if_sequence_number_equal(sequence_number);
            return condition;
        }

        /// <summary>
        /// Generates an access condition such that an operation will be performed only if the lease ID on the
        /// resource matches the specified lease ID.
        /// </summary>
        /// <param name="lease_id">The lease ID that must match the lease ID of the resource.</param>
        /// <returns>An <see cref="azure::storage::access_condition" /> object that represents the lease condition.</returns>
        static access_condition generate_lease_condition(utility::string_t lease_id)
        {
            access_condition condition;
            condition.set_lease_id(std::move(lease_id));
            return condition;
        }

        /// <summary>
        /// Generates an access condition such that an operation will be performed only if the size of the blob after
        /// committing the block is less than or equal to the specified value.
        /// </summary>
        /// <param name="max_size">The value specifying the maximum length to restrict the blob to when committing the block.</param>
        /// <returns>An <see cref="azure::storage::access_condition" /> object that represents the blob-condition-maxsize condition.</returns>
        static access_condition generate_if_max_size_less_than_or_equal_condition(int64_t max_size)
        {
            access_condition condition;
            condition.set_max_size(max_size);
            return condition;
        }

        /// <summary>
        /// Generates an access condition such that an operation will be performed only if the end position of the blob is equal to the specified value.
        /// </summary>
        /// <param name="append_position">The value to compare to the current end position of the blob.</param>
        /// <returns>An <see cref="azure::storage::access_condition" /> object that represents the blob-condition-appendpos condition.</returns>
        static access_condition generate_if_append_position_equal_condition(int64_t append_position)
        {
            access_condition condition;
            condition.set_append_position(append_position);
            return condition;
        }

        /// <summary>
        /// Gets an ETag value that must match the ETag of a resource.
        /// </summary>
        /// <returns>A string containing the ETag, in quotes.</returns>
        const utility::string_t& if_match_etag() const
        {
            return m_if_match_etag;
        }

        /// <summary>
        /// Sets an ETag that must match the ETag of a resource.
        /// </summary>
        /// <param name="value">A string containing the ETag, in quotes.</param>
        void set_if_match_etag(utility::string_t value)
        {
            m_if_match_etag = std::move(value);
        }

        /// <summary>
        /// Gets an ETag that must not match the ETag of a resource.
        /// </summary>
        /// <returns>A quoted ETag string, or <c>"*"</c> to match any ETag.</returns>
        const utility::string_t& if_none_match_etag() const
        {
            return m_if_none_match_etag;
        }

        /// <summary>
        /// Sets an ETag that must not match the ETag of a resource.
        /// </summary>
        /// <param name="value">A quoted ETag string, or <c>"*"</c> to match any ETag.</param>
        void set_if_none_match_etag(utility::string_t value)
        {
            m_if_none_match_etag = std::move(value);
        }

        /// <summary>
        /// Gets a time that must be before the last-modified time of a resource.
        /// </summary>
        /// <returns>A <see cref="utility::datetime" /> in UTC.</returns>
        utility::datetime if_modified_since_time() const
        {
            return m_if_modified_since_time;
        }

        /// <summary>
        /// Sets a time that must be before the last-modified time of a resource.
        /// </summary>
        /// <param name="value">A <see cref="utility::datetime" /> in UTC.</param>
        void set_if_modified_since_time(utility::datetime value)
        {
            m_if_modified_since_time = value;
        }

        /// <summary>
        /// Gets a time that must not be before the last-modified time of a resource.
        /// </summary>
        /// <returns>A <see cref="utility::datetime" /> in UTC.</returns>
        utility::datetime if_not_modified_since_time() const
        {
            return m_if_not_modified_since_time;
        }

        /// <summary>
        /// Sets a time that must not be before the last-modified time of a resource.
        /// </summary>
        /// <param name="value">A <see cref="utility::datetime" /> in UTC.</param>
        void set_if_not_modified_since_time(utility::datetime value)
        {
            m_if_not_modified_since_time = value;
        }

        /// <summary>
        /// Sets a sequence number that the current sequence number of a page blob must be less than or equal to 
        /// in order for the operation to proceed.
        /// </summary>
        /// <param name="value">A sequence number.</param>
        /// <remarks>This condition only applies to page blobs.</remarks>
        void set_if_sequence_number_less_than_or_equal(int64_t value)
        {
            utility::assert_in_bounds<int64_t>(_XPLATSTR("value"), value, 0);
            m_sequence_number = value;
            m_sequence_number_operator = sequence_number_operators::le;
        }

        /// <summary>
        /// Sets a sequence number that the current sequence number of a page blob must be less than in order 
        /// for the operation to proceed.
        /// </summary>
        /// <param name="value">A sequence number.</param>
        /// <remarks>This condition only applies to page blobs.</remarks>
        void set_if_sequence_number_less_than(int64_t value)
        {
            utility::assert_in_bounds<int64_t>(_XPLATSTR("value"), value, 0);
            m_sequence_number = value;
            m_sequence_number_operator = sequence_number_operators::lt;
        }

        /// <summary>
        /// Sets a sequence number that the current sequence number of a page blob must be equal to in order 
        /// for the operation to proceed.
        /// </summary>
        /// <param name="value">A sequence number.</param>
        /// <remarks>This condition only applies to page blobs.</remarks>
        void set_if_sequence_number_equal(int64_t value)
        {
            utility::assert_in_bounds<int64_t>(_XPLATSTR("value"), value, 0);
            m_sequence_number = value;
            m_sequence_number_operator = sequence_number_operators::eq;
        }

        /// <summary>
        /// Gets a sequence number that the current sequence number of a page blob must be equal to in order 
        /// for the operation to proceed.
        /// </summary>
        /// <returns>A sequence number.</returns>
        /// <remarks>This condition only applies to page blobs.</remarks>
        int64_t sequence_number() const
        {
            return m_sequence_number;
        }

        /// <summary>
        /// Gets the sequence number comparison operator specified for the access condition.
        /// </summary>
        /// <returns>The sequence number comparison operator specified for the <see cref="azure::storage::access_condition" /> object.</returns>
        sequence_number_operators sequence_number_operator() const
        {
            return m_sequence_number_operator;
        }

        /// <summary>
        /// Gets a number that indicates the maximum size in bytes to restrict the blob to when commiting the block.
        /// </summary>
        /// <returns>The maximum size in bytes, or -1 if no maximum size value is specified.</returns>
        /// <remarks>This condition only applies to append blobs.</remarks>
        int64_t max_size() const
        {
            return m_max_size;
        }

        /// <summary>
        /// Sets a number that indicates the maximum size in bytes to restrict the blob to when commiting the block.
        /// </summary>
        /// <param name="value">The maximum size in bytes.</param>
        /// <remarks>This condition only applies to append blobs.</remarks>
        void set_max_size(int64_t value)
        {
            utility::assert_in_bounds<int64_t>(_XPLATSTR("value"), value, 1);
            m_max_size = value;
        }

        /// <summary>
        /// Gets an append position that the end position of an append blob must be equal to in order
        /// for the operation to proceed.
        /// </summary>
        /// <returns>An append position number, or -1 if no append position value is specified.</returns>
        /// <remarks>This condition only applies to append blobs.</remarks>
        int64_t append_position() const
        {
            return m_append_position;
        }

        /// <summary>
        /// Sets the append position that the end position of an append blob must be equal to in order
        /// for the operation to proceed.
        /// </summary>
        /// <param name="value">An append position.</param>
        /// <remarks>This condition only applies to append blobs.</remarks>
        void set_append_position(int64_t value)
        {
            utility::assert_in_bounds<int64_t>(_XPLATSTR("value"), value, 0);
            m_append_position = value;
        }

        /// <summary>
        /// Gets a lease ID that must match the lease on a resource.
        /// </summary>
        /// <returns>A string containing the lease ID.</returns>
        const utility::string_t& lease_id() const
        {
            return m_lease_id;
        }

        /// <summary>
        /// Sets a lease ID that must match the lease on a resource.
        /// </summary>
        /// <param name="value">A string containing the lease ID.</param>
        void set_lease_id(utility::string_t value)
        {
            m_lease_id = std::move(value);
        }

        /// <summary>
        /// Indicates whether the <see cref="azure::storage::access_condition" /> object specifies a condition.
        /// </summary>
        /// <returns><c>true</c> if the access condition specifies a condition; otherwise, <c>false</c>.</returns>
        bool is_conditional() const
        {
            return !m_if_match_etag.empty() ||
                !m_if_none_match_etag.empty() ||
                m_if_modified_since_time.is_initialized() ||
                m_if_not_modified_since_time.is_initialized();
        }

    private:

        utility::string_t m_if_match_etag;
        utility::string_t m_if_none_match_etag;
        utility::datetime m_if_modified_since_time;
        utility::datetime m_if_not_modified_since_time;
        utility::string_t m_lease_id;
        int64_t m_sequence_number;
        sequence_number_operators m_sequence_number_operator;
        int64_t m_max_size;
        int64_t m_append_position;
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

    /// <summary>
    /// The tier of the block blob on a standard storage account.
    /// </summary>
    enum class standard_blob_tier
    {
        /// <summary>
        /// The tier is not recognized by this version of the library
        /// </summary>
        unknown,

        /// <summary>
        /// Hot Storage
        /// </summary>
        hot,

        /// <summary>
        /// Cool Storage
        /// </summary>
        cool,

        /// <summary>
        /// Archive Storage
        /// </summary>
        archive
    };

    /// <summary>
    /// The tier of the page blob.
    /// Please take a look at https://docs.microsoft.com/en-us/azure/storage/storage-premium-storage#scalability-and-performance-targets
    /// for detailed information on the corresponding IOPS and throughput per PremiumPageBlobTier.
    /// </summary>
    enum class premium_blob_tier
    {
        /// <summary>
        /// The tier is not recognized by this version of the library
        /// </summary>
        unknown,

        /// <summary>
        /// P4 Tier
        /// </summary>
        p4,

        /// <summary>
        /// P6 Tier
        /// </summary>
        p6,

        /// <summary>
        /// P10 Tier
        /// </summary>
        p10,

        /// <summary>
        /// P20 Tier
        /// </summary>
        p20,

        /// <summary>
        /// P30 Tier
        /// </summary>
        p30,

        /// <summary>
        /// P40 Tier
        /// </summary>
        p40,

        /// <summary>
        /// P50 Tier
        /// </summary>
        p50,

        /// <summary>
        /// P60 Tier
        /// </summary>
        p60
    };

    /// <summary>
    /// The status of the blob if being re-hydrated.
    /// </summary>
    enum class archive_status
    {
        /// <summary>
        /// The blob's archive status is unknown
        /// </summary>
        unknown,

        /// <summary>
        /// The blob is being re-hydrated to hot
        /// </summary>
        rehydrate_pending_to_hot,

        /// <summary>
        /// The blob is being re-hydrated to cool
        /// </summary>
        rehydrate_pending_to_cool
    };

    /// <summary>
    /// Represents the permissions for a container.
    /// </summary>
    class blob_container_permissions : public cloud_permissions<blob_shared_access_policy>
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::blob_container_permissions" /> class.
        /// </summary>
        blob_container_permissions()
            : cloud_permissions(), m_public_access(blob_container_public_access_type::off)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::blob_container_permissions" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::blob_container_permissions" /> object.</param>
        blob_container_permissions(blob_container_permissions&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::blob_container_permissions" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::blob_container_permissions" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::blob_container_permissions" /> object with properties set.</returns>
        blob_container_permissions& operator=(blob_container_permissions&& other)
        {
            if (this != &other)
            {
                cloud_permissions<blob_shared_access_policy>::operator=(std::move(other));
                m_public_access = std::move(other.m_public_access);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets the public access setting for the container.
        /// </summary>
        /// <returns>The public access setting for the container.</returns>
        blob_container_public_access_type public_access() const
        {
            return m_public_access;
        }

        /// <summary>
        /// Gets or sets the public access setting for the container.
        /// </summary>
        /// <param name="value">The public access setting for the container.</param>
        void set_public_access(blob_container_public_access_type value)
        {
            m_public_access = value;
        }

    private:

        blob_container_public_access_type m_public_access;
    };

    class list_blob_item;

    typedef result_segment<list_blob_item> list_blob_item_segment;
    typedef result_iterator<list_blob_item> list_blob_item_iterator;

    typedef result_segment<cloud_blob_container> container_result_segment;
    typedef result_iterator<cloud_blob_container> container_result_iterator;

    /// <summary>
    /// Represents the system properties for a blob.
    /// </summary>
    class cloud_blob_properties
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_properties" /> class.
        /// </summary>
        cloud_blob_properties()
            :  m_size(0), m_type(blob_type::unspecified), m_lease_status(azure::storage::lease_status::unspecified),
            m_lease_state(azure::storage::lease_state::unspecified),
            m_lease_duration(azure::storage::lease_duration::unspecified),
            m_page_blob_sequence_number(0), m_append_blob_committed_block_count(0),
            m_server_encrypted(false),
            m_is_incremental_copy(false)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_properties" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_blob_properties" /> object.</param>
        cloud_blob_properties(cloud_blob_properties&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_blob_properties" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_blob_properties" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::cloud_blob_properties" /> object with properties set.</returns>
        cloud_blob_properties& operator=(cloud_blob_properties&& other)
        {
            if (this != &other)
            {
                m_cache_control = std::move(other.m_cache_control);
                m_content_disposition = std::move(other.m_content_disposition);
                m_content_encoding = std::move(other.m_content_encoding);
                m_content_language = std::move(other.m_content_language);
                m_size = std::move(other.m_size);
                m_content_md5 = std::move(other.m_content_md5);
                m_content_type = std::move(other.m_content_type);
                m_etag = std::move(other.m_etag);
                m_last_modified = std::move(other.m_last_modified);
                m_type = std::move(other.m_type);
                m_lease_status = std::move(other.m_lease_status);
                m_lease_state = std::move(other.m_lease_state);
                m_lease_duration = std::move(other.m_lease_duration);
                m_page_blob_sequence_number = std::move(other.m_page_blob_sequence_number);
                m_append_blob_committed_block_count = std::move(other.m_append_blob_committed_block_count);
                m_server_encrypted = std::move(other.m_server_encrypted);
                m_is_incremental_copy = std::move(other.m_is_incremental_copy);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets the cache-control value stored for the blob.
        /// </summary>
        /// <returns>The blob's cache-control value.</returns>
        const utility::string_t& cache_control() const
        {
            return m_cache_control;
        }

        /// <summary>
        /// Sets the cache-control value stored for the blob.
        /// </summary>
        /// <param name="value">The blob's cache-control value.</param>
        void set_cache_control(utility::string_t value)
        {
            m_cache_control = std::move(value);
        }

        /// <summary>
        /// Gets the content-disposition value stored for the blob.
        /// </summary>
        /// <returns>The blob's content-disposition value.</returns>
        const utility::string_t& content_disposition() const
        {
            return m_content_disposition;
        }

        /// <summary>
        /// Sets the content-disposition value stored for the blob.
        /// </summary>
        /// <param name="value">The blob's content-disposition value.</param>
        void set_content_disposition(utility::string_t value)
        {
            m_content_disposition = std::move(value);
        }

        /// <summary>
        /// Gets the content-encoding value stored for the blob.
        /// </summary>
        /// <returns>The blob's content-encoding value.</returns>
        const utility::string_t& content_encoding() const
        {
            return m_content_encoding;
        }

        /// <summary>
        /// Sets the content-encoding value stored for the blob.
        /// </summary>
        /// <param name="value">The blob's content-encoding value.</param>
        void set_content_encoding(utility::string_t value)
        {
            m_content_encoding = std::move(value);
        }

        /// <summary>
        /// Gets the content-language value stored for the blob.
        /// </summary>
        /// <returns>The blob's content-language value.</returns>
        const utility::string_t& content_language() const
        {
            return m_content_language;
        }

        /// <summary>
        /// Sets the content-language value stored for the blob.
        /// </summary>
        /// <param name="value">The blob's content-language value.</param>
        void set_content_language(utility::string_t value)
        {
            m_content_language = std::move(value);
        }

        /// <summary>
        /// Gets the size of the blob, in bytes.
        /// </summary>
        /// <returns>The blob's size in bytes.</returns>
        utility::size64_t size() const
        {
            return m_size;
        }

        /// <summary>
        /// Gets the content-MD5 value stored for the blob.
        /// </summary>
        /// <returns>The blob's content-MD5 hash.</returns>
        const utility::string_t& content_md5() const
        {
            return m_content_md5;
        }

        /// <summary>
        /// Sets the content-MD5 value stored for the blob.
        /// </summary>
        /// <param name="value">The blob's content-MD5 hash.</param>
        void set_content_md5(utility::string_t value)
        {
            m_content_md5 = std::move(value);
        }

        /// <summary>
        /// Gets the content-type value stored for the blob.
        /// </summary>
        /// <returns>The blob's content-type value.</returns>
        const utility::string_t& content_type() const
        {
            return m_content_type;
        }

        /// <summary>
        /// Sets the content-type value stored for the blob.
        /// </summary>
        /// <param name="value">The blob's content-type value.</param>
        void set_content_type(utility::string_t value)
        {
            m_content_type = std::move(value);
        }

        /// <summary>
        /// Gets the blob's ETag value.
        /// </summary>
        /// <returns>The blob's ETag value.</returns>
        const utility::string_t& etag() const
        {
            return m_etag;
        }

        /// <summary>
        /// Gets the last-modified time for the blob, expressed as a UTC value.
        /// </summary>
        /// <returns>The blob's last-modified time, in UTC format.</returns>
        utility::datetime last_modified() const
        {
            return m_last_modified;
        }

        /// <summary>
        /// Gets the type of the blob.
        /// </summary>
        /// <returns>An <see cref="azure::storage::blob_type" /> object that indicates the type of the blob.</returns>
        blob_type type() const
        {
            return m_type;
        }

        /// <summary>
        /// Gets the blob's lease status.
        /// </summary>
        /// <returns>An <see cref="azure::storage::lease_status" /> object that indicates the blob's lease status.</returns>
        azure::storage::lease_status lease_status() const
        {
            return m_lease_status;
        }

        /// <summary>
        /// Gets the blob's lease state.
        /// </summary>
        /// <returns>An <see cref="azure::storage::lease_state" /> object that indicates the blob's lease state.</returns>
        azure::storage::lease_state lease_state() const
        {
            return m_lease_state;
        }

        /// <summary>
        /// Gets the blob's lease duration.
        /// </summary>
        /// <returns>An <see cref="azure::storage::lease_duration" /> object that indicates the blob's lease duration.</returns>
        azure::storage::lease_duration lease_duration() const
        {
            return m_lease_duration;
        }

        /// <summary>
        /// If the blob is a page blob, gets the blob's current sequence number.
        /// </summary>
        /// <returns>The blob's current sequence number.</returns>
        int64_t page_blob_sequence_number() const
        {
            return m_page_blob_sequence_number;
        }

        /// <summary>
        /// If the blob is an append blob, gets the number of committed blocks.
        /// </summary>
        /// <returns>The number of committed blocks</returns>
        int append_blob_committed_block_count() const
        {
            return m_append_blob_committed_block_count;
        }

        /// <summary>
        /// Gets server encryption states.
        /// </summary>
        /// <returns><c>true</c> if the blob is encrypted on server side; otherwise, <c>false</c>.</returns>
        bool server_encrypted() const
        {
            return m_server_encrypted;
        }

        /// <summary>
        /// Gets a value indicating whether or not this blob is an incremental copy.
        /// </summary>
        /// <returns><c>true</c> if the blob is an incremental copy; otherwise, <c>false</c>.</returns>
        bool is_incremental_copy() const
        {
            return m_is_incremental_copy;
        }

        /// <summary>
        /// Gets a value indicating the standard blob tier if the blob is a block blob.
        /// </summary>
        /// <returns>An <see cref="azure::storage::standard_blob_tier" /> enum that indicates the blob's tier.</returns>
        azure::storage::standard_blob_tier standard_blob_tier() const
        {
            return m_standard_blob_tier;
        }

        /// <summary>
        /// Gets a value indicating the premium blob tier if the blob is a page blob.
        /// </summary>
        /// <returns>An <see cref="azure::storage::premium_blob_tier" /> enum that indicates the blob's tier.</returns>
        azure::storage::premium_blob_tier premium_blob_tier() const
        {
            return m_premium_blob_tier;
        }

        /// <summary>
        /// Gets a value indicating the archive status of the blob.
        /// </summary>
        /// <returns>An <see cref="azure::storage::archive_status" /> object that indicates the blob's archive status.</returns>
        azure::storage::archive_status archive_status() const
        {
            return m_archive_status;
        }

        /// <summary>
        /// Gets a value indicating whether or not the access tier is inferred. 
        /// </summary>
        /// <returns><c>true</c> if the access tier is not explicitly set on a page blob on premium accounts; otherwise, <c>false</c>.</returns>
        bool access_tier_inferred() const
        {
            return m_access_tier_inferred;
        }

        /// <summary>
        /// Gets the access tier change time for the blob, expressed as a UTC value.
        /// </summary>
        /// <returns>The access tier change time, in UTC format.</returns>
        utility::datetime access_tier_change_time() const
        {
            return m_access_tier_change_time;
        }

    private:

        /// <summary>
        /// Initializes an existing instance of the <see cref="azure::storage::cloud_blob_properties" /> class.
        /// </summary>
        void initialization()
        {
            m_lease_state = azure::storage::lease_state::unspecified;
            m_lease_status = azure::storage::lease_status::unspecified;
            m_lease_duration = azure::storage::lease_duration::unspecified;
        }

        void set_type(blob_type value)
        {
            m_type = value;
        }

        utility::string_t m_cache_control;
        utility::string_t m_content_disposition;
        utility::string_t m_content_encoding;
        utility::string_t m_content_language;
        utility::size64_t m_size;
        utility::string_t m_content_md5;
        utility::string_t m_content_type;
        utility::string_t m_etag;
        utility::datetime m_last_modified;
        utility::datetime m_access_tier_change_time;
        blob_type m_type;
        azure::storage::lease_status m_lease_status;
        azure::storage::lease_state m_lease_state;
        azure::storage::lease_duration m_lease_duration;
        azure::storage::standard_blob_tier m_standard_blob_tier;
        azure::storage::premium_blob_tier m_premium_blob_tier;
        azure::storage::archive_status m_archive_status;
        int64_t m_page_blob_sequence_number;
        int m_append_blob_committed_block_count;
        bool m_server_encrypted;
        bool m_is_incremental_copy;
        bool m_access_tier_inferred;

        void copy_from_root(const cloud_blob_properties& root_blob_properties);
        void update_etag_and_last_modified(const cloud_blob_properties& parsed_properties);
        void update_size(const cloud_blob_properties& parsed_properties);
        void update_page_blob_sequence_number(const cloud_blob_properties& parsed_properties);
        void update_append_blob_committed_block_count(const cloud_blob_properties& parsed_properties);
        void update_all(const cloud_blob_properties& parsed_properties);
        
        void set_server_encrypted(bool server_encrypted)
        {
            m_server_encrypted = server_encrypted;
        }

        friend class cloud_blob;
        friend class cloud_block_blob;
        friend class cloud_page_blob;
        friend class cloud_append_blob;
        friend class protocol::blob_response_parsers;
        friend class protocol::list_blobs_reader;
    };

    /// <summary>
    /// Represents the optional headers that can be returned with a blob accessed via a shared access signature.
    /// </summary>
    class cloud_blob_shared_access_headers
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_shared_access_headers" /> class.
        /// </summary>
        cloud_blob_shared_access_headers()
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_shared_access_headers" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_blob_shared_access_headers" /> object.</param>
        cloud_blob_shared_access_headers(cloud_blob_shared_access_headers&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_blob_shared_access_headers" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_blob_shared_access_headers" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::cloud_blob_shared_access_headers" /> object with properties set.</returns>
        cloud_blob_shared_access_headers& operator=(cloud_blob_shared_access_headers&& other)
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
        /// Gets the value of the cache-control header returned with the blob.
        /// </summary>
        /// <returns>The cache-control value.</returns>
        const utility::string_t& cache_control() const
        {
            return m_cache_control;
        }

        /// <summary>
        /// Sets value of the cache-control header returned with the blob.
        /// </summary>
        /// <param name="value">The cache-control value.</param>
        void set_cache_control(utility::string_t value)
        {
            m_cache_control = std::move(value);
        }

        /// <summary>
        /// Gets the value of the content-disposition header returned with the blob.
        /// </summary>
        /// <returns>The content-disposition value.</returns>
        const utility::string_t& content_disposition() const
        {
            return m_content_disposition;
        }

        /// <summary>
        /// Sets the value of the content-disposition header returned with the blob.
        /// </summary>
        /// <param name="value">The content-disposition value.</param>
        void set_content_disposition(utility::string_t value)
        {
            m_content_disposition = std::move(value);
        }

        /// <summary>
        /// Gets the value of the content-encoding header returned with the blob.
        /// </summary>
        /// <returns>The content-encoding value.</returns>
        const utility::string_t& content_encoding() const
        {
            return m_content_encoding;
        }

        /// <summary>
        /// Sets the value of the content-encoding header returned with the blob.
        /// </summary>
        /// <param name="value">The content-encoding value.</param>
        void set_content_encoding(utility::string_t value)
        {
            m_content_encoding = std::move(value);
        }

        /// <summary>
        /// Gets the value of the content-language header returned with the blob.
        /// </summary>
        /// <returns>The content-language value.</returns>
        const utility::string_t& content_language() const
        {
            return m_content_language;
        }

        /// <summary>
        /// Sets the value of the content-language header returned with the blob.
        /// </summary>
        /// <param name="value">The content-language value.</param>
        void set_content_language(utility::string_t value)
        {
            m_content_language = std::move(value);
        }

        /// <summary>
        /// Gets the value of the content-type header returned with the blob.
        /// </summary>
        /// <returns>The content-type value.</returns>
        const utility::string_t& content_type() const
        {
            return m_content_type;
        }

        /// <summary>
        /// Sets the value of the content-type header returned with the blob.
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
    /// Represents a set of timeout and retry policy options that may be specified on a request against the Blob service.
    /// </summary>
    class blob_request_options : public request_options
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::blob_request_options" /> class.
        /// </summary>
        blob_request_options()
            : request_options(),
            m_use_transactional_md5(false),
            m_store_blob_content_md5(false),
            m_disable_content_md5_validation(false),
            m_parallelism_factor(1),
            m_single_blob_upload_threshold(protocol::default_single_blob_upload_threshold),
            m_stream_write_size(protocol::default_stream_write_size),
            m_stream_read_size(protocol::default_stream_read_size),
            m_absorb_conditional_errors_on_retry(false)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::blob_request_options" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::blob_request_options" /> object.</param>
        blob_request_options(blob_request_options&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::blob_request_options" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::blob_request_options" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::blob_request_options" /> object with properties set.</returns>
        blob_request_options& operator=(blob_request_options&& other)
        {
            if (this != &other)
            {
                request_options::operator=(std::move(other));
                m_use_transactional_md5 = std::move(other.m_use_transactional_md5);
                m_store_blob_content_md5 = std::move(other.m_store_blob_content_md5);
                m_disable_content_md5_validation = std::move(other.m_disable_content_md5_validation);
                m_parallelism_factor = std::move(other.m_parallelism_factor);
                m_single_blob_upload_threshold = std::move(other.m_single_blob_upload_threshold);
                m_stream_write_size = std::move(other.m_stream_write_size);
                m_stream_read_size = std::move(other.m_stream_read_size);
                m_absorb_conditional_errors_on_retry = std::move(other.m_absorb_conditional_errors_on_retry);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Applies the default set of request options.
        /// </summary>
        /// <param name="other">A reference to a set of <see cref="azure::storage::blob_request_options" />.</param>
        /// <param name="type">The blob type, specified by <see cref="azure::storage::blob_type" />.</param>
        /// <param name="apply_expiry">Specifies that an expiry time be applied to the
        /// request options. This parameter is used internally.</param>
        void apply_defaults(const blob_request_options& other, blob_type type, bool apply_expiry = true)
        {
            request_options::apply_defaults(other, apply_expiry);

            // To match the server's Put Blob API behavior, all block blob uploads
            // should automatically set the blob's Content-MD5 property.
            if (type == blob_type::block_blob)
            {
                m_store_blob_content_md5.merge(other.m_store_blob_content_md5, true);
            }
            else
            {
                m_store_blob_content_md5.merge(other.m_store_blob_content_md5);
            }

            m_use_transactional_md5.merge(other.m_use_transactional_md5);
            m_disable_content_md5_validation.merge(other.m_disable_content_md5_validation);
            m_parallelism_factor.merge(other.m_parallelism_factor);
            m_single_blob_upload_threshold.merge(other.m_single_blob_upload_threshold);
            m_stream_write_size.merge(other.m_stream_write_size);
            m_stream_read_size.merge(other.m_stream_read_size);
            m_absorb_conditional_errors_on_retry.merge(other.m_absorb_conditional_errors_on_retry);
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
        /// Gets a value indicating whether the content-MD5 hash will be calculated and stored when uploading a blob.
        /// </summary>
        /// <returns><c>true</c> to calculate and store the content-MD5 hash when uploading a blob; otherwise, <c>false</c>.</returns>
        bool store_blob_content_md5() const
        {
            return m_store_blob_content_md5;
        }

        /// <summary>
        /// Indicates whether to calculate and store the content-MD5 hash when uploading a blob.
        /// </summary>
        /// <param name="value"><c>true</c> to calculate and store the content-MD5 hash when uploading a blob; otherwise, <c>false</c>.</param>
        void set_store_blob_content_md5(bool value)
        {
            m_store_blob_content_md5 = value;
        }

        /// <summary>
        /// Gets a value indicating whether content-MD5 validation will be disabled when downloading blobs.
        /// </summary>
        /// <returns><c>true</c> to disable content-MD5 validation; otherwise, <c>false</c>.</returns>
        bool disable_content_md5_validation() const
        {
            return m_disable_content_md5_validation;
        }

        /// <summary>
        /// Indicates whether to disable content-MD5 validation when downloading blobs.
        /// </summary>
        /// <param name="value"><c>true</c> to disable content-MD5 validation; otherwise, <c>false</c>.</param>
        void set_disable_content_md5_validation(bool value)
        {
            m_disable_content_md5_validation = value;
        }

        /// <summary>
        /// Gets the maximum size of a blob in bytes that may be uploaded as a single blob.
        /// </summary>
        /// <returns>The maximum size of a blob, in bytes, that may be uploaded as a single blob,
        /// ranging from between 1 and 256 MB inclusive.</returns>
        utility::size64_t single_blob_upload_threshold_in_bytes() const
        {
            return m_single_blob_upload_threshold;
        }

        /// <summary>
        /// Sets the maximum size of a blob in bytes that may be uploaded as a single blob.
        /// </summary>
        /// <param name="value">The maximum size of a blob, in bytes, that may be uploaded as a single blob,
        /// ranging from between 1 and 256 MB inclusive.</param>
        void set_single_blob_upload_threshold_in_bytes(utility::size64_t value)
        {
            utility::assert_in_bounds<utility::size64_t>(_XPLATSTR("value"), value, 1 * 1024 * 1024, 256 * 1024 * 1024);
            m_single_blob_upload_threshold = value;
        }

        /// <summary>
        /// Gets the number of blocks or pages that may be simultaneously uploaded or downloaded when uploading or downloading a blob that is greater than
        /// the value specified by the <see cref="single_blob_upload_threshold_in_bytes" /> property in size.
        /// </summary>
        /// <returns>The number of parallel block or page upload or download operations that may proceed.</returns>
        int parallelism_factor() const
        {
            return m_parallelism_factor;
        }

        /// <summary>
        /// Sets the number of blocks or pages that may be simultaneously uploaded or downloaded when uploading or downloading a blob that is greater than
        /// the value specified by the <see cref="single_blob_upload_threshold_in_bytes" /> property in size.
        /// </summary>
        /// <param name="value">The number of parallel block or page upload or download operations that may proceed.</param>
        void set_parallelism_factor(int value)
        {
            utility::assert_in_bounds(_XPLATSTR("value"), value, 0);
            m_parallelism_factor = value;
        }

        /// <summary>
        /// Gets the minimum number of bytes to buffer when reading from a blob stream.
        /// </summary>
        /// <returns>The minimum number of bytes to buffer, being at least 16KB.</returns>
        option_with_default<size_t> stream_read_size_in_bytes() const
        {
            return m_stream_read_size;
        }

        /// <summary>
        /// Sets the minimum number of bytes to buffer when reading from a blob stream.
        /// </summary>
        /// <param name="value">The minimum number of bytes to buffer, being at least 16KB.</param>
        void set_stream_read_size_in_bytes(size_t value)
        {
            utility::assert_in_bounds<size_t>(_XPLATSTR("value"), value, 16 * 1024);
            m_stream_read_size = value;
        }

        /// <summary>
        /// Gets the minimum number of bytes to buffer when writing to a blob stream.
        /// </summary>
        /// <returns>The minimum number of bytes to buffer, ranging from between 16 KB and 100 MB inclusive.</returns>
        option_with_default<size_t> stream_write_size_in_bytes() const
        {
            return m_stream_write_size;
        }

        /// <summary>
        /// Sets the minimum number of bytes to buffer when writing to a blob stream.
        /// </summary>
        /// <param name="value">The minimum number of bytes to buffer, ranging from between 16 KB and 100 MB inclusive.</param>
        void set_stream_write_size_in_bytes(size_t value)
        {
            utility::assert_in_bounds<size_t>(_XPLATSTR("value"), value, 16 * 1024, 100 * 1024 * 1024);
            m_stream_write_size = value;
        }

        /// <summary>
        /// Gets the value that indicates whether a conditional failure should be absorbed on a retry attempt
        /// for the request. This option is only used by <see cref="cloud_append_blob"/> in upload_from methods and
        /// <see cref="core::cloud_append_blob_ostreambuf"/>. By default, it is set to <c>false</c>. Set this to <c>true</c> only for single writer scenario.
        /// Setting this to <c>true</c> in a multi writer scenario could lead to a corrupted blob on the service.
        /// </summary>
        /// <returns><c>true</c> to absorb a conditional failure on a retry attempt for the request; otherwise, <c>false</c>.</returns>
        bool absorb_conditional_errors_on_retry() const
        {
            return m_absorb_conditional_errors_on_retry;
        }

        /// <summary>
        /// Sets the value that indicates whether a conditional failure should be absorbed on a retry attempt
        /// for the request. This option is only used by <see cref="cloud_append_blob"/> in upload_from methods and
        /// <see cref="core::cloud_append_blob_ostreambuf"/>. By default, it is set to <c>false</c>. Set this to <c>true</c> only for single writer scenario.
        /// Setting this to <c>true</c> in a multi writer scenario could lead to a corrupted blob on the service.
        /// </summary>
        /// <param name="value"><c>true</c> to absorb a conditional failure on a retry attempt for the request; otherwise, <c>false</c>.</param>
        void set_absorb_conditional_errors_on_retry(bool value)
        {
            m_absorb_conditional_errors_on_retry = value;
        }

    private:

        option_with_default<bool> m_use_transactional_md5;
        option_with_default<bool> m_store_blob_content_md5;
        option_with_default<bool> m_disable_content_md5_validation;
        option_with_default<int> m_parallelism_factor;
        option_with_default<utility::size64_t> m_single_blob_upload_threshold;
        option_with_default<size_t> m_stream_write_size;
        option_with_default<size_t> m_stream_read_size;
        option_with_default<bool> m_absorb_conditional_errors_on_retry;
    };

    /// <summary>
    /// Represents a block in a block list.
    /// </summary>
    class block_list_item
    {
    public:
        /// <summary>
        /// Indicates which block lists should be searched to find a specified block. 
        /// </summary>
        enum block_mode
        {
            /// <summary>
            /// Search the committed block list only.
            /// </summary>
            committed,

            /// <summary>
            /// Search the uncommitted block list only.
            /// </summary>
            uncommitted,

            /// <summary>
            /// Search the uncommitted block list first, and if the block is not found there, search 
            /// the committed block list.
            /// </summary>
            latest
        };

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::block_list_item" /> class.
        /// </summary>
        /// <param name="id">The block name.</param>
        block_list_item(utility::string_t id)
            : m_id(std::move(id)), m_size(std::numeric_limits<size_t>::max()), m_mode(block_mode::latest)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::block_list_item" /> class.
        /// </summary>
        /// <param name="id">The block name.</param>
        /// <param name="mode">An <see cref="azure::storage::block_list_item::block_mode" /> value that indicates which block lists to search for the block.</param>
        block_list_item(utility::string_t id, block_mode mode)
            : m_id(std::move(id)), m_size(std::numeric_limits<size_t>::max()), m_mode(mode)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::block_list_item" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::block_list_item" /> object.</param>
        block_list_item(block_list_item&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::block_list_item" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::block_list_item" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::block_list_item" /> object with properties set.</returns>
        block_list_item& operator=(block_list_item&& other)
        {
            if (this != &other)
            {
                m_id = std::move(other.m_id);
                m_size = std::move(other.m_size);
                m_mode = std::move(other.m_mode);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets the name of the block.
        /// </summary>
        /// <returns>The block name.</returns>
        const utility::string_t& id() const
        {
            return m_id;
        }

        /// <summary>
        /// Gets the size of block in bytes.
        /// </summary>
        /// <returns>The block size.</returns>
        size_t size() const
        {
            return m_size;
        }

        /// <summary>
        /// Gets a value indicating whether the block has been committed.
        /// </summary>
        /// <returns>An <see cref="azure::storage::block_list_item::block_mode" /> value that indicates whether the block has been committed.</returns>
        block_mode mode() const
        {
            return m_mode;
        }

    private:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::block_list_item" /> class.
        /// </summary>
        /// <param name="id">The block name.</param>
        /// <param name="size">The size of the block.</param>
        /// <param name="committed"><c>true</c> indicates that the block has been committed; 
        /// <c>false</c> indicates that it is uncommitted.</param>
        block_list_item(utility::string_t id, size_t size, bool committed)
            : m_id(std::move(id)), m_size(size), m_mode(committed ? block_mode::committed : block_mode::uncommitted)
        {
        }

        utility::string_t m_id;
        size_t m_size;
        block_mode m_mode;

        friend class protocol::block_list_reader;
    };

    /// <summary>
    /// Represents a range of pages in a page blob.
    /// </summary>
    class page_range
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::page_range" /> class.
        /// </summary>
        /// <param name="start_offset">The starting offset.</param>
        /// <param name="end_offset">The ending offset.</param>
        page_range(int64_t start_offset, int64_t end_offset)
            : m_start_offset(start_offset), m_end_offset(end_offset)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::page_range" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::page_range" /> object.</param>
        page_range(page_range&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::page_range" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::page_range" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::page_range" /> object with properties set.</returns>
        page_range& operator=(page_range&& other)
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
        /// Returns the content of the page range as a string.
        /// </summary>
        /// <returns>The content of the page range.</returns>
        utility::string_t to_string() const
        {
            utility::ostringstream_t value;
            value << protocol::header_value_range_prefix << m_start_offset << _XPLATSTR('-') << m_end_offset;
            return value.str();
        }

        /// <summary>
        /// Gets the starting offset of the page range.
        /// </summary>
        /// <returns>The starting offset.</returns>
        int64_t start_offset() const
        {
            return m_start_offset;
        }

        /// <summary>
        /// Gets the ending offset of the page range.
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

    class page_diff_range : public page_range
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::page_diff_range" /> class.
        /// </summary>
        /// <param name="start_offset">The starting offset.</param>
        /// <param name="end_offset">The ending offset.</param>
        /// <param name="is_cleared"><c>true</c> if the range was cleared range; otherwise <c>false</c>. It was set as non-cleared range at default.</param>
        page_diff_range(int64_t start_offset, int64_t end_offset, bool is_cleared = false)
            : page_range(start_offset, end_offset),
            m_cleared_range(is_cleared)
        {
        }

        /// <summary>
        /// Checks the range type as cleared range or not.
        /// </summary>
        /// <returns><c>true</c> if the range was cleared range; otherwise <c>false</c>.</returns>
        bool is_cleared_rage() const
        {
            return m_cleared_range;
        }

    private:
        bool m_cleared_range;
    };

    /// <summary>
    /// Describes actions that can be performed on a page blob sequence number.
    /// </summary>
    class sequence_number
    {
    public:

        enum class sequence_number_action
        {
            /// <summary>
            /// Sets the sequence number to be the higher of two values: the value included on the request, or the value 
            /// currently stored for the blob.
            /// </summary>
            maximum,

            /// <summary>
            /// Sets the sequence number to the value included with the request.
            /// </summary>
            update,

            /// <summary>
            /// Increments the value of the sequence number by 1.
            /// </summary>
            increment
        };

        /// <summary>
        /// Constructs a sequence number action to set the higher of two values: the value included on the request, 
        /// or the value currently stored for the blob.
        /// </summary>
        /// <param name="value">The sequence number.</param>
        /// <returns>An <see cref="azure::storage::sequence_number" /> object that represents the action.</returns>
        static sequence_number maximum(int64_t value)
        {
            return sequence_number(sequence_number_action::maximum, value);
        }

        /// <summary>
        /// Constructs a sequence number action to set the sequence number to the value included with the request.
        /// </summary>
        /// <param name="value">The sequence number.</param>
        /// <returns>An <see cref="azure::storage::sequence_number" /> object that represents the action.</returns>
        static sequence_number update(int64_t value)
        {
            return sequence_number(sequence_number_action::update, value);
        }

        /// <summary>
        /// Constructs a sequence number action to increment the value of the sequence number by 1.
        /// </summary>
        /// <returns>An <see cref="azure::storage::sequence_number" /> object that represents the action.</returns>
        static sequence_number increment()
        {
            return sequence_number(sequence_number_action::increment, -1);
        }

        /// <summary>
        /// Gets the action that will be performed on a page blob sequence number.
        /// </summary>
        /// <returns>The sequence number action.</returns>
        sequence_number_action action() const
        {
            return m_action;
        }

        /// <summary>
        /// Gets the page blob's sequence number.
        /// </summary>
        /// <returns>The sequence number.</returns>
        int64_t value() const
        {
            return m_value;
        }

    private:

        sequence_number(sequence_number_action action, int64_t value)
            : m_action(action), m_value(value)
        {
        }

        sequence_number_action m_action;
        int64_t m_value;
    };

    /// <summary>
    /// Represents the system properties for a container.
    /// </summary>
    class cloud_blob_container_properties
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_container_properties" /> class.
        /// </summary>
        cloud_blob_container_properties()
            : m_lease_status(azure::storage::lease_status::unspecified), m_lease_state(azure::storage::lease_state::unspecified),
            m_lease_duration(azure::storage::lease_duration::unspecified),
            m_public_access(azure::storage::blob_container_public_access_type::off)
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_container_properties" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_blob_container_properties" /> object.</param>
        cloud_blob_container_properties(cloud_blob_container_properties&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_blob_container_properties" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_blob_container_properties" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::cloud_blob_container_properties" /> object with properties set.</returns>
        cloud_blob_container_properties& operator=(cloud_blob_container_properties&& other)
        {
            if (this != &other)
            {
                m_etag = std::move(other.m_etag);
                m_last_modified = std::move(other.m_last_modified);
                m_lease_status = std::move(other.m_lease_status);
                m_lease_state = std::move(other.m_lease_state);
                m_lease_duration = std::move(other.m_lease_duration);
                m_public_access = std::move(other.m_public_access);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets the ETag value for the container.
        /// </summary>
        /// <returns>The container's quoted ETag value.</returns>
        const utility::string_t& etag() const
        {
            return m_etag;
        }

        /// <summary>
        /// Gets the container's last-modified time.
        /// </summary>
        /// <returns>The container's last-modified time.</returns>
        utility::datetime last_modified() const
        {
            return m_last_modified;
        }

        /// <summary>
        /// Gets the container's lease status.
        /// </summary>
        /// <returns>An <see cref="azure::storage::lease_status" /> object that indicates the container's lease status.</returns>
        azure::storage::lease_status lease_status() const
        {
            return m_lease_status;
        }

        /// <summary>
        /// Gets the container's lease state.
        /// </summary>
        /// <returns>An <see cref="azure::storage::lease_state" /> object that indicates the container's lease state.</returns>
        azure::storage::lease_state lease_state() const
        {
            return m_lease_state;
        }

        /// <summary>
        /// Gets the container's lease duration.
        /// </summary>
        /// <returns>An <see cref="azure::storage::lease_duration" /> object that indicates the container's lease duration.</returns>
        azure::storage::lease_duration lease_duration() const
        {
            return m_lease_duration;
        }

        /// <summary>
        /// Gets the public access setting for the container.
        /// </summary>
        /// <returns>The public access setting for the container.</returns>
        azure::storage::blob_container_public_access_type public_access() const
        {
            return m_public_access;
        }

    private:

        /// <summary>
        /// Initializes an existing instance of the <see cref="azure::storage::cloud_blob_container_properties" /> class.
        /// </summary>
        void initialization()
        {
            m_lease_state = azure::storage::lease_state::unspecified;
            m_lease_status = azure::storage::lease_status::unspecified;
            m_lease_duration = azure::storage::lease_duration::unspecified;
        }

        utility::string_t m_etag;
        utility::datetime m_last_modified;
        azure::storage::lease_status m_lease_status;
        azure::storage::lease_state m_lease_state;
        azure::storage::lease_duration m_lease_duration;
        azure::storage::blob_container_public_access_type m_public_access;

        void update_etag_and_last_modified(const cloud_blob_container_properties& parsed_properties);

        friend class cloud_blob_container;
        friend class protocol::blob_response_parsers;
        friend class protocol::list_containers_reader;
    };

    /// <summary>
    /// Provides a client-side logical representation of the Windows Azure Blob Service. This client is used to configure and execute requests against the Blob Service.
    /// </summary>
    /// <remarks>The service client encapsulates the base URI for the Blob service. If the service client will be used for authenticated access, it also encapsulates the credentials for accessing the storage account.</remarks>
    class cloud_blob_client : public cloud_client
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_client" /> class.
        /// </summary>
        cloud_blob_client()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_client" /> class using the specified Blob service endpoint
        /// and anonymous credentials.
        /// </summary>
        /// <param name="base_uri">An <see cref="azure::storage::storage_uri" /> object containing the Blob service endpoint for all locations.</param>
        explicit cloud_blob_client(storage_uri base_uri)
            : cloud_client(std::move(base_uri))
        {
            initialize();
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_client" /> class using the specified Blob service endpoint
        /// and account credentials.
        /// </summary>
        /// <param name="base_uri">An <see cref="azure::storage::storage_uri" /> object containing the Blob service endpoint for all locations.</param>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        cloud_blob_client(storage_uri base_uri, storage_credentials credentials)
            : cloud_client(std::move(base_uri), std::move(credentials))
        {
            initialize();
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_client" /> class using the specified Blob service endpoint
        /// and account credentials.
        /// </summary>
        /// <param name="base_uri">An <see cref="azure::storage::storage_uri" /> object containing the Blob service endpoint for all locations.</param>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        /// <param name="default_request_options">The default <see cref="azure::storage::blob_request_options" /> object to use for all requests made with this client object.</param>
        cloud_blob_client(storage_uri base_uri, storage_credentials credentials, blob_request_options default_request_options)
            : cloud_client(std::move(base_uri), std::move(credentials)), m_default_request_options(std::move(default_request_options))
        {
            initialize();
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_client" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_blob_client" /> object.</param>
        cloud_blob_client(cloud_blob_client&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_blob_client" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_blob_client" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::cloud_blob_client" /> object with properties set.</returns>
        cloud_blob_client& operator=(cloud_blob_client&& other)
        {
            if (this != &other)
            {
                cloud_client::operator=(std::move(other));
                m_default_request_options = std::move(other.m_default_request_options);
                m_directory_delimiter = std::move(other.m_directory_delimiter);
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
        /// Returns an <see cref="azure::storage::container_result_iterator" /> that can be used to to lazily enumerate a collection of containers.
        /// </summary>
        /// <returns>An <see cref="azure::storage::container_result_iterator" /> that can be used to to lazily enumerate a collection of containers.</returns>
        container_result_iterator list_containers() const
        {
            return list_containers(utility::string_t(), container_listing_details::none, 0, blob_request_options(), operation_context());
        }

        /// <summary>
        /// Returns an <see cref="azure::storage::container_result_iterator" /> that can be used to to lazily enumerate a collection of containers.
        /// </summary>
        /// <param name="prefix">The container name prefix.</param>
        /// <returns>An <see cref="azure::storage::container_result_iterator" /> that can be used to to lazily enumerate a collection of containers.</returns>
        container_result_iterator list_containers(const utility::string_t& prefix) const
        {
            return list_containers(prefix, container_listing_details::none, 0, blob_request_options(), operation_context());
        }

        /// <summary>
        /// Returns an <see cref="azure::storage::container_result_iterator" /> that can be used to to lazily enumerate a collection of containers.
        /// </summary>
        /// <param name="prefix">The container name prefix.</param>
        /// <param name="includes">An <see cref="azure::storage::container_listing_details::values" /> enumeration describing which items to include in the listing.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned.
        /// If this value is zero, the maximum possible number of results will be returned.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An <see cref="azure::storage::container_result_iterator" /> that can be used to to lazily enumerate a collection of containers.</returns>
        WASTORAGE_API container_result_iterator list_containers(const utility::string_t& prefix, container_listing_details::values includes, int max_results, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Returns a result segment containing a collection of <see cref="azure::storage::cloud_blob_container" /> objects.
        /// </summary>
        /// <param name="token">An <see cref="azure::storage::continuation_token" /> returned by a previous listing operation.</param>
        /// <returns>An <see cref="azure::storage::container_result_segment" /> containing a collection of containers.</returns>
        container_result_segment list_containers_segmented(const continuation_token& token) const
        {
            return list_containers_segmented_async(token).get();
        }

        /// <summary>
        /// Returns a result segment containing a collection of <see cref="azure::storage::cloud_blob_container" /> objects.
        /// </summary>
        /// <param name="prefix">The container name prefix.</param>
        /// <param name="token">An <see cref="azure::storage::continuation_token" /> returned by a previous listing operation.</param>
        /// <returns>An <see cref="azure::storage::container_result_segment" /> containing a collection of containers.</returns>
        container_result_segment list_containers_segmented(const utility::string_t& prefix, const continuation_token& token) const
        {
            return list_containers_segmented_async(prefix, token).get();
        }

        /// <summary>
        /// Returns a result segment containing a collection of <see cref="azure::storage::cloud_blob_container" /> objects.
        /// </summary>
        /// <param name="prefix">The container name prefix.</param>
        /// <param name="includes">An <see cref="azure::storage::container_listing_details::values" /> enumeration describing which items to include in the listing.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned 
        /// in the result segment, up to the per-operation limit of 5000. If this value is 0, the maximum possible number of results will be returned, up to 5000.</param>
        /// <param name="token">An <see cref="azure::storage::continuation_token" /> returned by a previous listing operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An <see cref="azure::storage::container_result_segment" /> containing a collection of containers.</returns>
        container_result_segment list_containers_segmented(const utility::string_t& prefix, container_listing_details::values includes, int max_results, const continuation_token& token, const blob_request_options& options, operation_context context) const
        {
            return list_containers_segmented_async(prefix, includes, max_results, token, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to return a result segment containing a collection of <see cref="azure::storage::cloud_blob_container" /> objects.
        /// </summary>
        /// <param name="token">An <see cref="azure::storage::continuation_token" /> returned by a previous listing operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::container_result_segment" /> that represents the current operation.</returns>
        pplx::task<container_result_segment> list_containers_segmented_async(const continuation_token& token) const
        {
            return list_containers_segmented_async(utility::string_t(), token);
        }

        /// <summary>
        /// Initiates an asynchronous operation to return a result segment containing a collection of <see cref="azure::storage::cloud_blob_container" /> objects.
        /// </summary>
        /// <param name="prefix">The container name prefix.</param>
        /// <param name="token">An <see cref="azure::storage::continuation_token" /> returned by a previous listing operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::container_result_segment" /> that represents the current operation.</returns>
        pplx::task<container_result_segment> list_containers_segmented_async(const utility::string_t& prefix, const continuation_token& token) const
        {
            return list_containers_segmented_async(prefix, container_listing_details::none, 0, token, blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to return a result segment containing a collection of <see cref="azure::storage::cloud_blob_container" /> objects.
        /// </summary>
        /// <param name="prefix">The container name prefix.</param>
        /// <param name="includes">An <see cref="azure::storage::container_listing_details::values" /> enumeration describing which items to include in the listing.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned 
        /// in the result segment, up to the per-operation limit of 5000. If this value is 0, the maximum possible number of results will be returned, up to 5000.</param>
        /// <param name="token">An <see cref="azure::storage::continuation_token" /> returned by a previous listing operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::container_result_segment" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<container_result_segment> list_containers_segmented_async(const utility::string_t& prefix, container_listing_details::values includes, int max_results, const continuation_token& token, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Returns an <see cref="azure::storage::list_blob_item_iterator" /> that can be used to to lazily enumerate a collection of blob items.
        /// </summary>
        /// <param name="prefix">The blob name prefix.</param>
        /// <returns>An <see cref="azure::storage::list_blob_item_iterator" /> that can be used to to lazily enumerate a collection of blob items.</returns>
        list_blob_item_iterator list_blobs(const utility::string_t& prefix) const
        {
            return list_blobs(prefix, false, blob_listing_details::none, 0, blob_request_options(), operation_context());
        }

        /// <summary>
        /// Returns an <see cref="azure::storage::list_blob_item_iterator" /> that can be used to to lazily enumerate a collection of blob items.
        /// </summary>
        /// <param name="prefix">The blob name prefix.</param>
        /// <param name="use_flat_blob_listing">Indicates whether to list blobs in a flat listing, or whether to list blobs hierarchically, by virtual directory.</param>
        /// <param name="includes">An <see cref="azure::storage::blob_listing_details::values" /> enumeration describing which items to include in the listing.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned.
        /// If this value is zero, the maximum possible number of results will be returned.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An <see cref="azure::storage::list_blob_item_iterator" /> that can be used to to lazily enumerate a collection of blob items.</returns>
        WASTORAGE_API list_blob_item_iterator list_blobs(const utility::string_t& prefix, bool use_flat_blob_listing, blob_listing_details::values includes, int max_results, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Returns an <see cref="azure::storage::list_blob_item_segment" /> containing a collection of blob items in the container.
        /// </summary>
        /// <param name="prefix">The blob name prefix.</param>
        /// <param name="token">An <see cref="azure::storage::continuation_token" /> returned by a previous listing operation.</param>
        /// <returns>An <see cref="azure::storage::list_blob_item_segment" /> containing a collection of blob items in the container.</returns>
        list_blob_item_segment list_blobs_segmented(const utility::string_t& prefix, const continuation_token& token) const
        {
            return list_blobs_segmented_async(prefix, token).get();
        }

        /// <summary>
        /// Returns an <see cref="azure::storage::list_blob_item_segment" /> containing a collection of blob items in the container.
        /// </summary>
        /// <param name="prefix">The blob name prefix.</param>
        /// <param name="use_flat_blob_listing">Indicates whether to list blobs in a flat listing, or whether to list blobs hierarchically, by virtual directory.</param>
        /// <param name="includes">An <see cref="azure::storage::blob_listing_details::values" /> enumeration describing which items to include in the listing.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned at a time, up to the 
        /// per-operation limit of 5000. If this value is 0, the maximum possible number of results will be returned, up to 5000.</param>
        /// <param name="token">An <see cref="azure::storage::continuation_token" /> returned by a previous listing operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An <see cref="azure::storage::list_blob_item_segment" /> containing a collection of blob items in the container.</returns>
        list_blob_item_segment list_blobs_segmented(const utility::string_t& prefix, bool use_flat_blob_listing, blob_listing_details::values includes, int max_results, const continuation_token& token, const blob_request_options& options, operation_context context) const
        {
            return list_blobs_segmented_async(prefix, use_flat_blob_listing, includes, max_results, token, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to return an <see cref="azure::storage::list_blob_item_segment" /> containing a collection of blob items in the container.
        /// </summary>
        /// <param name="prefix">The blob name prefix.</param>
        /// <param name="token">An <see cref="azure::storage::continuation_token" /> returned by a previous listing operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::list_blob_item_segment" /> that represents the current operation.</returns>
        pplx::task<list_blob_item_segment> list_blobs_segmented_async(const utility::string_t& prefix, const continuation_token& token) const
        {
            return list_blobs_segmented_async(prefix, false, blob_listing_details::none, 0, token, blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to return an <see cref="azure::storage::list_blob_item_segment" /> containing a collection of blob items in the container.
        /// </summary>
        /// <param name="prefix">The blob name prefix.</param>
        /// <param name="use_flat_blob_listing">Indicates whether to list blobs in a flat listing, or whether to list blobs hierarchically, by virtual directory.</param>
        /// <param name="includes">An <see cref="azure::storage::blob_listing_details::values" /> enumeration describing which items to include in the listing.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned at a time, up to the 
        /// per-operation limit of 5000. If this value is 0, the maximum possible number of results will be returned, up to 5000.</param>
        /// <param name="token">An <see cref="azure::storage::continuation_token" /> returned by a previous listing operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::list_blob_item_segment" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<list_blob_item_segment> list_blobs_segmented_async(const utility::string_t& prefix, bool use_flat_blob_listing, blob_listing_details::values includes, int max_results, const continuation_token& token, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Gets the service properties for the Blob service client.
        /// </summary>
        /// <returns>The <see cref="azure::storage::service_properties" /> for the Blob service client.</returns>
        service_properties download_service_properties() const
        {
            return download_service_properties_async().get();
        }

        /// <summary>
        /// Gets the service properties for the Blob service client.
        /// </summary>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The <see cref="azure::storage::service_properties" /> for the Blob service client.</returns>
        service_properties download_service_properties(const blob_request_options& options, operation_context context) const
        {
            return download_service_properties_async(options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to get the properties of the service.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::service_properties" /> that represents the current operation.</returns>
        pplx::task<service_properties> download_service_properties_async() const
        {
            return download_service_properties_async(blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to get the properties of the service.
        /// </summary>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::service_properties" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<service_properties> download_service_properties_async(const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Sets the service properties for the Blob service client.
        /// </summary>
        /// <param name="properties">The <see cref="azure::storage::service_properties" /> for the Blob service client.</param>
        /// <param name="includes">An <see cref="azure::storage::service_properties_includes" /> enumeration describing which items to include when setting service properties.</param>
        void upload_service_properties(const service_properties& properties, const service_properties_includes& includes) const
        {
            upload_service_properties_async(properties, includes).wait();
        }

        /// <summary>
        /// Sets the service properties for the Blob service client.
        /// </summary>
        /// <param name="properties">The <see cref="azure::storage::service_properties" /> for the Blob service client.</param>
        /// <param name="includes">An <see cref="azure::storage::service_properties_includes" /> enumeration describing which items to include when setting service properties.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_service_properties(const service_properties& properties, const service_properties_includes& includes, const blob_request_options& options, operation_context context) const
        {
            upload_service_properties_async(properties, includes, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to set the service properties for the Blob service client.
        /// </summary>
        /// <param name="properties">The <see cref="azure::storage::service_properties" /> for the Blob service client.</param>
        /// <param name="includes">An <see cref="azure::storage::service_properties_includes" /> enumeration describing which items to include when setting service properties.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_service_properties_async(const service_properties& properties, const service_properties_includes& includes) const
        {
            return upload_service_properties_async(properties, includes, blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to set the service properties for the Blob service client.
        /// </summary>
        /// <param name="properties">The <see cref="azure::storage::service_properties" /> for the Blob service client.</param>
        /// <param name="includes">An <see cref="azure::storage::service_properties_includes" /> enumeration describing which items to include when setting service properties.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_service_properties_async(const service_properties& properties, const service_properties_includes& includes, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Gets the service stats for the Blob service client.
        /// </summary>
        /// <returns>The <see cref="azure::storage::service_stats" /> for the Blob service client.</returns>
        service_stats download_service_stats() const
        {
            return download_service_stats_async().get();
        }

        /// <summary>
        /// Gets the service stats for the Blob service client.
        /// </summary>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The <see cref="azure::storage::service_stats" /> for the Blob service client.</returns>
        service_stats download_service_stats(const blob_request_options& options, operation_context context) const
        {
            return download_service_stats_async(options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to get the stats of the service.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::service_stats" /> that represents the current operation.</returns>
        pplx::task<service_stats> download_service_stats_async() const
        {
            return download_service_stats_async(blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to get the stats of the service.
        /// </summary>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::service_stats" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<service_stats> download_service_stats_async(const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_blob_container" /> object.
        /// </summary>
        /// <returns>A reference to the root container, of type <see cref="azure::storage::cloud_blob_container" />.</returns>
        WASTORAGE_API cloud_blob_container get_root_container_reference() const;

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_blob_container" /> object with the specified name.
        /// </summary>
        /// <param name="container_name">The name of the container, or an absolute URI to the container.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_blob_container" />.</returns>
        WASTORAGE_API cloud_blob_container get_container_reference(utility::string_t container_name) const;

        /// <summary>
        /// Returns the default set of request options.
        /// </summary>
        /// <returns>An <see cref="azure::storage::blob_request_options" /> object.</returns>
        const blob_request_options& default_request_options() const
        {
            return m_default_request_options;
        }
        
        /// <summary>
        /// Gets the default delimiter that may be used to create a virtual directory structure of blobs.
        /// </summary>
        /// <returns>A string containing the default delimiter.</returns>
        const utility::string_t& directory_delimiter() const
        {
            return m_directory_delimiter;
        }

        /// <summary>
        /// Sets the default delimiter that may be used to create a virtual directory structure of blobs.
        /// </summary>
        /// <param name="value">A string containing the default delimiter.</param>
        void set_directory_delimiter(utility::string_t value)
        {
            if (value.empty())
            {
                throw std::invalid_argument("value");
            }

            m_directory_delimiter = std::move(value);
        }

    private:

        void initialize()
        {
            set_authentication_scheme(azure::storage::authentication_scheme::shared_key);
            if (!m_default_request_options.retry_policy().is_valid())
                m_default_request_options.set_retry_policy(exponential_retry_policy());
            m_directory_delimiter = protocol::directory_delimiter;
        }

        static void parse_blob_name_prefix(const utility::string_t& prefix, utility::string_t& container_name, utility::string_t& actual_prefix);

        blob_request_options m_default_request_options;
        utility::string_t m_directory_delimiter;
    };

    /// <summary>
    /// Represents a container in the Windows Azure Blob service.
    /// </summary>
    class cloud_blob_container
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_container" /> class.
        /// </summary>
        cloud_blob_container() {}

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_container" /> class.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the container for all locations.</param>
        WASTORAGE_API cloud_blob_container(storage_uri uri);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_container" /> class.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the container for all locations.</param>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        WASTORAGE_API cloud_blob_container(storage_uri uri, storage_credentials credentials);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_container" /> class.
        /// </summary>
        /// <param name="name">The name of the container.</param>
        /// <param name="client">The Blob service client.</param>
        WASTORAGE_API cloud_blob_container(utility::string_t name, cloud_blob_client client);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_container" /> class.
        /// </summary>
        /// <param name="name">The container name.</param>
        /// <param name="client">The Blob service client.</param>
        /// <param name="properties">The properties for the container.</param>
        /// <param name="metadata">The metadata for the container.</param>
        WASTORAGE_API cloud_blob_container(utility::string_t name, cloud_blob_client client, cloud_blob_container_properties properties, cloud_metadata metadata);

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_container" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_blob_container" /> object.</param>
        cloud_blob_container(cloud_blob_container&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_blob_container" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_blob_container" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::cloud_blob_container" /> object with properties set.</returns>
        cloud_blob_container& operator=(cloud_blob_container&& other)
        {
            if (this != &other)
            {
                m_client = std::move(other.m_client);
                m_name = std::move(other.m_name);
                m_uri = std::move(other.m_uri);
                m_metadata = std::move(other.m_metadata);
                m_properties = std::move(other.m_properties);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Returns a shared access signature for the container.
        /// </summary>
        /// <param name="policy">The access policy for the shared access signature.</param>
        /// <returns>A string containing a shared access signature.</returns>
        utility::string_t get_shared_access_signature(const blob_shared_access_policy& policy) const
        {
            return get_shared_access_signature(policy, utility::string_t());
        }

        /// <summary>
        /// Returns a shared access signature for the container.
        /// </summary>
        /// <param name="policy">The access policy for the shared access signature.</param>
        /// <param name="stored_policy_identifier">A container-level access policy.</param>
        /// <returns>A string containing a shared access signature.</returns>
        WASTORAGE_API utility::string_t get_shared_access_signature(const blob_shared_access_policy& policy, const utility::string_t& stored_policy_identifier) const;

        /// <summary>
        /// Gets a reference to a blob in this container.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_blob" />.</returns>
        WASTORAGE_API cloud_blob get_blob_reference(utility::string_t blob_name) const;

        /// <summary>
        /// Gets a reference to a blob in this container.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_blob" />.</returns>
        WASTORAGE_API cloud_blob get_blob_reference(utility::string_t blob_name, utility::string_t snapshot_time) const;

        /// <summary>
        /// Gets a reference to a page blob in this container.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_page_blob" />.</returns>
        WASTORAGE_API cloud_page_blob get_page_blob_reference(utility::string_t blob_name) const;

        /// <summary>
        /// Gets a reference to a page blob in this container.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_page_blob" />.</returns>
        WASTORAGE_API cloud_page_blob get_page_blob_reference(utility::string_t blob_name, utility::string_t snapshot_time) const;

        /// <summary>
        /// Gets a reference to a block blob in this container.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_block_blob" />.</returns>
        WASTORAGE_API cloud_block_blob get_block_blob_reference(utility::string_t blob_name) const;

        /// <summary>
        /// Gets a reference to a block blob in this container.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_block_blob" />.</returns>
        WASTORAGE_API cloud_block_blob get_block_blob_reference(utility::string_t blob_name, utility::string_t snapshot_time) const;

        /// <summary>
        /// Gets a reference to an append blob in this container.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <returns>A reference to a <see cref="cloud_append_blob" />.</returns>
        WASTORAGE_API cloud_append_blob get_append_blob_reference(utility::string_t blob_name) const;

        /// <summary>
        /// Gets a reference to an append blob in this container.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <returns>A reference to a <see cref="cloud_append_blob" />.</returns>
        WASTORAGE_API cloud_append_blob get_append_blob_reference(utility::string_t blob_name, utility::string_t snapshot_time) const;

        /// <summary>
        /// Gets a reference to a virtual blob directory beneath this container.
        /// </summary>
        /// <param name="directory_name">The name of the virtual blob directory.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_blob_directory" />.</returns>
        WASTORAGE_API cloud_blob_directory get_directory_reference(utility::string_t directory_name) const;

        /// <summary>
        /// Retrieves the container's attributes.
        /// </summary>
        void download_attributes()
        {
            download_attributes_async().wait();
        }

        /// <summary>
        /// Retrieves the container's attributes.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void download_attributes(const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            download_attributes_async(condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to retrieve the container's attributes.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> download_attributes_async()
        {
            return download_attributes_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to retrieve the container's attributes.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> download_attributes_async(const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Sets the container's user-defined metadata.
        /// </summary>
        void upload_metadata()
        {
            upload_metadata_async().wait();
        }

        /// <summary>
        /// Sets the container's user-defined metadata.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_metadata(const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_metadata_async(condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to set the container's user-defined metadata.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_metadata_async()
        {
            return upload_metadata_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to set the container's user-defined metadata.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_metadata_async(const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Acquires a lease on the container.
        /// </summary>
        /// <param name="duration">An <see cref="azure::storage::lease_time" /> representing the span of time for which to acquire the lease.</param>
        /// <param name="proposed_lease_id">A string representing the proposed lease ID for the new lease. May be an empty string if no lease ID is proposed.</param>
        /// <returns>A string containing the ID of the acquired lease.</returns>
        utility::string_t acquire_lease(const azure::storage::lease_time& duration, const utility::string_t& proposed_lease_id) const
        {
            return acquire_lease_async(duration, proposed_lease_id).get();
        }

        /// <summary>
        /// Acquires a lease on the container.
        /// </summary>
        /// <param name="duration">An <see cref="azure::storage::lease_time" /> representing the span of time for which to acquire the lease.</param>
        /// <param name="proposed_lease_id">A string representing the proposed lease ID for the new lease. May be an empty string if no lease ID is proposed.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A string containing the ID of the acquired lease.</returns>
        utility::string_t acquire_lease(const azure::storage::lease_time& duration, const utility::string_t& proposed_lease_id, const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            return acquire_lease_async(duration, proposed_lease_id, condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to acquire a lease on the container.
        /// </summary>
        /// <param name="duration">An <see cref="azure::storage::lease_time" /> object representing the span of time for which to acquire the lease.</param>
        /// <param name="proposed_lease_id">A string representing the proposed lease ID for the new lease. May be an empty string if no lease ID is proposed..</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        pplx::task<utility::string_t> acquire_lease_async(const azure::storage::lease_time& duration, const utility::string_t& proposed_lease_id) const
        {
            return acquire_lease_async(duration, proposed_lease_id, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to acquire a lease on the container.
        /// </summary>
        /// <param name="duration">An <see cref="azure::storage::lease_time" /> representing the span of time for which to acquire the lease.</param>
        /// <param name="proposed_lease_id">A string representing the proposed lease ID for the new lease. May be an empty string if no lease ID is proposed.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<utility::string_t> acquire_lease_async(const azure::storage::lease_time& duration, const utility::string_t& proposed_lease_id, const access_condition& condition, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Renews a lease on the container.
        /// </summary>
        void renew_lease() const
        {
            renew_lease_async().wait();
        }

        /// <summary>
        /// Renews a lease on the container.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation, including a required lease ID.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void renew_lease(const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            renew_lease_async(condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to renew a lease on the container.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> renew_lease_async() const
        {
            return renew_lease_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to renew a lease on the container.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation, including a required lease ID.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> renew_lease_async(const access_condition& condition, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Changes the lease ID for a lease on the container.
        /// </summary>
        /// <param name="proposed_lease_id">A string containing the proposed lease ID for the lease. May not be empty.</param>
        /// <returns>The new lease ID.</returns>
        utility::string_t change_lease(const utility::string_t& proposed_lease_id) const
        {
            return change_lease_async(proposed_lease_id).get();
        }

        /// <summary>
        /// Changes the lease ID for a lease on the container.
        /// </summary>
        /// <param name="proposed_lease_id">A string containing the proposed lease ID for the lease. May not be empty.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation, including a required lease ID.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The new lease ID.</returns>
        utility::string_t change_lease(const utility::string_t& proposed_lease_id, const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            return change_lease_async(proposed_lease_id, condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to change the lease ID for a lease on the container.
        /// </summary>
        /// <param name="proposed_lease_id">A string containing the proposed lease ID for the lease. May not be empty.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        pplx::task<utility::string_t> change_lease_async(const utility::string_t& proposed_lease_id) const
        {
            return change_lease_async(proposed_lease_id, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to change the lease ID for a lease on the container.
        /// </summary>
        /// <param name="proposed_lease_id">A string containing the proposed lease ID for the lease. May not be empty.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation, including a required lease ID.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<utility::string_t> change_lease_async(const utility::string_t& proposed_lease_id, const access_condition& condition, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Releases a lease on the container.
        /// </summary>
        void release_lease() const
        {
            release_lease_async().wait();
        }

        /// <summary>
        /// Releases a lease on the container.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation, including a required lease ID.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void release_lease(const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            release_lease_async(condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to release a lease on the container.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> release_lease_async() const
        {
            return release_lease_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to release a lease on the container.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation, including a required lease ID.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> release_lease_async(const access_condition& condition, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Breaks the current lease on the container.
        /// </summary>
        /// <param name="break_period">An <see cref="azure::storage::lease_break_period" /> representing the amount of time to allow the lease to remain.</param>
        /// <returns>The time until the lease ends, in seconds.</returns>
        std::chrono::seconds break_lease(const azure::storage::lease_break_period& break_period) const
        {
            return break_lease_async(break_period).get();
        }

        /// <summary>
        /// Breaks the current lease on the container.
        /// </summary>
        /// <param name="break_period">An <see cref="azure::storage::lease_break_period" /> representing the amount of time to allow the lease to remain.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation, including a required lease ID.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The time until the lease ends, in seconds.</returns>
        std::chrono::seconds break_lease(const azure::storage::lease_break_period& break_period, const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            return break_lease_async(break_period, condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to break the current lease on the container.
        /// </summary>
        /// <param name="break_period">An <see cref="azure::storage::lease_break_period" /> representing the amount of time to allow the lease to remain.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::chrono::seconds" /> that represents the current operation.</returns>
        pplx::task<std::chrono::seconds> break_lease_async(const azure::storage::lease_break_period& break_period) const
        {
            return break_lease_async(break_period, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to break the current lease on the container.
        /// </summary>
        /// <param name="break_period">An <see cref="azure::storage::lease_break_period" /> representing the amount of time to allow the lease to remain.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation, including a required lease ID.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::chrono::seconds" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<std::chrono::seconds> break_lease_async(const azure::storage::lease_break_period& break_period, const access_condition& condition, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Creates the container.
        /// </summary>
        void create()
        {
            create_async().wait();
        }

        /// <summary>
        /// Creates the container and specifies the level of access to the container's data.
        /// </summary>
        /// <param name="public_access">An <see cref="azure::storage::blob_container_public_access_type" /> value that specifies whether data in the container may be accessed publicly and what level of access is to be allowed.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation. This object
        /// is used to track requests to the storage service, and to provide additional runtime information about the operation. </param>
        void create(blob_container_public_access_type public_access, const blob_request_options& options, operation_context context)
        {
            create_async(public_access, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to create the container.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> create_async()
        {
            return create_async(blob_container_public_access_type::off, blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to create the container.
        /// </summary>
        /// <param name="public_access">An <see cref="azure::storage::blob_container_public_access_type" /> value that specifies whether data in the container may be accessed publicly and what level of access is to be allowed.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> create_async(blob_container_public_access_type public_access, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Creates the container if it does not already exist.
        /// </summary>
        /// <returns><c>true</c> if the container did not already exist and was created; otherwise <c>false</c>.</returns>
        bool create_if_not_exists()
        {
            return create_if_not_exists_async().get();
        }

        /// <summary>
        /// Creates the container if it does not already exist and specifies the level of public access to the container's data.
        /// </summary>
        /// <param name="public_access">An <see cref="azure::storage::blob_container_public_access_type" /> value that specifies whether data in the container may be accessed publicly and what level of access is to be allowed.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns><c>true</c> if the container did not already exist and was created; otherwise <c>false</c>.</returns>
        bool create_if_not_exists(blob_container_public_access_type public_access, const blob_request_options& options, operation_context context)
        {
            return create_if_not_exists_async(public_access, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to create the container if it does not already exist.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<bool> create_if_not_exists_async()
        {
            return create_if_not_exists_async(blob_container_public_access_type::off, blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to create the container if it does not already exist and specify the level of public access to the container's data.
        /// </summary>
        /// <param name="public_access">An <see cref="azure::storage::blob_container_public_access_type" /> value that specifies whether data in the container may be accessed publicly and what level of access is to be allowed.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<bool> create_if_not_exists_async(blob_container_public_access_type public_access, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Deletes the container.
        /// </summary>
        void delete_container()
        {
            delete_container_async().wait();
        }

        /// <summary>
        /// Deletes the container.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void delete_container(const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            delete_container_async(condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to delete the container.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> delete_container_async()
        {
            return delete_container_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to delete the container.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> delete_container_async(const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Deletes the container if it already exists.
        /// </summary>
        /// <returns><c>true</c> if the container did not already exist and was created; otherwise <c>false</c>.</returns>
        bool delete_container_if_exists()
        {
            return delete_container_if_exists_async().get();
        }

        /// <summary>
        /// Deletes the container if it already exists.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns><c>true</c> if the container did not already exist and was created; otherwise <c>false</c>.</returns>
        bool delete_container_if_exists(const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return delete_container_if_exists_async(condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to delete the container if it already exists.
        /// </summary>
        /// <returns><c>true</c> if the container did not already exist and was created; otherwise <c>false</c>.</returns>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<bool> delete_container_if_exists_async()
        {
            return delete_container_if_exists_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to delete the container if it already exists.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns><c>true</c> if the container did not already exist and was created; otherwise <c>false</c>.</returns>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<bool> delete_container_if_exists_async(const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Returns an <see cref="azure::storage::list_blob_item_iterator" /> that can be used to to lazily enumerate a collection of blob items in the container.
        /// </summary>
        /// <returns>An <see cref="azure::storage::list_blob_item_iterator" /> that can be used to to lazily enumerate a collection of blob items in the container.</returns>
        list_blob_item_iterator list_blobs() const
        {
            return list_blobs(utility::string_t(), false, blob_listing_details::none, 0, blob_request_options(), operation_context());
        }

        /// <summary>
        /// Returns an <see cref="azure::storage::list_blob_item_iterator" /> that can be used to to lazily enumerate a collection of blob items in the the container.
        /// </summary>
        /// <param name="prefix">The blob name prefix.</param>
        /// <param name="use_flat_blob_listing">Indicates whether to list blobs in a flat listing, or whether to list blobs hierarchically, by virtual directory.</param>
        /// <param name="includes">An <see cref="azure::storage::blob_listing_details::values" /> enumeration describing which items to include in the listing.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned.
        /// If this value is zero, the maximum possible number of results will be returned.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An <see cref="azure::storage::list_blob_item_iterator" /> that can be used to to lazily enumerate a collection of blob items in the the container.</returns>
        WASTORAGE_API list_blob_item_iterator list_blobs(const utility::string_t& prefix, bool use_flat_blob_listing, blob_listing_details::values includes, int max_results, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Returns a result segment containing a collection of blob items in the container.
        /// </summary>
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <returns>An <see cref="azure::storage::list_blob_item_segment" /> containing blob items, which may implement <see cref="azure::storage::cloud_blob" /> or <see cref="azure::storage::cloud_blob_directory" />.</returns>
        list_blob_item_segment list_blobs_segmented(const continuation_token& token) const
        {
            return list_blobs_segmented_async(token).get();
        }

        /// <summary>
        /// Returns a result segment containing a collection of blob items in the container.
        /// </summary>
        /// <param name="prefix">The blob name prefix.</param>
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <returns>A r<see cref="azure::storage::list_blob_item_segment" /> containing objects that implement <see cref="azure::storage::cloud_blob" /> and <see cref="azure::storage::cloud_blob_directory" />.</returns>
        list_blob_item_segment list_blobs_segmented(const utility::string_t& prefix, const continuation_token& token) const
        {
            return list_blobs_segmented_async(prefix, token).get();
        }

        /// <summary>
        /// Returns a result segment containing a collection of blob items in the container.
        /// </summary>
        /// <param name="prefix">The blob name prefix.</param>
        /// <param name="use_flat_blob_listing">Indicates whether to list blobs in a flat listing, or whether to list blobs hierarchically, by virtual directory.</param>
        /// <param name="includes">An <see cref="azure::storage::blob_listing_details::values" /> enumeration describing which items to include in the listing.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned at a time, up to the 
        /// per-operation limit of 5000. If this value is 0, the maximum possible number of results will be returned, up to 5000.</param>
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An <see cref="azure::storage::list_blob_item_segment" /> containing a collection of blob items in the container.</returns>
        list_blob_item_segment list_blobs_segmented(const utility::string_t& prefix, bool use_flat_blob_listing, blob_listing_details::values includes, int max_results, const continuation_token& token, const blob_request_options& options, operation_context context) const
        {
            return list_blobs_segmented_async(prefix, use_flat_blob_listing, includes, max_results, token, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to return an <see cref="azure::storage::list_blob_item_segment" /> containing a collection of blob items
        /// in the container.
        /// </summary>
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::list_blob_item_segment" /> that represents the current operation.</returns>
        pplx::task<list_blob_item_segment> list_blobs_segmented_async(const continuation_token& token) const
        {
            return list_blobs_segmented_async(utility::string_t(), token);
        }

        /// <summary>
        /// Initiates an asynchronous operation to return an <see cref="azure::storage::list_blob_item_segment" /> containing a collection of blob items
        /// in the container.
        /// </summary>
        /// <param name="prefix">The blob name prefix.</param>
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::list_blob_item_segment" /> that represents the current operation.</returns>
        pplx::task<list_blob_item_segment> list_blobs_segmented_async(const utility::string_t& prefix, const continuation_token& token) const
        {
            return list_blobs_segmented_async(prefix, false, blob_listing_details::none, 0, token, blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to return an <see cref="azure::storage::list_blob_item_segment" /> containing a collection of blob items
        /// in the container.
        /// </summary>
        /// <param name="prefix">The blob name prefix.</param>
        /// <param name="use_flat_blob_listing">Indicates whether to list blobs in a flat listing, or whether to list blobs hierarchically, by virtual directory.</param>
        /// <param name="includes">An <see cref="azure::storage::blob_listing_details::values" /> enumeration describing which items to include in the listing.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned at a time, up to the 
        /// per-operation limit of 5000. If this value is 0, the maximum possible number of results will be returned, up to 5000.</param>
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::list_blob_item_segment" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<list_blob_item_segment> list_blobs_segmented_async(const utility::string_t& prefix, bool use_flat_blob_listing, blob_listing_details::values includes, int max_results, const continuation_token& token, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Sets permissions for the container.
        /// </summary>
        /// <param name="permissions">The permissions to apply to the container.</param>
        void upload_permissions(const blob_container_permissions& permissions)
        {
            upload_permissions_async(permissions).wait();
        }

        /// <summary>
        /// Sets permissions for the container.
        /// </summary>
        /// <param name="permissions">The permissions to apply to the container.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_permissions(const blob_container_permissions& permissions, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_permissions_async(permissions, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to set permissions for the container.
        /// </summary>
        /// <param name="permissions">The permissions to apply to the container.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_permissions_async(const blob_container_permissions& permissions)
        {
            return upload_permissions_async(permissions, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to set permissions for the container.
        /// </summary>
        /// <param name="permissions">The permissions to apply to the container.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_permissions_async(const blob_container_permissions& permissions, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Gets the permissions settings for the container.
        /// </summary>
        /// <returns>The container's permissions.</returns>
        blob_container_permissions download_permissions()
        {
            return download_permissions_async().get();
        }

        /// <summary>
        /// Gets the permissions settings for the container.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The container's permissions.</returns>
        blob_container_permissions download_permissions(const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return download_permissions_async(condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to get permissions settings for the container.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::blob_container_permissions" /> that represents the current operation.</returns>
        pplx::task<blob_container_permissions> download_permissions_async()
        {
            return download_permissions_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to get permissions settings for the container.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::blob_container_permissions" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<blob_container_permissions> download_permissions_async(const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Checks existence of the container.
        /// </summary>
        /// <returns><c>true</c> if the container exists.</returns>
        bool exists()
        {
            return exists_async().get();
        }

        /// <summary>
        /// Checks existence of the container.
        /// </summary>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns><c>true</c> if the container exists.</returns>
        bool exists(const blob_request_options& options, operation_context context)
        {
            return exists_async(options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to check the existence of the container.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<bool> exists_async()
        {
            return exists_async(blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to check the existence of the container.
        /// </summary>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that that represents the current operation.</returns>
        pplx::task<bool> exists_async(const blob_request_options& options, operation_context context)
        {
            return exists_async(false, options, context);
        }

        /// <summary>
        /// Gets the Blob service client for the container.
        /// </summary>
        /// <returns>An <see cref="azure::storage::cloud_blob_client" /> object that specifies the endpoint for the Blob service.</returns>
        const cloud_blob_client& service_client() const
        {
            return m_client;
        }

        /// <summary>
        /// Gets the name of the container.
        /// </summary>
        /// <returns>A string containing the container name.</returns>
        const utility::string_t& name() const
        {
            return m_name;
        }

        /// <summary>
        /// Gets the container URI for all locations.
        /// </summary>
        /// <returns>An <see cref="azure::storage::storage_uri" /> containing the container URI for all locations.</returns>
        const storage_uri& uri() const
        {
            return m_uri;
        }

        /// <summary>
        /// Gets the container's metadata.
        /// </summary>
        /// <returns>An <see cref="azure::storage::cloud_metadata" /> object containing the container's metadata.</returns>
        cloud_metadata& metadata()
        {
            return *m_metadata;
        }

        /// <summary>
        /// Gets the container's metadata.
        /// </summary>
        /// <returns>An <see cref="azure::storage::cloud_metadata" /> object containing the container's metadata.</returns>
        const cloud_metadata& metadata() const
        {
            return *m_metadata;
        }

        /// <summary>
        /// Gets the container's system properties.
        /// </summary>
        /// <returns>The container's properties.</returns>
        const cloud_blob_container_properties& properties() const
        {
            return *m_properties;
        }

        /// <summary>
        /// Indicates whether the <see cref="azure::storage::cloud_blob_container" /> object is valid.
        /// </summary>
        /// <returns><c>true</c> if the <see cref="azure::storage::cloud_blob_container" /> object is valid; otherwise, <c>false</c>.</returns>
        bool is_valid() const
        {
            return !m_name.empty();
        }

    private:

        void init(storage_credentials credentials);
        WASTORAGE_API pplx::task<bool> exists_async(bool primary_only, const blob_request_options& options, operation_context context);

        cloud_blob_client m_client;
        utility::string_t m_name;
        storage_uri m_uri;
        std::shared_ptr<cloud_metadata> m_metadata;
        std::shared_ptr<cloud_blob_container_properties> m_properties;
    };

    /// <summary>
    /// Represents a virtual directory of blobs, designated by a delimiter character.
    /// </summary>
    /// <remarks>Containers, which are encapsulated as <see cref="azure::storage::cloud_blob_container" /> objects, hold directories, and directories hold block blobs and page blobs. Directories can also contain sub-directories.</remarks>
    class cloud_blob_directory
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_directory" /> class.
        /// </summary>
        cloud_blob_directory()
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_directory" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_blob_directory" /> object.</param>
        cloud_blob_directory(cloud_blob_directory&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_blob_directory" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_blob_directory" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::cloud_blob_directory" /> object with properties set.</returns>
        cloud_blob_directory& operator=(cloud_blob_directory&& other)
        {
            if (this != &other)
            {
                m_name = std::move(other.m_name);
                m_container = std::move(other.m_container);
                m_uri = std::move(other.m_uri);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets a reference to a blob in this virtual directory.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_blob" />.</returns>
        WASTORAGE_API cloud_blob get_blob_reference(utility::string_t blob_name) const;

        /// <summary>
        /// Gets a reference to a blob in this virtual directory.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_blob" />.</returns>
        WASTORAGE_API cloud_blob get_blob_reference(utility::string_t blob_name, utility::string_t snapshot_time) const;

        /// <summary>
        /// Gets a reference to a page blob in this virtual directory.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_page_blob" />.</returns>
        WASTORAGE_API cloud_page_blob get_page_blob_reference(utility::string_t blob_name) const;

        /// <summary>
        /// Gets a reference to a page blob in this virtual directory.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_page_blob" />.</returns>
        WASTORAGE_API cloud_page_blob get_page_blob_reference(utility::string_t blob_name, utility::string_t snapshot_time) const;

        /// <summary>
        /// Gets a reference to a block blob in this virtual directory.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_block_blob" />.</returns>
        WASTORAGE_API cloud_block_blob get_block_blob_reference(utility::string_t blob_name) const;

        /// <summary>
        /// Gets a reference to a block blob in this virtual directory.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <returns>A reference to an <see cref="azure::storage::cloud_block_blob" />.</returns>
        WASTORAGE_API cloud_block_blob get_block_blob_reference(utility::string_t blob_name, utility::string_t snapshot_time) const;

        /// <summary>
        /// Gets a reference to an append blob in this virtual directory.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <returns>A reference to a <see cref="cloud_append_blob" />.</returns>
        WASTORAGE_API cloud_append_blob get_append_blob_reference(utility::string_t blob_name) const;

        /// <summary>
        /// Gets a reference to an append blob in this virtual directory.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <returns>A reference to a <see cref="cloud_append_blob" />.</returns>
        WASTORAGE_API cloud_append_blob get_append_blob_reference(utility::string_t blob_name, utility::string_t snapshot_time) const;

        /// <summary>
        /// Returns a virtual subdirectory within this virtual directory.
        /// </summary>
        /// <param name="name">The name of the virtual subdirectory.</param>
        /// <returns>An <see cref="azure::storage::cloud_blob_directory" /> object representing the virtual subdirectory.</returns>
        WASTORAGE_API cloud_blob_directory get_subdirectory_reference(utility::string_t name) const;

        /// <summary>
        /// Gets the <see cref="azure::storage::cloud_blob_directory" /> object representing the
        /// parent directory for the current virtual directory.
        /// </summary>
        /// <returns>The virtual directory's parent directory.</returns>
        WASTORAGE_API cloud_blob_directory get_parent_reference() const;

        /// <summary>
        /// Returns an <see cref="azure::storage::list_blob_item_iterator" /> that can be used to to lazily enumerate a collection of blob items in the virtual directory.
        /// </summary>
        /// <returns>An <see cref="azure::storage::list_blob_item_iterator" /> that can be used to to lazily enumerate a collection of blob items in the virtual directory.</returns>
        list_blob_item_iterator list_blobs() const
        {
            return list_blobs(false, blob_listing_details::none, 0, blob_request_options(), operation_context());
        }

        /// <summary>
        /// Returns an <see cref="azure::storage::list_blob_item_iterator" /> that can be used to to lazily enumerate a collection of blob items in the the virtual directory.
        /// </summary>
        /// <param name="use_flat_blob_listing">Indicates whether to list blobs in a flat listing, or whether to list blobs hierarchically, by virtual directory.</param>
        /// <param name="includes">An <see cref="azure::storage::blob_listing_details::values" /> enumeration describing which items to include in the listing.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned.
        /// If this value is zero, the maximum possible number of results will be returned.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An <see cref="azure::storage::list_blob_item_iterator" /> that can be used to to lazily enumerate a collection of blob items in the the virtual directory.</returns>
        WASTORAGE_API list_blob_item_iterator list_blobs(bool use_flat_blob_listing, blob_listing_details::values includes, int max_results, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Returns a result segment containing a collection of blob items
        /// in the container.
        /// </summary>
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <returns>An <see cref="azure::storage::list_blob_item_segment" /> containing a collection of blob items in the container.</returns>
        list_blob_item_segment list_blobs_segmented(const continuation_token& token) const
        {
            return list_blobs_segmented_async(token).get();
        }

        /// <summary>
        /// Returns an <see cref="azure::storage::list_blob_item_segment" /> containing a collection of blob items in the container.
        /// </summary>
        /// <param name="use_flat_blob_listing">Indicates whether to list blobs in a flat listing, or whether to list blobs hierarchically, by virtual directory.</param>
        /// <param name="includes">An <see cref="azure::storage::blob_listing_details::values" /> enumeration describing which items to include in the listing.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned at a time, up to the 
        /// per-operation limit of 5000. If this value is 0, the maximum possible number of results will be returned, up to 5000.</param>    
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An <see cref="azure::storage::list_blob_item_segment" /> containing a collection of blob items in the container.</returns>
        list_blob_item_segment list_blobs_segmented(bool use_flat_blob_listing, blob_listing_details::values includes, int max_results, const continuation_token& token, const blob_request_options& options, operation_context context) const
        {
            return list_blobs_segmented_async(use_flat_blob_listing, includes, max_results, token, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to return an <see cref="azure::storage::list_blob_item_segment" /> containing a collection of blob items
        /// in the container.
        /// </summary>
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::list_blob_item_segment" /> that that represents the current operation.</returns>
        pplx::task<list_blob_item_segment> list_blobs_segmented_async(const continuation_token& token) const
        {
            return list_blobs_segmented_async(false, blob_listing_details::none, 0, token, blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to return an <see cref="azure::storage::list_blob_item_segment" /> containing a collection of blob items
        /// in the container.
        /// </summary>
        /// <param name="use_flat_blob_listing">Indicates whether to list blobs in a flat listing, or whether to list blobs hierarchically, by virtual directory.</param>
        /// <param name="includes">An <see cref="azure::storage::blob_listing_details::values" /> enumeration describing which items to include in the listing.</param>
        /// <param name="max_results">A non-negative integer value that indicates the maximum number of results to be returned at a time, up to the 
        /// per-operation limit of 5000. If this value is 0, the maximum possible number of results will be returned, up to 5000.</param>    
        /// <param name="token">A continuation token returned by a previous listing operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::list_blob_item_segment" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<list_blob_item_segment> list_blobs_segmented_async(bool use_flat_blob_listing, blob_listing_details::values includes, int max_results, const continuation_token& token, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Gets the Blob service client for the virtual directory.
        /// </summary>
        /// <returns>A client object that specifies the endpoint for the Windows Azure Blob service.</returns>
        const cloud_blob_client& service_client() const
        {
            return m_container.service_client();
        }

        /// <summary>
        /// Gets an <see cref="azure::storage::cloud_blob_container" /> object representing the virtual directory's container.
        /// </summary>
        /// <returns>The virtual directory's container.</returns>
        const cloud_blob_container& container() const
        {
            return m_container;
        }

        /// <summary>
        /// Gets the virtual directory URI for all locations.
        /// </summary>
        /// <returns>An <see cref="azure::storage::storage_uri" /> containing the virtual directory URI for all locations.</returns>
        const storage_uri& uri() const
        {
            return m_uri;
        }

        /// <summary>
        /// Gets the prefix for the virtual directory.
        /// </summary>
        /// <returns>A string containing the prefix.</returns>
        const utility::string_t& prefix() const
        {
            return m_name;
        }

        /// <summary>
        /// Indicates whether the <see cref="azure::storage::cloud_blob_directory" /> object is valid.
        /// </summary>
        /// <returns><c>true</c> if the <see cref="azure::storage::cloud_blob_directory" /> object is valid; otherwise, <c>false</c>.</returns>
        bool is_valid() const
        {
            return !m_name.empty();
        }

    private:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob_directory" /> class.
        /// </summary>
        /// <param name="name">Name of the virtual directory.</param>
        /// <param name="container">The container.</param>
        WASTORAGE_API cloud_blob_directory(utility::string_t name, cloud_blob_container container);

        utility::string_t m_name;
        cloud_blob_container m_container;
        storage_uri m_uri;

        friend class cloud_blob_container;
        friend class cloud_blob;
        friend class list_blob_item;
    };

    /// <summary>
    /// A class for Windows Azure blob types. The <see cref="azure::storage::cloud_block_blob" /> and 
    /// <see cref="azure::storage::cloud_page_blob" /> classes derive from the <see cref="azure::storage::cloud_blob" /> class.
    /// </summary>
    class cloud_blob
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob" /> class.
        /// </summary>
        cloud_blob()
            : m_properties(std::make_shared<cloud_blob_properties>()), m_metadata(std::make_shared<cloud_metadata>()), m_copy_state(std::make_shared<azure::storage::copy_state>())
        {
            set_type(blob_type::unspecified);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob" /> class using an absolute URI to the blob.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the blob for all locations.</param>
        WASTORAGE_API cloud_blob(storage_uri uri);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob" /> class using an absolute URI to the blob.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the blob for all locations.</param>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        WASTORAGE_API cloud_blob(storage_uri uri, storage_credentials credentials);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob" /> class using an absolute URI to the blob.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the blob for all locations.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        WASTORAGE_API cloud_blob(storage_uri uri, utility::string_t snapshot_time, storage_credentials credentials);

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+, 
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_blob" /> object.</param>
        cloud_blob(cloud_blob&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_blob" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_blob" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::cloud_blob" /> object with properties set.</returns>
        cloud_blob& operator=(cloud_blob&& other)
        {
            if (this != &other)
            {
                m_properties = std::move(other.m_properties);
                m_metadata = std::move(other.m_metadata);
                m_copy_state = std::move(other.m_copy_state);
                m_name = std::move(other.m_name);
                m_snapshot_time = std::move(other.m_snapshot_time);
                m_container = std::move(other.m_container);
                m_uri = std::move(other.m_uri);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Gets the <see cref="azure::storage::cloud_blob_directory" /> object representing the
        /// virtual parent directory for the blob.
        /// </summary>
        /// <returns>The blob's virtual parent directory.</returns>
        WASTORAGE_API cloud_blob_directory get_parent_reference() const;

        /// <summary>
        /// Gets the snapshot-qualified URI to the blob.
        /// </summary>
        /// <returns>The blob's snapshot-qualified URI, if the blob is a snapshot; otherwise, the absolute URI to the blob.</returns>
        WASTORAGE_API storage_uri snapshot_qualified_uri() const;

        /// <summary>
        /// Returns a shared access signature for the blob.
        /// </summary>
        /// <param name="policy">The access policy for the shared access signature.</param>
        /// <returns>A string containing a shared access signature.</returns>
        utility::string_t get_shared_access_signature(const blob_shared_access_policy& policy) const
        {
            return get_shared_access_signature(policy, utility::string_t());
        }

        /// <summary>
        /// Returns a shared access signature for the blob.
        /// </summary>
        /// <param name="policy">The access policy for the shared access signature.</param>
        /// <param name="stored_policy_identifier">A container-level access policy.</param>
        /// <returns>A string containing a shared access signature.</returns>
        utility::string_t get_shared_access_signature(const blob_shared_access_policy& policy, const utility::string_t& stored_policy_identifier) const
        {
            return get_shared_access_signature(policy, stored_policy_identifier, cloud_blob_shared_access_headers());
        }

        /// <summary>
        /// Returns a shared access signature for the blob.
        /// </summary>
        /// <param name="policy">The access policy for the shared access signature.</param>
        /// <param name="stored_policy_identifier">A container-level access policy.</param>
        /// <param name="headers">The optional header values to set for a blob returned with this SAS.</param>
        /// <returns>A string containing a shared access signature.</returns>
        WASTORAGE_API utility::string_t get_shared_access_signature(const blob_shared_access_policy& policy, const utility::string_t& stored_policy_identifier, const cloud_blob_shared_access_headers& headers) const;

        /// <summary>
        /// Opens a stream for reading from the blob.
        /// </summary>
        /// <returns>A stream to be used for reading from the blob.</returns>
        concurrency::streams::istream open_read()
        {
            return open_read_async().get();
        }

        /// <summary>
        /// Opens a stream for reading from the blob.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A stream to be used for reading from the blob.</returns>
        concurrency::streams::istream open_read(const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return open_read_async(condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to open a stream for reading from the blob.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="concurrency::streams::istream" /> that represents the current operation.</returns>
        pplx::task<concurrency::streams::istream> open_read_async()
        {
            return open_read_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to open a stream for reading from the blob.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="concurrency::streams::istream" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<concurrency::streams::istream> open_read_async(const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Checks existence of the blob.
        /// </summary>
        /// <returns><c>true</c> if the blob exists.</returns>
        bool exists()
        {
            return exists_async().get();
        }

        /// <summary>
        /// Checks existence of the blob.
        /// </summary>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns><c>true</c> if the blob exists.</returns>
        bool exists(const blob_request_options& options, operation_context context)
        {
            return exists_async(options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to check the existence of the blob.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<bool> exists_async()
        {
            return exists_async(blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to check the existence of the blob.
        /// </summary>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<bool> exists_async(const blob_request_options& options, operation_context context)
        {
            return exists_async(false, options, context);
        }

        /// <summary>
        /// Populates a blob's properties and metadata.
        /// </summary>
        void download_attributes()
        {
            download_attributes_async().wait();
        }

        /// <summary>
        /// Populates a blob's properties and metadata.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void download_attributes(const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            download_attributes_async(condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to populate a blob's properties and metadata.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> download_attributes_async()
        {
            return download_attributes_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to populate a blob's properties and metadata.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> download_attributes_async(const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Updates the blob's metadata.
        /// </summary>
        void upload_metadata()
        {
            upload_metadata_async().wait();
        }

        /// <summary>
        /// Updates the blob's metadata.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_metadata(const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_metadata_async(condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to update the blob's metadata.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_metadata_async()
        {
            return upload_metadata_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to update the blob's metadata.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_metadata_async(const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Updates the blob's properties.
        /// </summary>
        void upload_properties()
        {
            upload_properties_async().wait();
        }

        /// <summary>
        /// Updates the blob's properties.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_properties(const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_properties_async(condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to update the blob's properties.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_properties_async()
        {
            return upload_properties_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to update the blob's properties.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_properties_async(const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Deletes the blob.
        /// </summary>
        void delete_blob()
        {
            delete_blob_async().wait();
        }

        /// <summary>
        /// Deletes the blob.
        /// </summary>
        /// <param name="snapshots_option">Indicates whether to delete only the blob, to delete the blob and all snapshots, or to delete only snapshots.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void delete_blob(delete_snapshots_option snapshots_option, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            delete_blob_async(snapshots_option, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to delete the blob.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> delete_blob_async()
        {
            return delete_blob_async(delete_snapshots_option::none, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to delete the blob.
        /// </summary>
        /// <param name="snapshots_option">Indicates whether to delete only the blob, to delete the blob and all snapshots, or to delete only snapshots.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> delete_blob_async(delete_snapshots_option snapshots_option, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Deletes the blob if it already exists.
        /// </summary>
        /// <returns><c>true</c> if the blob did already exist and was deleted; otherwise <c>false</c>.</returns>
        bool delete_blob_if_exists()
        {
            return delete_blob_if_exists_async().get();
        }

        /// <summary>
        /// Deletes the blob if it already exists.
        /// </summary>
        /// <param name="snapshots_option">Indicates whether to delete only the blob, to delete the blob and all snapshots, or to delete only snapshots.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns><c>true</c> if the blob did already exist and was deleted; otherwise <c>false</c>.</returns>
        bool delete_blob_if_exists(delete_snapshots_option snapshots_option, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return delete_blob_if_exists_async(snapshots_option, condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to delete the blob if it already exists.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<bool> delete_blob_if_exists_async()
        {
            return delete_blob_if_exists_async(delete_snapshots_option::none, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to delete the blob if it already exists.
        /// </summary>
        /// <param name="snapshots_option">Indicates whether to delete only the blob, to delete the blob and all snapshots, or to delete only snapshots.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<bool> delete_blob_if_exists_async(delete_snapshots_option snapshots_option, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Acquires a lease on the blob.
        /// </summary>
        /// <param name="duration">An <see cref="azure::storage::lease_time" /> representing the span of time for which to acquire the lease.</param>
        /// <param name="proposed_lease_id">A string representing the proposed lease ID for the new lease. May be an empty string if no lease ID is proposed.</param>
        /// <returns>A string containing the lease ID.</returns>
        utility::string_t acquire_lease(const azure::storage::lease_time& duration, const utility::string_t& proposed_lease_id) const
        {
            return acquire_lease_async(duration, proposed_lease_id).get();
        }

        /// <summary>
        /// Acquires a lease on the blob.
        /// </summary>
        /// <param name="duration">An <see cref="azure::storage::lease_time" /> representing the span of time for which to acquire the lease.</param>
        /// <param name="proposed_lease_id">A string representing the proposed lease ID for the new lease. May be an empty string if no lease ID is proposed.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A string containing the lease ID.</returns>
        utility::string_t acquire_lease(const azure::storage::lease_time& duration, const utility::string_t& proposed_lease_id, const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            return acquire_lease_async(duration, proposed_lease_id, condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to acquire a lease on the blob.
        /// </summary>
        /// <param name="duration">An <see cref="azure::storage::lease_time" /> representing the span of time for which to acquire the lease.</param>
        /// <param name="proposed_lease_id">A string representing the proposed lease ID for the new lease. May be an empty string if no lease ID is proposed.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        pplx::task<utility::string_t> acquire_lease_async(const azure::storage::lease_time& duration, const utility::string_t& proposed_lease_id) const
        {
            return acquire_lease_async(duration, proposed_lease_id, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to acquire a lease on the blob.
        /// </summary>
        /// <param name="duration">An <see cref="azure::storage::lease_time" /> representing the span of time for which to acquire the lease.</param>
        /// <param name="proposed_lease_id">A string representing the proposed lease ID for the new lease. May be an empty string if no lease ID is proposed.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<utility::string_t> acquire_lease_async(const azure::storage::lease_time& duration, const utility::string_t& proposed_lease_id, const access_condition& condition, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Renews a lease on the blob.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access conditions for the blob, including a required lease ID.</param>
        void renew_lease(const access_condition& condition) const
        {
            renew_lease_async(condition).wait();
        }

        /// <summary>
        /// Renews a lease on the blob.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access conditions for the blob, including a required lease ID.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void renew_lease(const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            renew_lease_async(condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to renew a lease on the blob.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access conditions for the blob, including a required lease ID.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> renew_lease_async(const access_condition& condition) const
        {
            return renew_lease_async(condition, blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to renew a lease on the blob.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access conditions for the blob, including a required lease ID.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> renew_lease_async(const access_condition& condition, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Changes the lease ID on the blob.
        /// </summary>
        /// <param name="proposed_lease_id">A string containing the proposed lease ID for the lease. May not be empty.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access conditions for the blob, including a required lease ID.</param>
        /// <returns>The new lease ID.</returns>
        utility::string_t change_lease(const utility::string_t& proposed_lease_id, const access_condition& condition) const
        {
            return change_lease_async(proposed_lease_id, condition).get();
        }

        /// <summary>
        /// Changes the lease ID on the blob.
        /// </summary>
        /// <param name="proposed_lease_id">A string containing the proposed lease ID for the lease. May not be empty.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access conditions for the blob, including a required lease ID.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The new lease ID.</returns>
        utility::string_t change_lease(const utility::string_t& proposed_lease_id, const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            return change_lease_async(proposed_lease_id, condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to change the lease ID on the blob.
        /// </summary>
        /// <param name="proposed_lease_id">A string containing the proposed lease ID for the lease. May not be empty.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access conditions for the blob, including a required lease ID.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        pplx::task<utility::string_t> change_lease_async(const utility::string_t& proposed_lease_id, const access_condition& condition) const
        {
            return change_lease_async(proposed_lease_id, condition, blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to change the lease ID on the blob.
        /// </summary>
        /// <param name="proposed_lease_id">A string containing the proposed lease ID for the lease. May not be empty.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access conditions for the blob, including a required lease ID.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<utility::string_t> change_lease_async(const utility::string_t& proposed_lease_id, const access_condition& condition, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Releases the lease on the blob.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access conditions for the blob, including a required lease ID.</param>
        void release_lease(const access_condition& condition) const
        {
            release_lease_async(condition).wait();
        }

        /// <summary>
        /// Releases the lease on the blob.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access conditions for the blob, including a required lease ID.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void release_lease(const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            release_lease_async(condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to release the lease on the blob.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access conditions for the blob, including a required lease ID.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> release_lease_async(const access_condition& condition) const
        {
            return release_lease_async(condition, blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to release the lease on the blob.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access conditions for the blob, including a required lease ID.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> release_lease_async(const access_condition& condition, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Breaks the current lease on the blob.
        /// </summary>
        /// <param name="break_period">An <see cref="azure::storage::lease_break_period" /> representing the amount of time to allow the lease to remain.</param>
        /// <returns>The time until the lease ends, in seconds.</returns>
        std::chrono::seconds break_lease(const azure::storage::lease_break_period& break_period) const
        {
            return break_lease_async(break_period).get();
        }

        /// <summary>
        /// Breaks the current lease on the blob.
        /// </summary>
        /// <param name="break_period">An <see cref="azure::storage::lease_break_period" /> representing the amount of time to allow the lease to remain.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access conditions for the blob, including a required lease ID.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The time until the lease ends, in seconds.</returns>
        std::chrono::seconds break_lease(const azure::storage::lease_break_period& break_period, const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            return break_lease_async(break_period, condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to break the current lease on the blob.
        /// </summary>
        /// <param name="break_period">An <see cref="azure::storage::lease_break_period" /> representing the amount of time to allow the lease to remain.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::chrono::seconds" /> that represents the current operation.</returns>
        pplx::task<std::chrono::seconds> break_lease_async(const azure::storage::lease_break_period& break_period) const
        {
            return break_lease_async(break_period, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to break the current lease on the blob.
        /// </summary>
        /// <param name="break_period">An <see cref="azure::storage::lease_break_period" /> representing the amount of time to allow the lease to remain.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access conditions for the blob, including a required lease ID.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::chrono::seconds" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<std::chrono::seconds> break_lease_async(const azure::storage::lease_break_period& break_period, const access_condition& condition, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Downloads the contents of a blob to a stream.
        /// </summary>
        /// <param name="target">The target stream.</param>
        void download_to_stream(concurrency::streams::ostream target)
        {
            download_to_stream_async(target).wait();
        }

        /// <summary>
        /// Downloads the contents of a blob to a stream.
        /// </summary>
        /// <param name="target">The target stream.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void download_to_stream(concurrency::streams::ostream target, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            download_to_stream_async(target, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to download the contents of a blob to a stream.
        /// </summary>
        /// <param name="target">The target stream.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> download_to_stream_async(concurrency::streams::ostream target)
        {
            return download_to_stream_async(target, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to download the contents of a blob to a stream.
        /// </summary>
        /// <param name="target">The target stream.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> download_to_stream_async(concurrency::streams::ostream target, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return download_range_to_stream_async(target, std::numeric_limits<utility::size64_t>::max(), 0, condition, options, context);
        }

        /// <summary>
        /// Downloads a range of bytes in a blob to a stream.
        /// </summary>
        /// <param name="target">The target stream.</param>
        /// <param name="offset">The offset at which to begin downloading the blob, in bytes.</param>
        /// <param name="length">The length of the data to download from the blob, in bytes.</param>
        void download_range_to_stream(concurrency::streams::ostream target, utility::size64_t offset, utility::size64_t length)
        {
            download_range_to_stream_async(target, offset, length).wait();
        }

        /// <summary>
        /// Downloads a range of bytes in a blob to a stream.
        /// </summary>
        /// <param name="target">The target stream.</param>
        /// <param name="offset">The offset at which to begin downloading the blob, in bytes.</param>
        /// <param name="length">The length of the data to download from the blob, in bytes.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void download_range_to_stream(concurrency::streams::ostream target, utility::size64_t offset, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            download_range_to_stream_async(target, offset, length, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to download a range of bytes in a blob to a stream.
        /// </summary>
        /// <param name="target">The target stream.</param>
        /// <param name="offset">The offset at which to begin downloading the blob, in bytes.</param>
        /// <param name="length">The length of the data to download from the blob, in bytes.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> download_range_to_stream_async(concurrency::streams::ostream target, int64_t offset, int64_t length)
        {
            return download_range_to_stream_async(target, offset, length, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to download a range of bytes in a blob to a stream.
        /// </summary>
        /// <param name="target">The target stream.</param>
        /// <param name="offset">The offset at which to begin downloading the blob, in bytes.</param>
        /// <param name="length">The length of the data to download from the blob, in bytes.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> download_range_to_stream_async(concurrency::streams::ostream target, utility::size64_t offset, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Downloads the contents of a blob to a file.
        /// </summary>
        /// <param name="path">The target file.</param>
        void download_to_file(const utility::string_t &path)
        {
            download_to_file_async(path).wait();
        }

        /// <summary>
        /// Downloads the contents of a blob to a file.
        /// </summary>
        /// <param name="path">The target file.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void download_to_file(const utility::string_t &path, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            download_to_file_async(path, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to download the contents of a blob to a file.
        /// </summary>
        /// <param name="path">The target file.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> download_to_file_async(const utility::string_t &path)
        {
            return download_to_file_async(path, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to download the contents of a blob to a file.
        /// </summary>
        /// <param name="path">The target file.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> download_to_file_async(const utility::string_t &path, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Begins an operation to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// <para>This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.</para>
        /// <para> This method is deprecated in favor of start_copy.</para>
        /// </remarks>
        DEPRECATED("Deprecated this method in favor of start_copy.")
        utility::string_t start_copy_from_blob(const web::http::uri& source)
        {
            return start_copy_async(source).get();
        }

        /// <summary>
        /// Begins an operation to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <param name="source_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the source blob.</param>
        /// <param name="destination_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// <para>This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.</para>
        /// <para> This method is deprecated in favor of start_copy.</para>
        /// </remarks>
        DEPRECATED("Deprecated this method in favor of start_copy.")
        utility::string_t start_copy_from_blob(const web::http::uri& source, const access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context)
        {
            return start_copy_async(source, source_condition, destination_condition, options, context).get();
        }

        /// <summary>
        /// Begins an operation to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// <para>This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.</para>
        /// <para> This method is deprecated in favor of start_copy.</para>
        /// </remarks>
        DEPRECATED("Deprecated this method in favor of start_copy.")
        utility::string_t start_copy_from_blob(const cloud_blob& source)
        {
            return start_copy_async(source).get();
        }

        /// <summary>
        /// Begins an operation to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <param name="source_condition">An object that represents the <see cref="access_condition" /> for the source blob.</param>
        /// <param name="destination_condition">An object that represents the <see cref="access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// <para>This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.</para>
        /// <para> This method is deprecated in favor of start_copy.</para>
        /// </remarks>
        DEPRECATED("Deprecated this method in favor of start_copy.")
        utility::string_t start_copy_from_blob(const cloud_blob& source, const access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context)
        {
            return start_copy_async(source, source_condition, destination_condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to begin to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// <para>This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.</para>
        /// <para> This method is deprecated in favor of start_copy.</para>
        /// </remarks>
        DEPRECATED("Deprecated this method in favor of start_copy_async.")
        pplx::task<utility::string_t> start_copy_from_blob_async(const web::http::uri& source)
        {
            return start_copy_async(source, access_condition(), access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to begin to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// <para>This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.</para>
        /// <para> This method is deprecated in favor of start_copy.</para>
        /// </remarks>
        DEPRECATED("Deprecated this method in favor of start_copy_async.")
        pplx::task<utility::string_t> start_copy_from_blob_async(const cloud_blob& source)
        {
            return start_copy_async(source, access_condition(), access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to begin to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <param name="source_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the source blob.</param>
        /// <param name="destination_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// <para>This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.</para>
        /// <para> This method is deprecated in favor of start_copy.</para>
        /// </remarks>
        DEPRECATED("Deprecated this method in favor of start_copy_async.")
        WASTORAGE_API pplx::task<utility::string_t> start_copy_from_blob_async(const web::http::uri& source, const access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Initiates an asynchronous operation to begin to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <param name="source_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the source blob.</param>
        /// <param name="destination_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// <para>This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.</para>
        /// <para> This method is deprecated in favor of start_copy.</para>
        /// </remarks>
        DEPRECATED("Deprecated this method in favor of start_copy_async.")
        WASTORAGE_API pplx::task<utility::string_t> start_copy_from_blob_async(const cloud_blob& source, const access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Begins an operation to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_copy(const web::http::uri& source)
        {
            return start_copy_async(source).get();
        }

        /// <summary>
        /// Begins an operation to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <param name="source_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the source blob.</param>
        /// <param name="destination_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_copy(const web::http::uri& source, const access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context)
        {
            return start_copy_async(source, source_condition, destination_condition, options, context).get();
        }

        /// <summary>
        /// Begins an operation to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_copy(const cloud_blob& source)
        {
            return start_copy_async(source).get();
        }

        /// <summary>
        /// Begins an operation to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <param name="source_condition">An object that represents the <see cref="access_condition" /> for the source blob.</param>
        /// <param name="destination_condition">An object that represents the <see cref="access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_copy(const cloud_blob& source, const access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context)
        {
            return start_copy_async(source, source_condition, destination_condition, options, context).get();
        }

        /// <summary>
        /// Begins an operation to copy a file's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source file.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_copy(const cloud_file& source)
        {
            return start_copy_async(source).get();
        }

        /// <summary>
        /// Begins an operation to copy a file's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source file.</param>
        /// <param name="source_condition">An object that represents the <see cref="access_condition" /> for the source blob.</param>
        /// <param name="destination_condition">An object that represents the <see cref="access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_copy(const cloud_file& source, const file_access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context)
        {
            return start_copy_async(source, source_condition, destination_condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to begin to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        pplx::task<utility::string_t> start_copy_async(const web::http::uri& source)
        {
            return start_copy_async(source, access_condition(), access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to begin to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        pplx::task<utility::string_t> start_copy_async(const cloud_blob& source)
        {
            return start_copy_async(source, access_condition(), access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to begin to copy a file's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source file.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        WASTORAGE_API pplx::task<utility::string_t> start_copy_async(const cloud_file& source);

        /// <summary>
        /// Initiates an asynchronous operation to begin to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <param name="source_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the source blob.</param>
        /// <param name="destination_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        pplx::task<utility::string_t> start_copy_async(const web::http::uri& source, const access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context)
        {
            return start_copy_async_impl(source, premium_blob_tier::unknown, source_condition, destination_condition, options, context);
        }

        /// <summary>
        /// Initiates an asynchronous operation to begin to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <param name="source_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the source blob.</param>
        /// <param name="destination_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        WASTORAGE_API pplx::task<utility::string_t> start_copy_async(const cloud_blob& source, const access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Initiates an asynchronous operation to begin to copy a file's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source file.</param>
        /// <param name="source_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the source blob.</param>
        /// <param name="destination_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        WASTORAGE_API pplx::task<utility::string_t> start_copy_async(const cloud_file& source, const file_access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Aborts an ongoing blob copy operation.
        /// </summary>
        /// <param name="copy_id">A string identifying the copy operation.</param>
        void abort_copy(const utility::string_t& copy_id) const
        {
            abort_copy_async(copy_id).wait();
        }

        /// <summary>
        /// Aborts an ongoing blob copy operation.
        /// </summary>
        /// <param name="copy_id">A string identifying the copy operation.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void abort_copy(const utility::string_t& copy_id, const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            abort_copy_async(copy_id, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to abort an ongoing blob copy operation.
        /// </summary>
        /// <param name="copy_id">A string identifying the copy operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> abort_copy_async(const utility::string_t& copy_id) const
        {
            return abort_copy_async(copy_id, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to abort an ongoing blob copy operation.
        /// </summary>
        /// <param name="copy_id">A string identifying the copy operation.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> abort_copy_async(const utility::string_t& copy_id, const access_condition& condition, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Creates a snapshot of the blob.
        /// </summary>
        /// <returns>A blob snapshot.</returns>
        cloud_blob create_snapshot()
        {
            return create_snapshot_async().get();
        }

        /// <summary>
        /// Creates a snapshot of the blob.
        /// </summary>
        /// <param name="metadata">A collection of name-value pairs defining the metadata of the snapshot.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A blob snapshot.</returns>
        cloud_blob create_snapshot(cloud_metadata metadata, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return create_snapshot_async(std::move(metadata), condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to create a snapshot of the blob.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::cloud_blob" /> that represents the current operation.</returns>
        pplx::task<azure::storage::cloud_blob> create_snapshot_async()
        {
            return create_snapshot_async(cloud_metadata(), access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to create a snapshot of the blob.
        /// </summary>
        /// <param name="metadata">A collection of name-value pairs defining the metadata of the snapshot.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="azure::storage::cloud_blob" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<azure::storage::cloud_blob> create_snapshot_async(cloud_metadata metadata, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Gets the <see cref="azure::storage::cloud_blob_client" /> object that represents the Blob service.
        /// </summary>
        /// <returns>A client object that specifies the Blob service endpoint.</returns>
        const cloud_blob_client& service_client() const
        {
            return m_container.service_client();
        }

        /// <summary>
        /// Gets an <see cref="azure::storage::cloud_blob_container" /> object representing the blob's container.
        /// </summary>
        /// <returns>The blob's container.</returns>
        const cloud_blob_container& container() const
        {
            return m_container;
        }

        /// <summary>
        /// Gets the blob's system properties.
        /// </summary>
        /// <returns>The blob's properties.</returns>
        cloud_blob_properties& properties()
        {
            return *m_properties;
        }

        /// <summary>
        /// Gets the blob's system properties.
        /// </summary>
        /// <returns>The blob's properties.</returns>
        const cloud_blob_properties& properties() const
        {
            return *m_properties;
        }

        /// <summary>
        /// Gets the user-defined metadata for the blob.
        /// </summary>
        /// <returns>The blob's metadata, as a collection of name-value pairs.</returns>
        cloud_metadata& metadata()
        {
            return *m_metadata;
        }

        /// <summary>
        /// Gets the user-defined metadata for the blob.
        /// </summary>
        /// <returns>The blob's metadata, as a collection of name-value pairs.</returns>
        const cloud_metadata& metadata() const
        {
            return *m_metadata;
        }

        /// <summary>
        /// Gets the date and time that the blob snapshot was taken, if this blob is a snapshot.
        /// </summary>
        /// <returns>The blob's snapshot time, if the blob is a snapshot; otherwise, returns an empty string.</returns>
        const utility::string_t& snapshot_time() const
        {
            return m_snapshot_time;
        }

        /// <summary>
        /// Gets a value indicating whether this blob is a snapshot.
        /// </summary>
        /// <returns><c>true</c> if this blob is a snapshot; otherwise, <c>false</c>.</returns>
        bool is_snapshot() const
        {
            return !m_snapshot_time.empty();
        }

        /// <summary>
        /// Gets the state of the most recent or pending copy operation.
        /// </summary>
        /// <returns>An <see cref="azure::storage::copy_state" /> object containing the copy state.</returns>
        const azure::storage::copy_state& copy_state() const
        {
            return *m_copy_state;
        }

        /// <summary>
        /// Gets the blob URI for all locations.
        /// </summary>
        /// <returns>An <see cref="azure::storage::storage_uri" /> containing the blob URI for all locations.</returns>
        const storage_uri& uri() const
        {
            return m_uri;
        }

        /// <summary>
        /// Gets the blob's name.
        /// </summary>
        /// <returns>The blob's name.</returns>
        const utility::string_t& name() const
        {
            return m_name;
        }

        /// <summary>
        /// Gets the type of the blob.
        /// </summary>
        /// <returns>The type of the blob.</returns>
        blob_type type() const
        {
            return m_properties->type();
        }

        /// <summary>
        /// Indicates whether the <see cref="azure::storage::cloud_blob" /> object is valid.
        /// </summary>
        /// <returns><c>true</c> if the <see cref="azure::storage::cloud_blob" /> object is valid; otherwise, <c>false</c>.</returns>
        bool is_valid() const
        {
            return !m_name.empty();
        }

    protected:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob" /> class.
        /// </summary>
        /// <param name="name">The name of the blob.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <param name="container">A reference to the parent container.</param>
        WASTORAGE_API cloud_blob(utility::string_t name, utility::string_t snapshot_time, cloud_blob_container container);

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_blob" /> class.
        /// </summary>
        /// <param name="name">The name of the blob.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <param name="container">A reference to the parent container.</param>
        /// <param name="properties">A set of properties for the blob.</param>
        /// <param name="metadata">User-defined metadata for the blob.</param>
        /// <param name="copy_state">the state of the most recent or pending copy operation.</param>
        WASTORAGE_API cloud_blob(utility::string_t name, utility::string_t snapshot_time, cloud_blob_container container, cloud_blob_properties properties, cloud_metadata metadata, azure::storage::copy_state copy_state);

        /// <summary>
        /// Initiates an asynchronous operation to begin to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <param name="tier">An enum that represents the <see cref="azure::storage::premium_blob_tier" /> for the destination blob.</param>
        /// <param name="source_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the source blob.</param>
        /// <param name="destination_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        WASTORAGE_API pplx::task<utility::string_t> start_copy_async_impl(const web::http::uri& source, const premium_blob_tier tier, const access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context);

        void assert_no_snapshot() const;

        void set_type(blob_type value)
        {
            m_properties->set_type(value);
        }

        utility::string_t get_premium_access_tier_string(const premium_blob_tier tier);

        std::shared_ptr<cloud_blob_properties> m_properties;
        std::shared_ptr<cloud_metadata> m_metadata;
        std::shared_ptr<azure::storage::copy_state> m_copy_state;

    private:

        void init(utility::string_t snapshot_time, storage_credentials credentials);
        WASTORAGE_API pplx::task<bool> exists_async(bool primary_only, const blob_request_options& options, operation_context context);
        WASTORAGE_API pplx::task<void> download_single_range_to_stream_async(concurrency::streams::ostream target, utility::size64_t offset, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context, bool update_properties = false);

        utility::string_t m_name;
        utility::string_t m_snapshot_time;
        cloud_blob_container m_container;
        storage_uri m_uri;

        friend class cloud_blob_container;
        friend class cloud_blob_directory;
        friend class list_blob_item;
    };

    /// <summary>
    /// Represents a blob that is uploaded as a set of blocks.
    /// </summary>
    class cloud_block_blob : public cloud_blob
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_block_blob" /> class.
        /// </summary>
        cloud_block_blob()
            : cloud_blob()
        {
            set_type(blob_type::block_blob);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_block_blob" /> class using an absolute URI to the blob.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the blob for all locations.</param>
        explicit cloud_block_blob(storage_uri uri)
            : cloud_blob(std::move(uri))
        {
            set_type(blob_type::block_blob);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_block_blob" /> class using an absolute URI to the blob.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the blob for all locations.</param>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        cloud_block_blob(storage_uri uri, storage_credentials credentials)
            : cloud_blob(std::move(uri), std::move(credentials))
        {
            set_type(blob_type::block_blob);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_block_blob" /> class using an absolute URI to the blob.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the blob for all locations.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        cloud_block_blob(storage_uri uri, utility::string_t snapshot_time, storage_credentials credentials)
            : cloud_blob(std::move(uri), std::move(snapshot_time), std::move(credentials))
        {
            set_type(blob_type::block_blob);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_block_blob" /> class.
        /// </summary>
        /// <param name="blob">Reference to the blob.</param>
        cloud_block_blob(const cloud_blob& blob)
            : cloud_blob(blob)
        {
            set_type(blob_type::block_blob);
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+,
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_block_blob" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_block_blob" /> object.</param>
        cloud_block_blob(cloud_block_blob&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_block_blob" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_block_blob" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::cloud_block_blob" /> object with properties set.</returns>
        cloud_block_blob& operator=(cloud_block_blob&& other)
        {
            if (this != &other)
            {
                cloud_blob::operator=(other);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Opens a stream for writing to the block blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <returns>A stream to be used for writing to the blob.</returns>
        concurrency::streams::ostream open_write()
        {
            return open_write_async().get();
        }

        /// <summary>
        /// Opens a stream for writing to the block blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A stream to be used for writing to the blob.</returns>
        /// <para>To avoid overwriting and instead throw an error if the blob exists, please pass in an <see cref="azure::storage::access_condition"/>
        /// parameter generated using <see cref="azure::storage::access_condition::generate_if_not_exists_condition"/></para>
        concurrency::streams::ostream open_write(const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return open_write_async(condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to open a stream for writing to the block blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="concurrency::streams::ostream" /> that represents the current operation.</returns>
        pplx::task<concurrency::streams::ostream> open_write_async()
        {
            return open_write_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to open a stream for writing to the block blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="concurrency::streams::ostream" /> that represents the current operation.</returns>
        /// <para>To avoid overwriting and instead throw an error if the blob exists, please pass in an <see cref="azure::storage::access_condition"/>
        /// parameter generated using <see cref="azure::storage::access_condition::generate_if_not_exists_condition"/></para>
        WASTORAGE_API pplx::task<concurrency::streams::ostream> open_write_async(const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Returns an enumerable collection of the blob's blocks, using the specified block list filter.
        /// </summary>
        /// <returns>An enumerable collection of objects implementing <see cref="azure::storage::block_list_item" />.</returns>
        std::vector<block_list_item> download_block_list() const
        {
            return download_block_list_async().get();
        }

        /// <summary>
        /// Returns an enumerable collection of the blob's blocks, using the specified block list filter.
        /// </summary>
        /// <param name="listing_filter">One of the enumeration values that indicates whether to return
        /// committed blocks, uncommitted blocks, or both.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An enumerable collection of objects implementing <see cref="azure::storage::block_list_item" />.</returns>
        std::vector<block_list_item> download_block_list(block_listing_filter listing_filter, const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            return download_block_list_async(listing_filter, condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to return an enumerable collection of the blob's blocks, 
        /// using the specified block list filter.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="azure::storage::block_list_item" />, that represents the current operation.</returns>
        pplx::task<std::vector<block_list_item>> download_block_list_async() const
        {
            return download_block_list_async(block_listing_filter::committed, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to return an enumerable collection of the blob's blocks, 
        /// using the specified block list filter.
        /// </summary>
        /// <param name="listing_filter">One of the enumeration values that indicates whether to return
        /// committed blocks, uncommitted blocks, or both.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="azure::storage::block_list_item" />, that represents the current operation.</returns>
        WASTORAGE_API pplx::task<std::vector<block_list_item>> download_block_list_async(block_listing_filter listing_filter, const access_condition& condition, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Downloads the blob's contents as a string.
        /// </summary>
        /// <returns>The contents of the blob, as a string.</returns>
        utility::string_t download_text()
        {
            return download_text_async().get();
        }

        /// <summary>
        /// Downloads the blob's contents as a string.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The contents of the blob, as a string.</returns>
        utility::string_t download_text(const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return download_text_async(condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to download the blob's contents as a string.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        pplx::task<utility::string_t> download_text_async()
        {
            return download_text_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to download the blob's contents as a string.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<utility::string_t> download_text_async(const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Sets standard account's blob tier.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::standard_blob_tier" /> enum that represents the blob tier to be set.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        void set_standard_blob_tier(const standard_blob_tier tier, const access_condition & condition, const blob_request_options & options, operation_context context)
        {
            set_standard_blob_tier_async(tier, condition, options, context).wait();
        }
        
        /// <summary>
        /// Initiates an asynchronous operation to set standard account's blob tier.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::standard_blob_tier" /> enum that represents the blob tier to be set.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> set_standard_blob_tier_async(const standard_blob_tier tier, const access_condition & condition, const blob_request_options & options, operation_context context);

        /// <summary>
        /// Uploads a single block.
        /// </summary>
        /// <param name="block_id">A Base64-encoded block ID that identifies the block.</param>
        /// <param name="block_data">A stream that provides the data for the block.</param>
        /// <param name="content_md5">An optional hash value that will be used to set the Content-MD5 property
        /// on the blob. May be an empty string.</param>
        void upload_block(const utility::string_t& block_id, concurrency::streams::istream block_data, const utility::string_t& content_md5) const
        {
            upload_block_async(block_id, block_data, content_md5).wait();
        }

        /// <summary>
        /// Uploads a single block.
        /// </summary>
        /// <param name="block_id">A Base64-encoded block ID that identifies the block.</param>
        /// <param name="block_data">A stream that provides the data for the block.</param>
        /// <param name="content_md5">An optional hash value that will be used to set the Content-MD5 property
        /// on the blob. May be an empty string.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_block(const utility::string_t& block_id, concurrency::streams::istream block_data, const utility::string_t& content_md5, const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            upload_block_async(block_id, block_data, content_md5, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a single block.
        /// </summary>
        /// <param name="block_id">A Base64-encoded block ID that identifies the block.</param>
        /// <param name="block_data">A stream that provides the data for the block.</param>
        /// <param name="content_md5">An optional hash value that will be used to set the Content-MD5 property
        /// on the blob. May be an empty string.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_block_async(const utility::string_t& block_id, concurrency::streams::istream block_data, const utility::string_t& content_md5) const
        {
            return upload_block_async(block_id, block_data, content_md5, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a single block.
        /// </summary>
        /// <param name="block_id">A Base64-encoded block ID that identifies the block.</param>
        /// <param name="block_data">A stream that provides the data for the block.</param>
        /// <param name="content_md5">An optional hash value that will be used to set the Content-MD5 property
        /// on the blob. May be an empty string.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_block_async(const utility::string_t& block_id, concurrency::streams::istream block_data, const utility::string_t& content_md5, const access_condition& condition, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Uploads a list of blocks to a new or existing blob. 
        /// </summary>
        /// <param name="block_list">An enumerable collection of block IDs, as Base64-encoded strings.</param>
        void upload_block_list(const std::vector<block_list_item>& block_list)
        {
            upload_block_list_async(block_list).wait();
        }

        /// <summary>
        /// Uploads a list of blocks to a new or existing blob. 
        /// </summary>
        /// <param name="block_list">An enumerable collection of block IDs, as Base64-encoded strings.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_block_list(const std::vector<block_list_item>& block_list, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_block_list_async(block_list, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a list of blocks to a new or existing blob. 
        /// </summary>
        /// <param name="block_list">An enumerable collection of block IDs, as Base64-encoded strings.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_block_list_async(const std::vector<block_list_item>& block_list)
        {
            return upload_block_list_async(block_list, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a list of blocks to a new or existing blob. 
        /// </summary>
        /// <param name="block_list">An enumerable collection of block IDs, as Base64-encoded strings.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_block_list_async(const std::vector<block_list_item>& block_list, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Uploads a stream to a block blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        void upload_from_stream(concurrency::streams::istream source)
        {
            upload_from_stream_async(source).wait();
        }

        /// <summary>
        /// Uploads a stream to a block blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_from_stream(concurrency::streams::istream source, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_from_stream_async(source, condition, options, context).wait();
        }

        /// <summary>
        /// Uploads a stream to a block blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        void upload_from_stream(concurrency::streams::istream source, utility::size64_t length)
        {
            upload_from_stream_async(source, length).wait();
        }

        /// <summary>
        /// Uploads a stream to a block blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_from_stream(concurrency::streams::istream source, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_from_stream_async(source, length, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a stream to a block blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_from_stream_async(concurrency::streams::istream source)
        {
            return upload_from_stream_async(source, std::numeric_limits<utility::size64_t>::max());
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a stream to a block blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_from_stream_async(concurrency::streams::istream source, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return upload_from_stream_async(source, std::numeric_limits<utility::size64_t>::max(), condition, options, context);
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a stream to a block blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_from_stream_async(concurrency::streams::istream source, utility::size64_t length)
        {
            return upload_from_stream_async(source, length, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a stream to a block blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_from_stream_async(concurrency::streams::istream source, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Uploads a file to a block blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="path">The file providing the blob content.</param>
        void upload_from_file(const utility::string_t &path)
        {
            upload_from_file_async(path).wait();
        }

        /// <summary>
        /// Uploads a file to a block blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="path">The file providing the blob content.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_from_file(const utility::string_t &path, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_from_file_async(path, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a file to a block blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="path">The file providing the blob content.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_from_file_async(const utility::string_t &path)
        {
            return upload_from_file_async(path, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a file to a block blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="path">The file providing the blob content.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_from_file_async(const utility::string_t &path, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Uploads a string of text to a blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="content">A string containing the text to upload.</param>
        void upload_text(const utility::string_t& content)
        {
            upload_text_async(content).wait();
        }

        /// <summary>
        /// Uploads a string of text to a blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="content">A string containing the text to upload.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_text(const utility::string_t& content, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_text_async(content, condition, options, context).wait();
        }

        /// <summary>
        /// Uploads a string of text to a blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="content">A string containing the text to upload.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_text_async(const utility::string_t& content)
        {
            return upload_text_async(content, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Uploads a string of text to a blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="content">A string containing the text to upload.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_text_async(const utility::string_t& content, const access_condition& condition, const blob_request_options& options, operation_context context);

    private:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_block_blob" /> class.
        /// </summary>
        /// <param name="name">The name of the blob.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <param name="container">An <see cref="azure::storage::cloud_blob_container" /> object.</param>
        cloud_block_blob(utility::string_t name, utility::string_t snapshot_time, cloud_blob_container container)
            : cloud_blob(std::move(name), std::move(snapshot_time), std::move(container))
        {
            set_type(blob_type::block_blob);
        }

        friend class cloud_blob_container;
        friend class cloud_blob_directory;
};

    /// <summary>
    /// Represents a Windows Azure page blob.
    /// </summary>
    class cloud_page_blob : public cloud_blob
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_page_blob" /> class.
        /// </summary>
        cloud_page_blob()
            : cloud_blob()
        {
            set_type(blob_type::page_blob);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_page_blob" /> class using an absolute URI to the blob.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the blob for all locations.</param>
        explicit cloud_page_blob(storage_uri uri)
            : cloud_blob(std::move(uri))
        {
            set_type(blob_type::page_blob);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_page_blob" /> class using an absolute URI to the blob.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the blob for all locations.</param>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        cloud_page_blob(storage_uri uri, storage_credentials credentials)
            : cloud_blob(std::move(uri), std::move(credentials))
        {
            set_type(blob_type::page_blob);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_page_blob" /> class using an absolute URI to the blob.
        /// </summary>
        /// <param name="uri">An <see cref="azure::storage::storage_uri" /> object containing the absolute URI to the blob for all locations.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <param name="credentials">The <see cref="azure::storage::storage_credentials" /> to use.</param>
        cloud_page_blob(storage_uri uri, utility::string_t snapshot_time, storage_credentials credentials)
            : cloud_blob(std::move(uri), std::move(snapshot_time), std::move(credentials))
        {
            set_type(blob_type::page_blob);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_page_blob" /> class.
        /// </summary>
        /// <param name="blob">Reference to the blob.</param>
        cloud_page_blob(const cloud_blob& blob)
            : cloud_blob(blob)
        {
            set_type(blob_type::page_blob);
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+,
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_page_blob" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_page_blob" /> object.</param>
        cloud_page_blob(cloud_page_blob&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::cloud_page_blob" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::cloud_page_blob" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::cloud_page_blob" /> object with properties set.</returns>
        cloud_page_blob& operator=(cloud_page_blob&& other)
        {
            if (this != &other)
            {
                cloud_blob::operator=(other);
            }
            return *this;
        }
#endif

        /// <summary>
        /// Opens a stream for writing to an existing page blob.
        /// </summary>
        /// <returns>A stream to be used for writing to the blob.</returns>
        concurrency::streams::ostream open_write()
        {
            return open_write_async().get();
        }

        /// <summary>
        /// Opens a stream for writing to an existing page blob.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A stream to be used for writing to the blob.</returns>
        concurrency::streams::ostream open_write(const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return open_write_async(condition, options, context).get();
        }

        /// <summary>
        /// Opens a stream for writing to a new page blob.
        /// </summary>
        /// <param name="size">The size of the write operation, in bytes. The size must be a multiple of 512.</param>
        /// <returns>A stream to be used for writing to the blob.</returns>
        concurrency::streams::ostream open_write(utility::size64_t size)
        {
            return open_write_async(size).get();
        }

        /// <summary>
        /// Opens a stream for writing to a new page blob.
        /// </summary>
        /// <param name="size">The size of the write operation, in bytes. The size must be a multiple of 512.</param>
        /// <param name="sequence_number">A user-controlled number to track request sequence, whose value must be between 0 and 2^63 - 1.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A stream to be used for writing to the blob.</returns>
        concurrency::streams::ostream open_write(utility::size64_t size, int64_t sequence_number, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return open_write_async(size, sequence_number, condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to open a stream for writing to an existing page blob.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="concurrency::streams::ostream" /> that represents the current operation.</returns>
        pplx::task<concurrency::streams::ostream> open_write_async()
        {
            return open_write_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to open a stream for writing to an existing page blob.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="concurrency::streams::ostream" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<concurrency::streams::ostream> open_write_async(const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Initiates an asynchronous operation to open a stream for writing to a new page blob.
        /// </summary>
        /// <param name="size">The size of the write operation, in bytes. The size must be a multiple of 512.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="concurrency::streams::ostream" /> that represents the current operation.</returns>
        pplx::task<concurrency::streams::ostream> open_write_async(utility::size64_t size)
        {
            return open_write_async(size, 0, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to open a stream for writing to a new page blob.
        /// </summary>
        /// <param name="size">The size of the write operation, in bytes. The size must be a multiple of 512.</param>
        /// <param name="sequence_number">A user-controlled number to track request sequence, whose value must be between 0 and 2^63 - 1.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="concurrency::streams::ostream" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<concurrency::streams::ostream> open_write_async(utility::size64_t size, int64_t sequence_number, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Clears pages from a page blob.
        /// </summary>
        /// <param name="start_offset">The offset at which to begin clearing pages, in bytes. The offset must be a multiple of 512.</param>
        /// <param name="length">The length of the data range to be cleared, in bytes. The length must be a multiple of 512.</param>
        void clear_pages(int64_t start_offset, int64_t length)
        {
            clear_pages_async(start_offset, length).wait();
        }

        /// <summary>
        /// Clears pages from a page blob.
        /// </summary>
        /// <param name="start_offset">The offset at which to begin clearing pages, in bytes. The offset must be a multiple of 512.</param>
        /// <param name="length">The length of the data range to be cleared, in bytes. The length must be a multiple of 512.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void clear_pages(int64_t start_offset, int64_t length, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            clear_pages_async(start_offset, length, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to clear pages from a page blob.
        /// </summary>
        /// <param name="start_offset">The offset at which to begin clearing pages, in bytes. The offset must be a multiple of 512.</param>
        /// <param name="length">The length of the data range to be cleared, in bytes. The length must be a multiple of 512.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> clear_pages_async(int64_t start_offset, int64_t length)
        {
            return clear_pages_async(start_offset, length, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to clear pages from a page blob.
        /// </summary>
        /// <param name="start_offset">The offset at which to begin clearing pages, in bytes. The offset must be a multiple of 512.</param>
        /// <param name="length">The length of the data range to be cleared, in bytes. The length must be a multiple of 512.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> clear_pages_async(int64_t start_offset, int64_t length, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Gets a collection of valid page ranges and their starting and ending bytes.
        /// </summary>
        /// <returns>An enumerable collection of page ranges.</returns>
        std::vector<page_range> download_page_ranges() const
        {
            return download_page_ranges_async().get();
        }

        /// <summary>
        /// Gets a collection of valid page ranges and their starting and ending bytes.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An enumerable collection of page ranges.</returns>
        std::vector<page_range> download_page_ranges(const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            return download_page_ranges_async(condition, options, context).get();
        }

        /// <summary>
        /// Gets a collection of valid page ranges and their starting and ending bytes.
        /// </summary>
        /// <param name="offset">The starting offset of the data range over which to list page ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="length">The length of the data range over which to list page ranges, in bytes. Must be a multiple of 512.</param>
        /// <returns>An enumerable collection of page ranges.</returns>
        std::vector<page_range> download_page_ranges(utility::size64_t offset, utility::size64_t length) const
        {
            return download_page_ranges_async(offset, length).get();
        }

        /// <summary>
        /// Gets a collection of valid page ranges and their starting and ending bytes.
        /// </summary>
        /// <param name="offset">The starting offset of the data range over which to list page ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="length">The length of the data range over which to list page ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An enumerable collection of page ranges.</returns>
        std::vector<page_range> download_page_ranges(utility::size64_t offset, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            return download_page_ranges_async(offset, length, condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to get a collection of valid page ranges and their starting and ending bytes.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="azure::storage::page_range" />, that represents the current operation.</returns>
        pplx::task<std::vector<page_range>> download_page_ranges_async() const
        {
            return download_page_ranges_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to get a collection of valid page ranges and their starting and ending bytes.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="azure::storage::page_range" />, that represents the current operation.</returns>
        pplx::task<std::vector<page_range>> download_page_ranges_async(const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            return download_page_ranges_async(std::numeric_limits<utility::size64_t>::max(), 0, condition, options, context);
        }

        /// <summary>
        /// Initiates an asynchronous operation to get a collection of valid page ranges and their starting and ending bytes.
        /// </summary>
        /// <param name="offset">The starting offset of the data range over which to list page ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="length">The length of the data range over which to list page ranges, in bytes. Must be a multiple of 512.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="azure::storage::page_range" />, that represents the current operation.</returns>
        pplx::task<std::vector<page_range>> download_page_ranges_async(utility::size64_t offset, utility::size64_t length) const
        {
            return download_page_ranges_async(offset, length, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to get a collection of valid page ranges and their starting and ending bytes.
        /// </summary>
        /// <param name="offset">The starting offset of the data range over which to list page ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="length">The length of the data range over which to list page ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="azure::storage::page_range" />, that represents the current operation.</returns>
        WASTORAGE_API pplx::task<std::vector<page_range>> download_page_ranges_async(utility::size64_t offset, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Gets a collection of valid page ranges and their starting and ending bytes, only pages that were changed between target blob and previous snapshot.
        /// </summary>
        /// <param name="previous_snapshot_time">An snapshot time that represents previous snapshot.</param>
        /// <returns>An enumerable collection of page diff ranges.</returns>
        std::vector<page_diff_range> download_page_ranges_diff(utility::string_t previous_snapshot_time) const
        {
            return download_page_ranges_diff_async(previous_snapshot_time).get();
        }

        /// <summary>
        /// Gets a collection of valid page ranges and their starting and ending bytes, only pages that were changed between target blob and previous snapshot.
        /// </summary>
        /// <param name="previous_snapshot_time">An snapshot time that represents previous snapshot.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An enumerable collection of page diff ranges.</returns>
        std::vector<page_diff_range> download_page_ranges_diff(utility::string_t previous_snapshot_time, const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            return download_page_ranges_diff_async(previous_snapshot_time, condition, options, context).get();
        }

        /// <summary>
        /// Gets a collection of valid page ranges and their starting and ending bytes, only pages that were changed between target blob and previous snapshot.
        /// </summary>
        /// <param name="previous_snapshot_time">An snapshot time that represents previous snapshot.</param>
        /// <param name="offset">The starting offset of the data range over which to list page ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="length">The length of the data range over which to list page ranges, in bytes. Must be a multiple of 512.</param>
        /// <returns>An enumerable collection of page diff ranges.</returns>
        std::vector<page_diff_range> download_page_ranges_diff(utility::string_t previous_snapshot_time, utility::size64_t offset, utility::size64_t length) const
        {
            return download_page_ranges_diff_async(previous_snapshot_time, offset, length).get();
        }

        /// <summary>
        /// Gets a collection of valid page ranges and their starting and ending bytes, only pages that were changed between target blob and previous snapshot.
        /// </summary>
        /// <param name="previous_snapshot_time">An snapshot time that represents previous snapshot.</param>
        /// <param name="offset">The starting offset of the data range over which to list page ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="length">The length of the data range over which to list page ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>An enumerable collection of page diff ranges.</returns>
        std::vector<page_diff_range> download_page_ranges_diff(utility::string_t previous_snapshot_time, utility::size64_t offset, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            return download_page_ranges_diff_async(previous_snapshot_time, offset, length, condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to get a collection of valid page ranges and their starting and ending bytes, only pages that were changed between target blob and previous snapshot.
        /// </summary>
        /// <param name="previous_snapshot_time">An snapshot time that represents previous snapshot.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="azure::storage::page_diff_range" />, that represents the current operation.</returns>
        pplx::task<std::vector<page_diff_range>> download_page_ranges_diff_async(utility::string_t previous_snapshot_time) const
        {
            return download_page_ranges_diff_async(previous_snapshot_time, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to get a collection of valid page ranges and their starting and ending bytes, only pages that were changed between target blob and previous snapshot.
        /// </summary>
        /// <param name="previous_snapshot_time">An snapshot time that represents previous snapshot.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="azure::storage::page_diff_range" />, that represents the current operation.</returns>
        pplx::task<std::vector<page_diff_range>> download_page_ranges_diff_async(utility::string_t previous_snapshot_time, const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            return download_page_ranges_diff_async(previous_snapshot_time, std::numeric_limits<utility::size64_t>::max(), 0, condition, options, context);
        }

        /// <summary>
        /// Initiates an asynchronous operation to get a collection of valid page ranges and their starting and ending bytes, only pages that were changed between target blob and previous snapshot.
        /// </summary>
        /// <param name="previous_snapshot_time">An snapshot time that represents previous snapshot.</param>
        /// <param name="offset">The starting offset of the data range over which to list page ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="length">The length of the data range over which to list page ranges, in bytes. Must be a multiple of 512.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="azure::storage::page_diff_range" />, that represents the current operation.</returns>
        pplx::task<std::vector<page_diff_range>> download_page_ranges_diff_async(utility::string_t previous_snapshot_time, utility::size64_t offset, utility::size64_t length) const
        {
            return download_page_ranges_diff_async(previous_snapshot_time, offset, length, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to get a collection of valid page ranges and their starting and ending bytes, only pages that were changed between target blob and previous snapshot.
        /// </summary>
        /// <param name="previous_snapshot_time">An snapshot time that represents previous snapshot.</param>
        /// <param name="offset">The starting offset of the data range over which to list page ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="length">The length of the data range over which to list page ranges, in bytes. Must be a multiple of 512.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="std::vector" />, of type <see cref="azure::storage::page_diff_range" />, that represents the current operation.</returns>
        WASTORAGE_API pplx::task<std::vector<page_diff_range>> download_page_ranges_diff_async(utility::string_t previous_snapshot_time, utility::size64_t offset, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context) const;


        /// <summary>
        /// Writes pages to a page blob.
        /// </summary>
        /// <param name="page_data">A stream providing the page data.</param>
        /// <param name="start_offset">The offset at which to begin writing, in bytes. The offset must be a multiple of 512.</param>
        /// <param name="content_md5">An optional hash value that will be used to set the Content-MD5 property
        /// on the blob. May be an empty string.</param>
        void upload_pages(concurrency::streams::istream page_data, int64_t start_offset, const utility::string_t& content_md5)
        {
            upload_pages_async(page_data, start_offset, content_md5).wait();
        }

        /// <summary>
        /// Writes pages to a page blob.
        /// </summary>
        /// <param name="page_data">A stream providing the page data.</param>
        /// <param name="start_offset">The offset at which to begin writing, in bytes. The offset must be a multiple of 512.</param>
        /// <param name="content_md5">An optional hash value that will be used to set the Content-MD5 property
        /// on the blob. May be an empty string.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_pages(concurrency::streams::istream page_data, int64_t start_offset, const utility::string_t& content_md5, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_pages_async(page_data, start_offset, content_md5, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to write pages to a page blob.
        /// </summary>
        /// <param name="source">A stream providing the page data.</param>
        /// <param name="start_offset">The offset at which to begin writing, in bytes. The offset must be a multiple of 512.</param>
        /// <param name="content_md5">An optional hash value that will be used to set the Content-MD5 property
        /// on the blob. May be an empty string.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_pages_async(concurrency::streams::istream source, int64_t start_offset, const utility::string_t& content_md5)
        {
            return upload_pages_async(source, start_offset, content_md5, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to write pages to a page blob.
        /// </summary>
        /// <param name="source">A stream providing the page data.</param>
        /// <param name="start_offset">The offset at which to begin writing, in bytes. The offset must be a multiple of 512.</param>
        /// <param name="content_md5">An optional hash value that will be used to set the Content-MD5 property
        /// on the blob. May be an empty string.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_pages_async(concurrency::streams::istream source, int64_t start_offset, const utility::string_t& content_md5, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Uploads a stream to a page blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        void upload_from_stream(concurrency::streams::istream source)
        {
            upload_from_stream_async(source).wait();
        }

        /// <summary>
        /// Uploads a stream to a page blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="sequence_number">A user-controlled number to track request sequence, whose value must be between 0 and 2^63 - 1.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_from_stream(concurrency::streams::istream source, int64_t sequence_number, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_from_stream_async(source, sequence_number, condition, options, context).wait();
        }

        /// <summary>
        /// Uploads a stream to a page blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        void upload_from_stream(concurrency::streams::istream source, utility::size64_t length)
        {
            upload_from_stream_async(source, length).wait();
        }

        /// <summary>
        /// Uploads a stream to a page blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        /// <param name="sequence_number">A user-controlled number to track request sequence, whose value must be between 0 and 2^63 - 1.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_from_stream(concurrency::streams::istream source, utility::size64_t length, int64_t sequence_number, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_from_stream_async(source, length, sequence_number, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a stream to a page blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_from_stream_async(concurrency::streams::istream source)
        {
            return upload_from_stream_async(source, std::numeric_limits<utility::size64_t>::max());
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a stream to a page blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="sequence_number">A user-controlled number to track request sequence, whose value must be between 0 and 2^63 - 1.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_from_stream_async(concurrency::streams::istream source, int64_t sequence_number, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return upload_from_stream_async(source, std::numeric_limits<utility::size64_t>::max(), sequence_number, condition, options, context);
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a stream to a page blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_from_stream_async(concurrency::streams::istream source, utility::size64_t length)
        {
            return upload_from_stream_async(source, length, 0, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a stream to a page blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        /// <param name="sequence_number">A user-controlled number to track request sequence, whose value must be between 0 and 2^63 - 1.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_from_stream_async(concurrency::streams::istream source, utility::size64_t length, int64_t sequence_number, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Uploads a file to a page blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="path">The file providing the blob content.</param>
        void upload_from_file(const utility::string_t &path)
        {
            upload_from_file_async(path).wait();
        }

        /// <summary>
        /// Uploads a file to a page blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="path">The file providing the blob content.</param>
        /// <param name="sequence_number">A user-controlled number to track request sequence, whose value must be between 0 and 2^63 - 1.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void upload_from_file(const utility::string_t &path, int64_t sequence_number, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_from_file_async(path, sequence_number, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a file to a page blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="path">The file providing the blob content.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_from_file_async(const utility::string_t &path)
        {
            return upload_from_file_async(path, 0, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a file to a page blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="path">The file providing the blob content.</param>
        /// <param name="sequence_number">A user-controlled number to track request sequence, whose value must be between 0 and 2^63 - 1.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> upload_from_file_async(const utility::string_t &path, int64_t sequence_number, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Creates a page blob.
        /// </summary>
        /// <param name="size">The maximum size of the page blob, in bytes.</param>
        void create(utility::size64_t size)
        {
            create_async(size).wait();
        }

        /// <summary>
        /// Creates a page blob.
        /// </summary>
        /// <param name="size">The maximum size of the page blob, in bytes.</param>
        /// <param name="sequence_number">A user-controlled number to track request sequence, whose value must be between 0 and 2^63 - 1.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void create(utility::size64_t size, int64_t sequence_number, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            create_async(size, sequence_number, condition, options, context).wait();
        }

        /// <summary>
        /// Creates a page blob.
        /// </summary>
        /// <param name="size">The maximum size of the page blob, in bytes.</param>
        /// <param name="tier">A <see cref="azure::storage::premium_blob_tier" /> enum that represents the tier of the page blob to be created.</param>
        /// <param name="sequence_number">A user-controlled number to track request sequence, whose value must be between 0 and 2^63 - 1.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void create(utility::size64_t size, const premium_blob_tier tier, int64_t sequence_number, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            create_async(size, tier, sequence_number, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to create a page blob.
        /// </summary>
        /// <param name="size">The maximum size of the page blob, in bytes.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> create_async(utility::size64_t size)
        {
            return create_async(size, 0, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to create a page blob.
        /// </summary>
        /// <param name="size">The maximum size of the page blob, in bytes.</param>
        /// <param name="sequence_number">A user-controlled number to track request sequence, whose value must be between 0 and 2^63 - 1.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> create_async(utility::size64_t size, int64_t sequence_number, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return create_async(size, premium_blob_tier::unknown, sequence_number, condition, options, context);
        }

        /// <summary>
        /// Initiates an asynchronous operation to create a page blob.
        /// </summary>
        /// <param name="size">The maximum size of the page blob, in bytes.</param>
        /// <param name="tier">A <see cref="azure::storage::premium_blob_tier" /> object that represents the tier of the page blob to be created.</param>
        /// <param name="sequence_number">A user-controlled number to track request sequence, whose value must be between 0 and 2^63 - 1.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> create_async(utility::size64_t size, const premium_blob_tier tier, int64_t sequence_number, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Resizes the page blob to the specified size.
        /// </summary>
        /// <param name="size">The size of the page blob, in bytes.</param>
        void resize(utility::size64_t size)
        {
            resize_async(size).wait();
        }

        /// <summary>
        /// Resizes the page blob to the specified size.
        /// </summary>
        /// <param name="size">The size of the page blob, in bytes.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void resize(utility::size64_t size, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            resize_async(size, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to resize the page blob to the specified size.
        /// </summary>
        /// <param name="size">The size of the page blob, in bytes.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> resize_async(utility::size64_t size)
        {
            return resize_async(size, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to resize the page blob to the specified size.
        /// </summary>
        /// <param name="size">The size of the page blob, in bytes.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> resize_async(utility::size64_t size, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Sets the page blob's sequence number.
        /// </summary>
        /// <param name="sequence_number">A value of type <see cref="azure::storage::sequence_number" />, indicating the operation to perform on the sequence number.</param>
        void set_sequence_number(const azure::storage::sequence_number& sequence_number)
        {
            set_sequence_number_async(sequence_number).wait();
        }

        /// <summary>
        /// Sets the page blob's sequence number.
        /// </summary>
        /// <param name="sequence_number">A value of type <see cref="azure::storage::sequence_number" />, indicating the operation to perform on the sequence number.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void set_sequence_number(const azure::storage::sequence_number& sequence_number, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            set_sequence_number_async(sequence_number, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to set the page blob's sequence number.
        /// </summary>
        /// <param name="sequence_number">A value of type <see cref="azure::storage::sequence_number" />, indicating the operation to perform on the sequence number.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> set_sequence_number_async(const azure::storage::sequence_number& sequence_number)
        {
            return set_sequence_number_async(sequence_number, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to set the page blob's sequence number.
        /// </summary>
        /// <param name="sequence_number">A value of type <see cref="azure::storage::sequence_number" />, indicating the operation to perform on the sequence number.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> set_sequence_number_async(const azure::storage::sequence_number& sequence_number, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Begin to copy a snapshot of the source page blob and metadata to a destination page blob.
        /// </summary>
        /// <param name="source">The source page blob object specified a snapshot.</param>
        /// <returns>The copy ID associated with the incremental copy operation.</returns>
        /// <remarks>
        /// The destination of an incremental copy must either not exist, or must have been created with a previous incremental copy from the same source blob.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_incremental_copy(const cloud_page_blob& source)
        {
            return start_incremental_copy_async(source).get();
        }

        /// <summary>
        /// Begin to copy a snapshot of the source page blob and metadata to a destination page blob.
        /// </summary>
        /// <param name="source">The URI of a snapshot of source page blob.</param>
        /// <returns>The copy ID associated with the incremental copy operation.</returns>
        /// <remarks>
        /// The destination of an incremental copy must either not exist, or must have been created with a previous incremental copy from the same source blob.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_incremental_copy(const web::http::uri& source)
        {
            return start_incremental_copy_async(source).get();
        }

        /// <summary>
        /// Begin to copy a snapshot of the source page blob and metadata to a destination page blob.
        /// </summary>
        /// <param name="source">The source page blob object specified a snapshot.</param>
        /// <param name="condition">An object that represents the <see cref="azure::storage::access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The copy ID associated with the incremental copy operation.</returns>
        /// <remarks>
        /// The destination of an incremental copy must either not exist, or must have been created with a previous incremental copy from the same source blob.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_incremental_copy(const cloud_page_blob& source, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return start_incremental_copy_async(source, condition, options, context).get();
        }

        /// <summary>
        /// Begin to copy a snapshot of the source page blob and metadata to a destination page blob.
        /// </summary>
        /// <param name="source">The URI of a snapshot of source page blob.</param>
        /// <param name="condition">An object that represents the <see cref="azure::storage::access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The copy ID associated with the incremental copy operation.</returns>
        /// <remarks>
        /// The destination of an incremental copy must either not exist, or must have been created with a previous incremental copy from the same source blob.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_incremental_copy(const web::http::uri& source, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return start_incremental_copy_async(source, condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to begin to copy a snapshot of the source page blob and metadata to a destination page blob.
        /// </summary>
        /// <param name="source">The source page blob object specified a snapshot.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// The destination of an incremental copy must either not exist, or must have been created with a previous incremental copy from the same source blob.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        pplx::task<utility::string_t> start_incremental_copy_async(const cloud_page_blob& source)
        {
            return start_incremental_copy_async(source, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to begin to copy a snapshot of the source page blob and metadata to a destination page blob.
        /// </summary>
        /// <param name="source">The URI of a snapshot of source page blob.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// The destination of an incremental copy must either not exist, or must have been created with a previous incremental copy from the same source blob.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        pplx::task<utility::string_t> start_incremental_copy_async(const web::http::uri& source)
        {
            return start_incremental_copy_async(source, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to begin to copy a snapshot of the source page blob and metadata to a destination page blob.
        /// </summary>
        /// <param name="source">The source page blob object specified a snapshot.</param>
        /// <param name="condition">An object that represents the <see cref="azure::storage::access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// The destination of an incremental copy must either not exist, or must have been created with a previous incremental copy from the same source blob.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        WASTORAGE_API pplx::task<utility::string_t> start_incremental_copy_async(const cloud_page_blob& source, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Initiates an asynchronous operation to begin to copy a snapshot of the source page blob and metadata to a destination page blob.
        /// </summary>
        /// <param name="source">The URI of a snapshot of source page blob.</param>
        /// <param name="condition">An object that represents the <see cref="azure::storage::access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// The destination of an incremental copy must either not exist, or must have been created with a previous incremental copy from the same source blob.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        WASTORAGE_API pplx::task<utility::string_t> start_incremental_copy_async(const web::http::uri& source, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Sets premium account's page blob tier.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::premium_blob_tier" /> enum that represents the blob tier to be set.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        void set_premium_blob_tier(const premium_blob_tier tier, const access_condition & condition, const blob_request_options & options, operation_context context)
        {
            set_premium_blob_tier_async(tier, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to set premium account's blob tier.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::premium_blob_tier" /> enum that represents the blob tier to be set.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> set_premium_blob_tier_async(const premium_blob_tier tier, const access_condition & condition, const blob_request_options & options, operation_context context);

        /// <summary>
        /// Begins an operation to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <param name="tier">An enum that represents the <see cref="azure::storage::premium_blob_tier" /> for the destination blob.</param>
        /// <param name="source_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the source blob.</param>
        /// <param name="destination_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The copy ID associated with the copy operation.</returns>
        /// <remarks>
        /// This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        utility::string_t start_copy(const web::http::uri& source, const azure::storage::premium_blob_tier tier, const access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context)
        {
            return start_copy_async(source, tier, source_condition, destination_condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to begin to copy a blob's contents, properties, and metadata to a new blob.
        /// </summary>
        /// <param name="source">The URI of a source blob.</param>
        /// <param name="tier">An enum that represents the <see cref="azure::storage::premium_blob_tier" /> for the destination blob.</param>
        /// <param name="source_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the source blob.</param>
        /// <param name="destination_condition">An object that represents the <see cref="azure::storage::access_condition" /> for the destination blob.</param>
        /// <param name="options">An <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        /// <remarks>
        /// This method fetches the blob's ETag, last-modified time, and part of the copy state.
        /// The copy ID and copy status fields are fetched, and the rest of the copy state is cleared.
        /// </remarks>
        pplx::task<utility::string_t> start_copy_async(const web::http::uri& source, const premium_blob_tier tier, const access_condition& source_condition, const access_condition& destination_condition, const blob_request_options& options, operation_context context)
        {
            return start_copy_async_impl(source, tier, source_condition, destination_condition, options, context);
        }
    private:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_page_blob" /> class.
        /// </summary>
        /// <param name="name">The name of the blob.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <param name="container">A reference to the parent container.</param>
        cloud_page_blob(utility::string_t name, utility::string_t snapshot_time, cloud_blob_container container)
            : cloud_blob(std::move(name), std::move(snapshot_time), std::move(container))
        {
            set_type(blob_type::page_blob);
        }

        friend class cloud_blob_container;
        friend class cloud_blob_directory;
    };

    /// <summary>
    /// Represents a Windows Azure append blob.
    /// </summary>
    class cloud_append_blob : public cloud_blob
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_append_blob" /> class.
        /// </summary>
        cloud_append_blob()
            : cloud_blob()
        {
            set_type(blob_type::append_blob);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_append_blob" /> class using an absolute URI to the blob.
        /// </summary>
        /// <param name="uri">A <see cref="storage_uri" /> object containing the absolute URI to the blob for all locations.</param>
        explicit cloud_append_blob(storage_uri uri)
            : cloud_blob(std::move(uri))
        {
            set_type(blob_type::append_blob);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_append_blob" /> class using an absolute URI to the blob.
        /// </summary>
        /// <param name="uri">A <see cref="storage_uri" /> object containing the absolute URI to the blob for all locations.</param>
        /// <param name="credentials">The <see cref="storage_credentials" /> to use.</param>
        cloud_append_blob(storage_uri uri, storage_credentials credentials)
            : cloud_blob(std::move(uri), std::move(credentials))
        {
            set_type(blob_type::append_blob);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_append_blob" /> class using an absolute URI to the blob.
        /// </summary>
        /// <param name="uri">A <see cref="storage_uri" /> object containing the absolute URI to the blob for all locations.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <param name="credentials">The <see cref="storage_credentials" /> to use.</param>
        cloud_append_blob(storage_uri uri, utility::string_t snapshot_time, storage_credentials credentials)
            : cloud_blob(std::move(uri), std::move(snapshot_time), std::move(credentials))
        {
            set_type(blob_type::append_blob);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_append_blob" /> class.
        /// </summary>
        /// <param name="blob">Reference to the blob.</param>
        cloud_append_blob(const cloud_blob& blob)
            : cloud_blob(blob)
        {
            set_type(blob_type::append_blob);
        }

        /// <summary>
        /// Creates an empty append blob. If the blob already exists, this will replace it. To avoid overwriting and instead throw an error, please pass in an <see cref="azure::storage::access_condition"/>
        /// parameter generated using <see cref="azure::storage::access_condition::generate_if_not_exists_condition"/>
        /// </summary>
        void create_or_replace()
        {
            create_or_replace_async().wait();
        }

        /// <summary>
        /// Creates an empty append blob. If the blob already exists, this will replace it. To avoid overwriting and instead throw an error, please pass in an <see cref="azure::storage::access_condition"/>
        /// parameter generated using <see cref="azure::storage::access_condition::generate_if_not_exists_condition"/>
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void create_or_replace(const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            create_or_replace_async(condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to create an empty append blob. If the blob already exists, this will replace it. To avoid overwriting and instead throw an error, please pass in an <see cref="azure::storage::access_condition"/>
        /// parameter generated using <see cref="azure::storage::access_condition::generate_if_not_exists_condition"/>
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> create_or_replace_async()
        {
            return create_or_replace_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to create an empty append blob. If the blob already exists, this will replace it. To avoid overwriting and instead throw an error, please pass in an <see cref="azure::storage::access_condition"/>
        /// parameter generated using <see cref="azure::storage::access_condition::generate_if_not_exists_condition"/>
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> create_or_replace_async(const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Commits a new block of data to the end of the blob.
        /// </summary>
        /// <param name="block_data">A stream that provides the data for the block.</param>
        /// <param name="content_md5">An optional hash value that will be used to to ensure transactional integrity
        /// for the block. May be an empty string. </param>
        /// <returns>The offset in bytes at which the block was committed to.</returns>
        int64_t append_block(concurrency::streams::istream block_data, const utility::string_t& content_md5) const
        {
            return append_block_async(block_data, content_md5).get();
        }

        /// <summary>
        /// Commits a new block of data to the end of the blob.
        /// </summary>
        /// <param name="block_data">A stream that provides the data for the block.</param>
        /// <param name="content_md5">An optional hash value that will be used to to ensure transactional integrity
        /// for the block. May be an empty string. </param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The offset in bytes at which the block was committed to.</returns>
        int64_t append_block(concurrency::streams::istream block_data, const utility::string_t& content_md5, const access_condition& condition, const blob_request_options& options, operation_context context) const
        {
            return append_block_async(block_data, content_md5, condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to commit a new block of data to the end of the blob.
        /// </summary>
        /// <param name="block_data">A stream that provides the data for the block.</param>
        /// <param name="content_md5">An optional hash value that will be used to to ensure transactional integrity
        /// for the block. May be an empty string. </param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<int64_t> append_block_async(concurrency::streams::istream block_data, const utility::string_t& content_md5) const
        {
            return append_block_async(block_data, content_md5, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to commit a new block of data to the end of the blob.
        /// </summary>
        /// <param name="block_data">A stream that provides the data for the block.</param>
        /// <param name="content_md5">An optional hash value that will be used to to ensure transactional integrity
        /// for the block. May be an empty string. </param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<int64_t> append_block_async(concurrency::streams::istream block_data, const utility::string_t& content_md5, const access_condition& condition, const blob_request_options& options, operation_context context) const;

        /// <summary>
        /// Downloads the blob's contents as a string.
        /// </summary>
        /// <returns>The contents of the blob, as a string.</returns>
        utility::string_t download_text()
        {
            return download_text_async().get();
        }

        /// <summary>
        /// Downloads the blob's contents as a string.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>The contents of the blob, as a string.</returns>
        utility::string_t download_text(const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return download_text_async(condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to download the blob's contents as a string.
        /// </summary>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        pplx::task<utility::string_t> download_text_async()
        {
            return download_text_async(access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to download the blob's contents as a string.
        /// </summary>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="utility::string_t" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<utility::string_t> download_text_async(const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Opens a stream for writing to the append blob.
        /// </summary>
        /// <param name="create_new">Use <c>true</c> to create a new append blob or overwrite an existing one, <c>false</c> to append to an existing blob.</param>
        /// <returns>A stream to be used for writing to the blob.</returns>
        concurrency::streams::ostream open_write(bool create_new)
        {
            return open_write_async(create_new).get();
        }

        /// <summary>
        /// Opens a stream for writing to the append blob.
        /// </summary>
        /// <param name="create_new">Use <c>true</c> to create a new append blob or overwrite an existing one, <c>false</c> to append to an existing blob.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A stream to be used for writing to the blob.</returns>
        concurrency::streams::ostream open_write(bool create_new, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return open_write_async(create_new, condition, options, context).get();
        }

        /// <summary>
        /// Initiates an asynchronous operation to open a stream for writing to the append blob.
        /// </summary>
        /// <param name="create_new">Use <c>true</c> to create a new append blob or overwrite an existing one, <c>false</c> to append to an existing blob.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="concurrency::streams::ostream" /> that represents the current operation.</returns>
        pplx::task<concurrency::streams::ostream> open_write_async(bool create_new)
        {
            return open_write_async(create_new, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to open a stream for writing to the append blob.
        /// </summary>
        /// <param name="create_new">Use <c>true</c> to create a new append blob or overwrite an existing one, <c>false</c> to append to an existing blob.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object of type <see cref="concurrency::streams::ostream" /> that represents the current operation.</returns>
        WASTORAGE_API pplx::task<concurrency::streams::ostream> open_write_async(bool create_new, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Uploads a stream to an append blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <remarks>
        /// <para>This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks.</para>
        /// <para>If you want to append data to an already existing blob, please look at append_from_stream method.</para>
        /// </remarks>
        void upload_from_stream(concurrency::streams::istream source)
        {
            upload_from_stream_async(source).wait();
        }

        /// <summary>
        /// Uploads a stream to an append blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <remarks>
        /// <para>This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks.
        /// If you are guaranteed to have a single writer scenario, please look at <see cref="azure::storage::blob_request_options::absorb_conditional_errors_on_retry"/>
        /// and see if setting this flag to <c>true</c> is acceptable for you.</para>
        /// <para>If you want to append data to an already existing blob, please look at append_from_stream method.</para>
        /// </remarks>
        void upload_from_stream(concurrency::streams::istream source, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_from_stream_async(source, condition, options, context).wait();
        }

        /// <summary>
        /// Uploads a stream to an append blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        /// <remarks>
        /// <para>This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks.</para>
        /// <para>If you want to append data to an already existing blob, please look at append_from_stream method.</para>
        /// </remarks>
        void upload_from_stream(concurrency::streams::istream source, utility::size64_t length)
        {
            upload_from_stream_async(source, length).wait();
        }

        /// <summary>
        /// Uploads a stream to an append blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <remarks>
        /// <para>This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks.
        /// If you are guaranteed to have a single writer scenario, please look at <see cref="azure::storage::blob_request_options::absorb_conditional_errors_on_retry"/>
        /// and see if setting this flag to <c>true</c> is acceptable for you.</para>
        /// <para>If you want to append data to an already existing blob, please look at append_from_stream method.</para>
        /// </remarks>
        void upload_from_stream(concurrency::streams::istream source, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_from_stream_async(source, length, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a stream to the append blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        /// <remarks>
        /// <para>This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks. </para>
        /// <para>If you want to append data to an already existing blob, please look at append_from_stream_async method.</para>
        /// </remarks>
        pplx::task<void> upload_from_stream_async(concurrency::streams::istream source)
        {
            return upload_from_stream_async(source, std::numeric_limits<utility::size64_t>::max());
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a stream to the append blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        /// <remarks>
        /// <para>This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks.
        /// If you are guaranteed to have a single writer scenario, please look at <see cref="azure::storage::blob_request_options::absorb_conditional_errors_on_retry"/>
        /// and see if setting this flag to <c>true</c> is acceptable for you.</para>
        /// <para>If you want to append data to an already existing blob, please look at append_from_stream_async method.</para>
        /// </remarks>
        pplx::task<void> upload_from_stream_async(concurrency::streams::istream source, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return upload_from_stream_async(source, std::numeric_limits<utility::size64_t>::max(), condition, options, context);
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a stream to the append blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        /// <remarks>
        /// <para>This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks.</para>
        /// <para>If you want to append data to an already existing blob, please look at append_from_stream_async method.</para>
        /// </remarks>
        pplx::task<void> upload_from_stream_async(concurrency::streams::istream source, utility::size64_t length)
        {
            return upload_from_stream_async(source, length, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a stream to the append blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        /// <remarks>
        /// <para>This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks.
        /// If you are guaranteed to have a single writer scenario, please look at <see cref="azure::storage::blob_request_options::absorb_conditional_errors_on_retry"/>
        /// and see if setting this flag to <c>true</c> is acceptable for you.</para>
        /// <para>If you want to append data to an already existing blob, please look at append_from_stream_async method.</para>
        /// </remarks>
        WASTORAGE_API pplx::task<void> upload_from_stream_async(concurrency::streams::istream source, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Uploads a file to the append blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="path">The file providing the blob content.</param>
        /// <remarks>
        /// <para>This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks.</para>
        /// <para>If you want to append data to an already existing blob, please look at append_from_file method.</para>
        /// </remarks>
        void upload_from_file(const utility::string_t &path)
        {
            upload_from_file_async(path).wait();
        }

        /// <summary>
        /// Uploads a file to the append blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="path">The file providing the blob content.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <remarks>
        /// <para>This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks.
        /// If you are guaranteed to have a single writer scenario, please look at <see cref="azure::storage::blob_request_options::absorb_conditional_errors_on_retry"/>
        /// and see if setting this flag to <c>true</c> is acceptable for you.</para>
        /// <para>If you want to append data to an already existing blob, please look at append_from_file method.</para>
        /// </remarks>
        void upload_from_file(const utility::string_t &path, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_from_file_async(path, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a file to the append blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="path">The file providing the blob content.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        /// <remarks>
        /// <para>This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks.</para>
        /// <para>If you want to append data to an already existing blob, please look at append_from_file_async method.</para>
        /// </remarks>
        pplx::task<void> upload_from_file_async(const utility::string_t &path)
        {
            return upload_from_file_async(path, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to upload a file to the append blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="path">The file providing the blob content.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        /// <remarks>
        /// <para>This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks.
        /// If you are guaranteed to have a single writer scenario, please look at <see cref="azure::storage::blob_request_options::absorb_conditional_errors_on_retry"/>
        /// and see if setting this flag to <c>true</c> is acceptable for you.</para>
        /// <para>If you want to append data to an already existing blob, please look at append_from_file_async method.</para>
        /// </remarks>
        WASTORAGE_API pplx::task<void> upload_from_file_async(const utility::string_t &path, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Uploads a string of text to the append blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="content">A string containing the text to upload.</param>
        /// <remarks>
        /// <para>This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks.</para>
        /// <para>If you want to append data to an already existing blob, please look at append_text method.</para>
        /// </remarks>
        void upload_text(const utility::string_t& content)
        {
            upload_text_async(content).wait();
        }

        /// <summary>
        /// Uploads a string of text to the append blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="content">A string containing the text to upload.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <remarks>
        /// <para>This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks.
        /// If you are guaranteed to have a single writer scenario, please look at <see cref="azure::storage::blob_request_options::absorb_conditional_errors_on_retry"/>
        /// and see if setting this flag to <c>true</c> is acceptable for you.</para>
        /// <para>If you want to append data to an already existing blob, please look at append_text method.</para>
        /// </remarks>
        void upload_text(const utility::string_t& content, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            upload_text_async(content, condition, options, context).wait();
        }

        /// <summary>
        /// Uploads a string of text to the append blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="content">A string containing the text to upload.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        /// <remarks>
        /// <para>This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks.</para>
        /// <para>If you want to append data to an already existing blob, please look at append_text_async method.</para>
        /// </remarks>
        pplx::task<void> upload_text_async(const utility::string_t& content)
        {
            return upload_text_async(content, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Uploads a string of text to the append blob. If the blob already exists on the service, it will be overwritten.
        /// </summary>
        /// <param name="content">A string containing the text to upload.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        /// <remarks>
        /// <para>This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks.
        /// If you are guaranteed to have a single writer scenario, please look at <see cref="azure::storage::blob_request_options::absorb_conditional_errors_on_retry"/>
        /// and see if setting this flag to <c>true</c> is acceptable for you.</para>
        /// <para>If you want to append data to an already existing blob, please look at append_text_async method.</para>
        /// </remarks>
        WASTORAGE_API pplx::task<void> upload_text_async(const utility::string_t& content, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Appends a stream to an append blob. This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="source">A <see cref="concurrency::streams::istream"/> object providing the blob content.</param>
        void append_from_stream(concurrency::streams::istream source)
        {
            append_from_stream_async(source).wait();
        }

        /// <summary>
        /// Appends a stream to an append blob. This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="source">A <see cref="concurrency::streams::istream"/> object providing the blob content.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void append_from_stream(concurrency::streams::istream source, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            append_from_stream_async(source, condition, options, context).wait();
        }

        /// <summary>
        /// Appends a stream to an append blob. This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="source">A <see cref="concurrency::streams::istream"/> object providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        void append_from_stream(concurrency::streams::istream source, utility::size64_t length)
        {
            append_from_stream_async(source, length).wait();
        }

        /// <summary>
        /// Appends a stream to an append blob. This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="source">A <see cref="concurrency::streams::istream"/> object providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void append_from_stream(concurrency::streams::istream source, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            append_from_stream_async(source, length, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to append a stream to an append blob. This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="source">A <see cref="concurrency::streams::istream"/> object providing the blob content.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> append_from_stream_async(concurrency::streams::istream source)
        {
            return append_from_stream_async(source, std::numeric_limits<utility::size64_t>::max());
        }

        /// <summary>
        /// Initiates an asynchronous operation to append a stream to an append blob. This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="source">A <see cref="concurrency::streams::istream"/> object providing the blob content.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> append_from_stream_async(concurrency::streams::istream source, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            return append_from_stream_async(source, std::numeric_limits<utility::size64_t>::max(), condition, options, context);
        }

        /// <summary>
        /// Initiates an asynchronous operation to append a stream to an append blob. This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="source">A <see cref="concurrency::streams::istream"/> object providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> append_from_stream_async(concurrency::streams::istream source, utility::size64_t length)
        {
            return append_from_stream_async(source, length, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to append a stream to an append blob. This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="source">A <see cref="concurrency::streams::istream"/> object providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API  pplx::task<void> append_from_stream_async(concurrency::streams::istream source, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Appends a file to an append blob. This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="path">The file providing the blob content.</param>
        void append_from_file(const utility::string_t &path)
        {
            append_from_file_async(path).wait();
        }

        /// <summary>
        /// Appends a file to an append blob. This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="path">The file providing the blob content.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void append_from_file(const utility::string_t &path, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            append_from_file_async(path, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to append a file to an append blob. This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="path">The file providing the blob content.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> append_from_file_async(const utility::string_t &path)
        {
            return append_from_file_async(path, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to append a file to an append blob. This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="path">The file providing the blob content.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> append_from_file_async(const utility::string_t &path, const access_condition& condition, const blob_request_options& options, operation_context context);

        /// <summary>
        /// Appends a string of text to an append blob. This API should be used strictly in a single writer scenario 
        /// because the API internally uses the append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="content">A string containing the text to append.</param>
        void append_text(const utility::string_t& content)
        {
            append_text_async(content).wait();
        }

        /// <summary>
        /// Appends a string of text to an append blob. This API should be used strictly in a single writer scenario 
        /// because the API internally uses the append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="content">A string containing the text to append.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        void append_text(const utility::string_t& content, const access_condition& condition, const blob_request_options& options, operation_context context)
        {
            append_text_async(content, condition, options, context).wait();
        }

        /// <summary>
        /// Initiates an asynchronous operation to append a string of text to an append blob. This API should be used strictly in a single writer scenario 
        /// because the API internally uses the append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="content">A string containing the text to append.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> append_text_async(const utility::string_t& content)
        {
            return append_text_async(content, access_condition(), blob_request_options(), operation_context());
        }

        /// <summary>
        /// Initiates an asynchronous operation to append a string of text to an append blob. This API should be used strictly in a single writer scenario 
        /// because the API internally uses the append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="content">A string containing the text to append.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        WASTORAGE_API pplx::task<void> append_text_async(const utility::string_t& content, const access_condition& condition, const blob_request_options& options, operation_context context);

    private:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::cloud_append_blob" /> class.
        /// </summary>
        /// <param name="name">The name of the blob.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <param name="container">A reference to the parent container.</param>
        cloud_append_blob(utility::string_t name, utility::string_t snapshot_time, cloud_blob_container container)
            : cloud_blob(std::move(name), std::move(snapshot_time), std::move(container))
        {
            set_type(blob_type::append_blob);
        }

        /// <summary>
        /// Uploads a stream to a new or existing append blob. This API should be used strictly in a single writer scenario because the API internally uses the
        /// append-offset conditional header to avoid duplicate blocks which does not work in a multiple writer scenario.
        /// </summary>
        /// <param name="source">The stream providing the blob content.</param>
        /// <param name="length">The number of bytes to write from the source stream at its current position.</param>
        /// <param name="create_new"><c>true</c> if the append blob is newly created, <c>false</c> otherwise.</param>
        /// <param name="condition">An <see cref="azure::storage::access_condition" /> object that represents the access condition for the operation.</param>
        /// <param name="options">A <see cref="azure::storage::blob_request_options" /> object that specifies additional options for the request.</param>
        /// <param name="context">An <see cref="azure::storage::operation_context" /> object that represents the context for the current operation.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        pplx::task<void> upload_from_stream_internal_async(concurrency::streams::istream source, utility::size64_t length, bool create_new, const access_condition& condition, const blob_request_options& options, operation_context context);

        friend class cloud_blob_container;
        friend class cloud_blob_directory;
    };

    /// <summary>
    /// Represents an item that may be returned by a blob listing operation.
    /// </summary>
    class list_blob_item
    {
    public:

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::list_blob_item" /> class that represents a cloud blob.
        /// </summary>
        /// <param name="blob_name">The name of the blob.</param>
        /// <param name="snapshot_time">The snapshot timestamp, if the blob is a snapshot.</param>
        /// <param name="container">A reference to the parent container.</param>
        /// <param name="properties">A set of properties for the blob.</param>
        /// <param name="metadata">User-defined metadata for the blob.</param>
        /// <param name="copy_state">the state of the most recent or pending copy operation.</param>
        explicit list_blob_item(utility::string_t blob_name, utility::string_t snapshot_time, cloud_blob_container container, cloud_blob_properties properties, cloud_metadata metadata, copy_state copy_state)
            : m_is_blob(true), m_name(std::move(blob_name)), m_container(std::move(container)),
            m_snapshot_time(std::move(snapshot_time)), m_properties(std::move(properties)),
            m_metadata(std::move(metadata)), m_copy_state(std::move(copy_state))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::list_blob_item" /> class that represents a cloud blob directory.
        /// </summary>
        /// <param name="directory_name">Name of the virtual directory.</param>
        /// <param name="container">The container.</param>
        explicit list_blob_item(utility::string_t directory_name, cloud_blob_container container)
            : m_is_blob(false), m_name(std::move(directory_name)), m_container(std::move(container))
        {
        }

#if defined(_MSC_VER) && _MSC_VER < 1900
        // Compilers that fully support C++ 11 rvalue reference, e.g. g++ 4.8+, clang++ 3.3+ and Visual Studio 2015+,
        // have implicitly-declared move constructor and move assignment operator.

        /// <summary>
        /// Initializes a new instance of the <see cref="azure::storage::list_blob_item" /> class based on an existing instance.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::list_blob_item" /> object.</param>
        list_blob_item(list_blob_item&& other)
        {
            *this = std::move(other);
        }

        /// <summary>
        /// Returns a reference to an <see cref="azure::storage::list_blob_item" /> object.
        /// </summary>
        /// <param name="other">An existing <see cref="azure::storage::list_blob_item" /> object to use to set properties.</param>
        /// <returns>An <see cref="azure::storage::list_blob_item" /> object with properties set.</returns>
        list_blob_item& operator=(list_blob_item&& other)
        {
            if (this != &other)
            {
                m_is_blob = other.m_is_blob;
                m_name = std::move(other.m_name);
                m_container = std::move(other.m_container);
                m_snapshot_time = std::move(other.m_snapshot_time);
                m_properties = std::move(other.m_properties);
                m_metadata = std::move(other.m_metadata);
                m_copy_state = std::move(other.m_copy_state);
            }

            return *this;
        }
#endif

        /// <summary>
        /// Gets a value indicating whether this <see cref="azure::storage::list_blob_item" /> represents a cloud blob or a cloud blob directory.
        /// </summary>
        /// <returns><c>true</c> if this <see cref="azure::storage::list_blob_item" /> represents a cloud blob; otherwise, <c>false</c>.</returns>
        bool is_blob() const
        {
            return m_is_blob;
        }

        /// <summary>
        /// Returns the item as an <see cref="azure::storage::cloud_blob" /> object, if and only if it represents a cloud blob.
        /// </summary>
        /// <returns>An <see cref="azure::storage::cloud_blob" /> object.</returns>
        cloud_blob as_blob() const
        {
            if (!is_blob())
            {
                throw std::runtime_error("Cannot access a cloud blob directory as cloud blob ");
            }

            return cloud_blob(m_name, m_snapshot_time, m_container, m_properties, m_metadata, m_copy_state);
        }

        /// <summary>
        /// Returns the item as an <see cref="azure::storage::cloud_blob_directory" /> object, if and only if it represents a cloud blob directory.
        /// </summary>
        /// <returns>An <see cref="azure::storage::cloud_blob_directory" /> object.</returns>
        cloud_blob_directory as_directory() const
        {
            if (is_blob())
            {
                throw std::runtime_error("Cannot access a cloud blob as cloud blob directory");
            }

            return cloud_blob_directory(m_name, m_container);
        }

    private:

        bool m_is_blob;
        utility::string_t m_name;
        cloud_blob_container m_container;
        utility::string_t m_snapshot_time;
        cloud_blob_properties m_properties;
        cloud_metadata m_metadata;
        copy_state m_copy_state;
    };

}} // namespace azure::storage

#pragma pop_macro("max")
