// -----------------------------------------------------------------------------------------
// <copyright file="cloud_blob_directory.cpp" company="Microsoft">
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
#include "was/blob.h"
#include "wascore/util.h"

namespace azure { namespace storage {

    cloud_blob_directory::cloud_blob_directory(utility::string_t name, cloud_blob_container container)
        : m_name(std::move(name)), m_container(std::move(container))
    {
        auto& delimiter = m_container.service_client().directory_delimiter();
        if ((m_name.size() < delimiter.size()) ||
            !std::equal(delimiter.crbegin(), delimiter.crend(), m_name.crbegin()))
        {
            m_name.append(delimiter);
        }

        m_uri = core::append_path_to_uri(m_container.uri(), m_name);
    }

    cloud_blob cloud_blob_directory::get_blob_reference(utility::string_t blob_name) const
    {
        return get_blob_reference(std::move(blob_name), utility::string_t());
    }

    cloud_blob cloud_blob_directory::get_blob_reference(utility::string_t blob_name, utility::string_t snapshot_time) const
    {
        return cloud_blob(m_name + blob_name, std::move(snapshot_time), m_container);
    }

    cloud_page_blob cloud_blob_directory::get_page_blob_reference(utility::string_t blob_name) const
    {
        return get_page_blob_reference(std::move(blob_name), utility::string_t());
    }

    cloud_page_blob cloud_blob_directory::get_page_blob_reference(utility::string_t blob_name, utility::string_t snapshot_time) const
    {
        return cloud_page_blob(m_name + blob_name, std::move(snapshot_time), m_container);
    }

    cloud_block_blob cloud_blob_directory::get_block_blob_reference(utility::string_t blob_name) const
    {
        return get_block_blob_reference(std::move(blob_name), utility::string_t());
    }

    cloud_block_blob cloud_blob_directory::get_block_blob_reference(utility::string_t blob_name, utility::string_t snapshot_time) const
    {
        return cloud_block_blob(m_name + blob_name, std::move(snapshot_time), m_container);
    }

    cloud_append_blob cloud_blob_directory::get_append_blob_reference(utility::string_t blob_name) const
    {
        return get_block_blob_reference(std::move(blob_name), utility::string_t());
    }

    cloud_append_blob cloud_blob_directory::get_append_blob_reference(utility::string_t blob_name, utility::string_t snapshot_time) const
    {
        return cloud_append_blob(m_name + blob_name, std::move(snapshot_time), m_container);
    }

    cloud_blob_directory cloud_blob_directory::get_subdirectory_reference(utility::string_t name) const
    {
        return cloud_blob_directory(m_name + name, m_container);
    }

    cloud_blob_directory cloud_blob_directory::get_parent_reference() const
    {
        utility::string_t parent_name(core::get_parent_name(m_name, m_container.service_client().directory_delimiter()));
        if (parent_name.empty())
        {
            return cloud_blob_directory();
        }
        else
        {
            return cloud_blob_directory(parent_name, m_container);
        }
    }

    list_blob_item_iterator cloud_blob_directory::list_blobs(bool use_flat_blob_listing, blob_listing_details::values includes, int max_results, const blob_request_options& options, operation_context context) const
    {
        return m_container.list_blobs(m_name, use_flat_blob_listing, includes, max_results, options, context);
    }

    pplx::task<list_blob_item_segment> cloud_blob_directory::list_blobs_segmented_async(bool use_flat_blob_listing, blob_listing_details::values includes, int max_results, const continuation_token& token, const blob_request_options& options, operation_context context) const
    {
        return m_container.list_blobs_segmented_async(m_name, use_flat_blob_listing, includes, max_results, token, options, context);
    }

}} // namespace azure::storage
