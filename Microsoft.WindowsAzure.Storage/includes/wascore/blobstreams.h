// -----------------------------------------------------------------------------------------
// <copyright file="blobstreams.h" company="Microsoft">
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
#include "was/blob.h"

namespace azure { namespace storage { namespace core {

    class basic_cloud_blob_istreambuf : public basic_istreambuf<concurrency::streams::ostream::traits::char_type>
    {
    public:

        basic_cloud_blob_istreambuf(std::shared_ptr<cloud_blob> blob, const access_condition &condition, const blob_request_options& options, operation_context context)
            : basic_istreambuf<concurrency::streams::ostream::traits::char_type>(),
            m_blob(blob), m_condition(condition), m_options(options), m_context(context),
            m_current_blob_offset(0), m_next_blob_offset(0), m_buffer_size(options.stream_read_size_in_bytes()),
            m_next_buffer_size(options.stream_read_size_in_bytes()), m_buffer(std::ios_base::in)
        {
            if (!options.disable_content_md5_validation() && !m_blob->properties().content_md5().empty())
            {
                m_blob_hash_provider = hash_provider::create_md5_hash_provider();
            }
        }

        bool can_seek() const
        {
            return is_open();
        }

        bool has_size() const
        {
            return true;
        }

        utility::size64_t size() const
        {
            return m_blob->properties().size();
        }

        size_t buffer_size(std::ios_base::openmode direction) const
        {
            if (direction == std::ios_base::in)
            {
                return m_next_buffer_size;
            }
            else
            {
                return (size_t)0;
            }
        }

        void set_buffer_size(size_t size, std::ios_base::openmode direction)
        {
            if (direction == std::ios_base::in)
            {
                m_next_buffer_size = size;
            }
            else
            {
                // no-op, because this stream does not support writing
            }
        }

        size_t in_avail() const
        {
            return m_buffer.in_avail();
        }

        pos_type getpos(std::ios_base::openmode direction) const
        {
            if (direction == std::ios_base::in)
            {
                return (pos_type)m_current_blob_offset + m_buffer.getpos(direction);
            }
            else
            {
                return (pos_type)traits::eof();
            }
        }

        pos_type seekoff(off_type offset, std::ios_base::seekdir way, std::ios_base::openmode direction)
        {
            if (direction == std::ios_base::in)
            {
                pos_type new_pos;
                switch (way)
                {
                case std::ios_base::beg:
                    new_pos = (pos_type)offset;
                    break;

                case std::ios_base::cur:
                    new_pos = (pos_type)(offset + getpos(direction));
                    break;

                case std::ios_base::end:
                    new_pos = (pos_type)(offset + size());
                    break;
                }

                return seekpos(new_pos, direction);
            }
             
            return (pos_type)traits::eof();
        }

        bool acquire(_Out_writes_(count) char_type*& ptr, _In_ size_t& count)
        {
            UNREFERENCED_PARAMETER(ptr);
            UNREFERENCED_PARAMETER(count);
            return false;
        }

        void release(_Out_writes_(count) char_type* ptr, _In_ size_t count)
        {
            UNREFERENCED_PARAMETER(ptr);
            UNREFERENCED_PARAMETER(count);
            // no-op, as blob streams do not support acquire/release
        }

        pos_type seekpos(pos_type pos, std::ios_base::openmode direction);
        pplx::task<int_type> _bumpc();
        int_type _sbumpc();
        pplx::task<int_type> _getc();
        int_type _sgetc();
        pplx::task<int_type> _nextc();
        pplx::task<int_type> _ungetc();
        pplx::task<size_t> _getn(_Out_writes_(count) char_type* ptr, _In_ size_t count);
        size_t _scopy(_Out_writes_(count) char_type* ptr, _In_ size_t count);

    private:

        pplx::task<bool> download_if_necessary(size_t bytes_needed);
        pplx::task<bool> download();

        std::shared_ptr<cloud_blob> m_blob;
        access_condition m_condition;
        blob_request_options m_options;
        operation_context m_context;
        hash_provider m_blob_hash_provider;
        off_type m_current_blob_offset;
        off_type m_next_blob_offset;
        size_t m_buffer_size;
        size_t m_next_buffer_size;
        concurrency::streams::container_buffer<std::vector<char_type>> m_buffer;
    };


