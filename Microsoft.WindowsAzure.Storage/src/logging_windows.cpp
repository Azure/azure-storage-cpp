// -----------------------------------------------------------------------------------------
// <copyright file="logging_windows.cpp" company="Microsoft">
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
#include "wascore/logging.h"

#include <evntprov.h>
#include <evntrace.h>

namespace wa { namespace storage { namespace core {

    // {EE5D17C5-1B3E-4792-B0F9-F8C5FC6AC22A}
    static const GUID event_provider_guid = { 0xee5d17c5, 0x1b3e, 0x4792, { 0xb0, 0xf9, 0xf8, 0xc5, 0xfc, 0x6a, 0xc2, 0x2a } };
    static REGHANDLE g_event_provider_handle;

    UCHAR get_etw_log_level(client_log_level level)
    {
        switch (level)
        {
        case client_log_level::log_level_off:
            throw std::invalid_argument("level");

        case client_log_level::log_level_error:
            return TRACE_LEVEL_ERROR;

        case client_log_level::log_level_warning:
            return TRACE_LEVEL_WARNING;

        case client_log_level::log_level_informational:
            return TRACE_LEVEL_INFORMATION;
        }

        return TRACE_LEVEL_VERBOSE;
    }

    logger::logger()
    {
        if (EventRegister(&event_provider_guid, NULL, NULL, &g_event_provider_handle) != ERROR_SUCCESS)
        {
            g_event_provider_handle = NULL;
        }
    }

    logger::~logger()
    {
        if (g_event_provider_handle != NULL)
        {
            EventUnregister(g_event_provider_handle);
        }
    }

    void logger::log(wa::storage::operation_context context, client_log_level level, const utility::string_t& message) const
    {
        if (g_event_provider_handle != NULL)
        {
            auto utf16_message = utility::conversions::to_utf16string(message);
            EventWriteString(g_event_provider_handle, get_etw_log_level(level), 0, utf16_message.c_str());
        }
    }

    bool logger::should_log(wa::storage::operation_context context, client_log_level level) const
    {
        return (g_event_provider_handle != NULL) && (level <= context.log_level());
    }

}}} // namespace wa::storage::core
