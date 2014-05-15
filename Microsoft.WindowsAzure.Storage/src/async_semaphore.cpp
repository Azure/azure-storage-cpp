// -----------------------------------------------------------------------------------------
// <copyright file="async_semaphore.cpp" company="Microsoft">
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
#include "wascore/async_semaphore.h"

namespace azure { namespace storage {  namespace core {

    pplx::task<void> _async_semaphore::lock_async()
    {
        pplx::extensibility::scoped_rw_lock_t guard(m_mutex);
        if (m_count > 0)
        {
            if (m_count-- == m_initial_count)
            {
                m_empty_event = pplx::task_completion_event<void>();
            }

            return pplx::task_from_result();
        }
        else
        {
            pplx::task_completion_event<void> pending;
            m_queue.push(pending);
            return pplx::create_task(pending);
        }
    }

    pplx::task<void> _async_semaphore::wait_all_async()
    {
        return pplx::create_task(m_empty_event);
    }

    void _async_semaphore::unlock()
    {
        auto pending = dequeue_pending();
        pending.set();
    }

    pplx::task_completion_event<void> _async_semaphore::dequeue_pending()
    {
        pplx::extensibility::scoped_rw_lock_t guard(m_mutex);
        if (m_queue.empty())
        {
            if (++m_count == m_initial_count)
            {
                m_empty_event.set();
            }

            return pplx::task_completion_event<void>();
        }
        else
        {
            auto pending = m_queue.front();
            m_queue.pop();
            return pending;
        }
    }

}}} // namespace azure::storage::core
