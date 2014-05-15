// -----------------------------------------------------------------------------------------
// <copyright file="async_semaphore.h" company="Microsoft">
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

#include <mutex>
#include <queue>

#include "cpprest/asyncrt_utils.h"

#include "wascore/basic_types.h"

namespace azure { namespace storage { namespace core {

    class _async_semaphore
    {
    public:

        explicit _async_semaphore(int count)
            : m_count(count), m_initial_count(count)
        {
            m_empty_event.set();
        }

        pplx::task<void> lock_async();
        void unlock();
        pplx::task<void> wait_all_async();

    private:

        pplx::task_completion_event<void> dequeue_pending();

        int m_count;
        int m_initial_count;
        pplx::task_completion_event<void> m_empty_event;
        std::queue<pplx::task_completion_event<void>> m_queue;
        pplx::extensibility::reader_writer_lock_t m_mutex;
    };

    class async_semaphore
    {
    public:

        explicit async_semaphore(int count)
            : m_semaphore(std::make_shared<_async_semaphore>(count))
        {
        }

        pplx::task<void> lock_async()
        {
            return m_semaphore->lock_async();
        }

        void lock()
        {
            lock_async().wait();
        }

        void unlock()
        {
            return m_semaphore->unlock();
        }

        pplx::task<void> wait_all_async()
        {
            return m_semaphore->wait_all_async();
        }

    private:

        std::shared_ptr<_async_semaphore> m_semaphore;
    };

}}} // namespace azure::storage::core
