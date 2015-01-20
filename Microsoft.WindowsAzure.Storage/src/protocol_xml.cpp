// -----------------------------------------------------------------------------------------
// <copyright file="protocol_xml.cpp" company="Microsoft">
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

#include "stdafx.h"
#include "wascore/protocol.h"
#include "wascore/protocol_xml.h"

namespace azure { namespace storage { namespace protocol {

    void storage_error_reader::handle_element(const utility::string_t& element_name)
    {
        if (element_name == xml_code && get_parent_element_name() == xml_error_root)
        {
            m_error_code = get_current_element_text();
        }
        else if (element_name == xml_message && get_parent_element_name() == xml_error_root)
        {
            m_error_message = get_current_element_text();
        }
        else
        {
            m_details.insert(std::make_pair(element_name, get_current_element_text()));
        }
    }

    void list_containers_reader::handle_begin_element(const utility::string_t& element_name)
    {
        if (element_name == xml_enumeration_results)
        {
            if (move_to_first_attribute())
            {
                do
                {
                    if (get_current_element_name() == xml_service_endpoint)
                    {
                        m_service_uri = web::http::uri(get_current_element_text());
                    }
                } while (move_to_next_attribute());
            }
        }
    }

    void list_containers_reader::handle_element(const utility::string_t& element_name)
    {
        if (get_parent_element_name() == xml_metadata)
        {
            m_metadata[element_name] = get_current_element_text();
            return;
        }

        if (get_parent_element_name() == xml_properties)
        {
            if (element_name == xml_last_modified)
            {
                m_properties.m_last_modified = parse_last_modified(get_current_element_text());
                return;
            }

            if (element_name == xml_etag)
            {
                m_properties.m_etag = get_current_element_text();
                return;
            }

            if (element_name == xml_lease_status)
            {
                m_properties.m_lease_status = parse_lease_status(get_current_element_text());
                return;
            }

            if (element_name == xml_lease_state)
            {
                m_properties.m_lease_state = parse_lease_state(get_current_element_text());
                return;
            }

            if (element_name == xml_lease_duration)
            {
                m_properties.m_lease_duration = parse_lease_duration(get_current_element_text());
                return;
            }
        }

        if (element_name == xml_name)
        {
            m_name = get_current_element_text();
            m_uri = web::http::uri_builder(m_service_uri).append_path(m_name, true).to_uri();
            return;
        }

        if (element_name == xml_next_marker)
        {
            m_next_marker = get_current_element_text();
            return;
        }
    }

    void list_containers_reader::handle_end_element(const utility::string_t& element_name)
    {
        if (element_name == xml_container && get_parent_element_name() == xml_containers)
        {
            // End of the data for a Container. Create an item and add it to the list
            m_items.push_back(cloud_blob_container_list_item(std::move(m_uri), std::move(m_name), std::move(m_metadata), std::move(m_properties)));

            m_uri = web::uri();
            m_name = utility::string_t();
            m_metadata = azure::storage::cloud_metadata();
            m_properties = azure::storage::cloud_blob_container_properties();
        }
    }

    void list_blobs_reader::handle_begin_element(const utility::string_t& element_name)
    {
        if (element_name == xml_enumeration_results)
        {
            if (move_to_first_attribute())
            {
                utility::string_t container_name;
                do
                {
                    if (get_current_element_name() == xml_service_endpoint)
                    {
                        m_service_uri = web::http::uri(get_current_element_text());
                    }
                    else if (get_current_element_name() == xml_container_name)
                    {
                        container_name = get_current_element_text();
                    }
                } while (move_to_next_attribute());

                m_service_uri = web::http::uri_builder(m_service_uri).append_path(container_name).to_uri();
            }
        }
    }

