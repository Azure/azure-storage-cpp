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

namespace azure { namespace storage { namespace core {

    bool is_host_dns_name(const web::http::uri& uri)
    {
        const utility::string_t& host = uri.host();
        for (utility::string_t::const_iterator it = host.cbegin(); it != host.cend(); ++it)
        {
            utility::char_t c = *it;
            if ((c < _XPLATSTR('0') || c > _XPLATSTR('9')) && c != _XPLATSTR('.'))
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

    utility::string_t::size_type find_path_start(const web::http::uri& uri)
    {
        if (use_path_style(uri))
        {
            const utility::string_t& path = uri.path();
            if (path.size() > 0)
            {
                // The path should always start with "/", but this code would still handle the case where it does not.
                utility::string_t::size_type start_pos = path.find(_XPLATSTR('/'), 1);
                if (start_pos == utility::string_t::npos)
                {
                    start_pos = path.size();
                }

                return start_pos;
            }
        }

        return 0;
    }

    bool parse_container_uri(const web::http::uri& uri, utility::string_t& container_name)
    {
        std::vector<utility::string_t> segments = web::http::uri::split_path(uri.path());
        std::vector<utility::string_t>::const_iterator it = segments.cbegin();

        if (use_path_style(uri))
        {
            if (it == segments.cend())
            {
                return false;
            }

            ++it;
        }

        if (it == segments.cend())
        {
            container_name = protocol::root_container;
        }
        else
        {
            container_name = *it;
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
            utility::string_t blob_name_str;
            blob_name_str.reserve(256);
            blob_name_str.append(*iter);
            for (iter++; iter != segments.cend(); iter++)
            {
                blob_name_str.append(_XPLATSTR("/"));
                blob_name_str.append(*iter);
            }

            blob_name.swap(blob_name_str);
        }

        return true;
    }

    bool parse_blob_uri(const storage_uri& uri, utility::string_t& container_name, utility::string_t& blob_name)
    {
        return parse_blob_uri(uri.primary_uri(), container_name, blob_name);
    }

    bool parse_file_directory_uri(const web::http::uri& uri, utility::string_t& share_name, utility::string_t& directory_name)
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

        share_name = *(iter++);
        if (iter == segments.cend())
        {
            directory_name = utility::string_t();
            return false;
        }
        else
        {
            directory_name = *(segments.crbegin());
        }

        return true;
    }

    bool parse_file_directory_uri(const storage_uri& uri, utility::string_t& share_name, utility::string_t& directory_name)
    {
        return parse_blob_uri(uri.primary_uri(), share_name, directory_name);
    }

    bool parse_file_uri(const web::http::uri& uri, utility::string_t& share_name, utility::string_t& directory_name, utility::string_t& file_name)
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

        share_name = *(iter++);
        auto iter_end = segments.cend();
        --iter_end;
        if (iter == segments.cend())
        {
            directory_name = utility::string_t();
            file_name = utility::string_t();
            return false;
        }
        else if (iter == iter_end)
        {
            directory_name = utility::string_t();
            file_name = *(iter);
            return true;
        }
        else
        {
            utility::ostringstream_t directory_name_str;
            directory_name_str << *iter;
            for (iter++; iter != iter_end; iter++)
            {
                directory_name_str << _XPLATSTR('/') << *iter;
            }

            directory_name = directory_name_str.str();
            file_name = *iter_end;
        }

        return true;
    }

    bool parse_file_uri(const storage_uri& uri, utility::string_t& share_name, utility::string_t& directory_name, utility::string_t& file_name)
    {
        return parse_file_uri(uri.primary_uri(), share_name, directory_name, file_name);
    }

    web::http::uri create_stripped_uri(const web::http::uri& uri)
    {
        web::http::uri_builder builder;
        builder.set_scheme(uri.scheme());
        builder.set_host(uri.host());
        builder.set_port(uri.port());
        builder.set_path(uri.path());
        return builder.to_uri();
    }

    storage_uri create_stripped_uri(const storage_uri& uri)
    {
        return storage_uri(create_stripped_uri(uri.primary_uri()), create_stripped_uri(uri.secondary_uri()));
    }

    void parse_query_and_verify(const web::http::uri& uri, storage_credentials& credentials, bool require_signed_resource)
    {
        // Read the SAS credentials if present from the URI
        storage_credentials parsed_credentials = protocol::parse_query(uri, require_signed_resource);
        if (parsed_credentials.is_sas())
        {
            if (credentials.is_shared_key() || credentials.is_sas())
            {
                throw std::invalid_argument(protocol::error_multiple_credentials);
            }

            // Overwrite the given credentials with the SAS credentials read from the URI
            credentials = parsed_credentials;
        }
    }

    void parse_query_and_verify(const storage_uri& uri, storage_credentials& credentials, bool require_signed_resource)
    {
        parse_query_and_verify(uri.primary_uri(), credentials, require_signed_resource);
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
            const utility::string_t& parsed_snapshot = snapshot_iter->second;
            if (!parsed_snapshot.empty())
            {
                if (!snapshot.empty() && (parsed_snapshot != snapshot))
                {
                    throw std::invalid_argument(protocol::error_multiple_snapshots);
                }

                snapshot = parsed_snapshot;
            }
        }

        parse_query_and_verify(uri, credentials, true);

        return create_stripped_uri(uri);
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

    bool parse_object_uri(const web::http::uri& uri, utility::string_t& object_name)
    {
        std::vector<utility::string_t> segments = web::http::uri::split_path(uri.path());
        std::vector<utility::string_t>::const_iterator it = segments.cbegin();

        if (use_path_style(uri))
        {
            if (it != segments.cend())
            {
                ++it;
            }
        }

        if (it == segments.cend())
        {
            return false;
        }

        object_name = *it;
        return true;
    }

    bool parse_object_uri(const storage_uri& uri, utility::string_t& object_name)
    {
        return parse_object_uri(uri.primary_uri(), object_name);
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

}}} // namespace azure::storage::navigation
