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
#include "file_test_base.h"

#include "cpprest/producerconsumerstream.h"

#pragma region Fixture

bool blob_test_base::wait_for_copy(azure::storage::cloud_blob& blob)
{
    do
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
    } while (blob.copy_state().status() == azure::storage::copy_status::pending);

    return blob.copy_state().status() == azure::storage::copy_status::success;
}

azure::storage::operation_context blob_test_base::upload_and_download(azure::storage::cloud_blob& blob, size_t buffer_size, size_t buffer_offset, size_t blob_size, bool use_seekable_stream, const azure::storage::blob_request_options& options, size_t expected_request_count, bool expect_md5_header)
{
    azure::storage::operation_context context;
    print_client_request_id(context, _XPLATSTR("upload/download"));

    utility::string_t md5_header;
    context.set_sending_request([&md5_header] (web::http::http_request& request, azure::storage::operation_context)
    {
        if (!request.headers().match(_XPLATSTR("x-ms-blob-content-md5"), md5_header))
        {
            md5_header.clear();
        }
    });

    std::vector<uint8_t> buffer;
    buffer.resize(buffer_size);
    size_t target_blob_size = blob_size == 0 ? buffer_size - buffer_offset : blob_size;
    auto md5 = fill_buffer_and_get_md5(buffer, buffer_offset, target_blob_size);

    concurrency::streams::istream stream;
    if (use_seekable_stream)
    {
        stream = concurrency::streams::bytestream::open_istream(buffer);
        stream.seek(buffer_offset);
    }
    else
    {
        concurrency::streams::producer_consumer_buffer<uint8_t> pcbuffer;
        pcbuffer.putn_nocopy(buffer.data() + buffer_offset, buffer_size - buffer_offset);
        pcbuffer.close(std::ios_base::out);
        stream = pcbuffer.create_istream();
    }

    if (blob.type() == azure::storage::blob_type::block_blob)
    {
        azure::storage::cloud_block_blob block_blob(blob);
        if (blob_size == 0)
        {
            block_blob.upload_from_stream(stream, azure::storage::access_condition(), options, context);
        }
        else
        {
            block_blob.upload_from_stream(stream, blob_size, azure::storage::access_condition(), options, context);
        }
    }
    else if (blob.type() == azure::storage::blob_type::page_blob)
    {
        azure::storage::cloud_page_blob page_blob(blob);
        if (blob_size == 0)
        {
            page_blob.upload_from_stream(stream, 0, azure::storage::access_condition(), options, context);
        }
        else
        {
            page_blob.upload_from_stream(stream, blob_size, 0, azure::storage::access_condition(), options, context);
        }
    }
    else if (blob.type() == azure::storage::blob_type::append_blob)
    {
        azure::storage::cloud_append_blob append_blob(blob);
        if (blob_size == 0)
        {
            append_blob.upload_from_stream(stream, azure::storage::access_condition(), options, context);
        }
        else
        {
            append_blob.upload_from_stream(stream, blob_size, azure::storage::access_condition(), options, context);
        }
    }

    check_blob_no_stale_property(blob);
    CHECK_UTF8_EQUAL(expect_md5_header ? md5 : utility::string_t(), md5_header);
    CHECK_EQUAL(expected_request_count, context.request_results().size());

    azure::storage::blob_request_options download_options(options);
    download_options.set_use_transactional_md5(false);

    concurrency::streams::container_buffer<std::vector<uint8_t>> output_buffer;
    blob.download_to_stream(output_buffer.create_ostream(), azure::storage::access_condition(), download_options, context);
    CHECK_ARRAY_EQUAL(buffer.data() + buffer_offset, output_buffer.collection().data(),(int) target_blob_size);

    context.set_sending_request(std::function<void(web::http::http_request &, azure::storage::operation_context)>());
    return context;
}