    void list_blobs_reader::handle_element(const utility::string_t& element_name)
    {
        if (get_parent_element_name() == xml_metadata)
        {
            m_metadata[element_name] = get_current_element_text();
            return;
        }

        if (get_parent_element_name() == xml_properties)
        {
            if (element_name == xml_last_modified)
            {
                m_properties.m_last_modified = parse_last_modified(get_current_element_text());
                return;
            }

            if (element_name == xml_etag)
            {
                utility::ostringstream_t str;
                str << U('"') << get_current_element_text() << U('"');
                m_properties.m_etag = str.str();
                return;
            }

            if (element_name == xml_lease_status)
            {
                m_properties.m_lease_status = parse_lease_status(get_current_element_text());
                return;
            }

            if (element_name == xml_lease_state)
            {
                m_properties.m_lease_state = parse_lease_state(get_current_element_text());
                return;
            }

            if (element_name == xml_lease_duration)
            {
                m_properties.m_lease_duration = parse_lease_duration(get_current_element_text());
                return;
            }

            if (element_name == xml_content_length)
            {
                extract_current_element(m_properties.m_size);
                return;
            }

            if (element_name == xml_content_disposition)
            {
                m_properties.m_content_disposition = get_current_element_text();
                return;
            }

            if (element_name == xml_content_type)
            {
                m_properties.m_content_type = get_current_element_text();
                return;
            }

            if (element_name == xml_content_encoding)
            {
                m_properties.m_content_encoding = get_current_element_text();
                return;
            }

            if (element_name == xml_content_language)
            {
                m_properties.m_content_language = get_current_element_text();
                return;
            }

            if (element_name == xml_content_md5)
            {
                m_properties.m_content_md5 = get_current_element_text();
                return;
            }

            if (element_name == xml_cache_control)
            {
                m_properties.m_cache_control = get_current_element_text();
                return;
            }

            if (element_name == xml_blob_sequence_number)
            {
                extract_current_element(m_properties.m_page_blob_sequence_number);
                return;
            }

            if (element_name == xml_blob_type)
            {
                m_properties.m_type = blob_response_parsers::parse_blob_type(get_current_element_text());
                return;
            }

            if (element_name == xml_copy_id)
            {
                m_copy_state.m_copy_id = get_current_element_text();
                return;
            }

            if (element_name == xml_copy_status)
            {
                m_copy_state.m_status = blob_response_parsers::parse_copy_status(get_current_element_text());
                return;
            }

            if (element_name == xml_copy_source)
            {
                m_copy_state.m_source = get_current_element_text();
                return;
            }

            if (element_name == xml_copy_progress)
            {
                blob_response_parsers::parse_copy_progress(get_current_element_text(), m_copy_state.m_bytes_copied, m_copy_state.m_total_bytes);
                return;
            }

            if (element_name == xml_copy_completion_time)
            {
                m_copy_state.m_completion_time = blob_response_parsers::parse_copy_completion_time(get_current_element_text());
                return;
            }

            if (element_name == xml_copy_status_description)
            {
                m_copy_state.m_status_description = get_current_element_text();
                return;
            }
        }

        if (element_name == xml_snapshot)
        {
            m_snapshot_time = get_current_element_text();
            return;
        }

        if (element_name == xml_name)
        {
            m_name = get_current_element_text();
            m_uri = web::http::uri_builder(m_service_uri).append_path(m_name, true).to_uri();
            return;
        }

        if (element_name == xml_next_marker)
        {
            m_next_marker = get_current_element_text();
            return;
        }
    }

    void list_blobs_reader::handle_end_element(const utility::string_t& element_name)
    {
        if (get_parent_element_name() == xml_blobs)
        {
            if (element_name == xml_blob)
            {
                m_blob_items.push_back(cloud_blob_list_item(std::move(m_uri), std::move(m_name), std::move(m_snapshot_time), std::move(m_metadata), std::move(m_properties), std::move(m_copy_state)));
                m_uri = web::uri();
                m_name = utility::string_t();
                m_snapshot_time = utility::string_t();
                m_metadata = azure::storage::cloud_metadata();
                m_properties = azure::storage::cloud_blob_properties();
                m_copy_state = azure::storage::copy_state();
            }
            else if (element_name == xml_blob_prefix)
            {
                m_blob_prefix_items.push_back(cloud_blob_prefix_list_item(std::move(m_uri), std::move(m_name)));
                m_uri = web::uri();
                m_name = utility::string_t();
            }
        }
    }

    void page_list_reader::handle_element(const utility::string_t& element_name)
    {
        if (element_name == xml_start && m_start == -1)
        {
            extract_current_element(m_start);
        }
        else if (element_name == xml_end && m_end == -1)
        {
            extract_current_element(m_end);
        }
    }

    void page_list_reader::handle_end_element(const utility::string_t& element_name)
    {
        if (element_name == xml_page_range)
        {
            if (m_start != -1 && m_end != -1)
            {
                page_range range(m_start, m_end);
                m_page_list.push_back(range);
            }

            m_start = -1;
            m_end = -1;
        }
    }

    void block_list_reader::handle_begin_element(const utility::string_t& element_name)
    {
        if (element_name == xml_committed_blocks)
        {
            m_handling_what = 1;
        }
        else if (element_name == xml_uncommitted_blocks)
        {
            m_handling_what = 2;
        }
    }

