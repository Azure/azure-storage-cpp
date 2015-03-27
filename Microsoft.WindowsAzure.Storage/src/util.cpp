// -----------------------------------------------------------------------------------------
// <copyright file="util.cpp" company="Microsoft">
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
#include "wascore/resources.h"

#ifndef WIN32
#include "pplx/threadpool.h"
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <float.h>
#include <windows.h>
#include <rpc.h>
#include <agents.h>
#else
#include <chrono>
#include <thread>
#endif

namespace azure { namespace storage {  namespace core {

    const utility::char_t hex_alphabet[16] = {U('0'), U('1'), U('2'), U('3'), U('4'), U('5'), U('6'), U('7'), U('8'), U('9'), U('a'), U('b'), U('c'), U('d'), U('e'), U('f')};
    const utility::datetime::interval_type second_interval = 10000000;

    utility::string_t make_query_parameter_impl(const utility::string_t& parameter_name, const utility::string_t& parameter_value)
    {
        utility::string_t result;
        result.reserve(parameter_name.size() + parameter_value.size() + 1);

        result.append(parameter_name);
        result.push_back(U('='));
        result.append(parameter_value);

        return result;
    }

    utility::string_t make_query_parameter(const utility::string_t& parameter_name, const utility::string_t& parameter_value, bool do_encoding)
    {
        // TODO: Remove this function if the Casablanca library changes its default query parameter encoding to include all possible encoded characters

        if (do_encoding)
        {
            utility::string_t encoded_parameter_value = web::http::uri::encode_data_string(parameter_value);
            return make_query_parameter_impl(parameter_name, encoded_parameter_value);
        }
        else
        {
            return make_query_parameter_impl(parameter_name, parameter_value);
        }
    }

    utility::size64_t get_remaining_stream_length(concurrency::streams::istream stream)
    {
        if (stream.can_seek())
        {
            auto offset = stream.tell();
            auto end = stream.seek(0, std::ios_base::end);
            stream.seek(offset);
            return static_cast<utility::size64_t>(end - offset);
        }

        return std::numeric_limits<utility::size64_t>::max();
    }

    pplx::task<utility::size64_t> stream_copy_async(concurrency::streams::istream istream, concurrency::streams::ostream ostream, utility::size64_t length, utility::size64_t max_length)
    {
        size_t buffer_size(protocol::default_buffer_size);
        utility::size64_t istream_length = length == std::numeric_limits<utility::size64_t>::max() ? get_remaining_stream_length(istream) : length;
        if ((istream_length != std::numeric_limits<utility::size64_t>::max()) && (istream_length > max_length))
        {
            throw std::invalid_argument(protocol::error_stream_length);
        }

        if ((istream_length != std::numeric_limits<utility::size64_t>::max()) && (istream_length < buffer_size))
        {
            buffer_size = static_cast<size_t>(istream_length);
        }

        auto obuffer = ostream.streambuf();
        auto length_ptr = (length != std::numeric_limits<utility::size64_t>::max()) ? std::make_shared<utility::size64_t>(length) : nullptr;
        auto total_ptr = std::make_shared<utility::size64_t>(0);
        return pplx::details::do_while([istream, obuffer, buffer_size, length_ptr, total_ptr, max_length] () -> pplx::task<bool>
        {
            size_t read_length = buffer_size;
            if ((length_ptr != nullptr) && (*length_ptr < read_length))
            {
                read_length = static_cast<size_t>(*length_ptr);
            }

            return istream.read(obuffer, read_length).then([length_ptr, total_ptr, max_length] (size_t count) -> bool
            {
                *total_ptr += count;
                if (length_ptr != nullptr)
                {
                    *length_ptr -= count;
                }

                if (*total_ptr > max_length)
                {
                    throw std::invalid_argument(protocol::error_stream_length);
                }

                return (count > 0) && (length_ptr == nullptr || *length_ptr > 0);
            });
        }).then([total_ptr, length] (bool) -> utility::size64_t
        {
            if (length != std::numeric_limits<utility::size64_t>::max() && *total_ptr != length)
            {
                throw std::invalid_argument(protocol::error_stream_short);
            }

            return *total_ptr;
        });
    }

    utility::char_t utility_char_tolower(const utility::char_t& character)
    {
        int i = (int)character;
        int lower = tolower(i);
        return (utility::char_t)lower;
    }

