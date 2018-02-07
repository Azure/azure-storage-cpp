// -----------------------------------------------------------------------------------------
// <copyright file="cloud_core.cpp" company="Microsoft">
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
#include "wascore/resources.h"
#include "was/core.h"

namespace azure { namespace storage {

#ifdef _WIN32
    static std::shared_ptr<delayed_scheduler_interface> s_delayedScheduler;
#endif

    storage_uri::storage_uri(web::http::uri primary_uri)
        : m_primary_uri(std::move(primary_uri))
    {
        if (m_primary_uri.is_empty())
        {
            throw std::invalid_argument(protocol::error_storage_uri_empty);
        }
    }

    storage_uri::storage_uri(web::http::uri primary_uri, web::http::uri secondary_uri)
        : m_primary_uri(std::move(primary_uri)), m_secondary_uri(std::move(secondary_uri))
    {
        if (m_primary_uri.is_empty() && m_secondary_uri.is_empty())
        {
            throw std::invalid_argument(protocol::error_storage_uri_empty);
        }

        // Validate the query and path match if both URIs are supplied
        if (!m_primary_uri.is_empty() && !m_secondary_uri.is_empty())
        {
            if (m_primary_uri.query() != m_secondary_uri.query())
            {
                throw std::invalid_argument(protocol::error_storage_uri_mismatch);
            }

            utility::string_t::size_type primary_path_start = core::find_path_start(m_primary_uri);
            utility::string_t::size_type secondary_path_start = core::find_path_start(m_secondary_uri);
            if (m_primary_uri.path().compare(primary_path_start, utility::string_t::npos,
                m_secondary_uri.path(), secondary_path_start, utility::string_t::npos) != 0)
            {
                throw std::invalid_argument(protocol::error_storage_uri_mismatch);
            }
        }
    }

#ifdef _WIN32
    void __cdecl set_wastorage_ambient_scheduler(const std::shared_ptr<pplx::scheduler_interface>& scheduler)
    {
        pplx::set_ambient_scheduler(scheduler);
    }

#if defined(_MSC_VER) && _MSC_VER < 1900

	const std::shared_ptr<pplx::scheduler_interface> __cdecl get_wastorage_ambient_scheduler()
	{
		return pplx::get_ambient_scheduler();
	}

#else

	const std::shared_ptr<pplx::scheduler_interface>& __cdecl get_wastorage_ambient_scheduler()
	{
		return pplx::get_ambient_scheduler();
	}

#endif

    void __cdecl set_wastorage_ambient_delayed_scheduler(const std::shared_ptr<delayed_scheduler_interface>& scheduler)
    {
        s_delayedScheduler = scheduler;
    }

    const std::shared_ptr<delayed_scheduler_interface>& __cdecl get_wastorage_ambient_delayed_scheduler()
    {
        return s_delayedScheduler;
    }
#endif

}} // namespace azure::storage