void blob_test_base::check_access(const utility::string_t& sas_token, uint8_t permissions, const azure::storage::cloud_blob_shared_access_headers& headers, const azure::storage::cloud_blob& original_blob)
{
    azure::storage::storage_credentials credentials;
    if (!sas_token.empty())
    {
        credentials = azure::storage::storage_credentials(sas_token);
    }

    azure::storage::cloud_blob_container container(m_container.uri(), credentials);
    azure::storage::cloud_blob blob = container.get_blob_reference(original_blob.name());

    if (permissions & azure::storage::blob_shared_access_policy::permissions::list)
    {
        container.list_blobs_segmented(utility::string_t(), true, azure::storage::blob_listing_details::none, 0, azure::storage::continuation_token(), azure::storage::blob_request_options(), m_context);
    }
    else
    {
        CHECK_THROW(container.list_blobs_segmented(utility::string_t(), true, azure::storage::blob_listing_details::none, 0, azure::storage::continuation_token(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
    }

    if (permissions & azure::storage::blob_shared_access_policy::permissions::read)
    {
        blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

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
        CHECK_THROW(blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
    }

    if (permissions & azure::storage::blob_shared_access_policy::permissions::write)
    {
        blob.upload_metadata(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
    }
    else
    {
        CHECK_THROW(blob.upload_metadata(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
    }

    if (permissions & azure::storage::blob_shared_access_policy::permissions::del)
    {
        blob.delete_blob(azure::storage::delete_snapshots_option::none, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
    }
    else
    {
        CHECK_THROW(blob.delete_blob(azure::storage::delete_snapshots_option::none, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
    }
}

void blob_test_base::check_blob_no_stale_property(azure::storage::cloud_blob& blob)
{
    // check lease property
    CHECK(blob.properties().lease_status() == azure::storage::lease_status::unspecified);
    CHECK(blob.properties().lease_state() == azure::storage::lease_state::unspecified);
    CHECK(blob.properties().lease_duration() == azure::storage::lease_duration::unspecified);

    // check copy property
    CHECK(blob.copy_state().source() == web::http::uri());
    CHECK(blob.copy_state().status() == azure::storage::copy_status::invalid);
    CHECK(blob.copy_state().completion_time() == utility::datetime());
    CHECK(blob.copy_state().status_description() == utility::string_t());
    CHECK(blob.copy_state().copy_id() == utility::string_t());
    CHECK_EQUAL(blob.copy_state().bytes_copied(), 0);
    CHECK_EQUAL(blob.copy_state().total_bytes(), 0);
}

void container_test_base::check_container_no_stale_property(azure::storage::cloud_blob_container& container)
{
    // check lease property
    CHECK(container.properties().lease_status() == azure::storage::lease_status::unspecified);
    CHECK(container.properties().lease_state() == azure::storage::lease_state::unspecified);
    CHECK(container.properties().lease_duration() == azure::storage::lease_duration::unspecified);
}

#pragma endregion

SUITE(Blob)
{
    TEST_FIXTURE(blob_test_base, blob_delete)
    {
        auto blob = m_container.get_block_blob_reference(_XPLATSTR("blockblob"));

        CHECK(!blob.delete_blob_if_exists(azure::storage::delete_snapshots_option::none, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context));
        CHECK_THROW(blob.delete_blob(azure::storage::delete_snapshots_option::none, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);

        blob.upload_block_list(std::vector<azure::storage::block_list_item>());

        CHECK(blob.delete_blob_if_exists(azure::storage::delete_snapshots_option::none, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context));
        CHECK(!blob.delete_blob_if_exists(azure::storage::delete_snapshots_option::none, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context));
    }

    TEST_FIXTURE(blob_test_base, blob_exists)
    {
        auto blob = m_container.get_page_blob_reference(_XPLATSTR("pageblob"));

        CHECK(!blob.exists(azure::storage::blob_request_options(), m_context));
        blob.create(1024, 0, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto same_blob = m_container.get_page_blob_reference(_XPLATSTR("pageblob"));

        CHECK(same_blob.exists(azure::storage::blob_request_options(), m_context));
        CHECK_EQUAL(1024, same_blob.properties().size());
    }

    TEST_FIXTURE(blob_test_base, blob_properties)
    {
        azure::storage::blob_request_options options;
        options.set_disable_content_md5_validation(true);

        auto blob = m_container.get_page_blob_reference(_XPLATSTR("pageblob"));
        CHECK_THROW(blob.download_attributes(azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);

        blob.create(1024, 0, azure::storage::access_condition(), options, m_context);
        CHECK_EQUAL(1024, blob.properties().size());
        CHECK(!blob.properties().etag().empty());
        CHECK((utility::datetime::utc_now() - blob.properties().last_modified()) < (int64_t)utility::datetime::from_minutes(5));
        CHECK(blob.properties().cache_control().empty());
        CHECK(blob.properties().content_disposition().empty());
        CHECK(blob.properties().content_encoding().empty());
        CHECK(blob.properties().content_language().empty());
        CHECK(blob.properties().content_md5().empty());
        CHECK(blob.properties().content_type().empty());
        CHECK(azure::storage::lease_status::unspecified == blob.properties().lease_status());

        {
            auto same_blob = m_container.get_page_blob_reference(blob.name());
            same_blob.download_attributes(azure::storage::access_condition(), options, m_context);
            CHECK_EQUAL(1024, same_blob.properties().size());
            CHECK_UTF8_EQUAL(blob.properties().etag(), same_blob.properties().etag());
            CHECK(blob.properties().last_modified() == same_blob.properties().last_modified());
            CHECK(same_blob.properties().cache_control().empty());
            CHECK(same_blob.properties().content_disposition().empty());
            CHECK(same_blob.properties().content_encoding().empty());
            CHECK(same_blob.properties().content_language().empty());
            CHECK(same_blob.properties().content_md5().empty());
            CHECK_UTF8_EQUAL(_XPLATSTR("application/octet-stream"), same_blob.properties().content_type());
            CHECK(azure::storage::lease_status::unlocked == same_blob.properties().lease_status());

            std::this_thread::sleep_for(std::chrono::seconds(1));

            blob.properties().set_cache_control(_XPLATSTR("no-transform"));
            blob.properties().set_content_disposition(_XPLATSTR("attachment"));
            blob.properties().set_content_encoding(_XPLATSTR("gzip"));
            blob.properties().set_content_language(_XPLATSTR("tr,en"));
            blob.properties().set_content_md5(dummy_md5);
            blob.properties().set_content_type(_XPLATSTR("text/html"));
            blob.upload_properties(azure::storage::access_condition(), options, m_context);
            CHECK(blob.properties().etag() != same_blob.properties().etag());
            CHECK(blob.properties().last_modified().to_interval() > same_blob.properties().last_modified().to_interval());

            same_blob.download_attributes(azure::storage::access_condition(), options, m_context);
            check_blob_properties_equal(blob.properties(), same_blob.properties(), true);
        }

        {
            auto same_blob = m_container.get_page_blob_reference(blob.name());
            auto stream = concurrency::streams::container_stream<std::vector<uint8_t>>::open_ostream();
            same_blob.download_to_stream(stream, azure::storage::access_condition(), options, m_context);
            check_blob_properties_equal(blob.properties(), same_blob.properties(), true);
        }

        {
            auto same_blob = m_container.get_page_blob_reference(blob.name());
            auto stream = concurrency::streams::container_stream<std::vector<uint8_t>>::open_ostream();
            azure::storage::blob_request_options options;
            options.set_use_transactional_md5(true);
            same_blob.download_range_to_stream(stream, 0, 128, azure::storage::access_condition(), options, azure::storage::operation_context());
            check_blob_properties_equal(blob.properties(), same_blob.properties(), true);
        }
        
        {
            auto listing = list_all_blobs(utility::string_t(), azure::storage::blob_listing_details::all, 0, options);
            check_blob_properties_equal(blob.properties(), listing.front().properties(), true);
        }
    }

    TEST_FIXTURE(blob_test_base, blob_type)
    {
        auto page_blob = m_container.get_page_blob_reference(_XPLATSTR("pageblob"));
        CHECK(azure::storage::blob_type::page_blob == page_blob.type());
        page_blob.create(0, 0, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto block_blob = m_container.get_block_blob_reference(_XPLATSTR("blockblob"));
        CHECK(azure::storage::blob_type::block_blob == block_blob.type());
        block_blob.upload_text(utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto same_page_blob = m_container.get_blob_reference(_XPLATSTR("pageblob"));
        CHECK(azure::storage::blob_type::unspecified == same_page_blob.type());
        same_page_blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(azure::storage::blob_type::page_blob == same_page_blob.type());

        auto same_block_blob = m_container.get_blob_reference(_XPLATSTR("blockblob"));
        CHECK(azure::storage::blob_type::unspecified == same_block_blob.type());
        same_block_blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(azure::storage::blob_type::block_blob == same_block_blob.type());

        auto invalid_page_blob = m_container.get_page_blob_reference(_XPLATSTR("blockblob"));
        CHECK_THROW(invalid_page_blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);

        auto invalid_block_blob = m_container.get_block_blob_reference(_XPLATSTR("pageblob"));
        CHECK_THROW(invalid_block_blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
    }

    TEST_FIXTURE(blob_test_base, blob_metadata)
    {
        // Create with 2 pairs
        auto blob = m_container.get_block_blob_reference(_XPLATSTR("blockblob"));
        blob.metadata()[_XPLATSTR("key1")] = _XPLATSTR("value1");
        blob.metadata()[_XPLATSTR("key2")] = _XPLATSTR("value2");
        blob.upload_text(utility::string_t());

        auto same_blob = m_container.get_blob_reference(blob.name());
        CHECK(same_blob.metadata().empty());
        same_blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2U, same_blob.metadata().size());
        CHECK_UTF8_EQUAL(_XPLATSTR("value1"), same_blob.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value2"), same_blob.metadata()[_XPLATSTR("key2")]);

        // Add 1 pair
        same_blob.metadata()[_XPLATSTR("key3")] = _XPLATSTR("value3");
        same_blob.upload_metadata(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(3U, same_blob.metadata().size());
        CHECK_UTF8_EQUAL(_XPLATSTR("value1"), blob.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value2"), blob.metadata()[_XPLATSTR("key2")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value3"), blob.metadata()[_XPLATSTR("key3")]);

        // Overwrite with 1 pair
        blob.metadata().clear();
        blob.metadata()[_XPLATSTR("key4")] = _XPLATSTR("value4");
        blob.upload_metadata(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        same_blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(1U, same_blob.metadata().size());
        CHECK_UTF8_EQUAL(_XPLATSTR("value4"), same_blob.metadata()[_XPLATSTR("key4")]);

        // Clear all pairs
        same_blob.metadata().clear();
        same_blob.upload_metadata(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(blob.metadata().empty());
    }

    TEST_FIXTURE(blob_test_base, blob_whitespace_metadata)
    {
        // Create with 3 pairs that has space in value.
        auto blob = m_container.get_block_blob_reference(_XPLATSTR("blockblob"));
        blob.metadata()[_XPLATSTR("key1")] = _XPLATSTR("   value1   ");
        blob.metadata()[_XPLATSTR("key2")] = _XPLATSTR("   value2");
        blob.metadata()[_XPLATSTR("key3")] = _XPLATSTR("value3   ");
        blob.upload_text(utility::string_t());

        auto same_blob = m_container.get_blob_reference(blob.name());
        CHECK(same_blob.metadata().empty());
        same_blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(3U, same_blob.metadata().size());
        CHECK_UTF8_EQUAL(_XPLATSTR("value1"), same_blob.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value2"), same_blob.metadata()[_XPLATSTR("key2")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value3"), same_blob.metadata()[_XPLATSTR("key3")]);

        // Add 1 pair with only spaces in name
        auto same_blob1 = m_container.get_blob_reference(blob.name());
        same_blob1.metadata()[_XPLATSTR("   ")] = _XPLATSTR("value");
        CHECK_THROW(same_blob1.upload_metadata(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);

        // Add 1 pair with trailing spaces in name
        auto same_blob2 = m_container.get_blob_reference(blob.name());
        same_blob2.metadata()[_XPLATSTR("key1   ")] = _XPLATSTR("value");
        CHECK_THROW(same_blob2.upload_metadata(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);

        // Add 1 pair with beginning spaces in name
        auto same_blob3 = m_container.get_blob_reference(blob.name());
        same_blob3.metadata()[_XPLATSTR("   key")] = _XPLATSTR("value");
        CHECK_THROW(same_blob3.upload_metadata(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);

        // Add 1 pair with spaces in name
        auto same_blob4 = m_container.get_blob_reference(blob.name());
        same_blob4.metadata()[_XPLATSTR("key   key")] = _XPLATSTR("value");
        CHECK_THROW(same_blob4.upload_metadata(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);

        // Add 1 pair with empty name
        auto same_blob5 = m_container.get_blob_reference(blob.name());
        same_blob5.metadata()[_XPLATSTR("")] = _XPLATSTR("value");
        CHECK_THROW(same_blob5.upload_metadata(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);
    }

    TEST_FIXTURE(blob_test_base, blob_invalid_sas_and_snapshot)
    {
        azure::storage::blob_shared_access_policy policy;
        policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));
        policy.set_permissions(azure::storage::blob_shared_access_policy::permissions::read);
        auto sas_token = m_container.get_shared_access_signature(policy);

        web::http::uri_builder builder(m_container.uri().primary_uri());
        builder.append_path(_XPLATSTR("/blob"));
        azure::storage::cloud_blob blob1(azure::storage::storage_uri(builder.to_uri()), m_container.service_client().credentials());

        builder.set_query(sas_token);
        CHECK_THROW(azure::storage::cloud_blob(azure::storage::storage_uri(builder.to_uri()), m_container.service_client().credentials()), std::invalid_argument);

        utility::string_t snapshot_time(_XPLATSTR("2013-08-15T11:22:33.1234567Z"));
        utility::string_t invalid_snapshot_time(_XPLATSTR("2013-08-15T12:22:33.1234567Z"));
        builder.set_query(utility::string_t());
        azure::storage::cloud_blob blob2(azure::storage::storage_uri(builder.to_uri()), snapshot_time, m_container.service_client().credentials());

        builder.append_query(_XPLATSTR("snapshot"), invalid_snapshot_time);
        azure::storage::cloud_blob blob3(azure::storage::storage_uri(builder.to_uri()), invalid_snapshot_time, m_container.service_client().credentials());
        CHECK_THROW(azure::storage::cloud_blob(azure::storage::storage_uri(builder.to_uri()), snapshot_time, m_container.service_client().credentials()), std::invalid_argument);

        auto sas_container = azure::storage::cloud_blob_container(m_container.uri(), azure::storage::storage_credentials(sas_token));
        CHECK_THROW(sas_container.get_shared_access_signature(policy), std::logic_error);
        auto sas_blob = sas_container.get_blob_reference(_XPLATSTR("blob"));
        CHECK_THROW(sas_blob.get_shared_access_signature(policy), std::logic_error);

        auto anonymous_container = azure::storage::cloud_blob_container(m_container.uri());
        CHECK_THROW(anonymous_container.get_shared_access_signature(policy), std::logic_error);
        auto anonymous_blob = anonymous_container.get_blob_reference(_XPLATSTR("blob"));
        CHECK_THROW(anonymous_blob.get_shared_access_signature(policy), std::logic_error);
    }

    TEST_FIXTURE(blob_test_base, container_sas_combinations)
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            auto permissions = i;

            azure::storage::blob_shared_access_policy policy;
            policy.set_permissions(permissions);
            policy.set_start(utility::datetime::utc_now() - utility::datetime::from_minutes(5));
            policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));
            auto sas_token = m_container.get_shared_access_signature(policy);

            auto blob = m_container.get_block_blob_reference(_XPLATSTR("blob") + utility::conversions::print_string((int)i));
            blob.properties().set_cache_control(_XPLATSTR("no-transform"));
            blob.properties().set_content_disposition(_XPLATSTR("attachment"));
            blob.properties().set_content_encoding(_XPLATSTR("gzip"));
            blob.properties().set_content_language(_XPLATSTR("tr,en"));
            blob.properties().set_content_type(_XPLATSTR("text/html"));
            blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

            check_access(sas_token, permissions, azure::storage::cloud_blob_shared_access_headers(), blob);
        }
    }

    TEST_FIXTURE(blob_test_base, blob_sas_combinations)
    {
        for (uint8_t i = 0; i < 8; i++)
        {
            auto permissions = i;

            azure::storage::blob_shared_access_policy policy;
            policy.set_permissions(permissions);
            policy.set_start(utility::datetime::utc_now() - utility::datetime::from_minutes(5));
            policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));

            auto blob = m_container.get_block_blob_reference(_XPLATSTR("blob") + utility::conversions::print_string((int)i));
            blob.properties().set_cache_control(_XPLATSTR("no-transform"));
            blob.properties().set_content_disposition(_XPLATSTR("attachment"));
            blob.properties().set_content_encoding(_XPLATSTR("gzip"));
            blob.properties().set_content_language(_XPLATSTR("tr,en"));
            blob.properties().set_content_type(_XPLATSTR("text/html"));
            blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

            auto sas_token = blob.get_shared_access_signature(policy);
            check_access(sas_token, permissions, azure::storage::cloud_blob_shared_access_headers(), blob);
        }
    }

    TEST_FIXTURE(blob_test_base, blob_sas_combinations_headers)
    {
        azure::storage::blob_shared_access_policy policy;
        policy.set_permissions(azure::storage::blob_shared_access_policy::permissions::read);
        policy.set_start(utility::datetime::utc_now() - utility::datetime::from_minutes(5));
        policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));

        auto blob = m_container.get_block_blob_reference(_XPLATSTR("blob"));
        blob.properties().set_cache_control(_XPLATSTR("no-transform"));
        blob.properties().set_content_disposition(_XPLATSTR("attachment"));
        blob.properties().set_content_encoding(_XPLATSTR("gzip"));
        blob.properties().set_content_language(_XPLATSTR("tr,en"));
        blob.properties().set_content_type(_XPLATSTR("text/html"));
        blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        azure::storage::cloud_blob_shared_access_headers headers;
        headers.set_cache_control(_XPLATSTR("s-maxage"));
        headers.set_content_disposition(_XPLATSTR("inline"));
        headers.set_content_encoding(_XPLATSTR("chunked"));
        headers.set_content_language(_XPLATSTR("en"));
        headers.set_content_type(_XPLATSTR("plain/text"));

        auto sas_token = blob.get_shared_access_signature(policy, utility::string_t(), headers);
        check_access(sas_token, azure::storage::blob_shared_access_policy::permissions::read, headers, blob);
    }

    TEST_FIXTURE(blob_test_base, blob_sas_invalid_time)
    {
        azure::storage::blob_shared_access_policy policy;
        policy.set_permissions(azure::storage::blob_shared_access_policy::permissions::read);

        auto blob = m_container.get_block_blob_reference(_XPLATSTR("blob"));
        blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        policy.set_start(utility::datetime::utc_now() + utility::datetime::from_minutes(5));
        policy.set_expiry(utility::datetime::utc_now() + utility::datetime::from_minutes(30));
        check_access(blob.get_shared_access_signature(policy), azure::storage::blob_shared_access_policy::permissions::none, azure::storage::cloud_blob_shared_access_headers(), blob);

        policy.set_start(utility::datetime::utc_now() - utility::datetime::from_minutes(10));
        policy.set_expiry(utility::datetime::utc_now() - utility::datetime::from_minutes(5));
        check_access(blob.get_shared_access_signature(policy), azure::storage::blob_shared_access_policy::permissions::none, azure::storage::cloud_blob_shared_access_headers(), blob);
    }

    TEST_FIXTURE(block_blob_test_base, blob_snapshot)
    {
        m_blob.upload_text(_XPLATSTR("1"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(!m_blob.is_snapshot());
        CHECK(m_blob.snapshot_time().empty());
        CHECK_UTF8_EQUAL(m_blob.snapshot_qualified_uri().primary_uri().to_string(), m_blob.uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.snapshot_qualified_uri().secondary_uri().to_string(), m_blob.uri().secondary_uri().to_string());

        auto snapshot1 = m_blob.create_snapshot(azure::storage::cloud_metadata(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(azure::storage::blob_type::block_blob == snapshot1.type());
        CHECK_UTF8_EQUAL(m_blob.properties().etag(), snapshot1.properties().etag());
        CHECK(m_blob.properties().last_modified() == snapshot1.properties().last_modified());
        CHECK(snapshot1.is_snapshot());
        CHECK(!snapshot1.snapshot_time().empty());
        CHECK_UTF8_EQUAL(m_blob.uri().primary_uri().to_string(), snapshot1.uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.uri().secondary_uri().to_string(), snapshot1.uri().secondary_uri().to_string());
        CHECK(m_blob.snapshot_qualified_uri().primary_uri() != snapshot1.snapshot_qualified_uri().primary_uri());
        CHECK(m_blob.snapshot_qualified_uri().secondary_uri() != snapshot1.snapshot_qualified_uri().secondary_uri());
        CHECK(snapshot1.snapshot_qualified_uri().primary_uri().query().find(_XPLATSTR("snapshot")) != utility::string_t::npos);
        CHECK(snapshot1.snapshot_qualified_uri().secondary_uri().query().find(_XPLATSTR("snapshot")) != utility::string_t::npos);

        CHECK_THROW(snapshot1.upload_properties(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::logic_error);
        CHECK_THROW(snapshot1.upload_metadata(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::logic_error);
        CHECK_THROW(snapshot1.create_snapshot(azure::storage::cloud_metadata(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::logic_error);

        std::this_thread::sleep_for(std::chrono::seconds(1));

        auto snapshot2 = m_blob.create_snapshot(azure::storage::cloud_metadata(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(snapshot2.snapshot_time() > snapshot1.snapshot_time());

        snapshot1.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        m_blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        check_blob_properties_equal(m_blob.properties(), snapshot1.properties(), true);

        web::http::uri snapshot1_primary_uri(m_blob.uri().primary_uri().to_string() + _XPLATSTR("?snapshot=") + snapshot1.snapshot_time());
        web::http::uri snapshot1_secondary_uri(m_blob.uri().secondary_uri().to_string() + _XPLATSTR("?snapshot=") + snapshot1.snapshot_time());
        azure::storage::cloud_blob snapshot1_clone(azure::storage::storage_uri(snapshot1_primary_uri, snapshot1_secondary_uri), m_blob.service_client().credentials());
        CHECK(snapshot1.snapshot_time() == snapshot1_clone.snapshot_time());
        snapshot1_clone.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        check_blob_properties_equal(snapshot1.properties(), snapshot1_clone.properties(), true);

        azure::storage::cloud_blob snapshot1_clone2(m_blob.uri(), snapshot1.snapshot_time(), m_blob.service_client().credentials());
        CHECK(snapshot1.snapshot_time() == snapshot1_clone2.snapshot_time());
        snapshot1_clone2.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        check_blob_properties_equal(snapshot1.properties(), snapshot1_clone2.properties(), true);

        m_blob.upload_text(_XPLATSTR("2"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_UTF8_EQUAL(_XPLATSTR("1"), azure::storage::cloud_block_blob(snapshot1).download_text(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context));

        auto snapshot1_copy = m_container.get_block_blob_reference(m_blob.name() + _XPLATSTR("copy"));
        snapshot1_copy.start_copy(defiddler(snapshot1.snapshot_qualified_uri().primary_uri()), azure::storage::access_condition(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(wait_for_copy(snapshot1_copy));
        CHECK_UTF8_EQUAL(_XPLATSTR("1"), snapshot1_copy.download_text(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context));

        auto blobs = list_all_blobs(utility::string_t(), azure::storage::blob_listing_details::all, 0, azure::storage::blob_request_options());
        CHECK_EQUAL(4U, blobs.size());
        check_blob_equal(snapshot1, blobs[0]);
        check_blob_equal(snapshot2, blobs[1]);
        check_blob_equal(m_blob, blobs[2]);
        check_blob_equal(snapshot1_copy, blobs[3]);
    }

    TEST_FIXTURE(blob_test_base, blob_snapshot_delete)
    {
        auto delete_root_only_blob = m_container.get_block_blob_reference(_XPLATSTR("delete_root_only_not_permitted"));
        delete_root_only_blob.upload_text(_XPLATSTR("1"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        auto delete_root_only_snapshot = delete_root_only_blob.create_snapshot(azure::storage::cloud_metadata(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_THROW(delete_root_only_blob.delete_blob(azure::storage::delete_snapshots_option::none, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
        CHECK(delete_root_only_blob.exists(azure::storage::blob_request_options(), m_context));
        CHECK(delete_root_only_snapshot.exists(azure::storage::blob_request_options(), m_context));

        auto delete_snapshots_only_blob = m_container.get_block_blob_reference(_XPLATSTR("delete_snapshots_only"));
        delete_snapshots_only_blob.upload_text(_XPLATSTR("1"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        auto delete_snapshots_only_snapshot = delete_snapshots_only_blob.create_snapshot(azure::storage::cloud_metadata(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        delete_snapshots_only_blob.delete_blob(azure::storage::delete_snapshots_option::delete_snapshots_only, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(delete_snapshots_only_blob.exists(azure::storage::blob_request_options(), m_context));
        CHECK(!delete_snapshots_only_snapshot.exists(azure::storage::blob_request_options(), m_context));

        auto delete_everything_blob = m_container.get_block_blob_reference(_XPLATSTR("delete_everything"));
        delete_everything_blob.upload_text(_XPLATSTR("1"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        auto delete_everything_snapshot = delete_everything_blob.create_snapshot(azure::storage::cloud_metadata(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        delete_everything_blob.delete_blob(azure::storage::delete_snapshots_option::include_snapshots, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(!delete_everything_blob.exists(azure::storage::blob_request_options(), m_context));
        CHECK(!delete_everything_snapshot.exists(azure::storage::blob_request_options(), m_context));

        CHECK_THROW(delete_root_only_snapshot.delete_blob(azure::storage::delete_snapshots_option::include_snapshots, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);
        CHECK_THROW(delete_root_only_snapshot.delete_blob(azure::storage::delete_snapshots_option::delete_snapshots_only, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), std::invalid_argument);
        delete_root_only_snapshot.delete_blob(azure::storage::delete_snapshots_option::none, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
    }

    TEST_FIXTURE(blob_test_base, blob_copy)
    {
        auto blob = m_container.get_block_blob_reference(_XPLATSTR("blob"));
        blob.upload_text(_XPLATSTR("1"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto copy = m_container.get_block_blob_reference(_XPLATSTR("copy"));
        CHECK_THROW(copy.start_copy(defiddler(blob.uri().primary_uri()), azure::storage::access_condition::generate_if_match_condition(_XPLATSTR("\"0xFFFFFFFFFFFFFFF\"")), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
        CHECK_EQUAL(web::http::status_codes::PreconditionFailed, m_context.request_results().back().http_status_code());
        auto copy_id = copy.start_copy(defiddler(blob.uri().primary_uri()), azure::storage::access_condition::generate_if_match_condition(blob.properties().etag()), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(wait_for_copy(copy));
        CHECK_THROW(copy.abort_copy(copy_id, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
        CHECK_EQUAL(web::http::status_codes::Conflict, m_context.request_results().back().http_status_code());
        CHECK_THROW(copy.start_copy(defiddler(blob.uri().primary_uri()), azure::storage::access_condition::generate_if_match_condition(blob.properties().etag()), azure::storage::access_condition::generate_if_match_condition(_XPLATSTR("\"0xFFFFFFFFFFFFFFF\"")), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
        CHECK_EQUAL(web::http::status_codes::PreconditionFailed, m_context.request_results().back().http_status_code());

        // copy from cloud_blob object within same account using shared key.
        auto copy2 = m_container.get_block_blob_reference(_XPLATSTR("copy2"));
        copy2.start_copy(blob, azure::storage::access_condition::generate_if_match_condition(blob.properties().etag()), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(wait_for_copy(copy2));
        CHECK_THROW(copy2.start_copy(blob, azure::storage::access_condition::generate_if_match_condition(blob.properties().etag()), azure::storage::access_condition::generate_if_match_condition(_XPLATSTR("\"0xFFFFFFFFFFFFFFF\"")), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
    }

    TEST_FIXTURE(blob_test_base, blob_copy_with_premium_access_tier)
    {
        m_premium_container.create(azure::storage::blob_container_public_access_type::off, azure::storage::blob_request_options(), m_context);
        auto blob = m_premium_container.get_page_blob_reference(_XPLATSTR("source"));
        blob.create(1024);

        auto dest = m_premium_container.get_page_blob_reference(_XPLATSTR("dest"));
        azure::storage::blob_request_options options;

        dest.start_copy(defiddler(blob.uri().primary_uri()), azure::storage::premium_blob_tier::p30, azure::storage::access_condition(), azure::storage::access_condition(), options, m_context);
        CHECK(azure::storage::premium_blob_tier::p30 == dest.properties().premium_blob_tier());
        dest.download_attributes();
        CHECK(azure::storage::premium_blob_tier::p30 == dest.properties().premium_blob_tier());
    }

    /// <summary>
    /// Test blob copy from a cloud_blob object using sas token.
    /// </summary>
    TEST_FIXTURE(blob_test_base, blob_copy_with_sas_token)
    {
        azure::storage::blob_shared_access_policy read_policy(utility::datetime::utc_now() + utility::datetime::from_minutes(10), azure::storage::blob_shared_access_policy::permissions::read);
        azure::storage::blob_shared_access_policy write_policy(utility::datetime::utc_now() + utility::datetime::from_minutes(10), azure::storage::blob_shared_access_policy::permissions::write);

        for (size_t i = 0; i < 2; ++i)
        {
            auto source_blob_name = this->get_random_string();
            auto source = m_container.get_block_blob_reference(source_blob_name);
            source.upload_text(_XPLATSTR("1"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

            /// create source blob with specified sas credentials, only read access to source blob.
            auto source_sas = source.get_shared_access_signature(read_policy);
            azure::storage::cloud_block_blob source_blob = azure::storage::cloud_block_blob(source.uri(), azure::storage::storage_credentials(source_sas));

            /// create dest blobs with specified sas credentials, only read access to dest read blob and only write access to dest write blob.
            auto dest_blob_name = this->get_random_string();
            auto dest = m_container.get_block_blob_reference(dest_blob_name);

            auto dest_read_sas = dest.get_shared_access_signature(read_policy);
            azure::storage::cloud_block_blob dest_read_blob = azure::storage::cloud_block_blob(dest.uri(), azure::storage::storage_credentials(dest_read_sas));

            auto dest_write_sas = dest.get_shared_access_signature(write_policy);
            azure::storage::cloud_block_blob dest_write_blob = azure::storage::cloud_block_blob(dest.uri(), azure::storage::storage_credentials(dest_write_sas));

            /// try to copy from source blob to dest blob with wrong access condition.
            CHECK_THROW(dest_write_blob.start_copy(source_blob, azure::storage::access_condition::generate_if_match_condition(_XPLATSTR("\"0xFFFFFFFFFFFFFFF\"")), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
            CHECK_EQUAL(web::http::status_codes::PreconditionFailed, m_context.request_results().back().http_status_code());

            /// try to copy from source blob to dest blob, use dest_read_blob to check copy stats.
            auto copy_id = dest_write_blob.start_copy(source_blob, azure::storage::access_condition::generate_if_match_condition(source.properties().etag()), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
            CHECK(wait_for_copy(dest_read_blob));
            CHECK_THROW(dest_write_blob.abort_copy(copy_id, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
            CHECK_EQUAL(web::http::status_codes::Conflict, m_context.request_results().back().http_status_code());
        }
    }

    /// <summary>
    /// Test blob copy from a cloud_file object.
    /// </summary>
    TEST_FIXTURE(blob_test_base, blob_copy_from_file)
    {
        azure::storage::file_shared_access_policy read_policy(utility::datetime::utc_now() + utility::datetime::from_minutes(10), azure::storage::file_shared_access_policy::permissions::read);
        azure::storage::blob_shared_access_policy write_policy(utility::datetime::utc_now() + utility::datetime::from_minutes(10), azure::storage::blob_shared_access_policy::permissions::write);

        for (size_t i = 0; i < 2; ++i)
        {
            auto file_name = this->get_random_string();
            auto share = test_config::instance().account().create_cloud_file_client().get_share_reference(_XPLATSTR("testshare"));
            share.create_if_not_exists();
            auto source = share.get_root_directory_reference().get_file_reference(file_name);
            source.upload_text(_XPLATSTR("1"), azure::storage::file_access_condition(), azure::storage::file_request_options(), m_context);

            /// create source file with specified sas credentials, only read access to source blob.
            auto source_sas = source.get_shared_access_signature(read_policy);
            azure::storage::cloud_file source_file = azure::storage::cloud_file(source.uri(), azure::storage::storage_credentials(source_sas));

            /// create dest blobs with specified sas credentials, only read access to dest read blob and only write access to dest write blob.
            auto dest_blob_name = this->get_random_string();
            auto dest = m_container.get_block_blob_reference(dest_blob_name);

            /// try to copy from source file to dest blob, use dest_read_blob to check copy stats.
            auto copy_id = dest.start_copy(source_file, azure::storage::file_access_condition(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
            CHECK(wait_for_copy(dest));
            CHECK_THROW(dest.abort_copy(copy_id, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
            CHECK_EQUAL(web::http::status_codes::Conflict, m_context.request_results().back().http_status_code());
        }
    }

    /// <summary>
    /// Test parallel download
    /// </summary>
    TEST_FIXTURE(blob_test_base, parallel_download)
    {
        // download blob smaller than 32MB.
        {
            auto blob_name = get_random_string(20);
            auto blob = m_container.get_block_blob_reference(blob_name);
            size_t target_length = 31 * 1024 * 1024;
            azure::storage::blob_request_options option;
            option.set_parallelism_factor(2);
            std::vector<uint8_t> data;
            data.resize(target_length);
            for (size_t i = 0; i < target_length; ++i)
            {
                data[i] = i % 255;
            }
            concurrency::streams::container_buffer<std::vector<uint8_t>> upload_buffer(data);
            blob.upload_from_stream(upload_buffer.create_istream(), azure::storage::access_condition(), option, m_context);

            // download target blob in parallel.
            azure::storage::operation_context context;
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;
            blob.download_to_stream(download_buffer.create_ostream(), azure::storage::access_condition(), option, context);

            check_parallelism(context, 1);
            CHECK(blob.properties().size() == target_length);
            CHECK(download_buffer.collection().size() == target_length);
            CHECK(std::equal(data.begin(), data.end(), download_buffer.collection().begin()));
        }

        // blob with size larger than 32MB.
        {
            auto blob_name = get_random_string(20);
            auto blob = m_container.get_block_blob_reference(blob_name);
            size_t target_length = 100 * 1024 * 1024;
            azure::storage::blob_request_options option;
            option.set_parallelism_factor(2);
            std::vector<uint8_t> data;
            data.resize(target_length);
            for (size_t i = 0; i < target_length; ++i)
            {
                data[i] = i % 255;
            }
            concurrency::streams::container_buffer<std::vector<uint8_t>> upload_buffer(data);
            blob.upload_from_stream(upload_buffer.create_istream(), azure::storage::access_condition(), option, m_context);

            // download target blob in parallel.
            azure::storage::operation_context context;
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;
            blob.download_to_stream(download_buffer.create_ostream(), azure::storage::access_condition(), option, context);

            check_parallelism(context, 2);
            CHECK(blob.properties().size() == target_length);
            CHECK(download_buffer.collection().size() == target_length);
            CHECK(std::equal(data.begin(), data.end(), download_buffer.collection().begin()));
        }
    }

    /// <summary>
    /// Test parallel download wit offset
    /// </summary>
    TEST_FIXTURE(blob_test_base, parallel_download_with_offset)
    {
        // blob with size larger than 32MB.
        // With offset not zero.
        {
            auto blob_name = get_random_string(20);
            auto blob = m_container.get_block_blob_reference(blob_name);
            size_t target_length = 100 * 1024 * 1024;
            azure::storage::blob_request_options option;
            option.set_parallelism_factor(2);
            std::vector<uint8_t> data;
            data.resize(target_length);
            for (size_t i = 0; i < target_length; ++i)
            {
                data[i] = i % 255;
            }
            concurrency::streams::container_buffer<std::vector<uint8_t>> upload_buffer(data);
            blob.upload_from_stream(upload_buffer.create_istream(), azure::storage::access_condition(), option, m_context);

            // download target blob in parallel.
            azure::storage::operation_context context;
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;

            utility::size64_t actual_offset = rand() % 255 + 1;
            utility::size64_t actual_length = target_length - actual_offset;
            blob.download_range_to_stream(download_buffer.create_ostream(), actual_offset, actual_length, azure::storage::access_condition(), option, context);

            check_parallelism(context, 2);
            CHECK(blob.properties().size() == target_length);
            CHECK(download_buffer.collection().size() == actual_length);
            CHECK(std::equal(data.begin() + actual_offset, data.end(), download_buffer.collection().begin()));
        }

        // blob with size larger than 32MB.
        // With offset not zero, length = max.
        {
            auto blob_name = get_random_string(20);
            auto blob = m_container.get_block_blob_reference(blob_name);
            size_t target_length = 100 * 1024 * 1024;
            azure::storage::blob_request_options option;
            option.set_parallelism_factor(2);
            std::vector<uint8_t> data;
            data.resize(target_length);
            for (size_t i = 0; i < target_length; ++i)
            {
                data[i] = i % 255;
            }
            concurrency::streams::container_buffer<std::vector<uint8_t>> upload_buffer(data);
            blob.upload_from_stream(upload_buffer.create_istream(), azure::storage::access_condition(), option, m_context);

            // download target blob in parallel.
            azure::storage::operation_context context;
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;

            utility::size64_t actual_offset = rand() % 255 + 1;
            utility::size64_t actual_length = target_length - actual_offset;
            blob.download_range_to_stream(download_buffer.create_ostream(), actual_offset, std::numeric_limits<utility::size64_t>::max(), azure::storage::access_condition(), option, context);

            check_parallelism(context, 2);
            CHECK(blob.properties().size() == target_length);
            CHECK(download_buffer.collection().size() == actual_length);
            CHECK(std::equal(data.begin() + actual_offset, data.end(), download_buffer.collection().begin()));
        }
    }

    /// <summary>
    /// Test parallel download wit length too large
    /// </summary>
    TEST_FIXTURE(blob_test_base, parallel_download_with_length_too_large)
    {
        // blob with size larger than 32MB.
        // With offset not zero.
        {
            auto blob_name = get_random_string(20);
            auto blob = m_container.get_block_blob_reference(blob_name);
            size_t target_length = 100 * 1024 * 1024;
            azure::storage::blob_request_options option;
            option.set_parallelism_factor(10);
            std::vector<uint8_t> data;
            data.resize(target_length);
            for (size_t i = 0; i < target_length; ++i)
            {
                data[i] = i % 255;
            }
            concurrency::streams::container_buffer<std::vector<uint8_t>> upload_buffer(data);
            blob.upload_from_stream(upload_buffer.create_istream(), azure::storage::access_condition(), option, m_context);

            // download target blob in parallel.
            azure::storage::operation_context context;
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;

            utility::size64_t actual_offset = rand() % 255 + 1;
            utility::size64_t actual_length = target_length - actual_offset;
            blob.download_range_to_stream(download_buffer.create_ostream(), actual_offset, actual_length * 2, azure::storage::access_condition(), option, context);

            check_parallelism(context, 10);
            CHECK(blob.properties().size() == target_length);
            CHECK(download_buffer.collection().size() == actual_length);
            CHECK(std::equal(data.begin() + actual_offset, data.end(), download_buffer.collection().begin()));
        }
    }

    TEST_FIXTURE(blob_test_base, parallel_download_with_md5)
    {
        // transactional md5 enabled.
        // download blob smaller than 4MB.
        {
            auto blob_name = get_random_string(20);
            auto blob = m_container.get_block_blob_reference(blob_name);
            size_t target_length = 1 * 1024 * 1024;
            azure::storage::blob_request_options option;
            option.set_parallelism_factor(2);
            option.set_use_transactional_md5(true);
            std::vector<uint8_t> data;
            data.resize(target_length);
            for (size_t i = 0; i < target_length; ++i)
            {
                data[i] = i % 255;
            }
            concurrency::streams::container_buffer<std::vector<uint8_t>> upload_buffer(data);
            blob.upload_from_stream(upload_buffer.create_istream(), azure::storage::access_condition(), option, m_context);

            // download target blob in parallel.
            azure::storage::operation_context context;
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;
            blob.download_to_stream(download_buffer.create_ostream(), azure::storage::access_condition(), option, context);

            check_parallelism(context, 1);
            CHECK(blob.properties().size() == target_length);
            CHECK(download_buffer.collection().size() == target_length);
            CHECK(std::equal(data.begin(), data.end(), download_buffer.collection().begin()));
        }

        // download blob larger than 4MB.
        {
            auto blob_name = get_random_string(20);
            auto blob = m_container.get_block_blob_reference(blob_name);
            size_t target_length = 21 * 1024 * 1024;
            azure::storage::blob_request_options option;
            option.set_parallelism_factor(2);
            option.set_use_transactional_md5(true);
            std::vector<uint8_t> data;
            data.resize(target_length);
            for (size_t i = 0; i < target_length; ++i)
            {
                data[i] = i % 255;
            }
            concurrency::streams::container_buffer<std::vector<uint8_t>> upload_buffer(data);
            blob.upload_from_stream(upload_buffer.create_istream(), azure::storage::access_condition(), option, m_context);

            // download target blob in parallel.
            azure::storage::operation_context context;
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;
            blob.download_to_stream(download_buffer.create_ostream(), azure::storage::access_condition(), option, context);

            check_parallelism(context, 2);
            CHECK(blob.properties().size() == target_length);
            CHECK(download_buffer.collection().size() == target_length);
            CHECK(std::equal(data.begin(), data.end(), download_buffer.collection().begin()));
        }
    }

    TEST_FIXTURE(blob_test_base, parallel_download_empty_blob)
    {
        auto blob_name = get_random_string(20);
        auto blob = m_container.get_block_blob_reference(blob_name);
        size_t target_length = 0;
        azure::storage::blob_request_options option;
        option.set_parallelism_factor(2);
        option.set_use_transactional_md5(true);
        std::vector<uint8_t> data;
        data.resize(target_length);
        concurrency::streams::container_buffer<std::vector<uint8_t>> upload_buffer(data);
        blob.upload_from_stream(upload_buffer.create_istream(), azure::storage::access_condition(), option, m_context);

        // download target blob in parallel.
        azure::storage::operation_context context;
        concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;
        blob.download_to_stream(download_buffer.create_ostream(), azure::storage::access_condition(), option, context);

        check_parallelism(context, 1);
        CHECK(blob.properties().size() == target_length);
    }

    TEST_FIXTURE(blob_test_base, read_blob_with_invalid_if_none_match)
    {
        auto blob_name = get_random_string(20);
        auto blob = m_container.get_block_blob_reference(blob_name);
        blob.upload_text(_XPLATSTR("test"));

        azure::storage::operation_context context;
        azure::storage::access_condition condition;
        condition.set_if_none_match_etag(_XPLATSTR("*"));
        CHECK_THROW(blob.download_text(condition, azure::storage::blob_request_options(), context), azure::storage::storage_exception);
        CHECK_EQUAL(web::http::status_codes::BadRequest, context.request_results().back().http_status_code());
    }
}
