// -----------------------------------------------------------------------------------------
// <copyright file="logging.cpp" company="Microsoft">
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

#ifdef _WIN32
#include <evntprov.h>
#include <evntrace.h>
#else
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#endif

namespace azure { namespace storage { namespace core {

#ifdef _WIN32
    const std::wstring wconnector(L" : ");

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

    void logger::log(azure::storage::operation_context context, client_log_level level, const std::string& message) const
    {
        if (g_event_provider_handle != NULL)
        {
            log(context, level, utility::conversions::to_utf16string(message));
        }
    }

    void logger::log(azure::storage::operation_context context, client_log_level level, const std::wstring& message) const
    {
        if (g_event_provider_handle != NULL)
        {
            utf16string utf16_message;
            utf16string req_id_16 = utility::conversions::to_utf16string(context.client_request_id());

            utf16_message.reserve(req_id_16.length() + wconnector.length() + message.length());
            utf16_message.append(req_id_16);
            utf16_message.append(wconnector);
            utf16_message.append(message);

            EventWriteString(g_event_provider_handle, get_etw_log_level(level), 0, utf16_message.c_str());
        }
    }

    bool logger::should_log(azure::storage::operation_context context, client_log_level level) const
    {
        return (g_event_provider_handle != NULL) && (level <= context.log_level());
    }

#else

    const std::string connector(" : ");

    logger::logger()
    {
    }

    logger::~logger()
    {
    }

    boost::log::trivial::severity_level get_boost_log_level(client_log_level level)
    {
        switch (level)
        {
            case client_log_level::log_level_error:
                return boost::log::trivial::severity_level::error;

            case client_log_level::log_level_warning:
                return boost::log::trivial::warning;

            case client_log_level::log_level_informational:
                return boost::log::trivial::info;

            case client_log_level::log_level_verbose:
                return boost::log::trivial::trace;
        }

        throw std::invalid_argument("level");
    }

    void logger::log(azure::storage::operation_context context, client_log_level level, const std::string& message) const
    {
        std::string utf8_message;
        utf8_message.reserve(context.client_request_id().length() + connector.length() + message.length());
        utf8_message.append(context.client_request_id());
        utf8_message.append(connector);
        utf8_message.append(message);

        BOOST_LOG_SEV(context.logger(), get_boost_log_level(level)) << utf8_message;
    }

    bool logger::should_log(azure::storage::operation_context context, client_log_level level) const
    {
        return (level != client_log_level::log_level_off) && (level <= context.log_level());
    }

#endif // _WIN32

    logger logger::m_instance;

}}} // namespace azure::storage::core

