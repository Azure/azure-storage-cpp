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
#include "was/file.h"
#include "wascore/constants.h"

const utility::string_t test_uri(_XPLATSTR("http://test/abc"));
const utility::string_t token(_XPLATSTR("sp=abcde&sig=1"));
const utility::string_t token_with_api_version(utility::string_t(azure::storage::protocol::uri_query_sas_api_version) + _XPLATSTR("=") + azure::storage::protocol::header_value_storage_version + _XPLATSTR("&sig=1&sp=abcde"));
const utility::string_t test_account_name(_XPLATSTR("test"));
const utility::string_t test_account_key(_XPLATSTR("Fby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="));
const utility::string_t test_endpoint_suffix(_XPLATSTR("fake.endpoint.suffix"));

void check_credentials_equal(const azure::storage::storage_credentials& a, const azure::storage::storage_credentials& b)
{
    CHECK_EQUAL(a.is_anonymous(), b.is_anonymous());
    CHECK_EQUAL(a.is_sas(), b.is_sas());
    CHECK_EQUAL(a.is_shared_key(), b.is_shared_key());
    CHECK_UTF8_EQUAL(a.account_name(), b.account_name());
    CHECK_UTF8_EQUAL(utility::conversions::to_base64(a.account_key()), utility::conversions::to_base64(b.account_key()));
}

void check_account_equal(azure::storage::cloud_storage_account& a, azure::storage::cloud_storage_account& b)
{
    CHECK_UTF8_EQUAL(a.blob_endpoint().primary_uri().to_string(), b.blob_endpoint().primary_uri().to_string());
    CHECK_UTF8_EQUAL(a.blob_endpoint().secondary_uri().to_string(), b.blob_endpoint().secondary_uri().to_string());
    CHECK_UTF8_EQUAL(a.queue_endpoint().primary_uri().to_string(), b.queue_endpoint().primary_uri().to_string());
    CHECK_UTF8_EQUAL(a.queue_endpoint().secondary_uri().to_string(), b.queue_endpoint().secondary_uri().to_string());
    CHECK_UTF8_EQUAL(a.table_endpoint().primary_uri().to_string(), b.table_endpoint().primary_uri().to_string());
    CHECK_UTF8_EQUAL(a.table_endpoint().secondary_uri().to_string(), b.table_endpoint().secondary_uri().to_string());
    CHECK_UTF8_EQUAL(a.file_endpoint().primary_uri().to_string(), b.file_endpoint().primary_uri().to_string());
    CHECK_UTF8_EQUAL(a.file_endpoint().secondary_uri().to_string(), b.file_endpoint().secondary_uri().to_string());
    CHECK_UTF8_EQUAL(a.to_string(), b.to_string(false));
    CHECK_UTF8_EQUAL(a.to_string(true), b.to_string(true));
    check_credentials_equal(a.credentials(), b.credentials());
}

void check_string_roundtrip(const utility::string_t& connection_string)
{
    auto account = azure::storage::cloud_storage_account::parse(connection_string);
    CHECK_UTF8_EQUAL(connection_string, account.to_string(true));

    auto account2 = azure::storage::cloud_storage_account::parse(account.to_string(true));
    check_account_equal(account, account2);
}

void check_account_sas_permission_blob(azure::storage::cloud_storage_account account, azure::storage::account_shared_access_policy policy)
{
    if ((policy.service_type() & azure::storage::account_shared_access_policy::service_types::blob) != azure::storage::account_shared_access_policy::service_types::blob)
    {
        return;
    }

    auto sas_token = account.get_shared_access_signature(policy);
    azure::storage::storage_credentials sas_cred(sas_token);
    azure::storage::cloud_blob_client sas_blob_client(account.blob_endpoint(), sas_cred);
    auto blob_client = account.create_cloud_blob_client();

    auto container_name = _XPLATSTR("c") + test_base::get_random_string();
    auto container = blob_client.get_container_reference(container_name);
    auto sas_container = sas_blob_client.get_container_reference(container_name);
    auto blob_name = _XPLATSTR("b") + test_base::get_random_string();
    auto blob = container.get_page_blob_reference(blob_name);
    auto sas_blob = sas_container.get_page_blob_reference(blob_name);

    auto permission = policy.permission();
    if ((((permission & azure::storage::account_shared_access_policy::permissions::create) == azure::storage::account_shared_access_policy::permissions::create)
        || ((permission & azure::storage::account_shared_access_policy::permissions::write) == azure::storage::account_shared_access_policy::permissions::write))
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::container) == azure::storage::account_shared_access_policy::resource_types::container))
    {
        sas_container.create();
    }
    else
    {
        CHECK_THROW(sas_container.create(), azure::storage::storage_exception);
        container.create();
    }
    CHECK(container.exists());

    if (((permission & azure::storage::account_shared_access_policy::permissions::list) == azure::storage::account_shared_access_policy::permissions::list)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::service) == azure::storage::account_shared_access_policy::resource_types::service))
    {
        CHECK_UTF8_EQUAL(container_name, sas_blob_client.list_containers(container_name)->name());
    }
    else
    {
        CHECK_THROW(sas_blob_client.list_containers(container_name), azure::storage::storage_exception);
    }

    size_t text_size = 512;
    if ((((permission & azure::storage::account_shared_access_policy::permissions::create) == azure::storage::account_shared_access_policy::permissions::create)
        || ((permission & azure::storage::account_shared_access_policy::permissions::write) == azure::storage::account_shared_access_policy::permissions::write))
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::object) == azure::storage::account_shared_access_policy::resource_types::object))
    {
        sas_blob.create(text_size);
    }
    else
    {
        CHECK_THROW(sas_blob.create(text_size), azure::storage::storage_exception);
        blob.create(text_size);
    }
    CHECK(blob.exists());

    utility::string_t text = test_base::get_random_string(text_size);
    auto utf8_body = utility::conversions::to_utf8string(text);
    if (((permission & azure::storage::account_shared_access_policy::permissions::write) == azure::storage::account_shared_access_policy::permissions::write)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::object) == azure::storage::account_shared_access_policy::resource_types::object))
    {
        sas_blob.upload_pages(concurrency::streams::bytestream::open_istream(utf8_body), 0, utility::string_t());
    }
    else
    {
        CHECK_THROW(sas_blob.upload_pages(concurrency::streams::bytestream::open_istream(utf8_body), 0, utility::string_t()), azure::storage::storage_exception);
        blob.upload_pages(concurrency::streams::bytestream::open_istream(utf8_body), 0, utility::string_t());
    }

    concurrency::streams::container_buffer<std::vector<uint8_t>> buffer;
    if (((permission & azure::storage::account_shared_access_policy::permissions::read) == azure::storage::account_shared_access_policy::permissions::read)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::object) == azure::storage::account_shared_access_policy::resource_types::object))
    {
        sas_blob.download_to_stream(buffer.create_ostream());
        std::string actual(reinterpret_cast<char*>(buffer.collection().data()), static_cast<unsigned int>(buffer.size()));
        CHECK_UTF8_EQUAL(text, actual);
    }
    else
    {
        CHECK_THROW(sas_blob.download_to_stream(buffer.create_ostream()), azure::storage::storage_exception);
    }

    if (((permission & azure::storage::account_shared_access_policy::permissions::del) == azure::storage::account_shared_access_policy::permissions::del)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::object) == azure::storage::account_shared_access_policy::resource_types::object))
    {
        sas_blob.delete_blob();
    }
    else
    {
        CHECK_THROW(sas_blob.delete_blob(), azure::storage::storage_exception);
        blob.delete_blob();
    }

    if (((permission & azure::storage::account_shared_access_policy::permissions::del) == azure::storage::account_shared_access_policy::permissions::del)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::container) == azure::storage::account_shared_access_policy::resource_types::container))
    {
        sas_container.delete_container();
    }
    else
    {
        CHECK_THROW(sas_container.delete_container(), azure::storage::storage_exception);
        container.delete_container();
    }
}