    std::vector<utility::string_t> string_split(const utility::string_t& string, const utility::string_t& separator)
    {
        std::vector<utility::string_t> result;
        utility::string_t::size_type pos(0);
        utility::string_t::size_type sep;

        do
        {
            sep = string.find(separator, pos);
            result.push_back(string.substr(pos, sep == utility::string_t::npos ? sep : sep - pos));
            pos = sep + separator.length();
        } while (sep != utility::string_t::npos);

        return result;
    }

    bool is_empty_or_whitespace(const utility::string_t& value)
    {
        for (utility::string_t::const_iterator it = value.cbegin(); it != value.cend(); ++it)
        {
            if (!isspace(*it))
            {
                return false;
            }
        }

        return true;
    }

    utility::string_t single_quote(const utility::string_t& value)
    {
        const utility::char_t SINGLE_QUOTE = U('\'');

        utility::string_t result;
        result.reserve(value.size() + 2U);

        result.push_back(SINGLE_QUOTE);

        for (utility::string_t::const_iterator itr = value.cbegin(); itr != value.cend(); ++itr)
        {
            utility::char_t ch = *itr;
            result.push_back(ch);
            if (ch == SINGLE_QUOTE)
            {
                result.push_back(SINGLE_QUOTE);
            }
        }

        result.push_back(SINGLE_QUOTE);

        return result;
    }

    bool is_nan(double value)
    {
#ifdef WIN32
        return _isnan(value) != 0;
#else
        return std::isnan(value);
#endif
    }

    bool is_finite(double value)
    {
#ifdef WIN32
        return _finite(value) != 0;
#else
        return std::isfinite(value);
#endif
    }

    bool is_integral(const utility::string_t& value)
    {
        // Check if the string consists entirely of an optional negative sign followed by one or more digits

        utility::string_t::const_iterator it = value.cbegin();

        if (it != value.cend())
        {
            // Skip the negative sign if present
            utility::char_t ch = *it;
            if (ch == U('-'))
            {
                ++it;
            }
        }

        if (it == value.cend())
        {
            return false;
        }

        do
        {
            // Check that all remaining characters are digits
            utility::char_t ch = *it;
            if (ch < U('0') || ch > U('9'))
            {
                return false;
            }
            ++it;
        } while (it != value.cend());

        return true;
    }

    utility::datetime truncate_fractional_seconds(utility::datetime value)
    {
        utility::datetime result;
        result = result + (value.to_interval() / second_interval * second_interval);
        return result;
    }

    utility::string_t convert_to_string(double value)
    {
        utility::ostringstream_t buffer;
        buffer.precision(std::numeric_limits<double>::digits10 + 2);
        buffer << value;
        return buffer.str();
    }

    utility::string_t convert_to_string(const std::vector<uint8_t>& value)
    {
        utility::string_t result;
        result.reserve(value.size() * 2);

        for (std::vector<uint8_t>::const_iterator itr = value.cbegin(); itr != value.cend(); ++itr)
        {
            uint8_t current = *itr;
            result.push_back(hex_alphabet[current >> 4]);
            result.push_back(hex_alphabet[current & 0xf]);
        }

        return result;
    }

