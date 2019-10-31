// -----------------------------------------------------------------------------------------
// <copyright file="OAuthGettingStarted.cpp" company="Microsoft">
//    Copyright 2019 Microsoft Corporation
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

#include "samples_common.h"

#include <was/storage_account.h>
#include <was/blob.h>


namespace azure { namespace storage { namespace samples {

    SAMPLE(OAuthGettingStarted, oauth_getting_started_sample)
    void oauth_getting_started_sample()
    {
        utility::string_t oauth_access_token(_XPLATSTR("PUT_YOUR_OAUTH_2_0_ACCESS_TOKEN_HERE"));

        using OAuthAccessToken = azure::storage::storage_credentials::bearer_token_credential;
        azure::storage::storage_credentials storage_cred(OAuthAccessToken{ oauth_access_token });

        azure::storage::cloud_storage_account storage_account(
            storage_cred,
            azure::storage::storage_uri(web::http::uri(_XPLATSTR("https://YOUR_STORAGE_ACCOUNT.blob.core.windows.net"))),
            azure::storage::storage_uri(web::http::uri(_XPLATSTR("https://YOUR_STORAGE_ACCOUNT.queue.core.windows.net"))),
            azure::storage::storage_uri(web::http::uri(_XPLATSTR("https://YOUR_STORAGE_ACCOUNT.table.core.windows.net")))
        );

        auto blob_client = storage_account.create_cloud_blob_client();
        auto blob_container = blob_client.get_container_reference(_XPLATSTR("YOUR_CONTAINER"));
        auto blob = blob_container.get_blob_reference(_XPLATSTR("FOO.BAR"));

        try
        {
            std::cout << blob.exists() << std::endl;
        }
        catch (const azure::storage::storage_exception& e)
        {
            std::cout << e.what() << std::endl;
        }

        // Here we make some copies of the storage credential.
        azure::storage::storage_credentials storage_cred2(storage_cred);
        azure::storage::storage_credentials storage_cred3;
        storage_cred3 = storage_cred2;
        azure::storage::storage_credentials storage_cred4(OAuthAccessToken{ oauth_access_token });

        // After a while, the access token may expire, refresh it.
        storage_cred.set_bearer_token(_XPLATSTR("YOUR_NEW_OAUTH_2_0_ACCESS_TOKEN"));
        // storage_cred2.set_bearer_token(_XPLATSTR("YOUR_NEW_OAUTH_2_0_ACCESS_TOKEN"));
        // storage_cred3.set_bearer_token(_XPLATSTR("YOUR_NEW_OAUTH_2_0_ACCESS_TOKEN"));
        // Note that, every storage_crentials struct copied directly or indirectly shares the same underlaying access token.
        // So the three lines above have the same effect.

        // But if you create another storage_crendials with the same access token, they are not interconnected because they are not created by coping or assigning.
        storage_cred4.set_bearer_token(_XPLATSTR("YOUR_NEW_OAUTH_2_0_ACCESS_TOKEN"));  // This doesn't change access token inside storage_cred{1,2,3}

        try
        {
            std::cout << blob.exists() << std::endl;
        }
        catch (const azure::storage::storage_exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
}}}  // namespace azure::storage::samples