void check_account_sas_permission_queue(azure::storage::cloud_storage_account account, azure::storage::account_shared_access_policy policy)
{
    if ((policy.service_type() & azure::storage::account_shared_access_policy::service_types::queue) != azure::storage::account_shared_access_policy::service_types::queue)
    {
        return;
    }

    auto sas_token = account.get_shared_access_signature(policy);
    azure::storage::storage_credentials sas_cred(sas_token);
    azure::storage::cloud_queue_client sas_queue_client(account.queue_endpoint(), sas_cred);
    auto queue_client = account.create_cloud_queue_client();

    auto queue_name = _XPLATSTR("q") + test_base::get_random_string();
    auto queue = queue_client.get_queue_reference(queue_name);
    auto sas_queue = sas_queue_client.get_queue_reference(queue_name);

    auto permission = policy.permission();
    if ((((permission & azure::storage::account_shared_access_policy::permissions::create) == azure::storage::account_shared_access_policy::permissions::create)
        || ((permission & azure::storage::account_shared_access_policy::permissions::write) == azure::storage::account_shared_access_policy::permissions::write))
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::container) == azure::storage::account_shared_access_policy::resource_types::container))
    {
        sas_queue.create();
    }
    else
    {
        CHECK_THROW(sas_queue.create(), azure::storage::storage_exception);
        queue.create();
    }
    CHECK(queue.exists());

    if (((permission & azure::storage::account_shared_access_policy::permissions::list) == azure::storage::account_shared_access_policy::permissions::list)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::service) == azure::storage::account_shared_access_policy::resource_types::service))
    {
        CHECK_UTF8_EQUAL(queue_name, sas_queue_client.list_queues(queue_name)->name());
    }
    else
    {
        CHECK_THROW(sas_queue_client.list_queues(queue_name), azure::storage::storage_exception);
    }

    utility::string_t text(_XPLATSTR("hello, world"));
    azure::storage::cloud_queue_message message(text);
    if (((permission & azure::storage::account_shared_access_policy::permissions::add) == azure::storage::account_shared_access_policy::permissions::add)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::object) == azure::storage::account_shared_access_policy::resource_types::object))
    {
        sas_queue.add_message(message);
    }
    else
    {
        CHECK_THROW(sas_queue.add_message(message), azure::storage::storage_exception);
        queue.add_message(message);
    }

    if (((permission & azure::storage::account_shared_access_policy::permissions::process) == azure::storage::account_shared_access_policy::permissions::process)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::object) == azure::storage::account_shared_access_policy::resource_types::object))
    {
        CHECK_UTF8_EQUAL(text, sas_queue.get_message().content_as_string());
    }
    else
    {
        CHECK_THROW(sas_queue.get_message(), azure::storage::storage_exception);
    }

    if (((permission & azure::storage::account_shared_access_policy::permissions::del) == azure::storage::account_shared_access_policy::permissions::del)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::object) == azure::storage::account_shared_access_policy::resource_types::object))
    {
        sas_queue.clear();
    }
    else
    {
        CHECK_THROW(sas_queue.clear(), azure::storage::storage_exception);
        queue.clear();
    }

    if (((permission & azure::storage::account_shared_access_policy::permissions::del) == azure::storage::account_shared_access_policy::permissions::del)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::container) == azure::storage::account_shared_access_policy::resource_types::container))
    {
        sas_queue.delete_queue();
    }
    else
    {
        CHECK_THROW(sas_queue.delete_queue(), azure::storage::storage_exception);
        queue.delete_queue();
    }
}

