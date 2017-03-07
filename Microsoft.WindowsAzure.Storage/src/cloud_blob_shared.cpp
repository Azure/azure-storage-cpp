// -----------------------------------------------------------------------------------------
// <copyright file="cloud_blob_shared.cpp" company="Microsoft">
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
#include "wascore/protocol.h"
#include "wascore/resources.h"

namespace azure { namespace storage {

    void cloud_blob_container_properties::update_etag_and_last_modified(const cloud_blob_container_properties& parsed_properties)
    {
        m_etag = parsed_properties.etag();
        m_last_modified = parsed_properties.last_modified();
    }

    void cloud_blob_properties::update_etag_and_last_modified(const cloud_blob_properties& parsed_properties)
    {
        m_etag = parsed_properties.etag();
        m_last_modified = parsed_properties.last_modified();
    }

    void cloud_blob_properties::copy_from_root(const cloud_blob_properties& root_blob_properties)
    {
        m_size = root_blob_properties.m_size;
        m_etag = root_blob_properties.m_etag;
        m_last_modified = root_blob_properties.m_last_modified;
        m_type = root_blob_properties.m_type;
        m_page_blob_sequence_number = root_blob_properties.m_page_blob_sequence_number;
        m_cache_control = root_blob_properties.m_cache_control;
        m_content_disposition = root_blob_properties.m_content_disposition;
        m_content_encoding = root_blob_properties.m_content_encoding;
        m_content_language = root_blob_properties.m_content_language;
        m_content_md5 = root_blob_properties.m_content_md5;
        m_content_type = root_blob_properties.m_content_type;
    }

    void cloud_blob_properties::update_size(const cloud_blob_properties& parsed_properties)
    {
        m_size = parsed_properties.size();
    }

    void cloud_blob_properties::update_page_blob_sequence_number(const cloud_blob_properties& parsed_properties)
    {
        m_page_blob_sequence_number = parsed_properties.page_blob_sequence_number();
    }

    void cloud_blob_properties::update_append_blob_committed_block_count(const cloud_blob_properties& parsed_properties)
    {
        m_append_blob_committed_block_count = parsed_properties.append_blob_committed_block_count();
    }

    void cloud_blob_properties::update_all(const cloud_blob_properties& parsed_properties)
    {
        if ((type() != blob_type::unspecified) && (type() != parsed_properties.type()))
        {
            throw storage_exception(protocol::error_blob_type_mismatch, false);
        }

        *this = parsed_properties;
    }

}} // namespace azure::storage
