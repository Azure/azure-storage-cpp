// -----------------------------------------------------------------------------------------
// <copyright file="cloud_block_blob.cpp" company="Microsoft">
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
#include "wascore/blobstreams.h"

namespace azure { namespace storage {

    pplx::task<void> cloud_block_blob::upload_block_async(const utility::string_t& block_id, concurrency::streams::istream block_data, const utility::string_t& content_md5, const access_condition& condition, const blob_request_options& options, operation_context context) const
    {
        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), type());

        bool needs_md5 = content_md5.empty() && modified_options.use_transactional_md5();

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response(std::bind(protocol::preprocess_response_void, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        return core::istream_descriptor::create(block_data, needs_md5, std::numeric_limits<utility::size64_t>::max(), protocol::max_block_size).then([command, context, block_id, content_md5, modified_options, condition](core::istream_descriptor request_body) -> pplx::task<void>
        {
            const utility::string_t& md5 = content_md5.empty() ? request_body.content_md5() : content_md5;
            command->set_build_request(std::bind(protocol::put_block, block_id, md5, condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            command->set_request_body(request_body);
            return core::executor<void>::execute_async(command, modified_options, context);
        });
    }

    pplx::task<void> cloud_block_blob::upload_block_list_async(const std::vector<block_list_item>& block_list, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), type());

        bool needs_md5 = modified_options.use_transactional_md5();

        protocol::block_list_writer writer;
        concurrency::streams::istream stream(concurrency::streams::bytestream::open_istream(writer.write(block_list)));

        auto properties = m_properties;
        
        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_properties(response));
        });
        return core::istream_descriptor::create(stream, needs_md5).then([command, properties, this, context, modified_options, condition] (core::istream_descriptor request_body) -> pplx::task<void>
        {
            command->set_build_request(std::bind(protocol::put_block_list, *properties, metadata(), request_body.content_md5(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            command->set_request_body(request_body);
            return core::executor<void>::execute_async(command, modified_options, context);
        });
    }

    pplx::task<std::vector<block_list_item>> cloud_block_blob::download_block_list_async(block_listing_filter listing_filter, const access_condition& condition, const blob_request_options& options, operation_context context) const
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), type());

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<std::vector<block_list_item>>>(uri());
        command->set_build_request(std::bind(protocol::get_block_list, listing_filter, snapshot_time(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_location_mode(core::command_location_mode::primary_or_secondary);
        command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context) -> std::vector<block_list_item>
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_properties(response));
            return std::vector<block_list_item>();
        });
        command->set_postprocess_response([] (const web::http::http_response& response, const request_result&, const core::ostream_descriptor&, operation_context context) -> pplx::task<std::vector<block_list_item>>
        {
            protocol::block_list_reader reader(response.body());
            return pplx::task_from_result(reader.move_result());
        });
        return core::executor<std::vector<block_list_item>>::execute_async(command, modified_options, context);
    }

    pplx::task<concurrency::streams::ostream> cloud_block_blob::open_write_async(const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), type(), false);

        pplx::task<void> check_condition_task;
        if (condition.is_conditional())
        {
            check_condition_task = download_attributes_async(condition, modified_options, context).then([condition] (pplx::task<void> download_attributes_task)
            {
                try
                {
                    download_attributes_task.wait();
                }
                catch (const storage_exception& e)
                {
                    if ((e.result().http_status_code() == web::http::status_codes::NotFound) &&
                        condition.if_match_etag().empty())
                    {
                        // If we got a 404 and the condition was not an If-Match,
                        // we should continue with the operation.
                    }
                    else
                    {
                        throw;
                    }
                }
            });
        }
        else
        {
            check_condition_task = pplx::task_from_result();
        }

        auto instance = std::make_shared<cloud_block_blob>(*this);
        return check_condition_task.then([instance, condition, modified_options, context] ()
        {
            return core::cloud_block_blob_ostreambuf(instance, condition, modified_options, context).create_ostream();
        });
    }

    pplx::task<void> cloud_block_blob::upload_from_stream_async(concurrency::streams::istream source, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), type());

        // This will be std::numeric_limits<utility::size64_t>::max() if the stream is not seekable.
        utility::size64_t remaining_stream_length = core::get_remaining_stream_length(source);

        // Before this line, 'length = max' means "no length was given by the user."  After this line, 'length = max' means "no length was given, and the stream is not seekable."
        if (length == std::numeric_limits<utility::size64_t>::max())
        {
            length = remaining_stream_length;
        }

        // If the stream is seekable, check for the case where the stream is too short.
        // If the stream is not seekable, this will be caught later, when we run out of bytes in the stream when uploading.
        if (source.can_seek() && (length > remaining_stream_length))
        {
            throw std::invalid_argument(protocol::error_stream_short);
        }

        if ((length != std::numeric_limits<utility::size64_t>::max()) &&
            (length <= modified_options.single_blob_upload_threshold_in_bytes()) &&
            (modified_options.parallelism_factor() == 1))
        {
            if (modified_options.use_transactional_md5() && !modified_options.store_blob_content_md5())
            {
                throw std::invalid_argument(protocol::error_md5_options_mismatch);
            }

            auto properties = m_properties;
            auto metadata = m_metadata;

            auto command = std::make_shared<core::storage_command<void>>(uri());
            command->set_authentication_handler(service_client().authentication_handler());
            command->set_preprocess_response([properties] (const web::http::http_response& response, const request_result& result, operation_context context)
            {
                protocol::preprocess_response_void(response, result, context);
                properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_properties(response));
            });
            return core::istream_descriptor::create(source, modified_options.store_blob_content_md5(), length, protocol::max_single_blob_upload_threshold).then([command, context, properties, metadata, condition, modified_options] (core::istream_descriptor request_body) -> pplx::task<void>
            {
                if (!request_body.content_md5().empty())
                {
                    properties->set_content_md5(request_body.content_md5());
                }

                command->set_build_request(std::bind(protocol::put_block_blob, *properties, *metadata, condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
                command->set_request_body(request_body);
                return core::executor<void>::execute_async(command, modified_options, context);
            });
        }

        // Check if the total required blocks for the upload exceeds the maximum allowable block limit.
        // Adjusts the block size to ensure a successful upload only if the value has not been explicitly set.
        // Otherwise, throws a storage_exception if the default value has been changed or if the blob size exceeds the maximum capacity.
        if (length != std::numeric_limits<utility::size64_t>::max())
        {
            auto totalBlocks = std::ceil(static_cast<double>(length) / modified_options.stream_write_size_in_bytes());

            // Check if the total required blocks for the upload exceeds the maximum allowable block limit.
            if (totalBlocks > protocol::max_block_number)
            {
                if (modified_options.stream_write_size_in_bytes().has_value() || length > protocol::max_block_blob_size)
                {
                    throw storage_exception(protocol::error_blob_over_max_block_limit);
                }
                else
                {
                    // Scale the block size to ensure a successful upload (only if the user did not specify a value).
                    modified_options.set_stream_write_size_in_bytes(static_cast<size_t>(std::ceil(static_cast<double>(length)) / protocol::max_block_number));
                }
            }
        }

        return open_write_async(condition, modified_options, context).then([source, length] (concurrency::streams::ostream blob_stream) -> pplx::task<void>
        {
            return core::stream_copy_async(source, blob_stream, length).then([blob_stream] (utility::size64_t) -> pplx::task<void>
            {
                return blob_stream.close();
            });
        });
    }

    pplx::task<void> cloud_block_blob::upload_from_file_async(const utility::string_t &path, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        auto instance = std::make_shared<cloud_block_blob>(*this);
        return concurrency::streams::file_stream<uint8_t>::open_istream(path).then([instance, condition, options, context] (concurrency::streams::istream stream) -> pplx::task<void>
        {
            return instance->upload_from_stream_async(stream, condition, options, context).then([stream] (pplx::task<void> upload_task) -> pplx::task<void>
            {
                return stream.close().then([upload_task]()
                {
                    upload_task.wait();
                });
            });
        });
    }

    pplx::task<void> cloud_block_blob::upload_text_async(const utility::string_t& content, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        auto utf8_body = utility::conversions::to_utf8string(content);
        auto length = utf8_body.size();
        auto stream = concurrency::streams::bytestream::open_istream(std::move(utf8_body));
        m_properties->set_content_type(protocol::header_value_content_type_utf8);
        return upload_from_stream_async(stream, length, condition, options, context);
    }

    pplx::task<utility::string_t> cloud_block_blob::download_text_async(const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        auto properties = m_properties;
        
        concurrency::streams::container_buffer<std::vector<uint8_t>> buffer;
        return download_to_stream_async(buffer.create_ostream(), condition, options, context).then([buffer, properties] () mutable -> utility::string_t
        {
            if (properties->content_type() != protocol::header_value_content_type_utf8)
            {
                throw std::logic_error(protocol::error_unsupported_text_blob);
            }

            std::string utf8_body(reinterpret_cast<char*>(buffer.collection().data()), static_cast<unsigned int>(buffer.size()));
            return utility::conversions::to_string_t(utf8_body);
        });
    }

    pplx::task<void> cloud_block_blob::set_standard_blob_tier_async(const standard_blob_tier tier, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), type());

        auto command = std::make_shared<core::storage_command<void>>(uri());
        utility::string_t tier_str;

        switch (tier)
        {
        case standard_blob_tier::archive:
            tier_str = protocol::header_value_access_tier_archive;
            break;

        case standard_blob_tier::hot:
            tier_str = protocol::header_value_access_tier_hot;
            break;

        case standard_blob_tier::cool:
            tier_str = protocol::header_value_access_tier_cool;
            break;

        default:
            tier_str = protocol::header_value_access_tier_unknown;
            break;
        }

        auto properties = m_properties;

        command->set_build_request(std::bind(protocol::set_blob_tier, tier_str, condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties, tier](const web::http::http_response& response, const request_result& result, operation_context context) -> void
        {
            protocol::preprocess_response_void(response, result, context);
            properties->m_standard_blob_tier = tier;
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

}} // namespace azure::storage
