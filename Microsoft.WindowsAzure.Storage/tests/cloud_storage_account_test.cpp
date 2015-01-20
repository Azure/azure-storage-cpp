// -----------------------------------------------------------------------------------------
// <copyright file="cloud_storage_account_test.cpp" company="Microsoft">
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

#include "test_base.h"
#include "check_macros.h"
#include "was/storage_account.h"
#include "was/blob.h"
#include "was/queue.h"
#include "was/table.h"
#include "wascore/constants.h"

const utility::string_t test_uri(U("http://test/abc"));
const utility::string_t token(U("sp=abcde&sig=1"));
const utility::string_t token_with_api_version(azure::storage::protocol::uri_query_sas_api_version + U("=") + azure::storage::protocol::header_value_storage_version + U("&sig=1&sp=abcde"));
const utility::string_t test_account_name(U("test"));
const utility::string_t test_account_key(U("Fby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="));
const utility::string_t test_endpoint_suffix(U("fake.endpoint.suffix"));

void check_credentials_equal(const azure::storage::storage_credentials& a, const azure::storage::storage_credentials& b)
{
    CHECK_EQUAL(a.is_anonymous(), b.is_anonymous());
    CHECK_EQUAL(a.is_sas(), b.is_sas());
    CHECK_EQUAL(a.is_shared_key(), b.is_shared_key());
    CHECK_UTF8_EQUAL(a.account_name(), b.account_name());
    CHECK_UTF8_EQUAL(utility::conversions::to_base64(a.account_key()), utility::conversions::to_base64(b.account_key()));
}

void check_account_equal(azure::storage::cloud_storage_account a, azure::storage::cloud_storage_account b)
{
    CHECK_UTF8_EQUAL(a.blob_endpoint().primary_uri().to_string(), b.blob_endpoint().primary_uri().to_string());
    CHECK_UTF8_EQUAL(a.blob_endpoint().secondary_uri().to_string(), b.blob_endpoint().secondary_uri().to_string());
    CHECK_UTF8_EQUAL(a.queue_endpoint().primary_uri().to_string(), b.queue_endpoint().primary_uri().to_string());
    CHECK_UTF8_EQUAL(a.queue_endpoint().secondary_uri().to_string(), b.queue_endpoint().secondary_uri().to_string());
    CHECK_UTF8_EQUAL(a.table_endpoint().primary_uri().to_string(), b.table_endpoint().primary_uri().to_string());
    CHECK_UTF8_EQUAL(a.table_endpoint().secondary_uri().to_string(), b.table_endpoint().secondary_uri().to_string());
    CHECK_UTF8_EQUAL(a.to_string(), b.to_string(false));
    CHECK_UTF8_EQUAL(a.to_string(true), b.to_string(true));
    check_credentials_equal(a.credentials(), b.credentials());
}

void check_string_roundtrip(const utility::string_t& connection_string)
{
    auto account = azure::storage::cloud_storage_account::parse(connection_string);
    CHECK_UTF8_EQUAL(connection_string, account.to_string(true));
    check_account_equal(account, azure::storage::cloud_storage_account::parse(account.to_string(true)));
}

