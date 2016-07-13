// -----------------------------------------------------------------------------------------
// <copyright file="cloud_file_client_test.cpp" company="Microsoft">
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

#pragma region Fixture

#pragma endregion

SUITE(File)
{
    TEST_FIXTURE(file_service_test_base, get_share_reference)
    {
        utility::string_t share_name = get_random_share_name();
        azure::storage::cloud_file_share share = m_client.get_share_reference(share_name);

        CHECK(!share.service_client().base_uri().primary_uri().is_empty());
        CHECK(share.service_client().credentials().is_shared_key());
        CHECK(share.name() == share_name);
        CHECK(!share.uri().primary_uri().is_empty());
        CHECK(share.metadata().empty());
        CHECK(share.properties().etag().empty());
        CHECK(!share.properties().last_modified().is_initialized());
        CHECK(share.is_valid());
    }

    /// <summary>
    /// Test list shares with create a number of shares and compare the list share results with created shares.
    /// </summary>
    TEST_FIXTURE(file_service_test_base_with_objects_to_delete, list_shares_with_prefix)
    {
        auto prefix = get_random_share_name();

        create_share(prefix, 10);

        auto listing = list_all_shares(prefix, true, 0, azure::storage::file_request_options());

        check_share_list(listing, prefix, true);
    }

    /// <summary>
    /// Test list shares with create a number of shares and compare the list share results with created shares.
    /// </summary>
    TEST_FIXTURE(file_service_test_base_with_objects_to_delete, list_shares)
    {
        auto prefix = get_random_share_name();

        create_share(prefix, 1);

        auto listing = list_all_shares(utility::string_t(), true, 0, azure::storage::file_request_options());

        check_share_list(listing, prefix, false);
    }

    // see in service_properties_test for file service properties test.

    TEST_FIXTURE(file_service_test_base_with_objects_to_delete, list_shares_with_continuation_token)
    {
        auto prefix = get_random_string();
        create_share(prefix, 10);

        std::vector<azure::storage::cloud_file_share> listing;
        azure::storage::continuation_token token;
        azure::storage::file_request_options options;
        
        do {
            auto results = m_client.list_shares_segmented(prefix, true, 3, token, options, m_context);
            CHECK(results.results().size() <= 3);

            std::copy(results.results().begin(), results.results().end(), std::back_inserter(listing));
            token = results.continuation_token();
        } while (!token.empty());

        check_share_list(listing, prefix, true);
    }
        
    TEST_FIXTURE(file_service_test_base, file_shared_key_lite)
    {
        auto client = test_config::instance().account().create_cloud_file_client();
        client.set_authentication_scheme(azure::storage::authentication_scheme::shared_key_lite);
        client.list_shares_segmented(utility::string_t(), true, 3, azure::storage::continuation_token(), azure::storage::file_request_options(), m_context);
    }

    // see in cloud_storage_account_test for file account sas test.
}