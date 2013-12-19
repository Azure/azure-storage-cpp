// -----------------------------------------------------------------------------------------
// <copyright file="cloud_blob_test.cpp" company="Microsoft">
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
#include "blob_test_base.h"
#include "check_macros.h"

#include "cpprest/producerconsumerstream.h"

#pragma region Fixture

bool blob_test_base::wait_for_copy(wa::storage::cloud_blob& blob)
{
    do
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        blob.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
    } while (blob.copy_state().status() == wa::storage::copy_status::pending);

    return blob.copy_state().status() == wa::storage::copy_status::success;
}

wa::storage::operation_context blob_test_base::upload_and_download(wa::storage::cloud_blob& blob, size_t buffer_size, size_t buffer_offset, size_t blob_size, bool use_seekable_stream, const wa::storage::blob_request_options& options, size_t expected_request_count, bool expect_md5_header)
{
    wa::storage::operation_context context;
    print_client_request_id(context, U("upload/download"));

    utility::string_t md5_header;
    context.set_sending_request([&md5_header] (web::http::http_request& request, wa::storage::operation_context)
    {
        if (!request.headers().match(U("x-ms-blob-content-md5"), md5_header))
        {
            md5_header.clear();
        }
    });

    std::vector<uint8_t> buffer;
    buffer.resize(buffer_size);
    auto md5 = fill_buffer_and_get_md5(buffer, buffer_offset, blob_size == 0 ? buffer_size - buffer_offset : blob_size);

    concurrency::streams::istream stream;
    if (use_seekable_stream)
    {
        stream = concurrency::streams::bytestream::open_istream(buffer);
        stream.seek(buffer_offset);
    }
    else
    {
        concurrency::streams::producer_consumer_buffer<uint8_t> pcbuffer;
        pcbuffer.putn(buffer.data() + buffer_offset, buffer_size - buffer_offset);
        pcbuffer.close(std::ios_base::out);
        stream = pcbuffer.create_istream();
    }

    if (blob.type() == wa::storage::blob_type::block_blob)
    {
        wa::storage::cloud_block_blob block_blob(blob);
        if (blob_size == 0)
        {
            block_blob.upload_from_stream(stream, wa::storage::access_condition(), options, context);
        }
        else
        {
            block_blob.upload_from_stream(stream, blob_size, wa::storage::access_condition(), options, context);
        }
    }
    else if (blob.type() == wa::storage::blob_type::page_blob)
    {
        wa::storage::cloud_page_blob page_blob(blob);
        if (blob_size == 0)
        {
            page_blob.upload_from_stream(stream, wa::storage::access_condition(), options, context);
        }
        else
        {
            page_blob.upload_from_stream(stream, blob_size, wa::storage::access_condition(), options, context);
        }
    }

    CHECK_UTF8_EQUAL(expect_md5_header ? md5 : utility::string_t(), md5_header);
    CHECK_EQUAL(expected_request_count, context.request_results().size());

    wa::storage::blob_request_options download_options(options);
    download_options.set_use_transactional_md5(false);

    concurrency::streams::container_buffer<std::vector<uint8_t>> output_buffer;
    blob.download_to_stream(output_buffer.create_ostream(), wa::storage::access_condition(), download_options, context);
    CHECK_ARRAY_EQUAL(buffer.data() + buffer_offset, output_buffer.collection().data(), blob_size == 0 ? (buffer_size - buffer_offset) : blob_size);

    context.set_sending_request(std::function<void(web::http::http_request &, wa::storage::operation_context)>());
    return context;
}

