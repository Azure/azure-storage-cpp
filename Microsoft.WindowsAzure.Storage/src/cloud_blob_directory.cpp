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

namespace wa { namespace storage {

    cloud_blob_directory::cloud_blob_directory(const utility::string_t& name, const cloud_blob_container& container)
        : m_name(name), m_container(container)
    {
        auto& delimiter = container.service_client().directory_delimiter();
        if ((name.size() < delimiter.size()) ||
            !std::equal(delimiter.crbegin(), delimiter.crend(), name.crbegin()))
        {
            m_name.append(delimiter);
        }

        m_uri = core::append_path_to_uri(container.uri(), m_name);
    }

    cloud_blob cloud_blob_directory::get_blob_reference(const utility::string_t& blob_name) const
    {
        return get_blob_reference(blob_name, utility::string_t());
    }

    cloud_blob cloud_blob_directory::get_blob_reference(const utility::string_t& blob_name, const utility::string_t& snapshot_time) const
    {
        return cloud_blob(m_name + blob_name, snapshot_time, m_container);
    }

    cloud_page_blob cloud_blob_directory::get_page_blob_reference(const utility::string_t& blob_name) const
    {
        return get_page_blob_reference(blob_name, utility::string_t());
    }

    cloud_page_blob cloud_blob_directory::get_page_blob_reference(const utility::string_t& blob_name, const utility::string_t& snapshot_time) const
    {
        return cloud_page_blob(m_name + blob_name, snapshot_time, m_container);
    }

    cloud_block_blob cloud_blob_directory::get_block_blob_reference(const utility::string_t& blob_name) const
    {
        return get_block_blob_reference(blob_name, utility::string_t());
    }

    cloud_block_blob cloud_blob_directory::get_block_blob_reference(const utility::string_t& blob_name, const utility::string_t& snapshot_time) const
    {
        return cloud_block_blob(m_name + blob_name, snapshot_time, m_container);
    }

    cloud_blob_directory cloud_blob_directory::get_subdirectory_reference(const utility::string_t& name) const
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

    pplx::task<blob_result_segment> cloud_blob_directory::list_blobs_segmented_async(bool use_flat_blob_listing, const blob_listing_includes& includes, int max_results, const blob_continuation_token& current_token, const blob_request_options& options, operation_context context) const
    {
        return m_container.list_blobs_segmented_async(m_name, use_flat_blob_listing, includes, max_results, current_token, options, context);
    }

}} // namespace wa::storage