void check_account_sas_permission_table(azure::storage::cloud_storage_account account, azure::storage::account_shared_access_policy policy)
{
    if ((policy.service_type() & azure::storage::account_shared_access_policy::service_types::table) != azure::storage::account_shared_access_policy::service_types::table)
    {
        return;
    }

    auto sas_token = account.get_shared_access_signature(policy);
    azure::storage::storage_credentials sas_cred(sas_token);
    azure::storage::cloud_table_client sas_table_client(account.table_endpoint(), sas_cred);
    auto table_client = account.create_cloud_table_client();

    auto table_name = _XPLATSTR("t") + test_base::get_random_string();
    auto table = table_client.get_table_reference(table_name);
    auto sas_table = sas_table_client.get_table_reference(table_name);

    auto permission = policy.permission();
    if ((((permission & azure::storage::account_shared_access_policy::permissions::create) == azure::storage::account_shared_access_policy::permissions::create)
        || ((permission & azure::storage::account_shared_access_policy::permissions::write) == azure::storage::account_shared_access_policy::permissions::write))
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::container) == azure::storage::account_shared_access_policy::resource_types::container))
    {
        sas_table.create();
    }
    else
    {
        CHECK_THROW(sas_table.create(), azure::storage::storage_exception);
        table.create();
    }
    CHECK(table.exists());

    if (((permission & azure::storage::account_shared_access_policy::permissions::list) == azure::storage::account_shared_access_policy::permissions::list)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::container) == azure::storage::account_shared_access_policy::resource_types::container))
    {
        CHECK_UTF8_EQUAL(table_name, sas_table_client.list_tables(table_name)->name());
    }
    else
    {
        CHECK_THROW(sas_table_client.list_tables(table_name), azure::storage::storage_exception);
    }

    auto op = azure::storage::table_operation::insert_entity(azure::storage::table_entity(_XPLATSTR("pk"), _XPLATSTR("rk")));
    if (((permission & azure::storage::account_shared_access_policy::permissions::add) == azure::storage::account_shared_access_policy::permissions::add)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::object) == azure::storage::account_shared_access_policy::resource_types::object))
    {
        sas_table.execute(op);
    }
    else
    {
        CHECK_THROW(sas_table.execute(op), azure::storage::storage_exception);
        table.execute(op);
    }

    azure::storage::table_query q;
    q.set_filter_string(azure::storage::table_query::combine_filter_conditions(azure::storage::table_query::generate_filter_condition(_XPLATSTR("PartitionKey"), azure::storage::query_comparison_operator::equal, _XPLATSTR("pk")),
        azure::storage::query_logical_operator::op_and,
        azure::storage::table_query::generate_filter_condition(_XPLATSTR("RowKey"), azure::storage::query_comparison_operator::equal, _XPLATSTR("rk"))));
    if (((permission & azure::storage::account_shared_access_policy::permissions::read) == azure::storage::account_shared_access_policy::permissions::read)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::object) == azure::storage::account_shared_access_policy::resource_types::object))
    {
        sas_table.execute_query(q);
    }
    else
    {
        CHECK_THROW(sas_table.execute_query(q), azure::storage::storage_exception);
        table.execute_query(q);
    }

    if (((permission & azure::storage::account_shared_access_policy::permissions::del) == azure::storage::account_shared_access_policy::permissions::del)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::container) == azure::storage::account_shared_access_policy::resource_types::container))
    {
        sas_table.delete_table();
    }
    else
    {
        CHECK_THROW(sas_table.delete_table(), azure::storage::storage_exception);
        table.delete_table();
    }
}

