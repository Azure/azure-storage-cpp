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
#include "was/error_code_strings.h"
#include "wascore/blobstreams.h"
#include "wascore/logging.h"
#include "wascore/resources.h"

namespace azure { namespace storage { namespace core {

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

    pplx::task<void> basic_cloud_block_blob_ostreambuf::commit_close()
    {
        auto this_pointer = std::dynamic_pointer_cast<basic_cloud_block_blob_ostreambuf>(shared_from_this());
        return _sync().then([this_pointer] (bool) -> pplx::task<void>
        {
            if (this_pointer->m_total_hash_provider.is_enabled())
            {
                this_pointer->m_blob->properties().set_content_md5(this_pointer->m_total_hash_provider.hash());
            }

            return this_pointer->m_blob->upload_block_list_async(this_pointer->m_block_list, this_pointer->m_condition, this_pointer->m_options, this_pointer->m_context);
        });
    }

    utility::string_t basic_cloud_block_blob_ostreambuf::get_next_block_id()
    {
        utility::ostringstream_t str;
        str << m_block_id_prefix << _XPLATSTR('-') << std::setw(6) << std::setfill(_XPLATSTR('0')) << m_block_list.size();
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

    pplx::task<void> basic_cloud_page_blob_ostreambuf::commit_close()
    {
        if (m_total_hash_provider.is_enabled())
        {
            auto this_pointer = std::dynamic_pointer_cast<basic_cloud_page_blob_ostreambuf>(shared_from_this());
            return _sync().then([this_pointer] (bool) -> pplx::task<void>
            {
                this_pointer->m_blob->properties().set_content_md5(this_pointer->m_total_hash_provider.hash());
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

    pplx::task<void> basic_cloud_append_blob_ostreambuf::upload_buffer()
    {
        auto buffer = prepare_buffer();
        if (buffer->is_empty())
        {
            return pplx::task_from_result();
        }

        auto offset = m_current_blob_offset;
        m_current_blob_offset += buffer->size();

        if (m_condition.max_size() != -1 && m_current_blob_offset > m_condition.max_size())
        {
            m_currentException = std::make_exception_ptr(std::invalid_argument(protocol::error_invalid_block_size));
            return pplx::task_from_result();
        }

        auto this_pointer = std::dynamic_pointer_cast<basic_cloud_append_blob_ostreambuf>(shared_from_this());
        return m_semaphore.lock_async().then([this_pointer, buffer, offset]()
        {
            if (this_pointer->m_currentException == nullptr)
            {
                try
                {
                    this_pointer->m_condition.set_append_position(offset);
                    auto previous_results_count = this_pointer->m_context.request_results().size();
                    this_pointer->m_blob->append_block_async(buffer->stream(), buffer->content_md5(), this_pointer->m_condition, this_pointer->m_options, this_pointer->m_context).then([this_pointer, previous_results_count](pplx::task<int64_t> upload_task)
                    {
                        std::lock_guard<async_semaphore> guard(this_pointer->m_semaphore, std::adopt_lock);
                        try
                        {
                            upload_task.wait();
                        }
                        catch (const azure::storage::storage_exception& ex)
                        {
                            if (this_pointer->m_options.absorb_conditional_errors_on_retry()
                                && ex.result().http_status_code() == web::http::status_codes::PreconditionFailed
                                && (ex.result().extended_error().code() == protocol::error_code_invalid_append_condition || ex.result().extended_error().code() == protocol::error_invalid_max_blob_size_condition)
                                && this_pointer->m_context.request_results().size() - previous_results_count > 1)
                            {
                                // Pre-condition failure on a retry should be ignored in a single writer scenario since the request
                                // succeeded in the first attempt.
                                if (logger::instance().should_log(this_pointer->m_context, client_log_level::log_level_warning))
                                {
                                    logger::instance().log(this_pointer->m_context, client_log_level::log_level_warning, protocol::error_precondition_failure_ignored);
                                }
                            }
                            else
                            {
                                this_pointer->m_currentException = std::current_exception();
                            }
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

    pplx::task<void> basic_cloud_append_blob_ostreambuf::commit_close()
    {
        if (m_total_hash_provider.is_enabled())
        {
            auto this_pointer = std::dynamic_pointer_cast<basic_cloud_append_blob_ostreambuf>(shared_from_this());
            return _sync().then([this_pointer](bool) -> pplx::task<void>
            {
                this_pointer->m_blob->properties().set_content_md5(this_pointer->m_total_hash_provider.hash());
                return this_pointer->m_blob->upload_properties_async(this_pointer->m_condition, this_pointer->m_options, this_pointer->m_context);
            });
        }
        else
        {
            return _sync().then([](bool)
            {
            });
        }
    }

}}} // namespace azure::storage::core
