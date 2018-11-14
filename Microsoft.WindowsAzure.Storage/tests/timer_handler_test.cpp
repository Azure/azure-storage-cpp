// -----------------------------------------------------------------------------------------
// <copyright file="timer_handler_test.cpp" company="Microsoft">
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
#include "check_macros.h"
#include "test_base.h"

#include "wascore/timer_handler.h"

SUITE(Core)
{
    TEST_FIXTURE(test_base, cancellation_token_source_multiple_cancel_test)
    {
        std::string exception_msg;
        try
        {
            pplx::cancellation_token_source source;
            for (auto i = 0; i < 100; ++i)
            {
                source.cancel();
            }
        }
        catch (std::exception& e)
        {
            exception_msg = e.what();
        }

        CHECK_EQUAL("", exception_msg);
    }

    TEST_FIXTURE(test_base, cancellation_token_source_multiple_cancel_concurrent_test)
    {
        std::string exception_msg;
        try
        {
            for (auto i = 0; i < 5000; ++i)
            {
                pplx::cancellation_token_source source;
                pplx::task_completion_event<void> tce;
                std::vector<pplx::task<void>> tasks;
                for (auto k = 0; k < 100; ++k)
                {
                    auto task = pplx::create_task(tce).then([source]() { source.cancel(); });
                    tasks.push_back(task);
                }
                tce.set();
                for (auto k = 0; k < 100; ++k)
                {
                    tasks[k].get();
                }
            }
        }
        catch (std::exception& e)
        {
            exception_msg = e.what();
        }

        CHECK_EQUAL("", exception_msg);
    }
}