void check_account_sas_permission_file(azure::storage::cloud_storage_account account, azure::storage::account_shared_access_policy policy)
{
    if ((policy.service_type() & azure::storage::account_shared_access_policy::service_types::file) != azure::storage::account_shared_access_policy::service_types::file)
    {
        return;
    }

    auto sas_token = account.get_shared_access_signature(policy);
    azure::storage::storage_credentials sas_cred(sas_token);
    azure::storage::cloud_file_client sas_file_client(account.file_endpoint(), sas_cred);
    auto file_client = account.create_cloud_file_client();

    auto share_name = _XPLATSTR("s") + test_base::get_random_string();
    auto share = file_client.get_share_reference(share_name);
    auto sas_share = sas_file_client.get_share_reference(share_name);
    auto file_name = _XPLATSTR("f") + test_base::get_random_string();
    auto file = share.get_root_directory_reference().get_file_reference(file_name);
    auto sas_file = sas_share.get_root_directory_reference().get_file_reference(file_name);

    auto permission = policy.permission();
    if ((((permission & azure::storage::account_shared_access_policy::permissions::create) == azure::storage::account_shared_access_policy::permissions::create)
        || ((permission & azure::storage::account_shared_access_policy::permissions::write) == azure::storage::account_shared_access_policy::permissions::write))
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::container) == azure::storage::account_shared_access_policy::resource_types::container))
    {
        sas_share.create();
    }
    else
    {
        CHECK_THROW(sas_share.create(), azure::storage::storage_exception);
        share.create();
    }
    CHECK(share.exists());

    if (((permission & azure::storage::account_shared_access_policy::permissions::list) == azure::storage::account_shared_access_policy::permissions::list)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::service) == azure::storage::account_shared_access_policy::resource_types::service))
    {
        CHECK_UTF8_EQUAL(share_name, sas_file_client.list_shares(share_name)->name());
    }
    else
    {
        CHECK_THROW(sas_file_client.list_shares(share_name), azure::storage::storage_exception);
    }

    size_t text_size = 512;
    if ((((permission & azure::storage::account_shared_access_policy::permissions::create) == azure::storage::account_shared_access_policy::permissions::create)
        || ((permission & azure::storage::account_shared_access_policy::permissions::write) == azure::storage::account_shared_access_policy::permissions::write))
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::object) == azure::storage::account_shared_access_policy::resource_types::object))
    {
        sas_file.create(text_size);
    }
    else
    {
        CHECK_THROW(sas_file.create(text_size), azure::storage::storage_exception);
        file.create(text_size);
    }
    CHECK(file.exists());

    utility::string_t text = test_base::get_random_string(text_size);
    auto utf8_body = utility::conversions::to_utf8string(text);
    if (((permission & azure::storage::account_shared_access_policy::permissions::write) == azure::storage::account_shared_access_policy::permissions::write)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::object) == azure::storage::account_shared_access_policy::resource_types::object))
    {
        sas_file.write_range(concurrency::streams::bytestream::open_istream(utf8_body), 0, utility::string_t());
    }
    else
    {
        CHECK_THROW(sas_file.write_range(concurrency::streams::bytestream::open_istream(utf8_body), 0, utility::string_t()), azure::storage::storage_exception);
        file.write_range(concurrency::streams::bytestream::open_istream(utf8_body), 0, utility::string_t());
    }

    concurrency::streams::container_buffer<std::vector<uint8_t>> buffer;
    if (((permission & azure::storage::account_shared_access_policy::permissions::read) == azure::storage::account_shared_access_policy::permissions::read)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::object) == azure::storage::account_shared_access_policy::resource_types::object))
    {
        sas_file.download_to_stream(buffer.create_ostream());
        std::string actual(reinterpret_cast<char*>(buffer.collection().data()), static_cast<unsigned int>(buffer.size()));
        CHECK_UTF8_EQUAL(text, actual);
    }
    else
    {
        CHECK_THROW(sas_file.download_to_stream(buffer.create_ostream()), azure::storage::storage_exception);
    }

    if (((permission & azure::storage::account_shared_access_policy::permissions::del) == azure::storage::account_shared_access_policy::permissions::del)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::object) == azure::storage::account_shared_access_policy::resource_types::object))
    {
        sas_file.delete_file();
    }
    else
    {
        CHECK_THROW(sas_file.delete_file(), azure::storage::storage_exception);
        file.delete_file();
    }

    if (((permission & azure::storage::account_shared_access_policy::permissions::del) == azure::storage::account_shared_access_policy::permissions::del)
        && ((policy.resource_type() & azure::storage::account_shared_access_policy::resource_types::container) == azure::storage::account_shared_access_policy::resource_types::container))
    {
        sas_share.delete_share();
    }
    else
    {
        CHECK_THROW(sas_share.delete_share(), azure::storage::storage_exception);
        share.delete_share();
    }
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
            CHECK_UTF8_EQUAL(test_uri + _XPLATSTR("?") + token_with_api_version, creds.transform_uri(uri).to_string());
        }

        {
            azure::storage::storage_credentials creds(_XPLATSTR("?") + token);

            CHECK_UTF8_EQUAL(token, creds.sas_token());
            CHECK(creds.account_name().empty());
            CHECK(creds.account_key().empty());
            CHECK(!creds.is_anonymous());
            CHECK(creds.is_sas());
            CHECK(!creds.is_shared_key());

            web::http::uri uri(test_uri);
            CHECK_UTF8_EQUAL(test_uri + _XPLATSTR("?") + token_with_api_version, creds.transform_uri(uri).to_string());
        }
    }

    TEST_FIXTURE(test_base, storage_credentials_empty_key)
    {
        const utility::string_t defaults_connection_string(_XPLATSTR("DefaultEndpointsProtocol=https;AccountName=") + test_account_name + _XPLATSTR(";AccountKey="));

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

    TEST_FIXTURE(test_base, storage_credentials_move_constructor)
    {
        azure::storage::storage_credentials creds(test_account_name, test_account_key);
        azure::storage::storage_credentials creds2 = std::move(creds);

        CHECK(creds2.sas_token().empty());
        CHECK_UTF8_EQUAL(test_account_name, creds2.account_name());
        CHECK_EQUAL(false, creds2.is_anonymous());
        CHECK_EQUAL(false, creds2.is_sas());
        CHECK_EQUAL(true, creds2.is_shared_key());
    }

    TEST_FIXTURE(test_base, cloud_storage_account_devstore)
    {
        auto account = azure::storage::cloud_storage_account::development_storage_account();
        CHECK_UTF8_EQUAL(_XPLATSTR("http://127.0.0.1:10000/devstoreaccount1"), account.blob_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://127.0.0.1:10001/devstoreaccount1"), account.queue_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://127.0.0.1:10002/devstoreaccount1"), account.table_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://127.0.0.1:10003/devstoreaccount1"), account.file_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://127.0.0.1:10000/devstoreaccount1-secondary"), account.blob_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://127.0.0.1:10001/devstoreaccount1-secondary"), account.queue_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://127.0.0.1:10002/devstoreaccount1-secondary"), account.table_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://127.0.0.1:10003/devstoreaccount1-secondary"), account.file_endpoint().secondary_uri().to_string());
    }

    TEST_FIXTURE(test_base, cloud_storage_account_default_http)
    {
        azure::storage::storage_credentials creds(test_account_name, test_account_key);
        azure::storage::cloud_storage_account account(creds, false);

        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR(".blob.core.windows.net/"), account.blob_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR(".queue.core.windows.net/"), account.queue_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR(".table.core.windows.net/"), account.table_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR(".file.core.windows.net/"), account.file_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR("-secondary.blob.core.windows.net/"), account.blob_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR("-secondary.queue.core.windows.net/"), account.queue_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR("-secondary.table.core.windows.net/"), account.table_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR("-secondary.file.core.windows.net/"), account.file_endpoint().secondary_uri().to_string());

        auto account2 = azure::storage::cloud_storage_account::parse(account.to_string(true));
        check_account_equal(account, account2);
    }

    TEST_FIXTURE(test_base, cloud_storage_account_default_https)
    {
        azure::storage::storage_credentials creds(test_account_name, test_account_key);
        azure::storage::cloud_storage_account account(creds, true);

        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR(".blob.core.windows.net/"), account.blob_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR(".queue.core.windows.net/"), account.queue_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR(".table.core.windows.net/"), account.table_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR(".file.core.windows.net/"), account.file_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR("-secondary.blob.core.windows.net/"), account.blob_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR("-secondary.queue.core.windows.net/"), account.queue_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR("-secondary.table.core.windows.net/"), account.table_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR("-secondary.file.core.windows.net/"), account.file_endpoint().secondary_uri().to_string());

        auto account2 = azure::storage::cloud_storage_account::parse(account.to_string(true));
        check_account_equal(account, account2);
    }

    TEST_FIXTURE(test_base, cloud_storage_account_endpoint_suffix_http)
    {
        utility::ostringstream_t str;
        str << _XPLATSTR("DefaultEndpointsProtocol=http;AccountName=") << test_account_name << _XPLATSTR(";AccountKey=") << test_account_key << _XPLATSTR(";EndpointSuffix=") << test_endpoint_suffix;
        auto account = azure::storage::cloud_storage_account::parse(str.str());

        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR(".blob.") + test_endpoint_suffix + _XPLATSTR("/"), account.blob_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR(".queue.") + test_endpoint_suffix + _XPLATSTR("/"), account.queue_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR(".table.") + test_endpoint_suffix + _XPLATSTR("/"), account.table_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR(".file.") + test_endpoint_suffix + _XPLATSTR("/"), account.file_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR("-secondary.blob.") + test_endpoint_suffix + _XPLATSTR("/"), account.blob_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR("-secondary.queue.") + test_endpoint_suffix + _XPLATSTR("/"), account.queue_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR("-secondary.table.") + test_endpoint_suffix + _XPLATSTR("/"), account.table_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR("-secondary.file.") + test_endpoint_suffix + _XPLATSTR("/"), account.file_endpoint().secondary_uri().to_string());

        azure::storage::storage_credentials creds(test_account_name, test_account_key);
        azure::storage::cloud_storage_account account2(creds, test_endpoint_suffix, false);
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR(".blob.") + test_endpoint_suffix + _XPLATSTR("/"), account2.blob_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR(".queue.") + test_endpoint_suffix + _XPLATSTR("/"), account2.queue_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR(".table.") + test_endpoint_suffix + _XPLATSTR("/"), account2.table_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR(".file.") + test_endpoint_suffix + _XPLATSTR("/"), account2.file_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR("-secondary.blob.") + test_endpoint_suffix + _XPLATSTR("/"), account2.blob_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR("-secondary.queue.") + test_endpoint_suffix + _XPLATSTR("/"), account2.queue_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR("-secondary.table.") + test_endpoint_suffix + _XPLATSTR("/"), account2.table_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://") + test_account_name + _XPLATSTR("-secondary.file.") + test_endpoint_suffix + _XPLATSTR("/"), account2.file_endpoint().secondary_uri().to_string());
    }

    TEST_FIXTURE(test_base, cloud_storage_account_endpoint_suffix_https)
    {
        utility::ostringstream_t str;
        str << _XPLATSTR("DefaultEndpointsProtocol=https;AccountName=") << test_account_name << _XPLATSTR(";AccountKey=") << test_account_key << _XPLATSTR(";EndpointSuffix=") << test_endpoint_suffix;
        auto account = azure::storage::cloud_storage_account::parse(str.str());

        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR(".blob.") + test_endpoint_suffix + _XPLATSTR("/"), account.blob_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR(".queue.") + test_endpoint_suffix + _XPLATSTR("/"), account.queue_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR(".table.") + test_endpoint_suffix + _XPLATSTR("/"), account.table_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR(".file.") + test_endpoint_suffix + _XPLATSTR("/"), account.file_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR("-secondary.blob.") + test_endpoint_suffix + _XPLATSTR("/"), account.blob_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR("-secondary.queue.") + test_endpoint_suffix + _XPLATSTR("/"), account.queue_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR("-secondary.table.") + test_endpoint_suffix + _XPLATSTR("/"), account.table_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR("-secondary.file.") + test_endpoint_suffix + _XPLATSTR("/"), account.file_endpoint().secondary_uri().to_string());

        azure::storage::storage_credentials creds(test_account_name, test_account_key);
        azure::storage::cloud_storage_account account2(creds, test_endpoint_suffix, true);
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR(".blob.") + test_endpoint_suffix + _XPLATSTR("/"), account2.blob_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR(".queue.") + test_endpoint_suffix + _XPLATSTR("/"), account2.queue_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR(".table.") + test_endpoint_suffix + _XPLATSTR("/"), account2.table_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR(".file.") + test_endpoint_suffix + _XPLATSTR("/"), account2.file_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR("-secondary.blob.") + test_endpoint_suffix + _XPLATSTR("/"), account2.blob_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR("-secondary.queue.") + test_endpoint_suffix + _XPLATSTR("/"), account2.queue_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR("-secondary.table.") + test_endpoint_suffix + _XPLATSTR("/"), account2.table_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("https://") + test_account_name + _XPLATSTR("-secondary.file.") + test_endpoint_suffix + _XPLATSTR("/"), account2.file_endpoint().secondary_uri().to_string());
    }

    TEST_FIXTURE(test_base, cloud_storage_account_string_roundtrip)
    {
        check_string_roundtrip(_XPLATSTR("UseDevelopmentStorage=true"));
        check_string_roundtrip(_XPLATSTR("DevelopmentStorageProxyUri=http://ipv4.fiddler/;UseDevelopmentStorage=true"));
        check_string_roundtrip(_XPLATSTR("DefaultEndpointsProtocol=http;AccountName=test;AccountKey=abc="));
        check_string_roundtrip(_XPLATSTR("DefaultEndpointsProtocol=http;EndpointSuffix=test.suffix;AccountName=test;AccountKey=abc="));
        check_string_roundtrip(_XPLATSTR("DefaultEndpointsProtocol=http;EndpointSuffix=test.suffix;QueueEndpoint=https://alternate.queue.endpoint/;AccountName=test;AccountKey=abc="));
        check_string_roundtrip(_XPLATSTR("DefaultEndpointsProtocol=http;QueueEndpoint=https://alternate.queue.endpoint/;AccountName=test;AccountKey=abc="));
        check_string_roundtrip(_XPLATSTR("BlobEndpoint=https://alternate.blob.endpoint/;AccountName=test;AccountKey=abc="));
        check_string_roundtrip(_XPLATSTR("QueueEndpoint=https://alternate.queue.endpoint/;AccountName=test;AccountKey=abc="));
        check_string_roundtrip(_XPLATSTR("TableEndpoint=https://alternate.table.endpoint/;AccountName=test;AccountKey=abc="));
        check_string_roundtrip(_XPLATSTR("FileEndpoint=https://alternate.file.endpoint/;AccountName=test;AccountKey=abc="));
        check_string_roundtrip(_XPLATSTR("BlobEndpoint=https://alternate.blob.endpoint/"));
        check_string_roundtrip(_XPLATSTR("QueueEndpoint=https://alternate.queue.endpoint/"));
        check_string_roundtrip(_XPLATSTR("TableEndpoint=https://alternate.table.endpoint/"));
        check_string_roundtrip(_XPLATSTR("FileEndpoint=https://alternate.file.endpoint/"));
        check_string_roundtrip(_XPLATSTR("BlobEndpoint=https://alternate.blob.endpoint/;SharedAccessSignature=abc=def"));
        check_string_roundtrip(_XPLATSTR("QueueEndpoint=https://alternate.queue.endpoint/;SharedAccessSignature=abc=def"));
        check_string_roundtrip(_XPLATSTR("TableEndpoint=https://alternate.table.endpoint/;SharedAccessSignature=abc=def"));
        check_string_roundtrip(_XPLATSTR("FileEndpoint=https://alternate.file.endpoint/;SharedAccessSignature=abc=def"));
    }

    TEST_FIXTURE(test_base, cloud_storage_account_string_empty_values)
    {
        auto account = azure::storage::cloud_storage_account::parse(_XPLATSTR(";BlobEndpoint=http://blobs/;;AccountName=test;;AccountKey=abc=;"));
        CHECK_UTF8_EQUAL(_XPLATSTR("BlobEndpoint=http://blobs/;AccountName=test;AccountKey=abc="), account.to_string(true));
    }

    TEST_FIXTURE(test_base, cloud_storage_account_clients)
    {
        azure::storage::storage_credentials creds(test_account_name, test_account_key);
        azure::storage::cloud_storage_account account(creds, false);

        auto blob_client = account.create_cloud_blob_client();
        CHECK_UTF8_EQUAL(account.blob_endpoint().primary_uri().to_string(), blob_client.base_uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(account.blob_endpoint().secondary_uri().to_string(), blob_client.base_uri().secondary_uri().to_string());

        azure::storage::blob_request_options blob_options;
        blob_options.set_parallelism_factor(10);
        blob_options.set_single_blob_upload_threshold_in_bytes(4 * 1024 * 1024);
        auto blob_client2 = account.create_cloud_blob_client(blob_options);
        CHECK_UTF8_EQUAL(account.blob_endpoint().primary_uri().to_string(), blob_client2.base_uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(account.blob_endpoint().secondary_uri().to_string(), blob_client2.base_uri().secondary_uri().to_string());
        CHECK_EQUAL(10, blob_client2.default_request_options().parallelism_factor());
        CHECK_EQUAL(4 * 1024 * 1024, blob_client2.default_request_options().single_blob_upload_threshold_in_bytes());

        auto queue_client = account.create_cloud_queue_client();
        CHECK_UTF8_EQUAL(account.queue_endpoint().primary_uri().to_string(), queue_client.base_uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(account.queue_endpoint().secondary_uri().to_string(), queue_client.base_uri().secondary_uri().to_string());

        azure::storage::queue_request_options queue_options;
        queue_options.set_location_mode(azure::storage::location_mode::secondary_only);
        auto queue_client2 = account.create_cloud_queue_client(queue_options);
        CHECK_UTF8_EQUAL(account.queue_endpoint().primary_uri().to_string(), queue_client2.base_uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(account.queue_endpoint().secondary_uri().to_string(), queue_client2.base_uri().secondary_uri().to_string());
        CHECK(azure::storage::location_mode::secondary_only == queue_client2.default_request_options().location_mode());

        auto table_client = account.create_cloud_table_client();
        CHECK_UTF8_EQUAL(account.table_endpoint().primary_uri().to_string(), table_client.base_uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(account.table_endpoint().secondary_uri().to_string(), table_client.base_uri().secondary_uri().to_string());

        azure::storage::table_request_options table_options;
        table_options.set_payload_format(azure::storage::table_payload_format::json_full_metadata);
        auto table_client2 = account.create_cloud_table_client(table_options);
        CHECK_UTF8_EQUAL(account.table_endpoint().primary_uri().to_string(), table_client2.base_uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(account.table_endpoint().secondary_uri().to_string(), table_client2.base_uri().secondary_uri().to_string());
        CHECK(azure::storage::table_payload_format::json_full_metadata == table_client2.default_request_options().payload_format());

        auto file_client = account.create_cloud_file_client();
        CHECK_UTF8_EQUAL(account.file_endpoint().primary_uri().to_string(), file_client.base_uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(account.file_endpoint().secondary_uri().to_string(), file_client.base_uri().secondary_uri().to_string());

        azure::storage::file_request_options file_options;
        file_options.set_parallelism_factor(10);
        auto file_client2 = account.create_cloud_file_client(file_options);
        CHECK_UTF8_EQUAL(account.file_endpoint().primary_uri().to_string(), file_client2.base_uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(account.file_endpoint().secondary_uri().to_string(), file_client2.base_uri().secondary_uri().to_string());
        CHECK_EQUAL(10, file_client2.default_request_options().parallelism_factor());
    }

    TEST_FIXTURE(test_base, cloud_storage_account_incorrect_devstore)
    {
        CHECK_THROW(azure::storage::cloud_storage_account::parse(_XPLATSTR("UseDevelopmentStorage=false")), std::invalid_argument);
        CHECK_THROW(azure::storage::cloud_storage_account::parse(_XPLATSTR("UseDevelopmentStorage=true;AccountName=devstoreaccount1")), std::invalid_argument);
        CHECK_THROW(azure::storage::cloud_storage_account::parse(_XPLATSTR("UseDevelopmentStorage=true;BlobEndpoint=http://127.0.0.1:1000/devstoreaccount1")), std::invalid_argument);
    }

    TEST_FIXTURE(test_base, cloud_storage_account_endpoints_in_connection_string)
    {
        const utility::string_t default_endpoint_suffix(_XPLATSTR("core.windows.net"));
        utility::string_t endpoints[] = {
            _XPLATSTR("http://customdomain.com/"),
            _XPLATSTR("http://customdomain2.com/"),
            _XPLATSTR("http://customdomain3.com/"),
            _XPLATSTR("http://customdomain4.com/") };

        utility::string_t scheme(_XPLATSTR("http"));
        utility::string_t account_name(_XPLATSTR("asdf"));
        utility::string_t account_key(_XPLATSTR("abc="));

        for (int i = 1; i < 16; i++) {
            utility::ostringstream_t str;
            str << _XPLATSTR("DefaultEndpointsProtocol=") << scheme << _XPLATSTR(";");
            if (i & 1) str << _XPLATSTR("BlobEndpoint=") << endpoints[0] << _XPLATSTR(";");
            if (i & 2) str << _XPLATSTR("QueueEndpoint=") << endpoints[1] << _XPLATSTR(";");
            if (i & 4) str << _XPLATSTR("TableEndpoint=") << endpoints[2] << _XPLATSTR(";");
            if (i & 8) str << _XPLATSTR("FileEndpoint=") << endpoints[3] << _XPLATSTR(";");
            str << _XPLATSTR("AccountName=") << account_name << _XPLATSTR(";");
            str << _XPLATSTR("AccountKey=") << account_key;

            auto account = azure::storage::cloud_storage_account::parse(str.str());
            if (i & 1) {
                CHECK_UTF8_EQUAL(endpoints[0], account.blob_endpoint().primary_uri().to_string());
                CHECK(account.blob_endpoint().secondary_uri().is_empty());
            }
            else {
                CHECK_UTF8_EQUAL(scheme + _XPLATSTR("://") + account_name + _XPLATSTR(".blob.") + default_endpoint_suffix + _XPLATSTR("/"), account.blob_endpoint().primary_uri().to_string());
                CHECK_UTF8_EQUAL(scheme + _XPLATSTR("://") + account_name + _XPLATSTR("-secondary.blob.") + default_endpoint_suffix + _XPLATSTR("/"), account.blob_endpoint().secondary_uri().to_string());
            }

            if (i & 2) {
                CHECK_UTF8_EQUAL(endpoints[1], account.queue_endpoint().primary_uri().to_string());
                CHECK(account.queue_endpoint().secondary_uri().is_empty());
            }
            else {
                CHECK_UTF8_EQUAL(scheme + _XPLATSTR("://") + account_name + _XPLATSTR(".queue.") + default_endpoint_suffix + _XPLATSTR("/"), account.queue_endpoint().primary_uri().to_string());
                CHECK_UTF8_EQUAL(scheme + _XPLATSTR("://") + account_name + _XPLATSTR("-secondary.queue.") + default_endpoint_suffix + _XPLATSTR("/"), account.queue_endpoint().secondary_uri().to_string());
            }

            if (i & 4) {
                CHECK_UTF8_EQUAL(endpoints[2], account.table_endpoint().primary_uri().to_string());
                CHECK(account.table_endpoint().secondary_uri().is_empty());
            }
            else {
                CHECK_UTF8_EQUAL(scheme + _XPLATSTR("://") + account_name + _XPLATSTR(".table.") + default_endpoint_suffix + _XPLATSTR("/"), account.table_endpoint().primary_uri().to_string());
                CHECK_UTF8_EQUAL(scheme + _XPLATSTR("://") + account_name + _XPLATSTR("-secondary.table.") + default_endpoint_suffix + _XPLATSTR("/"), account.table_endpoint().secondary_uri().to_string());
            }

            if (i & 8) {
                CHECK_UTF8_EQUAL(endpoints[3], account.file_endpoint().primary_uri().to_string());
                CHECK(account.file_endpoint().secondary_uri().is_empty());
            }
            else {
                CHECK_UTF8_EQUAL(scheme + _XPLATSTR("://") + account_name + _XPLATSTR(".file.") + default_endpoint_suffix + _XPLATSTR("/"), account.file_endpoint().primary_uri().to_string());
                CHECK_UTF8_EQUAL(scheme + _XPLATSTR("://") + account_name + _XPLATSTR("-secondary.file.") + default_endpoint_suffix + _XPLATSTR("/"), account.file_endpoint().secondary_uri().to_string());
            }
        }
    }

    TEST_FIXTURE(test_base, cloud_storage_account_endpoints_in_ctor)
    {
        const int COUNT = 2;
        azure::storage::storage_credentials creds(test_account_name, test_account_key);
        azure::storage::storage_uri blob_uris[] = { azure::storage::storage_uri(_XPLATSTR("http://customdomain.com/")), azure::storage::storage_uri() };
        azure::storage::storage_uri queue_uris[] = { azure::storage::storage_uri(_XPLATSTR("http://customdomain2.com/")), azure::storage::storage_uri() };
        azure::storage::storage_uri table_uris[] = { azure::storage::storage_uri(_XPLATSTR("http://customdomain3.com/")), azure::storage::storage_uri() };
        azure::storage::storage_uri file_uris[] = { azure::storage::storage_uri(_XPLATSTR("http://customdomain4.com/")), azure::storage::storage_uri() };

        for (int i = 0; i < COUNT; ++i) {
            for (int j = 0; j < COUNT; ++j) {
                for (int k = 0; k < COUNT; ++k) {
                    for (int l = 0; l < COUNT; ++l) {
                        azure::storage::cloud_storage_account account(creds, blob_uris[i], queue_uris[j], table_uris[k], file_uris[l]);
                        CHECK_UTF8_EQUAL(blob_uris[i].primary_uri().to_string(), account.blob_endpoint().primary_uri().to_string());
                        CHECK_UTF8_EQUAL(queue_uris[j].primary_uri().to_string(), account.queue_endpoint().primary_uri().to_string());
                        CHECK_UTF8_EQUAL(table_uris[k].primary_uri().to_string(), account.table_endpoint().primary_uri().to_string());
                        CHECK_UTF8_EQUAL(file_uris[l].primary_uri().to_string(), account.file_endpoint().primary_uri().to_string());

                        if (i == COUNT - 1 && j == COUNT - 1 && k == COUNT - 1 && l == COUNT -1)
                        {
                            // all endpoints are empty
                            CHECK_THROW(azure::storage::cloud_storage_account::parse(account.to_string(true)), std::invalid_argument);
                        }
                        else
                        {
                            azure::storage::cloud_storage_account account2 = azure::storage::cloud_storage_account::parse(account.to_string(true));
                            check_account_equal(account, account2);
                        }
                    }
                }
            }
        }
    }

    TEST_FIXTURE(test_base, cloud_storage_account_devstore_proxy)
    {
        auto account = azure::storage::cloud_storage_account::parse(_XPLATSTR("UseDevelopmentStorage=true;DevelopmentStorageProxyUri=http://ipv4.fiddler"));
        CHECK_UTF8_EQUAL(_XPLATSTR("http://ipv4.fiddler:10000/devstoreaccount1"), account.blob_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://ipv4.fiddler:10001/devstoreaccount1"), account.queue_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://ipv4.fiddler:10002/devstoreaccount1"), account.table_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://ipv4.fiddler:10003/devstoreaccount1"), account.file_endpoint().primary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://ipv4.fiddler:10000/devstoreaccount1-secondary"), account.blob_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://ipv4.fiddler:10001/devstoreaccount1-secondary"), account.queue_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://ipv4.fiddler:10002/devstoreaccount1-secondary"), account.table_endpoint().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(_XPLATSTR("http://ipv4.fiddler:10003/devstoreaccount1-secondary"), account.file_endpoint().secondary_uri().to_string());
    }

    TEST_FIXTURE(test_base, account_sas_permission)
    {
        auto account = test_config::instance().account();

        azure::storage::account_shared_access_policy policy;
        policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(90));
        policy.set_address_or_range(azure::storage::shared_access_policy::ip_address_or_range(_XPLATSTR("0.0.0.0"), _XPLATSTR("255.255.255.255")));
        policy.set_protocol(azure::storage::account_shared_access_policy::protocols::https_or_http);
        policy.set_service_type((azure::storage::account_shared_access_policy::service_types)0xF);
        policy.set_resource_type((azure::storage::account_shared_access_policy::resource_types)0x7);

        for (int i = 1; i < 0x100; i++)
        {
            policy.set_permissions((uint8_t)i);
            check_account_sas_permission_blob(account, policy);
            check_account_sas_permission_queue(account, policy);
            check_account_sas_permission_table(account, policy);
            check_account_sas_permission_file(account, policy);
        }
    }

    TEST_FIXTURE(test_base, account_sas_service_types)
    {
        auto account = test_config::instance().account();

        azure::storage::account_shared_access_policy policy;
        policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(90));
        policy.set_address_or_range(azure::storage::shared_access_policy::ip_address_or_range(_XPLATSTR("0.0.0.0"), _XPLATSTR("255.255.255.255")));
        policy.set_protocol(azure::storage::account_shared_access_policy::protocols::https_or_http);
        policy.set_permissions(0xFF);
        policy.set_resource_type((azure::storage::account_shared_access_policy::resource_types)0x7);

        for (int i = 1; i < 0x10; i++)
        {
            policy.set_service_type((azure::storage::account_shared_access_policy::service_types)i);
            check_account_sas_permission_blob(account, policy);
            check_account_sas_permission_queue(account, policy);
            check_account_sas_permission_table(account, policy);
            check_account_sas_permission_file(account, policy);
        }
    }

    TEST_FIXTURE(test_base, account_sas_resource_types)
    {
        auto account = test_config::instance().account();

        azure::storage::account_shared_access_policy policy;
        policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));
        policy.set_address_or_range(azure::storage::shared_access_policy::ip_address_or_range(_XPLATSTR("0.0.0.0"), _XPLATSTR("255.255.255.255")));
        policy.set_protocol(azure::storage::account_shared_access_policy::protocols::https_or_http);
        policy.set_permissions(0xFF);
        policy.set_service_type((azure::storage::account_shared_access_policy::service_types)0xF);
        
        for (int i = 1; i < 0x8; i++)
        {
            policy.set_resource_type((azure::storage::account_shared_access_policy::resource_types)i);
            check_account_sas_permission_blob(account, policy);
            check_account_sas_permission_queue(account, policy);
            check_account_sas_permission_table(account, policy);
            check_account_sas_permission_file(account, policy);
        }
    }

    TEST_FIXTURE(test_base, account_sas_expiry)
    {
        auto account = test_config::instance().account();

        azure::storage::account_shared_access_policy policy;
        policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_seconds(30));
        policy.set_address_or_range(azure::storage::shared_access_policy::ip_address_or_range(_XPLATSTR("0.0.0.0"), _XPLATSTR("255.255.255.255")));
        policy.set_protocol(azure::storage::account_shared_access_policy::protocols::https_or_http);
        policy.set_service_type((azure::storage::account_shared_access_policy::service_types)0xF);
        policy.set_resource_type((azure::storage::account_shared_access_policy::resource_types)0x7);
        policy.set_permissions(0xFF);

        auto sas_token = account.get_shared_access_signature(policy);
        azure::storage::storage_credentials sas_cred(sas_token);
        azure::storage::cloud_blob_client sas_blob_client(account.blob_endpoint(), sas_cred);
        sas_blob_client.list_containers(_XPLATSTR("prefix"));

        std::this_thread::sleep_for(std::chrono::seconds(40));
        CHECK_THROW(sas_blob_client.list_containers(_XPLATSTR("prefix")), azure::storage::storage_exception);
    }

    TEST_FIXTURE(test_base, account_sas_protocol)
    {
        auto account = test_config::instance().account();
        auto blob_host = account.blob_endpoint().primary_uri().host();
        auto blob_port = account.blob_endpoint().primary_uri().port();
        web::uri_builder blob_endpoint;
        blob_endpoint.set_scheme(_XPLATSTR("http"));
        blob_endpoint.set_host(blob_host);
        blob_endpoint.set_port(blob_port);
        
        azure::storage::account_shared_access_policy policy;
        policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_seconds(120));
        policy.set_address_or_range(azure::storage::shared_access_policy::ip_address_or_range(_XPLATSTR("0.0.0.0"), _XPLATSTR("255.255.255.255")));
        policy.set_protocol(azure::storage::account_shared_access_policy::protocols::https_or_http);
        policy.set_service_type((azure::storage::account_shared_access_policy::service_types)0xF);
        policy.set_resource_type((azure::storage::account_shared_access_policy::resource_types)0x7);
        policy.set_permissions(0xFF);

        auto sas_token = account.get_shared_access_signature(policy);
        azure::storage::storage_credentials sas_cred(sas_token);
        azure::storage::cloud_blob_client sas_blob_client(blob_endpoint.to_uri(), sas_cred);
        sas_blob_client.list_containers(_XPLATSTR("prefix"));

        policy.set_protocol(azure::storage::account_shared_access_policy::protocols::https_only);
        auto sas_token_https = account.get_shared_access_signature(policy);
        azure::storage::storage_credentials sas_cred_https(sas_token_https);
        azure::storage::cloud_blob_client sas_blob_client_https(blob_endpoint.to_uri(), sas_cred_https);
        CHECK_THROW(sas_blob_client_https.list_containers(_XPLATSTR("prefix")), azure::storage::storage_exception);
    }

    TEST_FIXTURE(test_base, account_sas_address)
    {
        auto account = test_config::instance().account();

        azure::storage::account_shared_access_policy policy;
        policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_seconds(60));
        policy.set_protocol(azure::storage::account_shared_access_policy::protocols::https_or_http);
        policy.set_service_type((azure::storage::account_shared_access_policy::service_types)0xF);
        policy.set_resource_type((azure::storage::account_shared_access_policy::resource_types)0x7);
        policy.set_permissions(0xFF);

        CHECK_THROW(policy.set_address_or_range(azure::storage::shared_access_policy::ip_address_or_range(_XPLATSTR("256.256.255.255"))), std::invalid_argument);
        CHECK_THROW(policy.set_address_or_range(azure::storage::shared_access_policy::ip_address_or_range(_XPLATSTR("::1"))), std::invalid_argument);
        CHECK_THROW(policy.set_address_or_range(azure::storage::shared_access_policy::ip_address_or_range(_XPLATSTR("ipv4.fiddler"))), std::invalid_argument);
        
        utility::string_t min_addr(_XPLATSTR("0.0.0.0"));
        utility::string_t max_addr(_XPLATSTR("255.0.0.0"));
        utility::string_t expected_range = min_addr + _XPLATSTR("-") + max_addr;
        policy.set_address_or_range(azure::storage::shared_access_policy::ip_address_or_range(min_addr, max_addr));
        CHECK_UTF8_EQUAL(expected_range, policy.address_or_range().to_string());
        policy.set_address_or_range(azure::storage::shared_access_policy::ip_address_or_range(max_addr, min_addr));
        CHECK_UTF8_EQUAL(expected_range, policy.address_or_range().to_string());

        policy.set_address_or_range(azure::storage::shared_access_policy::ip_address_or_range(_XPLATSTR("255.255.255.255")));
        auto sas_token = account.get_shared_access_signature(policy);
        azure::storage::storage_credentials sas_cred(sas_token);
        azure::storage::cloud_blob_client sas_blob_client(account.blob_endpoint(), sas_cred);
        azure::storage::operation_context op = m_context;
        CHECK_THROW(sas_blob_client.list_containers(_XPLATSTR("prefix"), azure::storage::container_listing_details::none, 0, azure::storage::blob_request_options(), op), azure::storage::storage_exception);
        auto error_details = op.request_results().back().extended_error().details();
        auto source_ip = error_details[_XPLATSTR("SourceIP")];

        policy.set_address_or_range(azure::storage::shared_access_policy::ip_address_or_range(source_ip));
        sas_token = account.get_shared_access_signature(policy);
        azure::storage::cloud_blob_client(account.blob_endpoint(), azure::storage::storage_credentials(sas_token)).list_containers(_XPLATSTR("prefix"));
    }
}
