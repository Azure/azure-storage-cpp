// -----------------------------------------------------------------------------------------
// <copyright file="cloud_file_ostreambuf.cpp" company="Microsoft">
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
#include "wascore/filestream.h"
#include "wascore/logging.h"
#include "wascore/resources.h"

namespace azure { namespace storage { namespace core {

    pplx::task<bool> basic_cloud_file_ostreambuf::_sync()
    {
        upload_buffer();

        auto this_pointer = std::dynamic_pointer_cast<basic_cloud_file_ostreambuf>(shared_from_this());
        return m_semaphore.wait_all_async().then([this_pointer]() -> pplx::task<bool>
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

    pplx::task<void> basic_cloud_file_ostreambuf::commit_close()
    {
        auto this_pointer = std::dynamic_pointer_cast<basic_cloud_file_ostreambuf>(shared_from_this());
        return _sync().then([this_pointer](bool) -> pplx::task<void>
        {
            if (this_pointer->m_total_hash_provider.is_enabled())
            {
                this_pointer->m_file->properties().set_content_md5(this_pointer->m_total_hash_provider.hash());
                return this_pointer->m_file->upload_properties_async(this_pointer->m_condition, this_pointer->m_options, this_pointer->m_context);
            }

            return pplx::task_from_result();
        });
    }

    pplx::task<void> basic_cloud_file_ostreambuf::upload_buffer()
    {
        auto buffer = prepare_buffer();
        if (buffer->is_empty())
        {
            return pplx::task_from_result();
        }

        auto offset = m_current_file_offset;
        m_current_file_offset += buffer->size();

        auto this_pointer = std::dynamic_pointer_cast<basic_cloud_file_ostreambuf>(shared_from_this());
        return m_semaphore.lock_async().then([this_pointer, buffer, offset]()
        {
            if (this_pointer->m_currentException == nullptr)
            {
                try
                {
                    this_pointer->m_file->write_range_async(buffer->stream(), offset, buffer->content_md5(), this_pointer->m_condition, this_pointer->m_options, this_pointer->m_context).then([this_pointer](pplx::task<void> upload_task)
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

}}} // namespace azure::storage::core
