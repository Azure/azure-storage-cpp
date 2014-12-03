// -----------------------------------------------------------------------------------------
// <copyright file="cloud_blob_ostreambuf.cpp" company="Microsoft">
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

    pplx::task<void> basic_cloud_blob_ostreambuf::_close_write()
    {
        if (m_committed)
        {
            throw std::logic_error(protocol::error_closed_stream);
        }

        m_committed = true;
        basic_ostreambuf<basic_cloud_blob_ostreambuf::char_type>::_close_write().wait();
        
        if (m_blob_hash_provider.is_enabled())
        {
            m_blob_hash_provider.close();
        }

        return commit_blob();
    }

    pplx::task<bool> basic_cloud_blob_ostreambuf::_sync()
    {
        upload_buffer();

        auto this_pointer = std::dynamic_pointer_cast<basic_cloud_blob_ostreambuf>(shared_from_this());
        return m_semaphore.wait_all_async().then([this_pointer] () -> pplx::task<bool>
        {
            if (this_pointer->m_currentException == nullptr)
            {
                return pplx::task_from_result<bool>(true);
            }
            else
            {
                return pplx::task_from_exception<bool>(this_pointer->m_currentException);
            }
        });
    }

    std::shared_ptr<basic_cloud_blob_ostreambuf::buffer_to_upload> basic_cloud_blob_ostreambuf::prepare_buffer()
    {
        utility::string_t block_md5;
        if (m_block_hash_provider.is_enabled())
        {
            m_block_hash_provider.close();
            block_md5 = m_block_hash_provider.hash();
            m_block_hash_provider = hash_provider::create_md5_hash_provider();
        }

        auto buffer = std::make_shared<basic_cloud_blob_ostreambuf::buffer_to_upload>(m_buffer, block_md5);
        m_buffer = concurrency::streams::container_buffer<std::vector<char_type>>();
        m_buffer_size = m_next_buffer_size;
        return buffer;
    }

    pplx::task<basic_cloud_blob_ostreambuf::int_type> basic_cloud_blob_ostreambuf::_putc(concurrency::streams::ostream::traits::char_type ch)
    {
        pplx::task<void> upload_task = pplx::task_from_result();
        
        auto result = m_buffer.putc(ch).get();
        if (m_buffer_size == m_buffer.in_avail())
        {
            upload_task = upload_buffer();
        }

        return upload_task.then([result] () -> basic_cloud_blob_ostreambuf::int_type
        {
            return result;
        });
    }

    pplx::task<size_t> basic_cloud_blob_ostreambuf::_putn(const concurrency::streams::ostream::traits::char_type* ptr, size_t count)
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

            if (m_block_hash_provider.is_enabled())
            {
                m_block_hash_provider.write(ptr, write_size);
            }

            if (m_blob_hash_provider.is_enabled())
            {
                m_blob_hash_provider.write(ptr, write_size);
            }

            // The streambuf is waited because it is a memory buffer, so does not involve async I/O
            m_buffer.putn(ptr, write_size).wait();
            if (m_buffer_size == m_buffer.size())
            {
                upload_task = upload_buffer();
            }

            ptr += write_size;
            remaining -= write_size;
        }

        return upload_task.then([count] () -> size_t
        {
            return count;
        });
    }

    pplx::task<void> basic_cloud_block_blob_ostreambuf::upload_buffer()
    {
        auto buffer = prepare_buffer();
        if (buffer->is_empty())
        {
            return pplx::task_from_result();
        }

        auto block_id = get_next_block_id();
        
        auto this_pointer = std::dynamic_pointer_cast<basic_cloud_block_blob_ostreambuf>(shared_from_this());
        return m_semaphore.lock_async().then([this_pointer, buffer, block_id] ()
        {
            if (this_pointer->m_currentException == nullptr)
            {
                try
                {
                    this_pointer->m_blob->upload_block_async(block_id, buffer->stream(), buffer->content_md5(), this_pointer->m_condition, this_pointer->m_options, this_pointer->m_context).then([this_pointer] (pplx::task<void> upload_task)
                    {
                        std::lock_guard<async_semaphore> guard(this_pointer->m_semaphore, std::adopt_lock);
                        try
                        {
                            upload_task.wait();
                        }
                        catch (const std::exception&)
                        {
                            this_pointer->m_currentException = std::current_exception();
                        }
                    });
                }
                catch (...)
                {
                    this_pointer->m_semaphore.unlock();
                }
            }
            else
            {
                this_pointer->m_semaphore.unlock();
            }
        });
    }

    pplx::task<void> basic_cloud_block_blob_ostreambuf::commit_blob()
    {
        auto this_pointer = std::dynamic_pointer_cast<basic_cloud_block_blob_ostreambuf>(shared_from_this());
        return _sync().then([this_pointer] (bool) -> pplx::task<void>
        {
            if (this_pointer->m_blob_hash_provider.is_enabled())
            {
                this_pointer->m_blob->properties().set_content_md5(this_pointer->m_blob_hash_provider.hash());
            }

            return this_pointer->m_blob->upload_block_list_async(this_pointer->m_block_list, this_pointer->m_condition, this_pointer->m_options, this_pointer->m_context);
        });
    }

    utility::string_t basic_cloud_block_blob_ostreambuf::get_next_block_id()
    {
        utility::ostringstream_t str;
        str << m_block_id_prefix << U('-') << std::setw(6) << std::setfill(U('0')) << m_block_list.size();
        auto utf8_block_id = utility::conversions::to_utf8string(str.str());
        std::vector<unsigned char> block_id_as_array(utf8_block_id.cbegin(), utf8_block_id.cend());
        utility::string_t block_id(utility::conversions::to_base64(block_id_as_array));
        m_block_list.push_back(block_list_item(block_id));
        return block_id;
    }

    pplx::task<void> basic_cloud_page_blob_ostreambuf::upload_buffer()
    {
        auto buffer = prepare_buffer();
        if (buffer->is_empty())
        {
            return pplx::task_from_result();
        }

        auto offset = m_current_blob_offset;
        m_current_blob_offset += buffer->size();

        auto this_pointer = std::dynamic_pointer_cast<basic_cloud_page_blob_ostreambuf>(shared_from_this());
        return m_semaphore.lock_async().then([this_pointer, buffer, offset] ()
        {
            if (this_pointer->m_currentException == nullptr)
            {
                try
                {
                    this_pointer->m_blob->upload_pages_async(buffer->stream(), offset, buffer->content_md5(), this_pointer->m_condition, this_pointer->m_options, this_pointer->m_context).then([this_pointer] (pplx::task<void> upload_task)
                    {
                        std::lock_guard<async_semaphore> guard(this_pointer->m_semaphore, std::adopt_lock);
                        try
                        {
                            upload_task.wait();
                        }
                        catch (const std::exception&)
                        {
                            this_pointer->m_currentException = std::current_exception();
                        }
                    });
                }
                catch (...)
                {
                    this_pointer->m_semaphore.unlock();
                }
            }
            else
            {
                this_pointer->m_semaphore.unlock();
            }
        });
    }

    pplx::task<void> basic_cloud_page_blob_ostreambuf::commit_blob()
    {
        if (m_blob_hash_provider.is_enabled())
        {
            auto this_pointer = std::dynamic_pointer_cast<basic_cloud_page_blob_ostreambuf>(shared_from_this());
            return _sync().then([this_pointer] (bool) -> pplx::task<void>
            {
                this_pointer->m_blob->properties().set_content_md5(this_pointer->m_blob_hash_provider.hash());
                return this_pointer->m_blob->upload_properties_async(this_pointer->m_condition, this_pointer->m_options, this_pointer->m_context);
            });
        }
        else
        {
            return _sync().then([] (bool)
            {
            });
        }
    }

}}} // namespace azure::storage::core
