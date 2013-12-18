// -----------------------------------------------------------------------------------------
// <copyright file="logging.h" company="Microsoft">
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

#include "wascore/basic_types.h"
#include "was/common.h"

namespace wa { namespace storage { namespace core {

    class logger
    {
    public:

        static const logger& instance()
        {
            // This is thread-safe in C++11 per ISO C++ Jan 2012 working draft
            // 6.7 Declaration statement [stmt.dcl] para. 4
            static logger singleton_instance;
            return singleton_instance;
        }

        ~logger();

        void log(wa::storage::operation_context context, client_log_level level, const utility::string_t& message) const;
        bool should_log(wa::storage::operation_context context, client_log_level level) const;

    private:

        logger();
    };

}}} // namespace wa::storage::core