    void block_list_reader::handle_element(const utility::string_t& element_name)
    {
        if (m_handling_what != 0)
        {
            if (element_name == xml_name)
            {
                m_name = get_current_element_text();
            }
            else if (element_name == xml_size)
            {
                extract_current_element(m_size);
            }
        }
    }

    void block_list_reader::handle_end_element(const utility::string_t& element_name)
    {
        if (m_handling_what != 0)
        {
            if ((element_name == xml_committed_blocks) || (element_name == xml_uncommitted_blocks))
            {
                m_handling_what = 0;
            }
            else if (element_name == xml_block)
            {
                if (!m_name.empty() && (m_size != std::numeric_limits<size_t>::max()))
                {
                    m_block_list.push_back(block_list_item(std::move(m_name), m_size, m_handling_what == 1));
                }

                m_size = std::numeric_limits<size_t>::max();
                m_name = utility::string_t();
            }
        }
    }

    void service_properties_reader::handle_element(const utility::string_t& element_name)
    {
        if (get_parent_element_name() == xml_service_properties_logging)
        {
            handle_logging(element_name);
        }
        else if (get_parent_element_name() == xml_service_properties_hour_metrics)
        {
            handle_metrics(m_service_properties.hour_metrics(), element_name);
        }
        else if (get_parent_element_name() == xml_service_properties_minute_metrics)
        {
            handle_metrics(m_service_properties.minute_metrics(), element_name);
        }
        else if (get_parent_element_name() == xml_service_properties_cors_rule)
        {
            handle_cors_rule(element_name);
        }
        else if (get_parent_element_name() == xml_service_properties_retention)
        {
            if (element_name == xml_service_properties_enabled)
            {
                m_current_retention_policy_enabled = get_current_element_text() == header_value_true;
            }
            else if (element_name == xml_service_properties_retention_days)
            {
                extract_current_element(m_current_retention_policy_days);
            }
        }
        else if (element_name == xml_service_properties_default_service_version)
        {
            m_service_properties.set_default_service_version(get_current_element_text());
        }
    }

    void service_properties_reader::handle_logging(const utility::string_t& element_name)
    {
        if (element_name == xml_service_properties_version)
        {
            m_service_properties.logging().set_version(get_current_element_text());
        }
        else if (element_name == xml_service_properties_delete)
        {
            m_service_properties.logging().set_delete_enabled(get_current_element_text() == header_value_true);
        }
        else if (element_name == xml_service_properties_read)
        {
            m_service_properties.logging().set_read_enabled(get_current_element_text() == header_value_true);
        }
        else if (element_name == xml_service_properties_write)
        {
            m_service_properties.logging().set_write_enabled(get_current_element_text() == header_value_true);
        }
    }

    void service_properties_reader::handle_metrics(service_properties::metrics_properties& metrics, const utility::string_t& element_name)
    {
        if (element_name == xml_service_properties_version)
        {
            metrics.set_version(get_current_element_text());
        }
        else if (element_name == xml_service_properties_enabled)
        {
            metrics.set_enabled(get_current_element_text() == header_value_true);
        }
        else if (element_name == xml_service_properties_include_apis)
        {
            metrics.set_include_apis(get_current_element_text() == header_value_true);
        }
    }

    void service_properties_reader::handle_cors_rule(const utility::string_t& element_name)
    {
        if (element_name == xml_service_properties_allowed_origins)
        {
            auto current_element_text = core::string_split(get_current_element_text(), U(","));
            m_current_cors_rule.allowed_origins().swap(current_element_text);
        }
        else if (element_name == xml_service_properties_allowed_methods)
        {
            auto current_element_text = core::string_split(get_current_element_text(), U(","));
            m_current_cors_rule.allowed_methods().swap(current_element_text);
        }
        else if (element_name == xml_service_properties_exposed_headers)
        {
            auto current_element_text = core::string_split(get_current_element_text(), U(","));
            m_current_cors_rule.exposed_headers().swap(current_element_text);
        }
        else if (element_name == xml_service_properties_allowed_headers)
        {
            auto current_element_text = core::string_split(get_current_element_text(), U(","));
            m_current_cors_rule.allowed_headers().swap(current_element_text);
        }
        else if (element_name == xml_service_properties_max_age)
        {
            int max_age_in_seconds;
            extract_current_element(max_age_in_seconds);
            m_current_cors_rule.set_max_age(std::chrono::seconds(max_age_in_seconds));
        }
    }

