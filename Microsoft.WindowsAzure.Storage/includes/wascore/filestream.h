// -----------------------------------------------------------------------------------------
// <copyright file="filestream.h" company="Microsoft">
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

#include "basic_types.h"
#include "streams.h"
#include "async_semaphore.h"
#include "util.h"
#include "was/file.h"

namespace azure { namespace storage { namespace core {

    class basic_cloud_file_ostreambuf : public basic_cloud_ostreambuf
    {
    public:
        basic_cloud_file_ostreambuf(std::shared_ptr<cloud_file> file, utility::size64_t length, const file_access_condition& access_condition, const file_request_options& options, operation_context context)
            : m_file(file), m_file_length(length), m_condition(access_condition), m_options(options), m_context(context),
            m_semaphore(options.parallelism_factor()),m_current_file_offset(0)
        {
            m_buffer_size = protocol::max_range_size;
            m_next_buffer_size = protocol::max_range_size;

            if (m_options.use_transactional_md5())
            {
                m_transaction_hash_provider = hash_provider::create_md5_hash_provider();
            }
            if (m_options.store_file_content_md5())
            {
                m_total_hash_provider = hash_provider::create_md5_hash_provider();
            }
        }

        bool can_seek() const
        {
            return can_write() && !m_options.store_file_content_md5();
        }

        bool has_size() const
        {
            return true;
        }

        utility::size64_t size() const
        {
            return m_file_length;
        }

        pos_type seekpos(pos_type pos, std::ios_base::openmode direction)
        {
            if (can_seek() && (direction == std::ios_base::out))
            {
                if ((pos < (pos_type)0) || (pos >(pos_type)size()))
                {
                    return (pos_type)traits::eof();
                }

                sync().wait();
                m_current_file_offset = pos;
                m_current_streambuf_offset = pos;
                return (pos_type)m_current_streambuf_offset;
            }
            else
            {
                return (pos_type)traits::eof();
            }
        }

        pplx::task<bool> _sync();

    protected:

        pplx::task<void> upload_buffer();
        pplx::task<void> commit_close();

        std::shared_ptr<cloud_file> m_file;
        utility::size64_t m_file_length;
        file_access_condition m_condition;
        file_request_options m_options;
        operation_context m_context;
        int64_t m_current_file_offset;
        async_semaphore m_semaphore;

    };

    class cloud_file_ostreambuf : public concurrency::streams::streambuf<basic_cloud_file_ostreambuf::char_type>
    {
    public:
        cloud_file_ostreambuf(std::shared_ptr<cloud_file> file, utility::size64_t length, const file_access_condition& access_condition, const file_request_options& options, operation_context context)
            : concurrency::streams::streambuf<basic_cloud_file_ostreambuf::char_type>(std::make_shared<basic_cloud_file_ostreambuf>(file, length, access_condition, options, context))
        {
        }

    };


}}} // azure::storage::core