    class cloud_blob_istreambuf : public concurrency::streams::streambuf<basic_cloud_blob_istreambuf::char_type>
    {
    public:
        cloud_blob_istreambuf(std::shared_ptr<cloud_blob> blob, const access_condition &condition, const blob_request_options& options, operation_context context)
            : concurrency::streams::streambuf<basic_cloud_blob_istreambuf::char_type>(std::make_shared<basic_cloud_blob_istreambuf>(blob, condition, options, context))
        {
        }
    };

    class basic_cloud_blob_ostreambuf : public basic_ostreambuf<concurrency::streams::ostream::traits::char_type>
    {
    public:
        basic_cloud_blob_ostreambuf(const access_condition &condition, const blob_request_options& options, operation_context context)
            : basic_ostreambuf<concurrency::streams::ostream::traits::char_type>(),
            m_condition(condition), m_options(options), m_context(context), m_semaphore(options.parallelism_factor()),
            m_buffer_size(options.stream_write_size_in_bytes()), m_next_buffer_size(options.stream_write_size_in_bytes()),
            m_current_streambuf_offset(0), m_committed(false)
        {
            if (options.use_transactional_md5())
            {
                m_block_hash_provider = hash_provider::create_md5_hash_provider();
            }

            if (options.store_blob_content_md5())
            {
                m_blob_hash_provider = hash_provider::create_md5_hash_provider();
            }
        }

        size_t buffer_size(std::ios_base::openmode direction) const
        {
            if (direction == std::ios_base::out)
            {
                return m_next_buffer_size;
            }
            else
            {
                return (size_t)0;
            }
        }

        void set_buffer_size(size_t size, std::ios_base::openmode direction)
        {
            if (direction == std::ios_base::out)
            {
                m_next_buffer_size = size;
            }
            else
            {
                // no-op, because blob streams do not support reading
            }
        }

        pos_type getpos(std::ios_base::openmode direction) const
        {
            if (direction == std::ios_base::out)
            {
                return (pos_type)m_current_streambuf_offset;
            }
            else
            {
                return (pos_type)traits::eof();
            }
        }

        pos_type seekoff(off_type offset, std::ios_base::seekdir way, std::ios_base::openmode direction)
        {
            if (direction == std::ios_base::out)
            {
                pos_type new_pos;
                switch (way)
                {
                case std::ios_base::beg:
                    new_pos = (pos_type)offset;
                    break;

                case std::ios_base::cur:
                    new_pos = (pos_type)(offset + getpos(direction));
                    break;

                case std::ios_base::end:
                    new_pos = (pos_type)(offset + size());
                    break;
                }

                return seekpos(new_pos, direction);
            }
         
            return (pos_type)traits::eof();
        }

        char_type* _alloc(_In_ size_t count)
        {
            UNREFERENCED_PARAMETER(count);
            return nullptr;
        }

        void _commit(_In_ size_t count)
        {
            UNREFERENCED_PARAMETER(count);
            // no-op, as blob streams do not support alloc/commit
        }

        pplx::task<void> _close_write();
        pplx::task<bool> _sync();
        pplx::task<int_type> _putc(char_type ch);
        pplx::task<size_t> _putn(const char_type* ptr, size_t count);

    protected:

        class buffer_to_upload
        {
        public:
            buffer_to_upload(concurrency::streams::container_buffer<std::vector<char_type>> buffer, const utility::string_t& content_md5)
                : m_size(buffer.size()),
                m_stream(concurrency::streams::container_stream<std::vector<char_type>>::open_istream(std::move(buffer.collection()))),
                m_content_md5(content_md5)
            {
            }

            concurrency::streams::istream stream() const
            {
                return m_stream;
            }
            
            utility::size64_t size() const
            {
                return m_size;
            }

            bool is_empty() const
            {
                return m_size == 0;
            }

            const utility::string_t& content_md5() const
            {
                return m_content_md5;
            }

        private:

            // Note: m_size must be initialized before m_stream, and thus must be listed first in this list.
            // This is because we use std::move to initialize m_stream, but we need to get the size first.
            utility::size64_t m_size;
            utility::string_t m_content_md5;
            concurrency::streams::istream m_stream;
        };