    void service_properties_reader::handle_end_element(const utility::string_t& element_name)
    {
        if (element_name == xml_service_properties_retention)
        {
            if (get_parent_element_name() == xml_service_properties_logging)
            {
                m_service_properties.logging().set_retention_policy_enabled(m_current_retention_policy_enabled);
                m_service_properties.logging().set_retention_days(m_current_retention_policy_days);
            }
            else if (get_parent_element_name() == xml_service_properties_hour_metrics)
            {
                m_service_properties.hour_metrics().set_retention_policy_enabled(m_current_retention_policy_enabled);
                m_service_properties.hour_metrics().set_retention_days(m_current_retention_policy_days);
            }
            else if (get_parent_element_name() == xml_service_properties_minute_metrics)
            {
                m_service_properties.minute_metrics().set_retention_policy_enabled(m_current_retention_policy_enabled);
                m_service_properties.minute_metrics().set_retention_days(m_current_retention_policy_days);
            }

            m_current_retention_policy_days = 0;
        }
        else if (element_name == xml_service_properties_cors_rule)
        {
            m_service_properties.cors().push_back(m_current_cors_rule);
            m_current_cors_rule = service_properties::cors_rule();
        }
    }

    std::string block_list_writer::write(const std::vector<block_list_item>& blocks)
    {
        std::ostringstream outstream;
        initialize(outstream);

        write_start_element(xml_block_list);

        for (auto block = blocks.cbegin(); block != blocks.cend(); ++block)
        {
            utility::string_t tag;
            switch (block->mode())
            {
            case block_list_item::latest:
                tag = xml_latest;
                break;

            case block_list_item::committed:
                tag = xml_committed;
                break;

            case block_list_item::uncommitted:
                tag = xml_uncommitted;
                break;
            }

            write_element(tag, block->id());
        }

        finalize();
        return outstream.str();
    }

    void list_queues_reader::handle_element(const utility::string_t& element_name)
    {
        if (element_name == xml_name)
        {
            m_name = get_current_element_text();
            return;
        }

        if (get_parent_element_name() == xml_metadata)
        {
            m_metadata[element_name] = get_current_element_text();
            return;
        }

        if (element_name == xml_next_marker)
        {
            m_next_marker = get_current_element_text();
            return;
        }
    }

    void list_queues_reader::handle_end_element(const utility::string_t& element_name)
    {
        if (element_name == U("Queue") && get_parent_element_name() == U("Queues"))
        {
            cloud_queue_list_item item(std::move(m_name), std::move(m_metadata));
            m_items.push_back(item);
            m_name = utility::string_t();
            m_metadata = cloud_metadata();
        }
    }

    void message_reader::handle_element(const utility::string_t& element_name)
    {
        if (element_name == U("MessageText"))
        {
            m_content = get_current_element_text();
        }
        else if (element_name == U("MessageId"))
        {
            m_id = get_current_element_text();
        }
        else if (element_name == U("PopReceipt"))
        {
            m_pop_receipt = get_current_element_text();
        }
        else if (element_name == U("InsertionTime"))
        {
            m_insertion_time = utility::datetime::from_string(get_current_element_text(), utility::datetime::RFC_1123);
        }
        else if (element_name == U("ExpirationTime"))
        {
            m_expiration_time = utility::datetime::from_string(get_current_element_text(), utility::datetime::RFC_1123);
        }
        else if (element_name == U("TimeNextVisible"))
        {
            m_next_visible_time = utility::datetime::from_string(get_current_element_text(), utility::datetime::RFC_1123);
        }
        else if (element_name == U("DequeueCount"))
        {
            utility::istringstream_t stream(get_current_element_text());
            stream >> m_dequeue_count;
        }
    }

    void message_reader::handle_end_element(const utility::string_t& element_name)
    {
        if (element_name == U("QueueMessage"))
        {
            cloud_message_list_item item(std::move(m_content), std::move(m_id), std::move(m_pop_receipt), m_insertion_time, m_expiration_time, m_next_visible_time, m_dequeue_count);
            m_items.push_back(item);
            m_content = utility::string_t();
            m_id = utility::string_t();
            m_pop_receipt = utility::string_t();
            m_insertion_time = utility::datetime();
            m_expiration_time = utility::datetime();
            m_next_visible_time = utility::datetime();
            m_dequeue_count = 0;
        }
    }

