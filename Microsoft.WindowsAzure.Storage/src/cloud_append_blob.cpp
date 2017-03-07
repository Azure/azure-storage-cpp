// -----------------------------------------------------------------------------------------
// <copyright file="cloud_append_blob.cpp" company="Microsoft">
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

    pplx::task<void> cloud_append_blob::create_or_replace_async(const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), type());

        auto properties = m_properties;

        auto command = std::make_shared<core::storage_command<void>>(uri());
        command->set_build_request(std::bind(protocol::put_append_blob, *properties, metadata(), condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)
        {
            protocol::preprocess_response_void(response, result, context);
            properties->update_etag_and_last_modified(protocol::blob_response_parsers::parse_blob_properties(response));
            properties->m_size = 0;
        });
        return core::executor<void>::execute_async(command, modified_options, context);
    }

    pplx::task<int64_t> cloud_append_blob::append_block_async(concurrency::streams::istream block_data, const utility::string_t& content_md5, const access_condition& condition, const blob_request_options& options, operation_context context) const
    {
        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), type());

        auto properties = m_properties;
        bool needs_md5 = content_md5.empty() && modified_options.use_transactional_md5();

        auto command = std::make_shared<core::storage_command<int64_t>>(uri());
        command->set_authentication_handler(service_client().authentication_handler());
        command->set_preprocess_response([properties](const web::http::http_response& response, const request_result& result, operation_context context)->int64_t
        {
            protocol::preprocess_response_void(response, result, context);

            auto parsed_properties = protocol::blob_response_parsers::parse_blob_properties(response);
            properties->update_etag_and_last_modified(parsed_properties);
            properties->update_append_blob_committed_block_count(parsed_properties);
            return utility::conversions::scan_string<int64_t>(protocol::get_header_value(response.headers(), protocol::ms_header_blob_append_offset));
        });
        return core::istream_descriptor::create(block_data, needs_md5, std::numeric_limits<utility::size64_t>::max(), protocol::max_append_block_size).then([command, context, content_md5, modified_options, condition] (core::istream_descriptor request_body) -> pplx::task<int64_t>
        {
            const utility::string_t& md5 = content_md5.empty() ? request_body.content_md5() : content_md5;
            command->set_build_request(std::bind(protocol::append_block, md5, condition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            command->set_request_body(request_body);
            return core::executor<int64_t>::execute_async(command, modified_options, context);
        });
    }

    pplx::task<utility::string_t> cloud_append_blob::download_text_async(const access_condition& condition, const blob_request_options& options, operation_context context)
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

    pplx::task<concurrency::streams::ostream> cloud_append_blob::open_write_async(bool create_new, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        assert_no_snapshot();
        blob_request_options modified_options(options);
        modified_options.apply_defaults(service_client().default_request_options(), type(), false);

        pplx::task<void> create_task;
        if (create_new)
        {
            create_task = create_or_replace_async(condition, modified_options, context);
        }
        else
        {
            if (modified_options.store_blob_content_md5())
            {
                throw std::logic_error(protocol::error_md5_not_possible);
            }

            create_task = download_attributes_async(condition, modified_options, context);
        }

        auto instance = std::make_shared<cloud_append_blob>(*this);
        return create_task.then([instance, condition, modified_options, context]()
        {
            auto modified_condition = access_condition::generate_lease_condition(condition.lease_id());
            if (condition.max_size() != -1)
            {
                modified_condition.set_max_size(condition.max_size());
            }

            if (condition.append_position() != -1)
            {
                modified_condition.set_append_position(condition.append_position());
            }

            return core::cloud_append_blob_ostreambuf(instance, modified_condition, modified_options, context).create_ostream();
        });
    }

    pplx::task<void> cloud_append_blob::upload_from_stream_async(concurrency::streams::istream source, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        return upload_from_stream_internal_async(source, length, true, condition, options, context);
    }

    pplx::task<void> cloud_append_blob::upload_from_stream_internal_async(concurrency::streams::istream source, utility::size64_t length, bool create_new, const access_condition& condition, const blob_request_options& options, operation_context context)
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

        return open_write_async(create_new, condition, modified_options, context).then([source, length](concurrency::streams::ostream blob_stream) -> pplx::task<void>
        {
            return core::stream_copy_async(source, blob_stream, length).then([blob_stream](utility::size64_t) -> pplx::task<void>
            {
                return blob_stream.close();
            });
        });
    }

    pplx::task<void> cloud_append_blob::upload_from_file_async(const utility::string_t &path, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        auto instance = std::make_shared<cloud_append_blob>(*this);
        return concurrency::streams::file_stream<uint8_t>::open_istream(path).then([instance, condition, options, context](concurrency::streams::istream stream) -> pplx::task<void>
        {
            return instance->upload_from_stream_async(stream, condition, options, context).then([stream](pplx::task<void> upload_task) -> pplx::task<void>
            {
                return stream.close().then([upload_task]()
                {
                    upload_task.wait();
                });
            });
        });
    }

    pplx::task<void> cloud_append_blob::upload_text_async(const utility::string_t& content, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        auto utf8_body = utility::conversions::to_utf8string(content);
        auto length = utf8_body.size();
        auto stream = concurrency::streams::bytestream::open_istream(std::move(utf8_body));
        m_properties->set_content_type(protocol::header_value_content_type_utf8);
        return upload_from_stream_async(stream, length, condition, options, context);
    }

    pplx::task<void> cloud_append_blob::append_from_stream_async(concurrency::streams::istream source, utility::size64_t length, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        return upload_from_stream_internal_async(source, length, false, condition, options, context);
    }

    pplx::task<void> cloud_append_blob::append_from_file_async(const utility::string_t &path, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        auto instance = std::make_shared<cloud_append_blob>(*this);
        return concurrency::streams::file_stream<uint8_t>::open_istream(path).then([instance, condition, options, context](concurrency::streams::istream stream) -> pplx::task<void>
        {
            return instance->append_from_stream_async(stream, condition, options, context).then([stream](pplx::task<void> upload_task) -> pplx::task<void>
            {
                return stream.close().then([upload_task]()
                {
                    upload_task.wait();
                });
            });
        });
    }

    pplx::task<void> cloud_append_blob::append_text_async(const utility::string_t& content, const access_condition& condition, const blob_request_options& options, operation_context context)
    {
        auto utf8_body = utility::conversions::to_utf8string(content);
        auto length = utf8_body.size();
        auto stream = concurrency::streams::bytestream::open_istream(std::move(utf8_body));
        m_properties->set_content_type(protocol::header_value_content_type_utf8);
        return append_from_stream_async(stream, length, condition, options, context);
    }
}} // namespace azure::storage
