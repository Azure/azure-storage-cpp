// -----------------------------------------------------------------------------------------
// <copyright file="streams.cpp" company="Microsoft">
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
#include "was/error_code_strings.h"
#include "wascore/blobstreams.h"
#include "wascore/logging.h"
#include "wascore/resources.h"

namespace azure { namespace storage { namespace core {


    pplx::task<void> basic_cloud_ostreambuf::_close_write()
    {
        if (m_committed)
        {
            throw std::logic_error(protocol::error_closed_stream);
        }

        m_committed = true;
        basic_ostreambuf<basic_cloud_ostreambuf::char_type>::_close_write().wait();

        if (m_total_hash_provider.is_enabled())
        {
            m_total_hash_provider.close();
        }

        return commit_close();
    }

    std::shared_ptr<basic_cloud_ostreambuf::buffer_to_upload> basic_cloud_ostreambuf::prepare_buffer()
    {
        utility::string_t block_md5;
        if (m_transaction_hash_provider.is_enabled())
        {
            m_transaction_hash_provider.close();
            block_md5 = m_transaction_hash_provider.hash();
            m_transaction_hash_provider = hash_provider::create_md5_hash_provider();
        }

        auto buffer = std::make_shared<basic_cloud_ostreambuf::buffer_to_upload>(m_buffer, block_md5);
        m_buffer = concurrency::streams::container_buffer<std::vector<char_type>>();
        m_buffer_size = m_next_buffer_size;
        return buffer;
    }

    pplx::task<basic_cloud_ostreambuf::int_type> basic_cloud_ostreambuf::_putc(concurrency::streams::ostream::traits::char_type ch)
    {
        pplx::task<void> upload_task = pplx::task_from_result();

        m_current_streambuf_offset += 1;
        auto result = m_buffer.putc(ch).get();
        if (m_buffer_size == m_buffer.in_avail())
        {
            upload_task = upload_buffer();
        }

        return upload_task.then([result]() -> basic_cloud_ostreambuf::int_type
        {
            return result;
        });
    }

    pplx::task<size_t> basic_cloud_ostreambuf::_putn(const concurrency::streams::ostream::traits::char_type* ptr, size_t count)
    {
        pplx::task<void> upload_task = pplx::task_from_result();

        m_current_streambuf_offset += count;
        auto remaining = count;
        while (remaining > 0)
        {
            auto write_size = m_buffer_size - m_buffer.in_avail();
            if (write_size > remaining)
            {
                write_size = remaining;
            }

            if (m_transaction_hash_provider.is_enabled())
            {
                m_transaction_hash_provider.write(ptr, write_size);
            }

            if (m_total_hash_provider.is_enabled())
            {
                m_total_hash_provider.write(ptr, write_size);
            }

            // The streambuf is waited because it is a memory buffer, so does not involve async I/O
            m_buffer.putn_nocopy(ptr, write_size).wait();
            if (m_buffer_size == m_buffer.size())
            {
                upload_task = upload_buffer();
            }

            ptr += write_size;
            remaining -= write_size;
        }

        return upload_task.then([count]() -> size_t
        {
            return count;
        });
    }

}}} // azure::storage::core