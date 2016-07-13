// -----------------------------------------------------------------------------------------
// <copyright file="file_test_base.cpp" company="Microsoft">
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
#include "file_test_base.h"
#include "check_macros.h"

utility::string_t file_service_test_base::get_random_share_name(size_t length)
{
    utility::string_t name;
    name.resize(length);
    std::generate_n(name.begin(), length, []() -> utility::char_t
    {
        const utility::char_t possible_chars[] = { _XPLATSTR("abcdefghijklmnopqrstuvwxyz1234567890") };
        return possible_chars[std::rand() % (sizeof(possible_chars) / sizeof(utility::char_t) - 1)];
    });

    return utility::conversions::print_string(utility::datetime::utc_now().to_interval()) + name;
}

void file_service_test_base::check_equal(const azure::storage::cloud_file_share& source, const azure::storage::cloud_file_share& target)
{
    CHECK(source.name() == target.name());
    CHECK(source.uri().primary_uri() == target.uri().primary_uri());
    CHECK(source.uri().secondary_uri() == target.uri().secondary_uri());
}

void file_service_test_base::check_equal(const azure::storage::cloud_file_directory& source, const azure::storage::cloud_file_directory& target)
{
    CHECK(source.name() == target.name());
    CHECK(source.uri().primary_uri() == target.uri().primary_uri());
    CHECK(source.uri().secondary_uri() == target.uri().secondary_uri());
}

void file_service_test_base::check_equal(const azure::storage::cloud_file& source, const azure::storage::cloud_file& target)
{
    CHECK(source.name() == target.name());
    CHECK(source.uri().primary_uri() == target.uri().primary_uri());
    CHECK(source.uri().secondary_uri() == target.uri().secondary_uri());
}

std::vector<azure::storage::cloud_file_share> file_service_test_base::list_all_shares(const utility::string_t& prefix, bool get_metadata, int max_results, const azure::storage::file_request_options& options)
{
    std::vector<azure::storage::cloud_file_share> result;
    for (auto&& item : m_client.list_shares(prefix, get_metadata, max_results, options, m_context))
    {
        result.push_back(item);
    }
    return result;
}

void file_service_test_base_with_objects_to_delete::create_share(const utility::string_t& prefix, std::size_t num)
{
    for (size_t i = 0; i < num; ++i)
    {
        auto index = utility::conversions::print_string(i);
        auto share = m_client.get_share_reference(prefix + index);
        m_shares_to_delete.push_back(share);
        share.metadata()[_XPLATSTR("index")] = index;
        share.create(azure::storage::file_request_options(), m_context);
    }
}

void file_service_test_base_with_objects_to_delete::check_share_list(const std::vector<azure::storage::cloud_file_share>& list, const utility::string_t& prefix, bool check_found)
{
    auto share_list_sorted = std::is_sorted(list.cbegin(), list.cend(), [](const azure::storage::cloud_file_share& a, const azure::storage::cloud_file_share& b)
    {
        return a.name() < b.name();
    });
    CHECK(share_list_sorted);

    std::vector<azure::storage::cloud_file_share> shares(m_shares_to_delete);

    for (auto list_iter = list.begin(); list_iter != list.end(); ++list_iter)
    {
        bool found = false;
        for (auto iter = shares.begin(); iter != shares.end(); ++iter)
        {
            if (iter->name() == list_iter->name())
            {
                auto index_str = list_iter->metadata().find(_XPLATSTR("index"));
                CHECK(index_str != list_iter->metadata().end());
                CHECK_UTF8_EQUAL(iter->name(), prefix + index_str->second);
                shares.erase(iter);
                found = true;
                break;
            }
        }
        if (check_found)
        {
            CHECK(found);
        }
    }
    CHECK(shares.empty());
}

utility::string_t file_share_test_base::get_random_directory_name(size_t length)
{
    return _XPLATSTR("dir") + get_random_string(length);
}

utility::string_t file_directory_test_base::get_random_file_name(size_t length)
{
    return _XPLATSTR("file") + get_random_string(length);
}


void file_share_test_base::check_access(const utility::string_t& sas_token, uint8_t permissions, const azure::storage::cloud_file_shared_access_headers& headers, const azure::storage::cloud_file& original_file)
{
    azure::storage::storage_credentials credentials;
    if (!sas_token.empty())
    {
        credentials = azure::storage::storage_credentials(sas_token);
    }

    azure::storage::cloud_file_share share(m_share.uri(), credentials);
    azure::storage::cloud_file file = share.get_root_directory_reference().get_file_reference(original_file.name());

    if (permissions & azure::storage::file_shared_access_policy::permissions::list)
    {
        share.get_root_directory_reference().list_files_and_directories_segmented(0, azure::storage::continuation_token(), azure::storage::file_request_options(), m_context);
    }
    else
    {
        CHECK_THROW(share.get_root_directory_reference().list_files_and_directories_segmented(0, azure::storage::continuation_token(), azure::storage::file_request_options(), m_context), azure::storage::storage_exception);
    }

    if (permissions & azure::storage::file_shared_access_policy::permissions::read)
    {
        file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

        if (!headers.cache_control().empty())
        {
            CHECK_UTF8_EQUAL(headers.cache_control(), file.properties().cache_control());
        }
        else
        {
            CHECK_UTF8_EQUAL(original_file.properties().cache_control(), file.properties().cache_control());
        }

        if (!headers.content_disposition().empty())
        {
            CHECK_UTF8_EQUAL(headers.content_disposition(), file.properties().content_disposition());
        }
        else
        {
            CHECK_UTF8_EQUAL(original_file.properties().content_disposition(), file.properties().content_disposition());
        }

        if (!headers.content_encoding().empty())
        {
            CHECK_UTF8_EQUAL(headers.content_encoding(), file.properties().content_encoding());
        }
        else
        {
            CHECK_UTF8_EQUAL(original_file.properties().content_encoding(), file.properties().content_encoding());
        }

        if (!headers.content_language().empty())
        {
            CHECK_UTF8_EQUAL(headers.content_language(), file.properties().content_language());
        }
        else
        {
            CHECK_UTF8_EQUAL(original_file.properties().content_language(), file.properties().content_language());
        }

        if (!headers.content_type().empty())
        {
            CHECK_UTF8_EQUAL(headers.content_type(), file.properties().content_type());
        }
        else
        {
            CHECK_UTF8_EQUAL(original_file.properties().content_type(), file.properties().content_type());
        }
    }
    else
    {
        CHECK_THROW(file.download_attributes(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context), azure::storage::storage_exception);
    }

    if (permissions & azure::storage::file_shared_access_policy::permissions::write)
    {
        file.upload_metadata(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
    }
    else
    {
        CHECK_THROW(file.upload_metadata(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context), azure::storage::storage_exception);
    }

    if (permissions & azure::storage::file_shared_access_policy::permissions::del)
    {
        file.delete_file(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);
    }
    else
    {
        CHECK_THROW(file.delete_file(azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context), azure::storage::storage_exception);
    }
}