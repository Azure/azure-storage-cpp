// -----------------------------------------------------------------------------------------
// <copyright file="cloud_common.cpp" company="Microsoft">
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

#include "was/common.h"
#include "wascore/resources.h"

#ifdef _WIN32
#include "Ws2tcpip.h"
#else
#include "boost/asio/ip/address.hpp"
#endif

namespace azure { namespace storage {

    WASTORAGE_API request_options::request_options()
        : m_location_mode(azure::storage::location_mode::primary_only), m_http_buffer_size(protocol::default_buffer_size),\
          m_maximum_execution_time(protocol::default_maximum_execution_time), m_server_timeout(protocol::default_server_timeout),\
          m_noactivity_timeout(protocol::default_noactivity_timeout)
    {
    }

    shared_access_policy::ip_address_or_range::ip_address shared_access_policy::ip_address_or_range::try_parse(const utility::string_t &address)
    {
        shared_access_policy::ip_address_or_range::ip_address ip;
#ifdef _WIN32
        IN_ADDR addr;
        int ret = InetPton(AF_INET, address.data(), &addr);
        if (ret == 1)
        {
            ip.ipv4 = true;
            ip.addr = ntohl(addr.S_un.S_addr);
            return ip;
        }

        if (ret == -1)
        {
            throw utility::details::create_system_error(WSAGetLastError());
        }

        if (ret == 0)
        {
            IN6_ADDR addr6;
            ret = InetPton(AF_INET6, address.data(), &addr6);
            if (ret == 1)
            {
                throw std::invalid_argument(protocol::error_ip_must_be_ipv4_in_sas);
            }

            if (ret == -1)
            {
                throw utility::details::create_system_error(WSAGetLastError());
            }
        }

        throw std::invalid_argument(protocol::error_invalid_ip_address);
#else
        boost::system::error_code error;
        auto addr = boost::asio::ip::address::from_string(address, error);
        if (error.value() == 0)
        {
            if (addr.is_v4())
            {
                return addr;
            }

            throw std::invalid_argument(protocol::error_ip_must_be_ipv4_in_sas);
        }

        throw std::invalid_argument(protocol::error_invalid_ip_address);
#endif
    }

    void shared_access_policy::ip_address_or_range::validate_range()
    {
        auto min_addr = try_parse(m_minimum_address);
        auto max_addr = try_parse(m_maximum_address);
#ifdef _WIN32
        if (min_addr.addr > max_addr.addr)
#else
        if (min_addr > max_addr)
#endif
        {
            std::swap(m_minimum_address, m_maximum_address);
        }
    }

}} // namespace azure::storage
