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

#ifdef WIN32
#include <float.h>
#endif

// TODO: Remove this include after switching to Casablanca's datetime parsing
#include <regex>

namespace wa { namespace storage {  namespace core {

    const wchar_t hex_alphabet[16] = {U('0'), U('1'), U('2'), U('3'), U('4'), U('5'), U('6'), U('7'), U('8'), U('9'), U('a'), U('b'), U('c'), U('d'), U('e'), U('f')};
    const utility::datetime::interval_type second_interval = 10000000;

    utility::size64_t get_remaining_stream_length(concurrency::streams::istream stream)
    {
        if (stream.can_seek())
        {
            auto offset = stream.tell();
            auto end = stream.seek(0, std::ios_base::end);
            stream.seek(offset);
            return static_cast<utility::size64_t>(end - offset);
        }

        return protocol::invalid_size64_t;
    }

    pplx::task<utility::size64_t> stream_copy_async(concurrency::streams::istream istream, concurrency::streams::ostream ostream, utility::size64_t length)
    {
        size_t buffer_size(protocol::default_buffer_size);
        utility::size64_t istream_length = length == protocol::invalid_size64_t ? get_remaining_stream_length(istream) : length;
        if ((istream_length != protocol::invalid_size64_t) && (istream_length < buffer_size))
        {
            buffer_size = static_cast<size_t>(istream_length);
        }

        auto obuffer = ostream.streambuf();
        auto length_ptr = (length != protocol::invalid_size64_t) ? std::make_shared<utility::size64_t>(length) : nullptr;
        auto total_ptr = std::make_shared<utility::size64_t>(0);
        return pplx::details::do_while([istream, obuffer, buffer_size, length_ptr, total_ptr] () -> pplx::task<bool>
        {
            size_t read_length = buffer_size;
            if ((length_ptr != nullptr) && (*length_ptr < read_length))
            {
                read_length = static_cast<size_t>(*length_ptr);
            }

            return istream.read(obuffer, read_length).then([length_ptr, total_ptr] (size_t count) -> bool
            {
                *total_ptr += count;
                if (length_ptr != nullptr)
                {
                    *length_ptr -= count;
                }

                return (count > 0) && (length_ptr == nullptr || *length_ptr > 0);
            });
        }).then([total_ptr, length] (bool) -> utility::size64_t
        {
            if (length != protocol::invalid_size64_t && *total_ptr != length)
            {
                throw std::invalid_argument(utility::conversions::to_utf8string(protocol::error_stream_short));
            }

            return *total_ptr;
        });
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

    /*
    utility::string_t replace(const utility::string_t& source, const utility::string_t& old_value, const utility::string_t& new_value)
    {
        utility::string_t result(source);

        utility::string_t::size_type position = result.find(old_value, 0U);
        while (position != utility::string_t::npos)
        {
            result.replace(position, old_value.size(), new_value);
            position = result.find(old_value, position + new_value.size());
        }
    }
    */


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

    utility::datetime truncate_fractional_seconds(const utility::datetime& value)
    {
        utility::datetime result;
        result = result + (value.to_interval() / second_interval * second_interval);
        return result;
        //return utility::datetime::from_seconds(value.to_interval() / seconds_per_interval);
    }

    /*
    const utility::datetime::interval_type half_second_interval = second_interval / 2;

    utility::datetime round_fractional_seconds(const utility::datetime& value)
    {
        utility::datetime result;
        result = result + ((value.to_interval() + half_second_interval) / second_interval * second_interval);
        return result;
    }
    */

    utility::string_t convert_to_string(int value)
    {
        // TODO: Try to use a standard function for this
        utility::ostringstream_t buffer;
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

    // TODO: Remove the following 4 functions and switch to Casablanca's datetime parsing when it is ready
    utility::string_t convert_to_string(utility::datetime value)
    {
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
            TCHAR dateStr[buffSize] = {0};
            status = GetDateFormat(LOCALE_INVARIANT, 0, &systemTime, "yyyy-MM-dd", dateStr, buffSize);
    #else
            wchar_t dateStr[buffSize] = {0};
            status = GetDateFormatEx(LOCALE_NAME_INVARIANT, 0, &systemTime, L"yyyy-MM-dd", dateStr, buffSize, NULL);
    #endif // _WIN32_WINNT < _WIN32_WINNT_VISTA 
            if (status == 0)
            {
                throw utility::details::create_system_error(GetLastError());
            }

    #if _WIN32_WINNT < _WIN32_WINNT_VISTA 
            TCHAR timeStr[buffSize] = {0};
            status = GetTimeFormat(LOCALE_INVARIANT, TIME_NOTIMEMARKER | TIME_FORCE24HOURFORMAT, &systemTime, "HH':'mm':'ss", timeStr, buffSize);
    #else
            wchar_t timeStr[buffSize] = {0};
            status = GetTimeFormatEx(LOCALE_NAME_INVARIANT, TIME_NOTIMEMARKER | TIME_FORCE24HOURFORMAT, &systemTime, L"HH':'mm':'ss", timeStr, buffSize);
    #endif // _WIN32_WINNT < _WIN32_WINNT_VISTA 
            if (status == 0)
            {
                throw utility::details::create_system_error(GetLastError());
            }

            outStream << dateStr << "T" << timeStr;
            uint64_t frac_sec = largeInt.QuadPart % static_cast<utility::datetime::interval_type>(10000000);
            if (frac_sec > 0)
            {
                // Append fractional second, which is a 7-digit value with no trailing zeros
                // This way, '1200' becomes '00012'
                char buf[9] = { 0 };
                sprintf_s(buf, sizeof(buf), ".%07d", frac_sec);
                // trim trailing zeros
                for (int i = 7; buf[i] == '0'; i--) buf[i] = '\0';
                outStream << buf;
            }
            outStream << "Z";

        return outStream.str();
    }

    bool system_type_to_datetime(void* pvsysTime, uint64_t seconds, utility::datetime * pdt)
    {
        SYSTEMTIME* psysTime = (SYSTEMTIME*)pvsysTime;
        FILETIME fileTime;

        if (SystemTimeToFileTime(psysTime, &fileTime))
        {
            ULARGE_INTEGER largeInt;
            largeInt.LowPart = fileTime.dwLowDateTime;
            largeInt.HighPart = fileTime.dwHighDateTime;

            // Add hundredths of nanoseconds
            largeInt.QuadPart += seconds;

            *pdt = utility::datetime() + largeInt.QuadPart;
            return true;
        }
        return false;
    }

    uint64_t timeticks_from_second(const utility::string_t& str)
    {
        _ASSERTE(str.size()>1);
        _ASSERTE(str[0]==U('.'));
        uint64_t ufrac_second = 0;
        for(int i=1; i<=7; ++i)
        {
            ufrac_second *= 10;
            auto add = i < (int)str.size() ? str[i] - U('0') : 0;
            ufrac_second += add;
        }
        return ufrac_second;
    }

    utility::datetime parse_datetime(utility::string_t dateString)
    {
        // avoid floating point math to preserve precision
        uint64_t ufrac_second = 0;

        utility::datetime result;

            // Unlike FILETIME, SYSTEMTIME does not have enough precision to hold seconds in 100 nanosecond
            // increments. Therefore, start with seconds and milliseconds set to 0, then add them separately

            // Try to extract the fractional second from the timestamp
            std::wregex r_frac_second(L"(.+)(\\.\\d+)(Z$)");
            std::wsmatch m;

            std::wstring input(dateString);
            if (std::regex_search(dateString, m, r_frac_second))
            {
                auto frac = m[2].str(); // this is the fractional second
                ufrac_second = timeticks_from_second(frac);
                input = m[1].str() + m[3].str();
            }

            {
                SYSTEMTIME sysTime = { 0 };
                const wchar_t * formatString = L"%4d-%2d-%2dT%2d:%2d:%2dZ";
                auto n = swscanf_s(input.c_str(), formatString,
                    &sysTime.wYear,  
                    &sysTime.wMonth,  
                    &sysTime.wDay, 
                    &sysTime.wHour, 
                    &sysTime.wMinute, 
                    &sysTime.wSecond);

                if (n == 3 || n == 6)
                {
                    if (system_type_to_datetime(&sysTime, ufrac_second, &result))
                    {
                        return result;
                    }
                }
            }
            {
                SYSTEMTIME sysTime = {0};
                DWORD date = 0;

                const wchar_t * formatString = L"%8dT%2d:%2d:%2dZ";
                auto n = swscanf_s(input.c_str(), formatString,
                    &date, 
                    &sysTime.wHour, 
                    &sysTime.wMinute, 
                    &sysTime.wSecond);

                if (n == 1 || n == 4)
                {
                    sysTime.wDay = date % 100;
                    date /= 100;
                    sysTime.wMonth = date % 100;
                    date /= 100;
                    sysTime.wYear = (WORD)date;

                    if (system_type_to_datetime(&sysTime, ufrac_second, &result))
                    {
                        return result;
                    }
                }
            }
            {
                SYSTEMTIME sysTime = {0};
                GetSystemTime(&sysTime);    // Fill date portion with today's information
                sysTime.wSecond = 0;
                sysTime.wMilliseconds = 0;
    
                const wchar_t * formatString = L"%2d:%2d:%2dZ";
                auto n = swscanf_s(input.c_str(), formatString,
                    &sysTime.wHour, 
                    &sysTime.wMinute, 
                    &sysTime.wSecond);

                if (n == 3)
                {
                    if (system_type_to_datetime(&sysTime, ufrac_second, &result))
                    {
                        return result;
                    }
                }
            }

        return utility::datetime();
    }

}}} // namespace wa::storage::core