    utility::string_t convert_to_string_with_fixed_length_fractional_seconds(utility::datetime value)
    {
        // TODO: Remove this function if Casablanca changes their datetime serialization to not trim trailing zeros in the fractional seconds component of a time

#ifdef WIN32
        int status;

        ULARGE_INTEGER largeInt;
        largeInt.QuadPart = value.to_interval();

        FILETIME ft;
        ft.dwHighDateTime = largeInt.HighPart;
        ft.dwLowDateTime = largeInt.LowPart;

        SYSTEMTIME systemTime;
        if (!FileTimeToSystemTime((const FILETIME *)&ft, &systemTime))
        {
            throw utility::details::create_system_error(GetLastError());
        }

        std::wostringstream outStream;

        const size_t buffSize = 64;
#if _WIN32_WINNT < _WIN32_WINNT_VISTA
        TCHAR dateStr[buffSize] = { 0 };
        status = GetDateFormat(LOCALE_INVARIANT, 0, &systemTime, "yyyy-MM-dd", dateStr, buffSize);
#else
        wchar_t dateStr[buffSize] = { 0 };
        status = GetDateFormatEx(LOCALE_NAME_INVARIANT, 0, &systemTime, L"yyyy-MM-dd", dateStr, buffSize, NULL);
#endif // _WIN32_WINNT < _WIN32_WINNT_VISTA
        if (status == 0)
        {
            throw utility::details::create_system_error(GetLastError());
        }

#if _WIN32_WINNT < _WIN32_WINNT_VISTA
        TCHAR timeStr[buffSize] = { 0 };
        status = GetTimeFormat(LOCALE_INVARIANT, TIME_NOTIMEMARKER | TIME_FORCE24HOURFORMAT, &systemTime, "HH':'mm':'ss", timeStr, buffSize);
#else
        wchar_t timeStr[buffSize] = { 0 };
        status = GetTimeFormatEx(LOCALE_NAME_INVARIANT, TIME_NOTIMEMARKER | TIME_FORCE24HOURFORMAT, &systemTime, L"HH':'mm':'ss", timeStr, buffSize);
#endif // _WIN32_WINNT < _WIN32_WINNT_VISTA
        if (status == 0)
        {
            throw utility::details::create_system_error(GetLastError());
        }

        outStream << dateStr << "T" << timeStr;
        uint64_t frac_sec = largeInt.QuadPart % second_interval;
        if (frac_sec > 0)
        {
            // Append fractional second, which is a 7-digit value
            // This way, '1200' becomes '0001200'
            char buf[9] = { 0 };
            sprintf_s(buf, sizeof(buf), ".%07ld", (long int)frac_sec);
            outStream << buf;
        }
        outStream << "Z";

        return outStream.str();
#else //LINUX
        uint64_t input = value.to_interval();
        uint64_t frac_sec = input % second_interval;
        input /= second_interval; // convert to seconds
        time_t time = (time_t)input - (time_t)11644473600LL;// diff between windows and unix epochs (seconds)

        struct tm datetime;
        gmtime_r(&time, &datetime);

        const int max_dt_length = 64;
        char output[max_dt_length + 1] = { 0 };

        if (frac_sec > 0)
        {
            // Append fractional second, which is a 7-digit value
            // This way, '1200' becomes '0001200'
            char buf[9] = { 0 };
            snprintf(buf, sizeof(buf), ".%07ld", (long int)frac_sec);
            // format the datetime into a separate buffer
            char datetime_str[max_dt_length + 1] = { 0 };
            strftime(datetime_str, sizeof(datetime_str), "%Y-%m-%dT%H:%M:%S", &datetime);
            // now print this buffer into the output buffer
            snprintf(output, sizeof(output), "%s%sZ", datetime_str, buf);
        }
        else
        {
            strftime(output, sizeof(output), "%Y-%m-%dT%H:%M:%SZ", &datetime);
        }

        return std::string(output);
#endif
    }

    class delay_event
    {
    public:
#ifdef WIN32
        delay_event(std::chrono::milliseconds timeout)
            : m_callback(new concurrency::call<int>(std::function<void(int)>(std::bind(&delay_event::timer_fired, this, std::placeholders::_1)))), m_timer(static_cast<unsigned int>(timeout.count()), 0, m_callback, false)
        {
        }

        ~delay_event()
        {
            delete m_callback;
        }

        void start()
        {
            m_timer.start();
        }
#else
        delay_event(std::chrono::milliseconds timeout)
            : m_timer(crossplat::threadpool::shared_instance().service(), boost::posix_time::milliseconds(timeout.count()))
        {
        }

        void start()
        {
            m_timer.async_wait(std::bind(&delay_event::timer_fired, this, std::placeholders::_1));
        }
#endif
        pplx::task<void> create_task()
        {
            return pplx::task<void>(m_completion_event);
        }

    private:
        pplx::task_completion_event<void> m_completion_event;
#ifdef WIN32
        concurrency::call<int>* m_callback;
        concurrency::timer<int> m_timer;
#else
        boost::asio::deadline_timer m_timer;
#endif

#ifdef WIN32
        void timer_fired(const int& dummy)
#else
        void timer_fired(const boost::system::error_code& dummy)
#endif
        {
            UNREFERENCED_PARAMETER(dummy);

            m_completion_event.set();
        }
    };

    pplx::task<void> complete_after(std::chrono::milliseconds timeout)
    {
        delay_event* event = new delay_event(timeout);
        event->start();

        return event->create_task().then([event]()
        {
            delete event;
        });
    }

}}} // namespace azure::storage::core