    std::string message_writer::write(const cloud_queue_message& message)
    {
        std::ostringstream outstream;
        initialize(outstream);

        write_start_element(U("QueueMessage"));
        write_element(U("MessageText"), message.content_as_string());

        finalize();
        return outstream.str();
    }

    std::string service_properties_writer::write(const service_properties& properties, const service_properties_includes& includes)
    {
        std::ostringstream outstream;
        initialize(outstream);

        write_start_element(xml_service_properties);

        if (includes.logging())
        {
            write_start_element(xml_service_properties_logging);
            write_logging(properties.logging());
            write_end_element();
        }

        if (includes.hour_metrics())
        {
            write_start_element(xml_service_properties_hour_metrics);
            write_metrics(properties.hour_metrics());
            write_end_element();
        }

        if (includes.minute_metrics())
        {
            write_start_element(xml_service_properties_minute_metrics);
            write_metrics(properties.minute_metrics());
            write_end_element();
        }

        if (includes.cors())
        {
            write_start_element(xml_service_properties_cors);
        
            for (auto iter = properties.cors().cbegin(); iter != properties.cors().cend(); ++iter)
            {
                write_start_element(xml_service_properties_cors_rule);
                write_cors_rule(*iter);
                write_end_element();
            }
        
            write_end_element();
        }

        if (!properties.default_service_version().empty())
        {
            write_element(xml_service_properties_default_service_version, properties.default_service_version());
        }

        finalize();
        return outstream.str();
    }

    void service_properties_writer::write_logging(const service_properties::logging_properties& logging)
    {
        write_element(xml_service_properties_version, logging.version());
        write_element(xml_service_properties_delete, logging.delete_enabled() ? header_value_true : header_value_false);
        write_element(xml_service_properties_read, logging.read_enabled() ? header_value_true : header_value_false);
        write_element(xml_service_properties_write, logging.write_enabled() ? header_value_true : header_value_false);
        write_retention_policy(logging.retention_policy_enabled(), logging.retention_days());
    }

    void service_properties_writer::write_metrics(const service_properties::metrics_properties& metrics)
    {
        write_element(xml_service_properties_version, metrics.version());
        write_element(xml_service_properties_enabled, metrics.enabled() ? header_value_true : header_value_false);
        
        if (metrics.enabled())
        {
            write_element(xml_service_properties_include_apis, metrics.include_apis() ? header_value_true : header_value_false);
        }

        write_retention_policy(metrics.retention_policy_enabled(), metrics.retention_days());
    }

    void service_properties_writer::write_cors_rule(const service_properties::cors_rule& rule)
    {
        write_element(xml_service_properties_allowed_origins, core::string_join(rule.allowed_origins(), U(",")));
        write_element(xml_service_properties_allowed_methods, core::string_join(rule.allowed_methods(), U(",")));
        write_element(xml_service_properties_max_age, rule.max_age().count());
        write_element(xml_service_properties_exposed_headers, core::string_join(rule.exposed_headers(), U(",")));
        write_element(xml_service_properties_allowed_headers, core::string_join(rule.allowed_headers(), U(",")));
    }

    void service_properties_writer::write_retention_policy(bool enabled, int days)
    {
        write_start_element(xml_service_properties_retention);
        
        if (enabled)
        {
            write_element(xml_service_properties_enabled, header_value_true);
            write_element(xml_service_properties_retention_days, days);
        }
        else
        {
            write_element(xml_service_properties_enabled, header_value_false);
        }

        write_end_element();
    }

    void service_stats_reader::handle_element(const utility::string_t& element_name)
    {
        if (get_parent_element_name() == xml_service_stats_geo_replication)
        {
            handle_geo_replication_status(element_name);
        }
    }

    void service_stats_reader::handle_geo_replication_status(const utility::string_t& element_name)
    {
        if (element_name == xml_service_stats_geo_replication_status)
        {
            auto status = get_current_element_text();
            if (status == xml_service_stats_geo_replication_status_live)
            {
                m_service_stats.geo_replication_private().set_status(geo_replication_status::live);
            }
            else if (status == xml_service_stats_geo_replication_status_bootstrap)
            {
                m_service_stats.geo_replication_private().set_status(geo_replication_status::bootstrap);
            }
        }
        else if (element_name == xml_service_stats_geo_replication_last_sync_time)
        {
            m_service_stats.geo_replication_private().set_last_sync_time(utility::datetime::from_string(get_current_element_text(), utility::datetime::RFC_1123));
        }
    }

}}} // namespace azure::storage::protocol