void blob_test_base::check_access(const utility::string_t& sas_token, uint8_t permissions, const wa::storage::cloud_blob_shared_access_headers& headers, const wa::storage::cloud_blob& original_blob)
{
    wa::storage::storage_credentials credentials;
    if (!sas_token.empty())
    {
        credentials = wa::storage::storage_credentials(sas_token);
    }

    wa::storage::cloud_blob_container container(m_container.uri(), credentials);
    wa::storage::cloud_blob blob = container.get_blob_reference(original_blob.name());

    if (permissions & wa::storage::blob_shared_access_policy::permissions::list)
    {
        container.list_blobs_segmented(utility::string_t(), true, wa::storage::blob_listing_includes(), 0, wa::storage::blob_continuation_token(), wa::storage::blob_request_options(), m_context);
    }
    else
    {
        CHECK_THROW(container.list_blobs_segmented(utility::string_t(), true, wa::storage::blob_listing_includes(), 0, wa::storage::blob_continuation_token(), wa::storage::blob_request_options(), m_context), wa::storage::storage_exception);
    }

    if (permissions & wa::storage::blob_shared_access_policy::permissions::read)
    {
        blob.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);

        if (!headers.cache_control().empty())
        {
            CHECK_UTF8_EQUAL(headers.cache_control(), blob.properties().cache_control());
        }
        else
        {
            CHECK_UTF8_EQUAL(original_blob.properties().cache_control(), blob.properties().cache_control());
        }

        if (!headers.content_disposition().empty())
        {
            CHECK_UTF8_EQUAL(headers.content_disposition(), blob.properties().content_disposition());
        }
        else
        {
            CHECK_UTF8_EQUAL(original_blob.properties().content_disposition(), blob.properties().content_disposition());
        }

        if (!headers.content_encoding().empty())
        {
            CHECK_UTF8_EQUAL(headers.content_encoding(), blob.properties().content_encoding());
        }
        else
        {
            CHECK_UTF8_EQUAL(original_blob.properties().content_encoding(), blob.properties().content_encoding());
        }

        if (!headers.content_language().empty())
        {
            CHECK_UTF8_EQUAL(headers.content_language(), blob.properties().content_language());
        }
        else
        {
            CHECK_UTF8_EQUAL(original_blob.properties().content_language(), blob.properties().content_language());
        }

        if (!headers.content_type().empty())
        {
            CHECK_UTF8_EQUAL(headers.content_type(), blob.properties().content_type());
        }
        else
        {
            CHECK_UTF8_EQUAL(original_blob.properties().content_type(), blob.properties().content_type());
        }
    }
    else
    {
        CHECK_THROW(blob.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context), wa::storage::storage_exception);
    }

    if (permissions & wa::storage::blob_shared_access_policy::permissions::write)
    {
        blob.upload_metadata(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
    }
    else
    {
        CHECK_THROW(blob.upload_metadata(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context), wa::storage::storage_exception);
    }

    if (permissions & wa::storage::blob_shared_access_policy::permissions::del)
    {
        blob.delete_blob(wa::storage::delete_snapshots_option::none, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
    }
    else
    {
        CHECK_THROW(blob.delete_blob(wa::storage::delete_snapshots_option::none, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context), wa::storage::storage_exception);
    }
}

#pragma endregion

SUITE(Blob)
{
    TEST_FIXTURE(blob_test_base, blob_delete)
    {
        auto blob = m_container.get_block_blob_reference(U("blockblob"));

        CHECK(!blob.delete_blob_if_exists(wa::storage::delete_snapshots_option::none, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context));
        CHECK_THROW(blob.delete_blob(wa::storage::delete_snapshots_option::none, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context), wa::storage::storage_exception);

        blob.upload_block_list(std::vector<wa::storage::block_list_item>());

        CHECK(blob.delete_blob_if_exists(wa::storage::delete_snapshots_option::none, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context));
        CHECK(!blob.delete_blob_if_exists(wa::storage::delete_snapshots_option::none, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context));
    }

    TEST_FIXTURE(blob_test_base, blob_exists)
    {
        auto blob = m_container.get_page_blob_reference(U("pageblob"));

        CHECK(!blob.exists(wa::storage::blob_request_options(), m_context));
        blob.create(1024, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);

        auto same_blob = m_container.get_page_blob_reference(U("pageblob"));

        CHECK(same_blob.exists(wa::storage::blob_request_options(), m_context));
        CHECK_EQUAL(1024, same_blob.properties().size());
    }

    TEST_FIXTURE(blob_test_base, blob_properties)
    {
        wa::storage::blob_request_options options;
        options.set_disable_content_md5_validation(true);

        auto blob = m_container.get_page_blob_reference(U("pageblob"));
        CHECK_THROW(blob.download_attributes(wa::storage::access_condition(), options, m_context), wa::storage::storage_exception);

        blob.create(1024, wa::storage::access_condition(), options, m_context);
        CHECK_EQUAL(1024, blob.properties().size());
        CHECK(!blob.properties().etag().empty());
        CHECK((utility::datetime::utc_now() - blob.properties().last_modified()) < utility::datetime::from_minutes(5));
        CHECK(blob.properties().cache_control().empty());
        CHECK(blob.properties().content_disposition().empty());
        CHECK(blob.properties().content_encoding().empty());
        CHECK(blob.properties().content_language().empty());
        CHECK(blob.properties().content_md5().empty());
        CHECK(blob.properties().content_type().empty());
        CHECK(wa::storage::lease_status::unspecified == blob.properties().lease_status());

        auto same_blob = m_container.get_page_blob_reference(blob.name());
        same_blob.download_attributes(wa::storage::access_condition(), options, m_context);
        CHECK_EQUAL(1024, same_blob.properties().size());
        CHECK_UTF8_EQUAL(blob.properties().etag(), same_blob.properties().etag());
        CHECK(blob.properties().last_modified() == same_blob.properties().last_modified());
        CHECK(same_blob.properties().cache_control().empty());
        CHECK(same_blob.properties().content_disposition().empty());
        CHECK(same_blob.properties().content_encoding().empty());
        CHECK(same_blob.properties().content_language().empty());
        CHECK(same_blob.properties().content_md5().empty());
        CHECK_UTF8_EQUAL(U("application/octet-stream"), same_blob.properties().content_type());
        CHECK(wa::storage::lease_status::unlocked == same_blob.properties().lease_status());

        std::this_thread::sleep_for(std::chrono::seconds(1));

        blob.properties().set_cache_control(U("no-transform"));
        blob.properties().set_content_disposition(U("attachment"));
        blob.properties().set_content_encoding(U("gzip"));
        blob.properties().set_content_language(U("tr,en"));
        blob.properties().set_content_md5(dummy_md5);
        blob.properties().set_content_type(U("text/html"));
        blob.upload_properties(wa::storage::access_condition(), options, m_context);
        CHECK(blob.properties().etag() != same_blob.properties().etag());
        CHECK(blob.properties().last_modified().to_interval() > same_blob.properties().last_modified().to_interval());

        same_blob.download_attributes(wa::storage::access_condition(), options, m_context);
        check_blob_properties_equal(blob.properties(), same_blob.properties());

        auto still_same_blob = m_container.get_page_blob_reference(blob.name());
        auto stream = concurrency::streams::container_stream<std::vector<uint8_t>>::open_ostream();
        still_same_blob.download_to_stream(stream, wa::storage::access_condition(), options, m_context);
        check_blob_properties_equal(blob.properties(), still_same_blob.properties());

        auto listing = list_all_blobs(utility::string_t(), wa::storage::blob_listing_includes::all(), 0, options);
        check_blob_properties_equal(blob.properties(), listing.front().properties());
    }

    TEST_FIXTURE(blob_test_base, blob_type)
    {
        auto page_blob = m_container.get_page_blob_reference(U("pageblob"));
        CHECK(wa::storage::blob_type::page_blob == page_blob.type());
        page_blob.create(0, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);

        auto block_blob = m_container.get_block_blob_reference(U("blockblob"));
        CHECK(wa::storage::blob_type::block_blob == block_blob.type());
        block_blob.upload_text(utility::string_t(), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);

        auto same_page_blob = m_container.get_blob_reference(U("pageblob"));
        CHECK(wa::storage::blob_type::unspecified == same_page_blob.type());
        same_page_blob.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK(wa::storage::blob_type::page_blob == same_page_blob.type());

        auto same_block_blob = m_container.get_blob_reference(U("blockblob"));
        CHECK(wa::storage::blob_type::unspecified == same_block_blob.type());
        same_block_blob.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK(wa::storage::blob_type::block_blob == same_block_blob.type());

        auto invalid_page_blob = m_container.get_page_blob_reference(U("blockblob"));
        CHECK_THROW(invalid_page_blob.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context), wa::storage::storage_exception);

        auto invalid_block_blob = m_container.get_block_blob_reference(U("pageblob"));
        CHECK_THROW(invalid_block_blob.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context), wa::storage::storage_exception);
    }

    TEST_FIXTURE(blob_test_base, blob_metadata)
    {
        // Create with 2 pairs
        auto blob = m_container.get_block_blob_reference(U("blockblob"));
        blob.metadata()[U("key1")] = U("value1");
        blob.metadata()[U("key2")] = U("value2");
        blob.upload_text(utility::string_t());

        auto same_blob = m_container.get_blob_reference(blob.name());
        CHECK(same_blob.metadata().empty());
        same_blob.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2, same_blob.metadata().size());
        CHECK_UTF8_EQUAL(U("value1"), same_blob.metadata()[U("key1")]);
        CHECK_UTF8_EQUAL(U("value2"), same_blob.metadata()[U("key2")]);

        // Add 1 pair
        same_blob.metadata()[U("key3")] = U("value3");
        same_blob.upload_metadata(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        blob.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK_EQUAL(3, same_blob.metadata().size());
        CHECK_UTF8_EQUAL(U("value1"), blob.metadata()[U("key1")]);
        CHECK_UTF8_EQUAL(U("value2"), blob.metadata()[U("key2")]);
        CHECK_UTF8_EQUAL(U("value3"), blob.metadata()[U("key3")]);

        // Overwrite with 1 pair
        blob.metadata().clear();
        blob.metadata()[U("key4")] = U("value4");
        blob.upload_metadata(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        same_blob.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK_EQUAL(1, same_blob.metadata().size());
        CHECK_UTF8_EQUAL(U("value4"), same_blob.metadata()[U("key4")]);

        // Clear all pairs
        same_blob.metadata().clear();
        same_blob.upload_metadata(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        blob.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK(blob.metadata().empty());
    }

    TEST_FIXTURE(blob_test_base, blob_invalid_sas_and_snapshot)
    {
        wa::storage::blob_shared_access_policy policy;
        policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));
        policy.set_permissions(wa::storage::blob_shared_access_policy::permissions::read);
        auto sas_token = m_container.get_shared_access_signature(policy);

        web::http::uri_builder builder(m_container.uri().primary_uri());
        builder.append_path(U("/blob"));
        wa::storage::cloud_blob blob1(wa::storage::storage_uri(builder.to_uri()), m_container.service_client().credentials());

        builder.set_query(sas_token);
        CHECK_THROW(wa::storage::cloud_blob(wa::storage::storage_uri(builder.to_uri()), m_container.service_client().credentials()), std::invalid_argument);

        utility::string_t snapshot_time(U("2013-08-15T11:22:33.1234567Z"));
        utility::string_t invalid_snapshot_time(U("2013-08-15T12:22:33.1234567Z"));
        builder.set_query(utility::string_t());
        wa::storage::cloud_blob blob2(wa::storage::storage_uri(builder.to_uri()), snapshot_time, m_container.service_client().credentials());

        builder.append_query(U("snapshot"), invalid_snapshot_time);
        wa::storage::cloud_blob blob3(wa::storage::storage_uri(builder.to_uri()), invalid_snapshot_time, m_container.service_client().credentials());
        CHECK_THROW(wa::storage::cloud_blob(wa::storage::storage_uri(builder.to_uri()), snapshot_time, m_container.service_client().credentials()), std::invalid_argument);

        auto sas_container = wa::storage::cloud_blob_container(m_container.uri(), wa::storage::storage_credentials(sas_token));
        CHECK_THROW(sas_container.get_shared_access_signature(policy), std::logic_error);
        auto sas_blob = sas_container.get_blob_reference(U("blob"));
        CHECK_THROW(sas_blob.get_shared_access_signature(policy), std::logic_error);

        auto anonymous_container = wa::storage::cloud_blob_container(m_container.uri());
        CHECK_THROW(anonymous_container.get_shared_access_signature(policy), std::logic_error);
        auto anonymous_blob = anonymous_container.get_blob_reference(U("blob"));
        CHECK_THROW(anonymous_blob.get_shared_access_signature(policy), std::logic_error);
    }

    TEST_FIXTURE(blob_test_base, container_sas_combinations)
    {
        for (int i = 0; i < 16; i++)
        {
            auto permissions = static_cast<wa::storage::blob_shared_access_policy::permissions>(i);

            wa::storage::blob_shared_access_policy policy;
            policy.set_permissions(permissions);
            policy.set_start(utility::datetime::utc_now() - utility::datetime::from_minutes(5));
            policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));
            auto sas_token = m_container.get_shared_access_signature(policy);

            auto blob = m_container.get_block_blob_reference(U("blob") + utility::conversions::print_string(i));
            blob.properties().set_cache_control(U("no-transform"));
            blob.properties().set_content_disposition(U("attachment"));
            blob.properties().set_content_encoding(U("gzip"));
            blob.properties().set_content_language(U("tr,en"));
            blob.properties().set_content_type(U("text/html"));
            blob.upload_text(U("test"), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);

            check_access(sas_token, permissions, wa::storage::cloud_blob_shared_access_headers(), blob);
        }
    }

    TEST_FIXTURE(blob_test_base, blob_sas_combinations)
    {
        for (int i = 0; i < 8; i++)
        {
            auto permissions = static_cast<wa::storage::blob_shared_access_policy::permissions>(i);

            wa::storage::blob_shared_access_policy policy;
            policy.set_permissions(permissions);
            policy.set_start(utility::datetime::utc_now() - utility::datetime::from_minutes(5));
            policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));

            auto blob = m_container.get_block_blob_reference(U("blob") + utility::conversions::print_string(i));
            blob.properties().set_cache_control(U("no-transform"));
            blob.properties().set_content_disposition(U("attachment"));
            blob.properties().set_content_encoding(U("gzip"));
            blob.properties().set_content_language(U("tr,en"));
            blob.properties().set_content_type(U("text/html"));
            blob.upload_text(U("test"), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);

            auto sas_token = blob.get_shared_access_signature(policy);
            check_access(sas_token, permissions, wa::storage::cloud_blob_shared_access_headers(), blob);
        }
    }

    TEST_FIXTURE(blob_test_base, blob_sas_combinations_headers)
    {
        wa::storage::blob_shared_access_policy policy;
        policy.set_permissions(wa::storage::blob_shared_access_policy::permissions::read);
        policy.set_start(utility::datetime::utc_now() - utility::datetime::from_minutes(5));
        policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));

        auto blob = m_container.get_block_blob_reference(U("blob"));
        blob.properties().set_cache_control(U("no-transform"));
        blob.properties().set_content_disposition(U("attachment"));
        blob.properties().set_content_encoding(U("gzip"));
        blob.properties().set_content_language(U("tr,en"));
        blob.properties().set_content_type(U("text/html"));
        blob.upload_text(U("test"), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);

        wa::storage::cloud_blob_shared_access_headers headers;
        headers.set_cache_control(U("s-maxage"));
        headers.set_content_disposition(U("inline"));
        headers.set_content_encoding(U("chunked"));
        headers.set_content_language(U("en"));
        headers.set_content_type(U("plain/text"));

        auto sas_token = blob.get_shared_access_signature(policy, utility::string_t(), headers);
        check_access(sas_token, wa::storage::blob_shared_access_policy::permissions::read, headers, blob);
    }

    TEST_FIXTURE(blob_test_base, blob_sas_invalid_time)
    {
        wa::storage::blob_shared_access_policy policy;
        policy.set_permissions(wa::storage::blob_shared_access_policy::permissions::read);

        auto blob = m_container.get_block_blob_reference(U("blob"));
        blob.upload_text(U("test"), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);

        policy.set_start(utility::datetime::utc_now() + utility::datetime::from_minutes(5));
        policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));
        check_access(blob.get_shared_access_signature(policy), wa::storage::blob_shared_access_policy::permissions::none, wa::storage::cloud_blob_shared_access_headers(), blob);

        policy.set_start(utility::datetime::utc_now() - utility::datetime::from_minutes(10));
        policy.set_expiry(utility::datetime::utc_now() - utility::datetime::from_minutes(5));
        check_access(blob.get_shared_access_signature(policy), wa::storage::blob_shared_access_policy::permissions::none, wa::storage::cloud_blob_shared_access_headers(), blob);
    }

    TEST_FIXTURE(block_blob_test_base, blob_snapshot)
    {
        m_blob.upload_text(U("1"), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK(!m_blob.is_snapshot());
        CHECK(m_blob.snapshot_time().empty());
        CHECK_UTF8_EQUAL(m_blob.snapshot_qualified_uri().primary_uri().to_string(), m_blob.uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.snapshot_qualified_uri().secondary_uri().to_string(), m_blob.uri().secondary_uri().to_string());

        auto snapshot1 = m_blob.create_snapshot(wa::storage::cloud_metadata(), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK(wa::storage::blob_type::block_blob == snapshot1.type());
        CHECK_UTF8_EQUAL(m_blob.properties().etag(), snapshot1.properties().etag());
        CHECK(m_blob.properties().last_modified() == snapshot1.properties().last_modified());
        CHECK(snapshot1.is_snapshot());
        CHECK(!snapshot1.snapshot_time().empty());
        CHECK_UTF8_EQUAL(m_blob.uri().primary_uri().to_string(), snapshot1.uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.uri().secondary_uri().to_string(), snapshot1.uri().secondary_uri().to_string());
        CHECK(m_blob.snapshot_qualified_uri().primary_uri() != snapshot1.snapshot_qualified_uri().primary_uri());
        CHECK(m_blob.snapshot_qualified_uri().secondary_uri() != snapshot1.snapshot_qualified_uri().secondary_uri());
        CHECK(snapshot1.snapshot_qualified_uri().primary_uri().query().find(U("snapshot")) != utility::string_t::npos);
        CHECK(snapshot1.snapshot_qualified_uri().secondary_uri().query().find(U("snapshot")) != utility::string_t::npos);
        
        CHECK_THROW(snapshot1.upload_properties(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context), std::logic_error);
        CHECK_THROW(snapshot1.upload_metadata(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context), std::logic_error);
        CHECK_THROW(snapshot1.create_snapshot(wa::storage::cloud_metadata(), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context), std::logic_error);

        std::this_thread::sleep_for(std::chrono::seconds(1));

        auto snapshot2 = m_blob.create_snapshot(wa::storage::cloud_metadata(), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK(snapshot2.snapshot_time() > snapshot1.snapshot_time());

        snapshot1.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        m_blob.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        check_blob_properties_equal(m_blob.properties(), snapshot1.properties());

        web::http::uri snapshot1_primary_uri(m_blob.uri().primary_uri().to_string() + U("?snapshot=") + snapshot1.snapshot_time());
        web::http::uri snapshot1_secondary_uri(m_blob.uri().secondary_uri().to_string() + U("?snapshot=") + snapshot1.snapshot_time());
        wa::storage::cloud_blob snapshot1_clone(wa::storage::storage_uri(snapshot1_primary_uri, snapshot1_secondary_uri), m_blob.service_client().credentials());
        CHECK(snapshot1.snapshot_time() == snapshot1_clone.snapshot_time());
        snapshot1_clone.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        check_blob_properties_equal(snapshot1.properties(), snapshot1_clone.properties());

        wa::storage::cloud_blob snapshot1_clone2(m_blob.uri(), snapshot1.snapshot_time(), m_blob.service_client().credentials());
        CHECK(snapshot1.snapshot_time() == snapshot1_clone2.snapshot_time());
        snapshot1_clone2.download_attributes(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        check_blob_properties_equal(snapshot1.properties(), snapshot1_clone2.properties());

        m_blob.upload_text(U("2"), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK_UTF8_EQUAL(U("1"), wa::storage::cloud_block_blob(snapshot1).download_text(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context));

        auto snapshot1_copy = m_container.get_block_blob_reference(m_blob.name() + U("copy"));
        snapshot1_copy.start_copy_from_blob(defiddler(snapshot1.snapshot_qualified_uri().primary_uri()), wa::storage::access_condition(), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK(wait_for_copy(snapshot1_copy));
        CHECK_UTF8_EQUAL(U("1"), snapshot1_copy.download_text(wa::storage::access_condition(), wa::storage::blob_request_options(), m_context));

        auto blobs = list_all_blobs(utility::string_t(), wa::storage::blob_listing_includes::all(), 0, wa::storage::blob_request_options());
        CHECK_EQUAL(4, blobs.size());
        check_blob_equal(snapshot1, blobs[0]);
        check_blob_equal(snapshot2, blobs[1]);
        check_blob_equal(m_blob, blobs[2]);
        check_blob_equal(snapshot1_copy, blobs[3]);
    }

    TEST_FIXTURE(blob_test_base, blob_snapshot_delete)
    {
        auto delete_root_only_blob = m_container.get_block_blob_reference(U("delete_root_only_not_permitted"));
        delete_root_only_blob.upload_text(U("1"), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        auto delete_root_only_snapshot = delete_root_only_blob.create_snapshot(wa::storage::cloud_metadata(), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK_THROW(delete_root_only_blob.delete_blob(wa::storage::delete_snapshots_option::none, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context), wa::storage::storage_exception);
        CHECK(delete_root_only_blob.exists(wa::storage::blob_request_options(), m_context));
        CHECK(delete_root_only_snapshot.exists(wa::storage::blob_request_options(), m_context));

        auto delete_snapshots_only_blob = m_container.get_block_blob_reference(U("delete_snapshots_only"));
        delete_snapshots_only_blob.upload_text(U("1"), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        auto delete_snapshots_only_snapshot = delete_snapshots_only_blob.create_snapshot(wa::storage::cloud_metadata(), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        delete_snapshots_only_blob.delete_blob(wa::storage::delete_snapshots_option::delete_snapshots_only, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK(delete_snapshots_only_blob.exists(wa::storage::blob_request_options(), m_context));
        CHECK(!delete_snapshots_only_snapshot.exists(wa::storage::blob_request_options(), m_context));

        auto delete_everything_blob = m_container.get_block_blob_reference(U("delete_everything"));
        delete_everything_blob.upload_text(U("1"), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        auto delete_everything_snapshot = delete_everything_blob.create_snapshot(wa::storage::cloud_metadata(), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        delete_everything_blob.delete_blob(wa::storage::delete_snapshots_option::include_snapshots, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK(!delete_everything_blob.exists(wa::storage::blob_request_options(), m_context));
        CHECK(!delete_everything_snapshot.exists(wa::storage::blob_request_options(), m_context));

        CHECK_THROW(delete_root_only_snapshot.delete_blob(wa::storage::delete_snapshots_option::include_snapshots, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context), std::invalid_argument);
        CHECK_THROW(delete_root_only_snapshot.delete_blob(wa::storage::delete_snapshots_option::delete_snapshots_only, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context), std::invalid_argument);
        delete_root_only_snapshot.delete_blob(wa::storage::delete_snapshots_option::none, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
    }

    TEST_FIXTURE(blob_test_base, blob_copy)
    {
        auto blob = m_container.get_block_blob_reference(U("blob"));
        blob.upload_text(U("1"), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);

        auto copy = m_container.get_block_blob_reference(U("copy"));
        CHECK_THROW(copy.start_copy_from_blob(defiddler(blob.uri().primary_uri()), wa::storage::access_condition::generate_if_match_condition(U("\"0xFFFFFFFFFFFFFFF\"")), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context), wa::storage::storage_exception);
        CHECK_EQUAL(web::http::status_codes::PreconditionFailed, m_context.request_results().back().http_status_code());
        auto copy_id = copy.start_copy_from_blob(defiddler(blob.uri().primary_uri()), wa::storage::access_condition::generate_if_match_condition(blob.properties().etag()), wa::storage::access_condition(), wa::storage::blob_request_options(), m_context);
        CHECK(wait_for_copy(copy));
        CHECK_THROW(copy.abort_copy(copy_id, wa::storage::access_condition(), wa::storage::blob_request_options(), m_context), wa::storage::storage_exception);
        CHECK_EQUAL(web::http::status_codes::Conflict, m_context.request_results().back().http_status_code());
        CHECK_THROW(copy.start_copy_from_blob(defiddler(blob.uri().primary_uri()), wa::storage::access_condition::generate_if_match_condition(blob.properties().etag()), wa::storage::access_condition::generate_if_match_condition(U("\"0xFFFFFFFFFFFFFFF\"")), wa::storage::blob_request_options(), m_context), wa::storage::storage_exception);
        CHECK_EQUAL(web::http::status_codes::PreconditionFailed, m_context.request_results().back().http_status_code());
        copy.start_copy_from_blob(defiddler(blob.uri().primary_uri()), wa::storage::access_condition::generate_if_match_condition(blob.properties().etag()), wa::storage::access_condition::generate_if_match_condition(copy.properties().etag()), wa::storage::blob_request_options(), m_context);
        CHECK(wait_for_copy(copy));
    }
}
