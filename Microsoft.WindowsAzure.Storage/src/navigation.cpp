// -----------------------------------------------------------------------------------------
// <copyright file="navigation.cpp" company="Microsoft">
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
#include "was/auth.h"

namespace wa { namespace storage { namespace core {

    const std::locale loc(std::locale::classic());

    bool is_host_dns_name(const web::http::uri& uri)
    {
        utility::string_t host = uri.host();
        for (auto iter = host.cbegin(); iter != host.cend(); iter++)
        {
            if (!std::isdigit(*iter, loc) && *iter != U('.'))
            {
                return true;
            }
        }

        return false;
    }

    bool use_path_style(const web::http::uri& uri)
    {
        return (uri.port() >= 10000) || !is_host_dns_name(uri);
    }

    bool use_path_style(const storage_uri& uri)
    {
        return use_path_style(uri.primary_uri());
    }

    bool parse_container_uri(const web::http::uri& uri, utility::string_t& container_name)
    {
        auto segments = web::http::uri::split_path(uri.path());
        auto iter = segments.cbegin();

        if (use_path_style(uri))
        {
            if (iter == segments.cend())
            {
                return false;
            }

            iter++;
        }

        if (iter == segments.cend())
        {
            container_name = protocol::root_container;
        }
        else
        {
            container_name = *iter;
        }

        return true;
    }

    bool parse_container_uri(const storage_uri& uri, utility::string_t& container_name)
    {
        return parse_container_uri(uri.primary_uri(), container_name);
    }

    bool parse_blob_uri(const web::http::uri& uri, utility::string_t& container_name, utility::string_t& blob_name)
    {
        auto segments = web::http::uri::split_path(uri.path());
        auto iter = segments.cbegin();

        if (use_path_style(uri))
        {
            if (iter == segments.cend())
            {
                return false;
            }

            iter++;
        }

        if (iter == segments.cend())
        {
            return false;
        }

        container_name = *(iter++);
        if (iter == segments.cend())
        {
            blob_name = container_name;
            container_name = protocol::root_container;
        }
        else
        {
            utility::ostringstream_t blob_name_str;
            blob_name_str << *iter;
            for (iter++; iter != segments.cend(); iter++)
            {
                blob_name_str << U('/') << *iter;
            }

            blob_name = blob_name_str.str();
        }

        return true;
    }

    bool parse_blob_uri(const storage_uri& uri, utility::string_t& container_name, utility::string_t& blob_name)
    {
        return parse_blob_uri(uri.primary_uri(), container_name, blob_name);
    }

    web::http::uri verify_blob_uri(const web::http::uri& uri, storage_credentials& credentials, utility::string_t& snapshot)
    {
        if (uri.host().empty())
        {
            return uri;
        }

        auto query = web::http::uri::split_query(uri.query());
        auto snapshot_iter = query.find(protocol::uri_query_snapshot);
        if (snapshot_iter != query.end())
        {
            utility::string_t parsed_snapshot = snapshot_iter->second;
            if (!parsed_snapshot.empty())
            {
                if (!snapshot.empty() && (parsed_snapshot != snapshot))
                {
                    throw std::invalid_argument(utility::conversions::to_utf8string(protocol::error_multiple_snapshots));
                }

                snapshot = parsed_snapshot;
            }
        }

        auto parsed_credentials = protocol::parse_query(uri, true);
        if (parsed_credentials.is_sas())
        {
            if (credentials.is_shared_key() || (credentials.is_sas() && parsed_credentials.sas_token() != credentials.sas_token()))
            {
                throw std::invalid_argument(utility::conversions::to_utf8string(protocol::error_multiple_credentials));
            }

            credentials = parsed_credentials;
        }

        web::http::uri_builder builder;
        builder.set_scheme(uri.scheme());
        builder.set_host(uri.host());
        builder.set_port(uri.port());
        builder.set_path(uri.path());
        return builder.to_uri();
    }

    storage_uri verify_blob_uri(const storage_uri& uri, storage_credentials& credentials, utility::string_t& snapshot)
    {
        return storage_uri(verify_blob_uri(uri.primary_uri(), credentials, snapshot),
            verify_blob_uri(uri.secondary_uri(), credentials, snapshot));
    }

    web::http::uri append_path_to_uri(const web::http::uri& uri, const utility::string_t& path)
    {
        if (uri.is_empty())
        {
            return uri;
        }

        web::http::uri_builder builder(uri);
        builder.append_path(path, true);
        return builder.to_uri();
    }

    storage_uri append_path_to_uri(const storage_uri& uri, const utility::string_t& path)
    {
        return storage_uri(append_path_to_uri(uri.primary_uri(), path),
            append_path_to_uri(uri.secondary_uri(), path));
    }

    utility::string_t get_parent_name(utility::string_t name, const utility::string_t& delimiter)
    {
        if (name.length() >= delimiter.length())
        {
            auto pos = name.rfind(delimiter);
            if (pos == name.length() - delimiter.length())
            {
                name.erase(pos);
                pos = name.rfind(delimiter);
            }

            if (pos != utility::string_t::npos)
            {
                name.erase(pos);
            }
            else
            {
                name.clear();
            }
        }

        return name;
    }

    web::http::uri get_service_client_uri(const web::http::uri& uri)
    {
        if (uri.is_empty())
        {
            return uri;
        }

        if (use_path_style(uri))
        {
            web::http::uri_builder builder(uri.authority());

            auto segments = web::http::uri::split_path(uri.path());
            auto iter = segments.cbegin();
            if (iter != segments.cend())
            {
                builder.append_path(*iter);
            }

            return builder.to_uri();
        }
        else
        {
            return uri.authority();
        }
    }

    storage_uri get_service_client_uri(const storage_uri& uri)
    {
        return storage_uri(get_service_client_uri(uri.primary_uri()),
            get_service_client_uri(uri.secondary_uri()));
    }

}}} // namespace wa::storage::navigation
