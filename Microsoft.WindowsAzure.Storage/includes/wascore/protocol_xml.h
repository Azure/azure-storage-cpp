// -----------------------------------------------------------------------------------------
// <copyright file="protocol_xml.h" company="Microsoft">
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

#include "wascore/basic_types.h"
#include "was/blob.h"
#include "was/queue.h"
#include "was/file.h"
#include "wascore/protocol.h"
#include "wascore/xmlhelpers.h"

#pragma push_macro("max")
#undef max

namespace azure { namespace storage { namespace protocol {

    class storage_error_reader : public core::xml::xml_reader
    {
    public:

        explicit storage_error_reader(concurrency::streams::istream error_response)
            : xml_reader(error_response)
        {
        }

        utility::string_t move_error_code()
        {
            parse();
            return std::move(m_error_code);
        }

        utility::string_t move_error_message()
        {
            parse();
            return std::move(m_error_message);
        }

        std::unordered_map<utility::string_t, utility::string_t> move_details()
        {
            parse();
            return std::move(m_details);
        }

    protected:

        virtual void handle_element(const utility::string_t& element_name);

        utility::string_t m_error_code;
        utility::string_t m_error_message;
        std::unordered_map<utility::string_t, utility::string_t> m_details;
    };

    class cloud_blob_container_list_item
    {
    public:

        cloud_blob_container_list_item(web::http::uri uri, utility::string_t name, cloud_metadata metadata, cloud_blob_container_properties properties)
            : m_uri(std::move(uri)), m_name(std::move(name)), m_metadata(std::move(metadata)), m_properties(std::move(properties))
        {
        }

        web::http::uri move_uri()
        {
            return std::move(m_uri);
        }

        utility::string_t move_name()
        {
            return std::move(m_name);
        }

        cloud_metadata move_metadata()
        {
            return std::move(m_metadata);
        }

        cloud_blob_container_properties move_properties()
        {
            return std::move(m_properties);
        }

    private:

        web::http::uri m_uri;
        utility::string_t m_name;
        cloud_metadata m_metadata;
        cloud_blob_container_properties m_properties;
    };

    class list_containers_reader : public core::xml::xml_reader
    {
    public:

        explicit list_containers_reader(concurrency::streams::istream stream)
            : xml_reader(stream)
        {
        }

        std::vector<cloud_blob_container_list_item> move_items()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_items);
        }

