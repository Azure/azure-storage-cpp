// -----------------------------------------------------------------------------------------
// <copyright file="cloud_blob_istreambuf.cpp" company="Microsoft">
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
#include "wascore/blobstreams.h"
#include "wascore/resources.h"

namespace azure { namespace storage { namespace core {

    basic_cloud_blob_istreambuf::pos_type basic_cloud_blob_istreambuf::seekpos(basic_cloud_blob_istreambuf::pos_type pos, std::ios_base::openmode direction)
    {
        if (direction & std::ios_base::in)
        {
            auto in_buffer_seek = m_buffer.seekpos(pos - m_current_blob_offset, std::ios_base::in);
            if (in_buffer_seek != static_cast<pos_type>(traits::eof()))
            {
                return in_buffer_seek + m_current_blob_offset;
            }

            pos_type end(size());
            if ((pos >= 0) && (pos <= end))
            {
                // Do not allow read beyond the end.
                m_current_blob_offset = pos;
                m_next_blob_offset = m_current_blob_offset;
                m_buffer = concurrency::streams::container_buffer<std::vector<char_type>>(std::ios_base::in);
                m_blob_hash_provider = hash_provider();
                return pos;
            }
        }
         
        return static_cast<pos_type>(traits::eof());
    }

    pplx::task<basic_cloud_blob_istreambuf::int_type> basic_cloud_blob_istreambuf::_bumpc()
    {
        auto this_pointer = std::dynamic_pointer_cast<basic_cloud_blob_istreambuf>(shared_from_this());
        return download_if_necessary(1).then([this_pointer] (bool) -> pplx::task<basic_cloud_blob_istreambuf::int_type>
        {
            return this_pointer->m_buffer.bumpc();
        });
    }

    basic_cloud_blob_istreambuf::int_type basic_cloud_blob_istreambuf::_sbumpc()
    {
        auto result = m_buffer.sbumpc();
        return (result == traits::eof()) ? traits::requires_async() : result;
    }

    pplx::task<basic_cloud_blob_istreambuf::int_type> basic_cloud_blob_istreambuf::_getc()
    {
        auto this_pointer = std::dynamic_pointer_cast<basic_cloud_blob_istreambuf>(shared_from_this());
        return download_if_necessary(1).then([this_pointer] (bool) -> pplx::task<basic_cloud_blob_istreambuf::int_type>
        {
            return this_pointer->m_buffer.getc();
        });
    }

    basic_cloud_blob_istreambuf::int_type basic_cloud_blob_istreambuf::_sgetc()
    {
        auto result = m_buffer.sgetc();
        return (result == traits::eof()) ? traits::requires_async() : result;
    }

    pplx::task<basic_cloud_blob_istreambuf::int_type> basic_cloud_blob_istreambuf::_nextc()
    {
        auto this_pointer = std::dynamic_pointer_cast<basic_cloud_blob_istreambuf>(shared_from_this());
        return download_if_necessary(2).then([this_pointer] (bool) -> pplx::task<basic_cloud_blob_istreambuf::int_type>
        {
            return this_pointer->m_buffer.nextc();
        });
    }

    pplx::task<basic_cloud_blob_istreambuf::int_type> basic_cloud_blob_istreambuf::_ungetc()
    {
        return m_buffer.ungetc();
    }

    pplx::task<size_t> basic_cloud_blob_istreambuf::_getn(_Out_writes_(count) char_type* ptr, _In_ size_t count)
    {
        auto this_pointer = std::dynamic_pointer_cast<basic_cloud_blob_istreambuf>(shared_from_this());
        return download_if_necessary(1).then([this_pointer, ptr, count] (bool) -> pplx::task<size_t>
        {
            return this_pointer->m_buffer.getn(ptr, count);
        });
    }

    size_t basic_cloud_blob_istreambuf::_scopy(_Out_writes_(count) char_type* ptr, _In_ size_t count)
    {
        return m_buffer.scopy(ptr, count);
    }
    
    pplx::task<bool> basic_cloud_blob_istreambuf::download_if_necessary(size_t bytes_needed)
    {
        if (m_buffer.in_avail() < bytes_needed)
        {
            return download();
        }
        else
        {
            return pplx::task_from_result<bool>(true);
        }
    }

    pplx::task<bool> basic_cloud_blob_istreambuf::download()
    {
        m_current_blob_offset = m_next_blob_offset;

        utility::size64_t read_size = size() - m_current_blob_offset;
        if (read_size == 0)
        {
            return pplx::task_from_result<bool>(false);
        }

        m_buffer_size = m_next_buffer_size;
        if (read_size > m_buffer_size)
        {
            read_size = m_buffer_size;
        }

        m_next_blob_offset = m_current_blob_offset + read_size;

        std::vector<char_type>& internal_buffer = m_buffer.collection();
        internal_buffer.resize(static_cast<std::vector<char_type>::size_type>(read_size));
        concurrency::streams::container_buffer<std::vector<char_type>> temp_buffer(std::move(internal_buffer), std::ios_base::out);
        temp_buffer.seekpos(0, std::ios_base::out);

        auto this_pointer = std::dynamic_pointer_cast<basic_cloud_blob_istreambuf>(shared_from_this());
        return m_blob->download_range_to_stream_async(temp_buffer.create_ostream(), m_current_blob_offset, read_size, m_condition, m_options, m_context).then([this_pointer, temp_buffer] (pplx::task<void> download_task) -> pplx::task<bool>
        {
            try
            {
                download_task.wait();
                this_pointer->m_buffer = concurrency::streams::container_buffer<std::vector<char_type>>(std::move(temp_buffer.collection()), std::ios_base::in);
                this_pointer->m_buffer.seekpos(0, std::ios_base::in);

                // Validate the blob's content MD5 hash
                if (this_pointer->m_blob_hash_provider.is_enabled())
                {
                    std::vector<char_type>& result_buffer = this_pointer->m_buffer.collection();
                    this_pointer->m_blob_hash_provider.write(result_buffer.data(), result_buffer.size());

                    if (((utility::size64_t) this_pointer->m_next_blob_offset) == this_pointer->size())
                    {
                        this_pointer->m_blob_hash_provider.close();
                        if (this_pointer->m_blob->properties().content_md5() != this_pointer->m_blob_hash_provider.hash())
                        {
                            throw storage_exception(protocol::error_md5_mismatch);
                        }
                    }
                }

                return pplx::task_from_result<bool>(true);
            }
            catch (const std::exception&)
            {
                this_pointer->m_currentException = std::current_exception();
                return pplx::task_from_result<bool>(false);
            }
        });
    }

}}} // namespace azure::storage::core
