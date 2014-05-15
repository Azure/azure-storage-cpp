// -----------------------------------------------------------------------------------------
// <copyright file="util_windows.cpp" company="Microsoft">
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
#include "wascore/util.h"
#include "wascore/constants.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <rpc.h>
#include <agents.h>

namespace azure { namespace storage {  namespace core {

    pplx::task<void> complete_after(std::chrono::milliseconds timeout)
    {
        // A task completion event that is set when a timer fires.
        pplx::task_completion_event<void> tce;

        // Create a non-repeating timer.
        auto fire_once = new concurrency::timer<int>(static_cast<unsigned int>(timeout.count()), 0, nullptr, false);

        // Create a call object that sets the completion event after the timer fires.
        auto callback = new concurrency::call<int>([tce] (int)
        {
            tce.set();
        });

        // Connect the timer to the callback and start the timer.
        fire_once->link_target(callback);
        fire_once->start();

        // Create a task that completes after the completion event is set.
        pplx::task<void> event_set(tce);

        // Create a continuation task that cleans up resources and 
        // and return that continuation task. 
        return event_set.then([callback, fire_once] ()
        {
            delete callback;
            delete fire_once;
        });
    }

}}} // namespace azure::storage::core

#endif