        utility::string_t move_next_marker()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_next_marker);
        }

    protected:

        virtual void handle_begin_element(const utility::string_t& element_name);
        virtual void handle_element(const utility::string_t& element_name);
        virtual void handle_end_element(const utility::string_t& element_name);

        std::vector<cloud_blob_container_list_item> m_items;
        utility::string_t m_next_marker;
        web::http::uri m_service_uri;

        utility::string_t m_name;
        web::http::uri m_uri;
        cloud_metadata m_metadata;
        cloud_blob_container_properties m_properties;
    };

    class cloud_blob_list_item
    {
    public:

        cloud_blob_list_item(web::http::uri uri, utility::string_t name, utility::string_t snapshot_time, cloud_metadata metadata, cloud_blob_properties properties, copy_state copy_state)
            : m_uri(std::move(uri)), m_name(std::move(name)), m_snapshot_time(std::move(snapshot_time)), m_metadata(std::move(metadata)), m_properties(std::move(properties)), m_copy_state(std::move(copy_state))
        {
        }

        web::http::uri move_uri()
        {
            return std::move(m_uri);
        }

        utility::string_t move_name()
        {
            return std::move(m_name);
        }

        utility::string_t move_snapshot_time()
        {
            return std::move(m_snapshot_time);
        }

        cloud_metadata move_metadata()
        {
            return std::move(m_metadata);
        }

        cloud_blob_properties move_properties()
        {
            return std::move(m_properties);
        }

        copy_state move_copy_state()
        {
            return std::move(m_copy_state);
        }

    private:

        web::http::uri m_uri;
        utility::string_t m_name;
        utility::string_t m_snapshot_time;
        cloud_metadata m_metadata;
        cloud_blob_properties m_properties;
        azure::storage::copy_state m_copy_state;
    };

    class cloud_blob_prefix_list_item
    {
    public:

        cloud_blob_prefix_list_item(web::http::uri uri, utility::string_t name)
            : m_uri(std::move(uri)), m_name(std::move(name))
        {
        }

        web::http::uri move_uri()
        {
            return std::move(m_uri);
        }

        utility::string_t move_name()
        {
            return std::move(m_name);
        }

    private:

        web::http::uri m_uri;
        utility::string_t m_name;
    };

    class list_blobs_reader : public core::xml::xml_reader
    {
    public:

        explicit list_blobs_reader(concurrency::streams::istream stream)
            : xml_reader(stream)
        {
        }

        std::vector<cloud_blob_list_item> move_blob_items()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_blob_items);
        }

        std::vector<cloud_blob_prefix_list_item> move_blob_prefix_items()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_blob_prefix_items);
        }

        utility::string_t move_next_marker()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_next_marker);
        }

    protected:

        virtual void handle_begin_element(const utility::string_t& element_name);
        virtual void handle_element(const utility::string_t& element_name);
        virtual void handle_end_element(const utility::string_t& element_name);

        std::vector<cloud_blob_list_item> m_blob_items;
        std::vector<cloud_blob_prefix_list_item> m_blob_prefix_items;
        utility::string_t m_next_marker;
        web::http::uri m_service_uri;

        utility::string_t m_name;
        web::http::uri m_uri;
        utility::string_t m_snapshot_time;
        cloud_metadata m_metadata;
        cloud_blob_properties m_properties;
        copy_state m_copy_state;
    };

    class page_list_reader : public core::xml::xml_reader
    {
    public:

        explicit page_list_reader(concurrency::streams::istream stream)
            : xml_reader(stream), m_start(-1), m_end(-1)
        {
        }

        // Extracts the result. This method can only be called once on this reader
        std::vector<page_range> move_result()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_page_list);
        }

    protected:

        virtual void handle_element(const utility::string_t& element_name);
        virtual void handle_end_element(const utility::string_t& element_name);

        std::vector<page_range> m_page_list;
        int64_t m_start;
        int64_t m_end;
    };

    class page_diff_list_reader : public core::xml::xml_reader
    {
    public:

        explicit page_diff_list_reader(concurrency::streams::istream stream)
            : xml_reader(stream), m_start(-1), m_end(-1)
        {
        }

        // Extracts the result. This method can only be called once on this reader
        std::vector<page_diff_range> move_result()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_page_list);
        }

    protected:

        virtual void handle_element(const utility::string_t& element_name);
        virtual void handle_end_element(const utility::string_t& element_name);

        std::vector<page_diff_range> m_page_list;
        int64_t m_start;
        int64_t m_end;
    };

    class block_list_reader : public core::xml::xml_reader
    {
    public:

        explicit block_list_reader(concurrency::streams::istream stream)
            : xml_reader(stream), m_handling_what(0), m_size(std::numeric_limits<size_t>::max())
        {
        }

        // Extracts the result. This method can only be called once on this reader
        std::vector<block_list_item> move_result()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_block_list);
        }

    protected:

        virtual void handle_begin_element(const utility::string_t& element_name);
        virtual void handle_element(const utility::string_t& element_name);
        virtual void handle_end_element(const utility::string_t& element_name);

        std::vector<block_list_item> m_block_list;

        // 0 -> nothing, 1 -> committed, 2 -> uncommitted
        int m_handling_what;

        size_t m_size;
        utility::string_t m_name;
    };

    class block_list_writer : public core::xml::xml_writer
    {
    public:

        block_list_writer()
        {
        }

        std::string write(const std::vector<block_list_item>& blocks);
    };

    template<typename Policy>
    class access_policy_reader : public core::xml::xml_reader
    {
    public:

        explicit access_policy_reader(concurrency::streams::istream stream)
            : xml_reader(stream)
        {
        }

        shared_access_policies<Policy> move_policies()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_policies);
        }

    protected:

        virtual void handle_element(const utility::string_t& element_name)
        {
            if (element_name == xml_signed_id)
            {
                m_current_identifier = get_current_element_text();
            }
            else if (element_name == xml_access_policy_start)
            {
                m_current_policy.set_start(utility::datetime::from_string(get_current_element_text(), utility::datetime::ISO_8601));
            }
            else if (element_name == xml_access_policy_expiry)
            {
                m_current_policy.set_expiry(utility::datetime::from_string(get_current_element_text(), utility::datetime::ISO_8601));
            }
            else if (element_name == xml_access_policy_permissions)
            {
                m_current_policy.set_permissions_from_string(get_current_element_text());
            }
        }

        virtual void handle_end_element(const utility::string_t& element_name)
        {
            if (element_name == xml_signed_identifier)
            {
                m_policies[m_current_identifier] = m_current_policy;
                m_current_policy.set_permissions(0);
                m_current_policy.set_start(utility::datetime());
                m_current_policy.set_expiry(utility::datetime());
            }
        }

    private:

        shared_access_policies<Policy> m_policies;
        utility::string_t m_current_identifier;
        Policy m_current_policy;
    };

    template<typename Policy>
    class access_policy_writer : public core::xml::xml_writer
    {
    public:

        access_policy_writer()
        {
        }

        std::string write(const shared_access_policies<Policy>& policies)
        {
            std::ostringstream outstream;
            initialize(outstream);

            write_start_element(xml_signed_identifiers);

            for (auto it = policies.cbegin(); it != policies.cend(); ++it)
            {
                write_start_element(xml_signed_identifier);
                write_element(xml_signed_id, it->first);
                auto& policy = it->second;
                write_start_element(xml_access_policy);

                if (policy.start().is_initialized())
                {
                    write_element(xml_access_policy_start, core::convert_to_string_with_fixed_length_fractional_seconds(policy.start()));
                }

                if (policy.expiry().is_initialized())
                {
                    write_element(xml_access_policy_expiry, core::convert_to_string_with_fixed_length_fractional_seconds(policy.expiry()));
                }

                if (policy.permission() != 0)
                {
                    write_element(xml_access_policy_permissions, policy.permissions_to_string());
                }

                write_end_element();
                write_end_element();
            }

            finalize();
            return outstream.str();
        }
    };

    class cloud_queue_list_item
    {
    public:

        cloud_queue_list_item(utility::string_t name, cloud_metadata metadata)
            : m_name(std::move(name)), m_metadata(std::move(metadata))
        {
        }

        utility::string_t move_name()
        {
            return std::move(m_name);
        }

        cloud_metadata move_metadata()
        {
            return std::move(m_metadata);
        }

    private:

        utility::string_t m_name;
        cloud_metadata m_metadata;
    };

    class list_queues_reader : public core::xml::xml_reader
    {
    public:

        explicit list_queues_reader(concurrency::streams::istream stream)
            : xml_reader(stream)
        {
        }

        std::vector<cloud_queue_list_item> move_items()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_items);
        }

        utility::string_t move_next_marker()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_next_marker);
        }

    protected:

        virtual void handle_element(const utility::string_t& element_name);
        virtual void handle_end_element(const utility::string_t& element_name);

        std::vector<cloud_queue_list_item> m_items;
        utility::string_t m_next_marker;

        utility::string_t m_name;
        cloud_metadata m_metadata;
    };

    class cloud_message_list_item
    {
    public:

        cloud_message_list_item(utility::string_t content, utility::string_t id, utility::string_t pop_receipt, utility::datetime insertion_time, utility::datetime expiration_time, utility::datetime next_visible_time, int dequeue_count)
            : m_content(std::move(content)), m_id(std::move(id)), m_pop_receipt(std::move(pop_receipt)), m_insertion_time(insertion_time), m_expiration_time(expiration_time), m_next_visible_time(next_visible_time), m_dequeue_count(dequeue_count)
        {
        }

        utility::string_t move_content()
        {
            return std::move(m_content);
        }

        utility::string_t move_id()
        {
            return std::move(m_id);
        }

        utility::string_t move_pop_receipt()
        {
            return std::move(m_pop_receipt);
        }

        utility::datetime insertion_time() const
        {
            return m_insertion_time;
        }

        utility::datetime expiration_time() const
        {
            return m_expiration_time;
        }

        utility::datetime next_visible_time() const
        {
            return m_next_visible_time;
        }

        int dequeue_count() const
        {
            return m_dequeue_count;
        }

    private:

        utility::string_t m_content;
        utility::string_t m_id;
        utility::string_t m_pop_receipt;
        utility::datetime m_insertion_time;
        utility::datetime m_expiration_time;
        utility::datetime m_next_visible_time;
        int m_dequeue_count;
    };

    class message_reader : public core::xml::xml_reader
    {
    public:

        explicit message_reader(concurrency::streams::istream stream)
            : xml_reader(stream), m_dequeue_count(0)
        {
        }

        std::vector<cloud_message_list_item> move_items()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                // This is not a retryable exception because Get operation for messages changes server content.
                throw storage_exception(protocol::error_xml_not_complete, false);
            }
            return std::move(m_items);
        }

    protected:

        virtual void handle_element(const utility::string_t& element_name);
        virtual void handle_end_element(const utility::string_t& element_name);

        std::vector<cloud_message_list_item> m_items;

        utility::string_t m_content;
        utility::string_t m_id;
        utility::string_t m_pop_receipt;
        utility::datetime m_insertion_time;
        utility::datetime m_expiration_time;
        utility::datetime m_next_visible_time;
        int m_dequeue_count;
    };

    class message_writer : public core::xml::xml_writer
    {
    public:

        message_writer()
        {
        }

        std::string write(const cloud_queue_message& message);
    };

    class cloud_file_share_list_item
    {
    public:

        cloud_file_share_list_item(web::http::uri uri, utility::string_t name, cloud_metadata metadata, cloud_file_share_properties properties)
            : m_uri(std::move(uri)), m_name(std::move(name)), m_metadata(std::move(metadata)), m_properties(std::move(properties))
        {
        }

        web::http::uri move_uri()
        {
            return std::move(m_uri);
        }

        utility::string_t move_name()
        {
            return std::move(m_name);
        }

        cloud_metadata move_metadata()
        {
            return std::move(m_metadata);
        }

        cloud_file_share_properties move_properties()
        {
            return std::move(m_properties);
        }

    private:

        web::http::uri m_uri;
        utility::string_t m_name;
        cloud_metadata m_metadata;
        cloud_file_share_properties m_properties;
    };

    class get_share_stats_reader : public core::xml::xml_reader
    {
    public:

        explicit get_share_stats_reader(concurrency::streams::istream stream)
            : xml_reader(stream), m_quota(maximum_share_quota)
        {
        }

        int32_t get()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return m_quota;
        }

    protected:

        virtual void handle_begin_element(const utility::string_t& element_name);
        virtual void handle_element(const utility::string_t& element_name);
        virtual void handle_end_element(const utility::string_t& element_name);

        int32_t m_quota;
    };

    class list_shares_reader : public core::xml::xml_reader
    {
    public:

        explicit list_shares_reader(concurrency::streams::istream stream)
            : xml_reader(stream)
        {
        }

        std::vector<cloud_file_share_list_item> move_items()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_items);
        }

        utility::string_t move_next_marker()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_next_marker);
        }

    protected:

        virtual void handle_begin_element(const utility::string_t& element_name);
        virtual void handle_element(const utility::string_t& element_name);
        virtual void handle_end_element(const utility::string_t& element_name);

        std::vector<cloud_file_share_list_item> m_items;
        utility::string_t m_next_marker;
        web::http::uri m_service_uri;

        utility::string_t m_name;
        web::http::uri m_uri;
        cloud_metadata m_metadata;
        cloud_file_share_properties m_properties;
    };

    class list_files_and_directories_reader : public core::xml::xml_reader
    {
    public:

        explicit list_files_and_directories_reader(concurrency::streams::istream stream)
            : xml_reader(stream), m_is_file(false), m_size(0)
        {
        }

        std::vector<list_file_and_directory_item> move_items()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_items);
        }

        utility::string_t move_next_marker()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_next_marker);
        }

    protected:

        virtual void handle_begin_element(const utility::string_t& element_name);
        virtual void handle_element(const utility::string_t& element_name);
        virtual void handle_end_element(const utility::string_t& element_name);

        std::vector<list_file_and_directory_item> m_items;
        utility::string_t m_next_marker;
        utility::string_t m_share_name;
        utility::string_t m_directory_path;
        web::http::uri m_service_uri;

        bool m_is_file;
        utility::string_t m_name;
        int64_t m_size;
    };

    class list_file_ranges_reader : public core::xml::xml_reader
    {
    public:

        explicit list_file_ranges_reader(concurrency::streams::istream stream)
            : xml_reader(stream), m_start(-1), m_end(-1)
        {
        }

        // Extracts the result. This method can only be called once on this reader
        std::vector<file_range> move_result()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_range_list);
        }

    protected:

        virtual void handle_element(const utility::string_t& element_name);
        virtual void handle_end_element(const utility::string_t& element_name);

        std::vector<file_range> m_range_list;
        int64_t m_start;
        int64_t m_end;
    };

    class service_properties_reader : public core::xml::xml_reader
    {
    public:

        explicit service_properties_reader(concurrency::streams::istream stream)
            : xml_reader(stream), m_current_retention_policy_days(0), m_current_retention_policy_enabled(false)
        {
        }

        service_properties move_properties()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_service_properties);
        }

    protected:

        virtual void handle_element(const utility::string_t& element_name);
        virtual void handle_end_element(const utility::string_t& element_name);

        service_properties m_service_properties;
        service_properties::cors_rule m_current_cors_rule;
        bool m_current_retention_policy_enabled;
        int m_current_retention_policy_days;

    private:

        void handle_logging(const utility::string_t& element_name);
        void handle_metrics(service_properties::metrics_properties& metrics, const utility::string_t& element_name);
        void handle_cors_rule(const utility::string_t& element_name);
    };

    class service_properties_writer : public core::xml::xml_writer
    {
    public:

        service_properties_writer()
        {
        }

        std::string write(const service_properties& properties, const service_properties_includes& includes);

    private:

        void write_logging(const service_properties::logging_properties& logging);
        void write_metrics(const service_properties::metrics_properties& metrics);
        void write_cors_rule(const service_properties::cors_rule& rule);
        void write_retention_policy(bool enabled, int days);
    };

    class service_stats_reader : public core::xml::xml_reader
    {
    public:

        explicit service_stats_reader(concurrency::streams::istream stream)
            : xml_reader(stream)
        {
        }

        service_stats move_stats()
        {
            auto result = parse();
            if (result == xml_reader::parse_result::xml_not_complete)
            {
                throw storage_exception(protocol::error_xml_not_complete, true);
            }
            return std::move(m_service_stats);
        }

    protected:

        virtual void handle_element(const utility::string_t& element_name);

        service_stats m_service_stats;

    private:

        void handle_geo_replication_status(const utility::string_t& element_name);
    };

}}} // namespace azure::storage::protocol

#pragma pop_macro("max")
