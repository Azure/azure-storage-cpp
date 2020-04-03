// -----------------------------------------------------------------------------------------
// <copyright file="blob_lease_test.cpp" company="Microsoft">
//    Copyright 2020 Microsoft Corporation
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
#include "blob_test_base.h"
#include "check_macros.h"

#include <set>

SUITE(Blob)
{
    TEST_FIXTURE(block_blob_test_base, blob_versioning_properties)
    {
        utility::string_t blob_content = _XPLATSTR("test");
        m_blob.upload_text(blob_content, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        auto version_id0 = m_blob.properties().version_id();
        CHECK(!version_id0.empty());

        m_blob.download_attributes();
        CHECK(version_id0 == m_blob.properties().version_id());

        m_blob.upload_text(blob_content, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        auto version_id1 = m_blob.properties().version_id();
        CHECK(version_id1 != version_id0);

        m_blob.metadata()[_XPLATSTR("k1")] = _XPLATSTR("value1");
        m_blob.upload_metadata();
        auto version_id2 = m_blob.properties().version_id();
        CHECK(version_id2 != version_id1);

        m_blob.create_snapshot();
        auto version_id3 = m_blob.properties().version_id();
        CHECK(version_id3 != version_id2);

        m_blob.properties().set_content_md5(utility::string_t());
        m_blob.upload_block_list(std::vector<azure::storage::block_list_item>());
        auto version_id4 = m_blob.properties().version_id();
        CHECK(version_id4 != version_id3);
        CHECK(utility::string_t() == m_blob.download_text());
        CHECK(version_id4 == m_blob.properties().version_id());


        m_blob.start_copy(m_blob.uri());
        auto version_id5 = m_blob.properties().version_id();
        CHECK(version_id5 != version_id4);

        auto blobs = m_container.list_blobs_segmented(utility::string_t(), true, azure::storage::blob_listing_details::none, 0, azure::storage::continuation_token(), azure::storage::blob_request_options(), azure::storage::operation_context());
        CHECK_EQUAL(1, blobs.results().size());
        CHECK(blobs.results()[0].is_blob());
        CHECK(blobs.results()[0].is_current_version());
        auto blob = blobs.results()[0].as_blob();
        CHECK(blob.version_id().empty());
        CHECK(!blob.properties().version_id().empty());

        blobs = m_container.list_blobs_segmented(utility::string_t(), true, azure::storage::blob_listing_details::versions, 0, azure::storage::continuation_token(), azure::storage::blob_request_options(), azure::storage::operation_context());
        std::set<utility::string_t> versions;

        for (const auto& t : blobs.results())
        {
            if (t.is_blob())
            {
                versions.emplace(t.as_blob().version_id());
            }
        }
        CHECK(versions.find(version_id0) != versions.end());
        CHECK(versions.find(version_id1) != versions.end());
        CHECK(versions.find(version_id2) != versions.end());
        CHECK(versions.find(version_id3) != versions.end());
        CHECK(versions.find(version_id4) != versions.end());
        CHECK(versions.find(version_id5) != versions.end());

        for (const auto& t : blobs.results())
        {
            if (t.is_blob())
            {
                blob = t.as_blob();
                CHECK(!blob.version_id().empty());
                if (t.is_current_version())
                {
                    CHECK(blob.version_id() == version_id5);
                    CHECK(blob.properties().version_id() == version_id5);
                }

                if (blob.version_id() == version_id0)
                {
                    azure::storage::cloud_block_blob block_blob(blob);
                    CHECK(blob_content == block_blob.download_text());

                    blob.download_attributes();
                    CHECK(blob.metadata() == azure::storage::cloud_metadata());
                }
            }
        }

        m_blob.delete_blob(azure::storage::delete_snapshots_option::include_snapshots, azure::storage::access_condition(), azure::storage::blob_request_options(), azure::storage::operation_context());

        blobs = m_container.list_blobs_segmented(utility::string_t(), true, azure::storage::blob_listing_details::versions, 0, azure::storage::continuation_token(), azure::storage::blob_request_options(), azure::storage::operation_context());
        CHECK(!blobs.results().empty());
        for (const auto&t : blobs.results())
        {
            if (t.is_blob())
            {
                CHECK(!t.is_current_version());
                blob = t.as_blob();
                blob.delete_blob();
            }
        }
        blobs = m_container.list_blobs_segmented(utility::string_t(), true, azure::storage::blob_listing_details::versions, 0, azure::storage::continuation_token(), azure::storage::blob_request_options(), azure::storage::operation_context());
        CHECK(blobs.results().empty());
    }
}