SUITE(Core)
{
    TEST_FIXTURE(test_base, storage_credentials_anonymous)
    {
        azure::storage::storage_credentials creds;

        CHECK(creds.sas_token().empty());
        CHECK_UTF8_EQUAL(utility::string_t(), creds.account_name());
        CHECK_EQUAL(true, creds.is_anonymous());
        CHECK_EQUAL(false, creds.is_sas());
        CHECK_EQUAL(false, creds.is_shared_key());

        web::http::uri uri(test_uri);
        CHECK_UTF8_EQUAL(test_uri, creds.transform_uri(uri).to_string());
    }

    TEST_FIXTURE(test_base, storage_credentials_shared_key)
    {
        azure::storage::storage_credentials creds(test_account_name, test_account_key);

        CHECK(creds.sas_token().empty());
        CHECK_UTF8_EQUAL(test_account_name, creds.account_name());
        CHECK_EQUAL(false, creds.is_anonymous());
        CHECK_EQUAL(false, creds.is_sas());
        CHECK_EQUAL(true, creds.is_shared_key());

        web::http::uri uri(test_uri);
        CHECK_UTF8_EQUAL(test_uri, creds.transform_uri(uri).to_string());

        CHECK_UTF8_EQUAL(test_account_key, utility::conversions::to_base64(creds.account_key()));
    }

    TEST_FIXTURE(test_base, storage_credentials_sas)
    {
        {
            azure::storage::storage_credentials creds(token);

            CHECK_UTF8_EQUAL(token, creds.sas_token());
            CHECK(creds.account_name().empty());
            CHECK(creds.account_key().empty());
            CHECK(!creds.is_anonymous());
            CHECK(creds.is_sas());
            CHECK(!creds.is_shared_key());

            web::http::uri uri(test_uri);
            CHECK_UTF8_EQUAL(test_uri + U("?") + token_with_api_version, creds.transform_uri(uri).to_string());
        }

        {
            azure::storage::storage_credentials creds(U("?") + token);

            CHECK_UTF8_EQUAL(token, creds.sas_token());
            CHECK(creds.account_name().empty());
            CHECK(creds.account_key().empty());
            CHECK(!creds.is_anonymous());
            CHECK(creds.is_sas());
            CHECK(!creds.is_shared_key());

            web::http::uri uri(test_uri);
            CHECK_UTF8_EQUAL(test_uri + U("?") + token_with_api_version, creds.transform_uri(uri).to_string());
        }
    }

    TEST_FIXTURE(test_base, storage_credentials_empty_key)
    {
        const utility::string_t defaults_connection_string(U("DefaultEndpointsProtocol=https;AccountName=") + test_account_name + U(";AccountKey="));

        azure::storage::storage_credentials creds(test_account_name, utility::string_t());
        CHECK(creds.sas_token().empty());
        CHECK_UTF8_EQUAL(test_account_name, creds.account_name());
        CHECK_EQUAL(false, creds.is_anonymous());
        CHECK_EQUAL(false, creds.is_sas());
        CHECK_EQUAL(true, creds.is_shared_key());
        CHECK_UTF8_EQUAL(utility::string_t(), utility::conversions::to_base64(creds.account_key()));

        azure::storage::cloud_storage_account account1(creds, true);
        CHECK_UTF8_EQUAL(defaults_connection_string, account1.to_string(true));
        check_credentials_equal(creds, account1.credentials());

        auto account2 = azure::storage::cloud_storage_account::parse(defaults_connection_string);
        CHECK_UTF8_EQUAL(defaults_connection_string, account2.to_string(true));
        check_credentials_equal(creds, account2.credentials());
    }

    TEST_FIXTURE(test_base, cloud_storage_account_devstore)
    {
        auto account = azure::storage::cloud_storage_account::development_storage_account();
        CHECK_UTF8_EQUAL(U("http://127.0.0.1:10000/devstoreaccount1"), account.blob_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://127.0.0.1:10001/devstoreaccount1"), account.queue_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://127.0.0.1:10002/devstoreaccount1"), account.table_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://127.0.0.1:10000/devstoreaccount1-secondary"), account.blob_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://127.0.0.1:10001/devstoreaccount1-secondary"), account.queue_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://127.0.0.1:10002/devstoreaccount1-secondary"), account.table_endpoint().secondary_uri().to_string());
    }

    TEST_FIXTURE(test_base, cloud_storage_account_default_http)
    {
        azure::storage::storage_credentials creds(test_account_name, test_account_key);
        azure::storage::cloud_storage_account account(creds, false);

        CHECK_UTF8_EQUAL(U("http://") + test_account_name + U(".blob.core.windows.net/"), account.blob_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://") + test_account_name + U(".queue.core.windows.net/"), account.queue_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://") + test_account_name + U(".table.core.windows.net/"), account.table_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://") + test_account_name + U("-secondary.blob.core.windows.net/"), account.blob_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://") + test_account_name + U("-secondary.queue.core.windows.net/"), account.queue_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://") + test_account_name + U("-secondary.table.core.windows.net/"), account.table_endpoint().secondary_uri().to_string());

        auto account2 = azure::storage::cloud_storage_account::parse(account.to_string(true));
        check_account_equal(account, account2);
    }

    TEST_FIXTURE(test_base, cloud_storage_account_default_https)
    {
        azure::storage::storage_credentials creds(test_account_name, test_account_key);
        azure::storage::cloud_storage_account account(creds, true);

        CHECK_UTF8_EQUAL(U("https://") + test_account_name + U(".blob.core.windows.net/"), account.blob_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("https://") + test_account_name + U(".queue.core.windows.net/"), account.queue_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("https://") + test_account_name + U(".table.core.windows.net/"), account.table_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("https://") + test_account_name + U("-secondary.blob.core.windows.net/"), account.blob_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(U("https://") + test_account_name + U("-secondary.queue.core.windows.net/"), account.queue_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(U("https://") + test_account_name + U("-secondary.table.core.windows.net/"), account.table_endpoint().secondary_uri().to_string());

        auto account2 = azure::storage::cloud_storage_account::parse(account.to_string(true));
        check_account_equal(account, account2);
    }

    TEST_FIXTURE(test_base, cloud_storage_account_endpoint_suffix_http)
    {
        utility::ostringstream_t str;
        str << U("DefaultEndpointsProtocol=http;AccountName=") << test_account_name << ";AccountKey=" << test_account_key << ";EndpointSuffix=" << test_endpoint_suffix;
        auto account = azure::storage::cloud_storage_account::parse(str.str());

        CHECK_UTF8_EQUAL(U("http://") + test_account_name + U(".blob.") + test_endpoint_suffix + U("/"), account.blob_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://") + test_account_name + U(".queue.") + test_endpoint_suffix + U("/"), account.queue_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://") + test_account_name + U(".table.") + test_endpoint_suffix + U("/"), account.table_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://") + test_account_name + U("-secondary.blob.") + test_endpoint_suffix + U("/"), account.blob_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://") + test_account_name + U("-secondary.queue.") + test_endpoint_suffix + U("/"), account.queue_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://") + test_account_name + U("-secondary.table.") + test_endpoint_suffix + U("/"), account.table_endpoint().secondary_uri().to_string());
    }

    TEST_FIXTURE(test_base, cloud_storage_account_endpoint_suffix_https)
    {
        utility::ostringstream_t str;
        str << U("DefaultEndpointsProtocol=https;AccountName=") << test_account_name << ";AccountKey=" << test_account_key << ";EndpointSuffix=" << test_endpoint_suffix;
        auto account = azure::storage::cloud_storage_account::parse(str.str());

        CHECK_UTF8_EQUAL(U("https://") + test_account_name + U(".blob.") + test_endpoint_suffix + U("/"), account.blob_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("https://") + test_account_name + U(".queue.") + test_endpoint_suffix + U("/"), account.queue_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("https://") + test_account_name + U(".table.") + test_endpoint_suffix + U("/"), account.table_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("https://") + test_account_name + U("-secondary.blob.") + test_endpoint_suffix + U("/"), account.blob_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(U("https://") + test_account_name + U("-secondary.queue.") + test_endpoint_suffix + U("/"), account.queue_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(U("https://") + test_account_name + U("-secondary.table.") + test_endpoint_suffix + U("/"), account.table_endpoint().secondary_uri().to_string());
    }

    TEST_FIXTURE(test_base, cloud_storage_account_string_roundtrip)
    {
        check_string_roundtrip(U("UseDevelopmentStorage=true"));
        check_string_roundtrip(U("DevelopmentStorageProxyUri=http://ipv4.fiddler/;UseDevelopmentStorage=true"));
        check_string_roundtrip(U("DefaultEndpointsProtocol=http;AccountName=test;AccountKey=abc="));
        check_string_roundtrip(U("DefaultEndpointsProtocol=http;EndpointSuffix=test.suffix;AccountName=test;AccountKey=abc="));
        check_string_roundtrip(U("DefaultEndpointsProtocol=http;EndpointSuffix=test.suffix;QueueEndpoint=https://alternate.queue.endpoint/;AccountName=test;AccountKey=abc="));
        check_string_roundtrip(U("DefaultEndpointsProtocol=http;QueueEndpoint=https://alternate.queue.endpoint/;AccountName=test;AccountKey=abc="));
        check_string_roundtrip(U("BlobEndpoint=https://alternate.blob.endpoint/;AccountName=test;AccountKey=abc="));
        check_string_roundtrip(U("QueueEndpoint=https://alternate.queue.endpoint/;AccountName=test;AccountKey=abc="));
        check_string_roundtrip(U("TableEndpoint=https://alternate.table.endpoint/;AccountName=test;AccountKey=abc="));
        check_string_roundtrip(U("BlobEndpoint=https://alternate.blob.endpoint/"));
        check_string_roundtrip(U("QueueEndpoint=https://alternate.queue.endpoint/"));
        check_string_roundtrip(U("TableEndpoint=https://alternate.table.endpoint/"));
        check_string_roundtrip(U("BlobEndpoint=https://alternate.blob.endpoint/;SharedAccessSignature=abc=def"));
        check_string_roundtrip(U("QueueEndpoint=https://alternate.queue.endpoint/;SharedAccessSignature=abc=def"));
        check_string_roundtrip(U("TableEndpoint=https://alternate.table.endpoint/;SharedAccessSignature=abc=def"));
    }

    TEST_FIXTURE(test_base, cloud_storage_account_string_empty_values)
    {
        auto account = azure::storage::cloud_storage_account::parse(U(";BlobEndpoint=http://blobs/;;AccountName=test;;AccountKey=abc=;"));
        CHECK_UTF8_EQUAL(U("BlobEndpoint=http://blobs/;AccountName=test;AccountKey=abc="), account.to_string(true));
    }

    TEST_FIXTURE(test_base, cloud_storage_account_clients)
    {
        azure::storage::storage_credentials creds(test_account_name, test_account_key);
        azure::storage::cloud_storage_account account(creds, false);

        auto blob_client = account.create_cloud_blob_client();
        CHECK_UTF8_EQUAL(account.blob_endpoint().primary_uri().to_string(), blob_client.base_uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(account.blob_endpoint().secondary_uri().to_string(), blob_client.base_uri().secondary_uri().to_string());

        auto queue_client = account.create_cloud_queue_client();
        CHECK_UTF8_EQUAL(account.queue_endpoint().primary_uri().to_string(), queue_client.base_uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(account.queue_endpoint().secondary_uri().to_string(), queue_client.base_uri().secondary_uri().to_string());

        auto table_client = account.create_cloud_table_client();
        CHECK_UTF8_EQUAL(account.table_endpoint().primary_uri().to_string(), table_client.base_uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(account.table_endpoint().secondary_uri().to_string(), table_client.base_uri().secondary_uri().to_string());
    }

    TEST_FIXTURE(test_base, cloud_storage_account_incorrect_devstore)
    {
        CHECK_THROW(azure::storage::cloud_storage_account::parse(U("UseDevelopmentStorage=false")), std::invalid_argument);
        CHECK_THROW(azure::storage::cloud_storage_account::parse(U("UseDevelopmentStorage=true;AccountName=devstoreaccount1")), std::invalid_argument);
        CHECK_THROW(azure::storage::cloud_storage_account::parse(U("UseDevelopmentStorage=true;BlobEndpoint=http://127.0.0.1:1000/devstoreaccount1")), std::invalid_argument);
    }

    TEST_FIXTURE(test_base, cloud_storage_account_blob_endpoint)
    {
        auto account = azure::storage::cloud_storage_account::parse(U("DefaultEndpointsProtocol=http;BlobEndpoint=http://customdomain.com/;AccountName=asdf;AccountKey=abc="));
        CHECK_UTF8_EQUAL(U("http://customdomain.com/"), account.blob_endpoint().primary_uri().to_string());
        CHECK(account.blob_endpoint().secondary_uri().is_empty());
    }

    TEST_FIXTURE(test_base, cloud_storage_account_devstore_proxy)
    {
        auto account = azure::storage::cloud_storage_account::parse(U("UseDevelopmentStorage=true;DevelopmentStorageProxyUri=http://ipv4.fiddler"));
        CHECK_UTF8_EQUAL(U("http://ipv4.fiddler:10000/devstoreaccount1"), account.blob_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://ipv4.fiddler:10001/devstoreaccount1"), account.queue_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://ipv4.fiddler:10002/devstoreaccount1"), account.table_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://ipv4.fiddler:10000/devstoreaccount1-secondary"), account.blob_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://ipv4.fiddler:10001/devstoreaccount1-secondary"), account.queue_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(U("http://ipv4.fiddler:10002/devstoreaccount1-secondary"), account.table_endpoint().secondary_uri().to_string());
    }
}