        concurrency::streams::container_buffer<std::vector<char_type>> m_buffer;
        pos_type m_current_streambuf_offset;
        hash_provider m_blob_hash_provider;
        hash_provider m_block_hash_provider;
        access_condition m_condition;
        blob_request_options m_options;
        operation_context m_context;
        async_semaphore m_semaphore;

        virtual pplx::task<void> upload_buffer() = 0;
        virtual pplx::task<void> commit_blob() = 0;
        std::shared_ptr<buffer_to_upload> prepare_buffer();

    private:

        size_t m_buffer_size;
        size_t m_next_buffer_size;
        bool m_committed;
    };

    class basic_cloud_block_blob_ostreambuf : public basic_cloud_blob_ostreambuf
    {
    public:
        basic_cloud_block_blob_ostreambuf(std::shared_ptr<cloud_block_blob> blob, const access_condition &condition, const blob_request_options& options, operation_context context)
            : basic_cloud_blob_ostreambuf(condition, options, context),
            m_blob(blob), m_block_id_prefix(utility::uuid_to_string(utility::new_uuid()))
        {
        }

        bool can_seek() const
        {
            return false;
        }

        bool has_size() const
        {
            return false;
        }

        utility::size64_t size() const
        {
            return (utility::size64_t)0;
        }

        pos_type seekpos(pos_type pos, std::ios_base::openmode direction)
        {
            UNREFERENCED_PARAMETER(pos);
            UNREFERENCED_PARAMETER(direction);
            return (pos_type)traits::eof();
        }

    protected:

        pplx::task<void> upload_buffer();
        pplx::task<void> commit_blob();

    private:

        utility::string_t get_next_block_id();

        std::shared_ptr<cloud_block_blob> m_blob;
        utility::string_t m_block_id_prefix;
        std::vector<block_list_item> m_block_list;
    };

    class cloud_block_blob_ostreambuf : public concurrency::streams::streambuf<basic_cloud_block_blob_ostreambuf::char_type>
    {
    public:

        cloud_block_blob_ostreambuf(std::shared_ptr<cloud_block_blob> blob,const access_condition &condition, const blob_request_options& options, operation_context context)
            : concurrency::streams::streambuf<basic_cloud_block_blob_ostreambuf::char_type>(std::make_shared<basic_cloud_block_blob_ostreambuf>(blob, condition, options, context))
        {
        }
    };

    class basic_cloud_page_blob_ostreambuf : public basic_cloud_blob_ostreambuf
    {
    public:

        basic_cloud_page_blob_ostreambuf(std::shared_ptr<cloud_page_blob> blob, utility::size64_t blob_size, const access_condition &condition, const blob_request_options& options, operation_context context)
            : basic_cloud_blob_ostreambuf(condition, options, context),
            m_blob(blob), m_blob_size(blob_size), m_current_blob_offset(0)
        {
        }

        bool can_seek() const
        {
            return can_write() && !m_options.store_blob_content_md5();
        }

        bool has_size() const
        {
            return true;
        }

        utility::size64_t size() const
        {
            return m_blob_size;
        }

        pos_type seekpos(pos_type pos, std::ios_base::openmode direction)
        {
            if (can_seek() && (direction == std::ios_base::out))
            {
                if ((pos < (pos_type)0) || (pos > (pos_type)size()))
                {
                    return (pos_type)traits::eof();
                }

                sync().wait();
                m_current_blob_offset = pos;
                m_current_streambuf_offset = pos;
                return (pos_type)m_current_streambuf_offset;
            }
            else
            {
                return (pos_type)traits::eof();
            }
        }

    protected:

        pplx::task<void> upload_buffer();
        pplx::task<void> commit_blob();

    private:

        std::shared_ptr<cloud_page_blob> m_blob;
        utility::size64_t m_blob_size;
        int64_t m_current_blob_offset;
    };

    class cloud_page_blob_ostreambuf : public concurrency::streams::streambuf<basic_cloud_page_blob_ostreambuf::char_type>
    {
    public:

        cloud_page_blob_ostreambuf(std::shared_ptr<cloud_page_blob> blob, utility::size64_t blob_size, const access_condition &condition, const blob_request_options& options, operation_context context)
            : concurrency::streams::streambuf<basic_cloud_page_blob_ostreambuf::char_type>(std::make_shared<basic_cloud_page_blob_ostreambuf>(blob, blob_size, condition, options, context))
        {
        }
    };

}}} // namespace azure::storage::core
