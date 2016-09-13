// -----------------------------------------------------------------------------------------
// <copyright file="streams.h" company="Microsoft">
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
#include "streambuf.h"
#include "async_semaphore.h"
#include "was/common.h"

namespace azure { namespace storage { namespace core {

    template<typename _CharType>
    class hash_wrapper_streambuf : public concurrency::streams::streambuf<_CharType>
    {
    public:
        hash_wrapper_streambuf()
            : concurrency::streams::streambuf<_CharType>()
        {
        }

        hash_wrapper_streambuf(concurrency::streams::streambuf<_CharType> inner_streambuf, hash_provider provider)
            : concurrency::streams::streambuf<_CharType>(std::make_shared<basic_hash_wrapper_streambuf<_CharType>>(inner_streambuf, provider))
        {
        }

        utility::size64_t total_written() const
        {
            const basic_hash_wrapper_streambuf<_CharType>* base = static_cast<basic_hash_wrapper_streambuf<_CharType>*>(Concurrency::streams::streambuf<_CharType>::get_base().get());
            return base->total_written();
        }

        utility::string_t hash() const
        {
            const basic_hash_wrapper_streambuf<_CharType>* base = static_cast<basic_hash_wrapper_streambuf<_CharType>*>(Concurrency::streams::streambuf<_CharType>::get_base().get());
            return base->hash();
        }
    };

    class basic_cloud_ostreambuf : public basic_ostreambuf<concurrency::streams::ostream::traits::char_type>
    {
    public:
        basic_cloud_ostreambuf()
            : basic_ostreambuf<concurrency::streams::ostream::traits::char_type>(),
            m_current_streambuf_offset(0), m_committed(false)
        {
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
        hash_provider m_total_hash_provider;
        hash_provider m_transaction_hash_provider;

        virtual pplx::task<void> upload_buffer() = 0;
        virtual pplx::task<void> commit_close() = 0;
        std::shared_ptr<buffer_to_upload> prepare_buffer();

        size_t m_buffer_size;
        size_t m_next_buffer_size;

    private:

        bool m_committed;
    };

}}} // namespace azure::storage::core
