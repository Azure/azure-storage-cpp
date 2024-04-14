// -----------------------------------------------------------------------------------------
// <copyright file="timer_handler.h" company="Microsoft">
//    Copyright 2018 Microsoft Corporation
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

#include "cpprest/http_client.h"
#include <mutex>

#include "wascore/constants.h"

#ifndef _WIN32
#include <chrono>
#include <boost/asio/basic_waitable_timer.hpp>
#include "pplx/threadpool.h"
#else
#include "wascore/basic_types.h"
#endif

namespace azure { namespace storage { namespace core {
    
    typedef void(__cdecl * TaskProc)(void *);

    //
    // Timer implementation
    //
    class windows_timer
    {
    public:
        windows_timer(TaskProc userFunc, _In_ void * context);

        ~windows_timer();

        void start(unsigned int ms, bool repeat);

        void stop(bool waitForCallbacks);

    private:

        static void CALLBACK _TimerCallback(PVOID context, BOOLEAN)
        {
            auto timer = static_cast<windows_timer *>(context);
            timer->m_userFunc(timer->m_userContext);
        }
        HANDLE m_hTimer;

        TaskProc m_userFunc;
        void * m_userContext;
    };

    /// <summary>
    /// Used for internal logic of timer handling, including timer creation, deletion and cancellation
    /// </summary>
    class timer_handler : public std::enable_shared_from_this<timer_handler>
    {
    public:
        WASTORAGE_API explicit timer_handler(const pplx::cancellation_token& token);

        WASTORAGE_API ~timer_handler();

        WASTORAGE_API void start_timer(const std::chrono::milliseconds& time);

        WASTORAGE_API void stop_timer();

        bool timer_started() const
        {
            return m_timer_started.load(std::memory_order_acquire);
        }

        pplx::cancellation_token get_cancellation_token() const
        {
            return m_worker_cancellation_token_source.get_token();
        }

        bool is_canceled() const
        {
            return m_worker_cancellation_token_source.get_token().is_canceled();
        }

        bool is_canceled_by_timeout() const
        {
            return m_is_canceled_by_timeout.load(std::memory_order_acquire);
        }

    private:
        pplx::cancellation_token_source m_worker_cancellation_token_source;
        pplx::cancellation_token_registration m_cancellation_token_registration;
        pplx::cancellation_token m_cancellation_token;
        pplx::task<void> m_timeout_task;
        std::atomic<bool> m_is_canceled_by_timeout;
        pplx::task_completion_event<void> m_tce;

        std::mutex m_mutex;

        WASTORAGE_API pplx::task<void> timeout_after(const std::chrono::milliseconds& time);

#ifndef _WIN32
        std::shared_ptr<boost::asio::basic_waitable_timer<std::chrono::steady_clock>> m_timer;
#else
        std::shared_ptr<windows_timer> m_timer;
#endif
        std::atomic<bool> m_timer_started;
    };
}}}
