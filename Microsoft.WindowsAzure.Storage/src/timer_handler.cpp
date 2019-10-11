// -----------------------------------------------------------------------------------------
// <copyright file="timer_handler.cpp" company="Microsoft">
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

#include "stdafx.h"
#include "wascore/timer_handler.h"

namespace azure {    namespace storage {    namespace core {

    timer_handler::timer_handler(const pplx::cancellation_token& token) :
        m_cancellation_token(token), m_is_canceled_by_timeout(false), m_timer_started(false)
    {
        if (m_cancellation_token != pplx::cancellation_token::none())
        {
            m_cancellation_token_registration = m_cancellation_token.register_callback([this]()
            {
                m_worker_cancellation_token_source.cancel();
                stop_timer();
            });
        }
    }

    timer_handler::~timer_handler()
    {
        if (m_cancellation_token != pplx::cancellation_token::none())
        {
            m_cancellation_token.deregister_callback(m_cancellation_token_registration);
        }

        stop_timer();
    }

    void timer_handler::start_timer(const std::chrono::milliseconds& time)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        if (m_timer_started.load(std::memory_order_acquire))
        {
            return;
        }
        m_timer_started.store(true, std::memory_order_release);
        std::weak_ptr<timer_handler> weak_this_pointer = shared_from_this();
        m_timeout_task = timeout_after(time).then([weak_this_pointer]()
        {
            auto this_pointer = weak_this_pointer.lock();
            if (this_pointer)
            {
                this_pointer->m_is_canceled_by_timeout.store(true, std::memory_order_release);
                this_pointer->m_worker_cancellation_token_source.cancel();
            }
        });
    }

    void timer_handler::stop_timer()
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        if (m_timer_started.load(std::memory_order_acquire) && m_timer)
        {
#ifndef _WIN32
            m_timer->cancel();
#else
            m_timer->stop();
#endif
            if (!m_tce._IsTriggered())
            {
                // If task_completion_event is not yet triggered, it means timeout has not been triggered.
                m_tce._Cancel();
            }
            m_timer.reset();
        }
    }

#ifndef _WIN32
    pplx::task<void> timer_handler::timeout_after(const std::chrono::milliseconds& time)
    {
        m_timer = std::make_shared<boost::asio::basic_waitable_timer<std::chrono::steady_clock>>(crossplat::threadpool::shared_instance().service());
        m_timer->expires_from_now(std::chrono::duration_cast<std::chrono::steady_clock::duration>(time));
        std::weak_ptr<timer_handler> weak_this_pointer = shared_from_this();
        auto callback = [weak_this_pointer](const boost::system::error_code& ec)
        {
            if (ec != boost::asio::error::operation_aborted)
            {
                auto this_pointer = weak_this_pointer.lock();
                if (this_pointer)
                {
                    std::lock_guard<std::mutex> guard(this_pointer->m_mutex);
                    if (!this_pointer->m_tce._IsTriggered())
                    {
                        this_pointer->m_tce.set();
                    }
                }
            }
        };
        m_timer->async_wait(callback);

        auto event_set = pplx::create_task(m_tce);

        return event_set.then([callback]() {});
    }
#else
    pplx::task<void> timer_handler::timeout_after(const std::chrono::milliseconds& time)
    {
        // Initialize the timer and connect the callback with completion event.
        m_timer = std::make_shared<concurrency::timer<int>>(static_cast<unsigned int>(time.count()), 0);
        std::weak_ptr<timer_handler> weak_this_pointer = shared_from_this();
        auto callback = std::make_shared<concurrency::call<int>>([weak_this_pointer](int)
        {
            auto this_pointer = weak_this_pointer.lock();
            if (this_pointer)
            {
                std::lock_guard<std::mutex> guard(this_pointer->m_mutex);
                if (!this_pointer->m_tce._IsTriggered())
                {
                    this_pointer->m_tce.set();
                }
            }
        });
        m_timer->link_target(callback.get());  // When timer stops, tce will trigger cancellation.
        m_timer->start();

        auto event_set = pplx::create_task(m_tce);

        // Timer and callback should be preserved before event set has been triggered.
        return event_set.then([callback]() {});
    }
#endif

}}}
