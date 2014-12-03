// -----------------------------------------------------------------------------------------
// <copyright file="cloud_page_blob_test.cpp" company="Microsoft">
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

#pragma region Fixture

void page_blob_test_base::check_page_ranges_equal(const std::vector<azure::storage::page_range>& page_ranges)
{
    auto downloaded_page_ranges = m_blob.download_page_ranges(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
    CHECK_EQUAL(page_ranges.size(), downloaded_page_ranges.size());
    for (size_t i = 0; i < page_ranges.size(); ++i)
    {
        CHECK_EQUAL(page_ranges[i].start_offset(), downloaded_page_ranges[i].start_offset());
        CHECK_EQUAL(page_ranges[i].end_offset(), downloaded_page_ranges[i].end_offset());
    }
}

#pragma endregion

SUITE(Blob)
{
    TEST_FIXTURE(page_blob_test_base, page_blob_create)
    {
        auto same_blob = m_container.get_page_blob_reference(m_blob.name());
        CHECK(!same_blob.exists(azure::storage::blob_request_options(), m_context));
        m_blob.create(1024, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(1024, m_blob.properties().size());
        CHECK_EQUAL(0, same_blob.properties().size());
        CHECK(same_blob.exists(azure::storage::blob_request_options(), m_context));
        CHECK_EQUAL(1024, same_blob.properties().size());
        m_blob.delete_blob(azure::storage::delete_snapshots_option::none, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(!same_blob.exists(azure::storage::blob_request_options(), m_context));
        CHECK_EQUAL(1024, same_blob.properties().size());
        m_blob.create(2048, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2048, m_blob.properties().size());
        CHECK_EQUAL(1024, same_blob.properties().size());
        CHECK(same_blob.exists(azure::storage::blob_request_options(), m_context));
        CHECK_EQUAL(2048, same_blob.properties().size());
    }

    TEST_FIXTURE(page_blob_test_base, page_blob_resize)
    {
        m_blob.properties().set_content_language(U("tr,en"));
        m_blob.create(1024, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(1024, m_blob.properties().size());

        m_blob.properties().set_content_language(U("en"));
        m_blob.resize(2048, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2048, m_blob.properties().size());

        CHECK_UTF8_EQUAL(U("en"), m_blob.properties().content_language());
        m_blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_UTF8_EQUAL(U("tr,en"), m_blob.properties().content_language());
    }

    TEST_FIXTURE(page_blob_test_base, page_blob_sequence_number)
    {
        m_blob.properties().set_content_language(U("tr,en"));
        m_blob.create(512, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(512, m_blob.properties().size());
        CHECK_EQUAL(0, m_blob.properties().page_blob_sequence_number());
        
        m_blob.properties().set_content_language(U("en"));
        m_blob.set_sequence_number(azure::storage::sequence_number::update(5), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(5, m_blob.properties().page_blob_sequence_number());

        m_blob.set_sequence_number(azure::storage::sequence_number::maximum(7), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(7, m_blob.properties().page_blob_sequence_number());

        m_blob.set_sequence_number(azure::storage::sequence_number::maximum(3), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(7, m_blob.properties().page_blob_sequence_number());

        m_blob.set_sequence_number(azure::storage::sequence_number::increment(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(8, m_blob.properties().page_blob_sequence_number());

        CHECK_UTF8_EQUAL(U("en"), m_blob.properties().content_language());
        m_blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_UTF8_EQUAL(U("tr,en"), m_blob.properties().content_language());
        CHECK_EQUAL(8, m_blob.properties().page_blob_sequence_number());

        m_blob.clear_pages(0, 512, azure::storage::access_condition::generate_if_sequence_number_equal_condition(8), azure::storage::blob_request_options(), m_context);
        m_blob.clear_pages(0, 512, azure::storage::access_condition::generate_if_sequence_number_less_than_or_equal_condition(8), azure::storage::blob_request_options(), m_context);
        m_blob.clear_pages(0, 512, azure::storage::access_condition::generate_if_sequence_number_less_than_or_equal_condition(9), azure::storage::blob_request_options(), m_context);
        m_blob.clear_pages(0, 512, azure::storage::access_condition::generate_if_sequence_number_less_than_condition(9), azure::storage::blob_request_options(), m_context);

        CHECK_THROW(m_blob.clear_pages(0, 512, azure::storage::access_condition::generate_if_sequence_number_equal_condition(7), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
        CHECK_THROW(m_blob.clear_pages(0, 512, azure::storage::access_condition::generate_if_sequence_number_less_than_or_equal_condition(7), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
        CHECK_THROW(m_blob.clear_pages(0, 512, azure::storage::access_condition::generate_if_sequence_number_less_than_or_equal_condition(7), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
        CHECK_THROW(m_blob.clear_pages(0, 512, azure::storage::access_condition::generate_if_sequence_number_less_than_condition(7), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
    }

    TEST_FIXTURE(page_blob_test_base, page_upload)
    {
        std::vector<uint8_t> buffer;
        buffer.resize(512);
        azure::storage::blob_request_options options;
        std::vector<azure::storage::page_range> pages;

        utility::string_t md5_header;
        m_context.set_sending_request([&md5_header] (web::http::http_request& request, azure::storage::operation_context)
        {
            if (!request.headers().match(web::http::header_names::content_md5, md5_header))
            {
                md5_header.clear();
            }
        });

        m_blob.create(1 * 1024 * 1024, azure::storage::access_condition(), options, m_context);

        options.set_use_transactional_md5(false);
        for (int i = 0; i < 3; ++i)
        {
            fill_buffer_and_get_md5(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            azure::storage::page_range range(i * 1024, i * 1024 + buffer.size() - 1);
            pages.push_back(range);
            m_blob.upload_pages(stream, range.start_offset(), utility::string_t(), azure::storage::access_condition(), options, m_context);
            CHECK_UTF8_EQUAL(utility::string_t(), md5_header);
        }

        check_page_ranges_equal(pages);

        options.set_use_transactional_md5(false);
        for (int i = 3; i < 6; ++i)
        {
            auto md5 = fill_buffer_and_get_md5(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            azure::storage::page_range range(i * 1536, i * 1536 + buffer.size() - 1);
            pages.push_back(range);
            m_blob.upload_pages(stream, range.start_offset(), md5, azure::storage::access_condition(), options, m_context);
            CHECK_UTF8_EQUAL(md5, md5_header);
        }

        check_page_ranges_equal(pages);

        options.set_use_transactional_md5(true);
        for (int i = 6; i < 9; ++i)
        {
            auto md5 = fill_buffer_and_get_md5(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            azure::storage::page_range range(i * 2048, i * 2048 + buffer.size() - 1);
            pages.push_back(range);
            m_blob.upload_pages(stream, range.start_offset(), utility::string_t(), azure::storage::access_condition(), options, m_context);
            CHECK_UTF8_EQUAL(md5, md5_header);
        }

        check_page_ranges_equal(pages);

        options.set_use_transactional_md5(true);
        fill_buffer_and_get_md5(buffer);
        auto stream = concurrency::streams::bytestream::open_istream(buffer);
        CHECK_THROW(m_blob.upload_pages(stream, 0, dummy_md5, azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);
        CHECK_UTF8_EQUAL(dummy_md5, md5_header);

        check_page_ranges_equal(pages);

        m_context.set_sending_request(std::function<void(web::http::http_request &, azure::storage::operation_context)>());
    }

    TEST_FIXTURE(page_blob_test_base, page_clear)
    {
        std::vector<uint8_t> buffer;
        buffer.resize(16 * 1024);
        azure::storage::blob_request_options options;

        fill_buffer_and_get_md5(buffer);
        auto stream = concurrency::streams::bytestream::open_istream(buffer);
        m_blob.upload_from_stream(stream, azure::storage::access_condition(), options, m_context);

        std::vector<azure::storage::page_range> pages;
        pages.push_back(azure::storage::page_range(0, 512 - 1));
        pages.push_back(azure::storage::page_range(512 * 2, 10 * 1024 - 1));
        pages.push_back(azure::storage::page_range(13 * 1024, buffer.size() - 1));

        for (size_t i = 1; i < pages.size(); i++)
        {
            int64_t start_offset = pages[i - 1].end_offset() + 1;
            int64_t length = pages[i].start_offset() - start_offset;
            m_blob.clear_pages(start_offset, length, azure::storage::access_condition(), options, m_context);
        }

        check_page_ranges_equal(pages);
    }

    TEST_FIXTURE(page_blob_test_base, page_blob_upload)
    {
        const size_t size = 6 * 1024 * 1024;
        azure::storage::blob_request_options options;
        options.set_use_transactional_md5(true);

        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 3, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_store_blob_content_md5(true);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 4, true), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_store_blob_content_md5(false);
        options.set_stream_write_size_in_bytes(1 * 1024 * 1024);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 7, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_parallelism_factor(4);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 7, false), 4);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_parallelism_factor(8);
        options.set_store_blob_content_md5(true);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 8, true), 6);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());
    }

    TEST_FIXTURE(page_blob_test_base, page_blob_upload_with_nonseekable)
    {
        const size_t size = 6 * 1024 * 1024;
        azure::storage::blob_request_options options;

        CHECK_THROW(upload_and_download(m_blob, size, 0, 0, false, options, 3, false), std::logic_error);
    }

    TEST_FIXTURE(page_blob_test_base, page_blob_upload_with_size)
    {
        const size_t buffer_size = 6 * 1024 * 1024;
        const size_t blob_size = 4 * 1024 * 1024;
        azure::storage::blob_request_options options;
        options.set_use_transactional_md5(true);

        const size_t buffer_offsets[2] = { 0, 1024 };
        for (auto buffer_offset : buffer_offsets)
        {
            options.set_stream_write_size_in_bytes(blob_size);
            options.set_store_blob_content_md5(false);
            options.set_parallelism_factor(1);
            check_parallelism(upload_and_download(m_blob, buffer_size, buffer_offset, blob_size, true, options, 2, false), 1);
            m_blob.delete_blob();
            m_blob.properties().set_content_md5(utility::string_t());

            options.set_store_blob_content_md5(true);
            check_parallelism(upload_and_download(m_blob, buffer_size, buffer_offset, blob_size, true, options, 3, true), 1);
            m_blob.delete_blob();
            m_blob.properties().set_content_md5(utility::string_t());

            options.set_stream_write_size_in_bytes(1 * 1024 * 1024);
            options.set_store_blob_content_md5(false);
            options.set_parallelism_factor(4);
            check_parallelism(upload_and_download(m_blob, buffer_size, buffer_offset, blob_size, true, options, 5, false), 4);
            m_blob.delete_blob();
            m_blob.properties().set_content_md5(utility::string_t());

            options.set_parallelism_factor(8);
            options.set_store_blob_content_md5(true);
            check_parallelism(upload_and_download(m_blob, buffer_size, buffer_offset, blob_size, true, options, 6, true), 4);
            m_blob.delete_blob();
            m_blob.properties().set_content_md5(utility::string_t());
        }
    }

    TEST_FIXTURE(page_blob_test_base, page_blob_upload_with_size_with_nonseekable)
    {
        const size_t buffer_size = 6 * 1024 * 1024;
        const size_t blob_size = 4 * 1024 * 1024;
        azure::storage::blob_request_options options;
        options.set_use_transactional_md5(true);

        const size_t buffer_offsets[2] = { 0, 1024 };
        for (auto buffer_offset : buffer_offsets)
        {
            options.set_stream_write_size_in_bytes(blob_size);
            options.set_store_blob_content_md5(false);
            options.set_parallelism_factor(1);
            check_parallelism(upload_and_download(m_blob, buffer_size, buffer_offset, blob_size, false, options, 2, false), 1);
            m_blob.delete_blob();
            m_blob.properties().set_content_md5(utility::string_t());

            options.set_store_blob_content_md5(true);
            check_parallelism(upload_and_download(m_blob, buffer_size, buffer_offset, blob_size, false, options, 3, true), 1);
            m_blob.delete_blob();
            m_blob.properties().set_content_md5(utility::string_t());

            options.set_stream_write_size_in_bytes(1 * 1024 * 1024);
            options.set_store_blob_content_md5(false);
            options.set_parallelism_factor(4);
            check_parallelism(upload_and_download(m_blob, buffer_size, buffer_offset, blob_size, false, options, 5, false), 4);
            m_blob.delete_blob();
            m_blob.properties().set_content_md5(utility::string_t());

            options.set_parallelism_factor(8);
            options.set_store_blob_content_md5(true);
            check_parallelism(upload_and_download(m_blob, buffer_size, buffer_offset, blob_size, false, options, 6, true), 4);
            m_blob.delete_blob();
            m_blob.properties().set_content_md5(utility::string_t());
        }
    }

    TEST_FIXTURE(page_blob_test_base, page_blob_upload_with_invalid_size)
    {
        const size_t buffer_size = 2 * 1024 * 1024;
        azure::storage::blob_request_options options;
        CHECK_THROW(upload_and_download(m_blob, buffer_size, 0, buffer_size + 1, true, options, 0, false), azure::storage::storage_exception);
        CHECK_THROW(upload_and_download(m_blob, buffer_size, 0, buffer_size + 1, false, options, 0, false), azure::storage::storage_exception);
        CHECK_THROW(upload_and_download(m_blob, buffer_size, 1024, buffer_size - 1023, true, options, 0, false), azure::storage::storage_exception);
        CHECK_THROW(upload_and_download(m_blob, buffer_size, 1024, buffer_size - 1023, false, options, 0, false), azure::storage::storage_exception);
    }

    TEST_FIXTURE(page_blob_test_base, page_blob_file_upload)
    {
        azure::storage::blob_request_options options;
        options.set_store_blob_content_md5(true);

        utility::string_t md5_header;
        m_context.set_sending_request([&md5_header] (web::http::http_request& request, azure::storage::operation_context)
        {
            if (!request.headers().match(U("x-ms-blob-content-md5"), md5_header))
            {
                md5_header.clear();
            }
        });

        temp_file invalid_file(1000);
        CHECK_THROW(m_blob.upload_from_file(invalid_file.path(), azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);

        temp_file file(1024);
        m_blob.upload_from_file(file.path(), azure::storage::access_condition(), options, m_context);
        CHECK_UTF8_EQUAL(file.content_md5(), md5_header);

        m_context.set_sending_request(std::function<void(web::http::http_request &, azure::storage::operation_context)>());

        temp_file file2(0);
        m_blob.download_to_file(file2.path(), azure::storage::access_condition(), options, m_context);

        concurrency::streams::container_buffer<std::vector<uint8_t>> original_file_buffer;
        auto original_file = concurrency::streams::file_stream<uint8_t>::open_istream(file.path()).get();
        original_file.read_to_end(original_file_buffer).wait();
        original_file.close().wait();

        concurrency::streams::container_buffer<std::vector<uint8_t>> downloaded_file_buffer;
        auto downloaded_file = concurrency::streams::file_stream<uint8_t>::open_istream(file2.path()).get();
        downloaded_file.read_to_end(downloaded_file_buffer).wait();
        downloaded_file.close().wait();

        CHECK_EQUAL(original_file_buffer.collection().size(), downloaded_file_buffer.collection().size());
        CHECK_ARRAY_EQUAL(original_file_buffer.collection(), downloaded_file_buffer.collection(), (int)downloaded_file_buffer.collection().size());

        m_blob.properties().set_content_md5(dummy_md5);
        m_blob.upload_properties();
        options.set_retry_policy(azure::storage::no_retry_policy());
        CHECK_THROW(m_blob.download_to_file(file2.path(), azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);
    }

    TEST_FIXTURE(page_blob_test_base, page_blob_constructor)
    {
        m_blob.create(0, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(!m_blob.properties().etag().empty());

        azure::storage::cloud_page_blob blob1(m_blob.uri());
        CHECK_UTF8_EQUAL(m_blob.name(), blob1.name());
        CHECK_UTF8_EQUAL(m_blob.uri().primary_uri().to_string(), blob1.uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.uri().secondary_uri().to_string(), blob1.uri().secondary_uri().to_string());
        CHECK(blob1.properties().etag().empty());

        azure::storage::cloud_blob blob2(m_blob);
        CHECK_UTF8_EQUAL(m_blob.name(), blob2.name());
        CHECK_UTF8_EQUAL(m_blob.uri().primary_uri().to_string(), blob2.uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.uri().secondary_uri().to_string(), blob2.uri().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.properties().etag(), blob2.properties().etag());

        azure::storage::cloud_page_blob blob3(blob2);
        CHECK_UTF8_EQUAL(m_blob.name(), blob3.name());
        CHECK_UTF8_EQUAL(m_blob.uri().primary_uri().to_string(), blob3.uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.uri().secondary_uri().to_string(), blob3.uri().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.properties().etag(), blob2.properties().etag());
    }

    TEST_FIXTURE(page_blob_test_base, page_blob_create_with_metadata)
    {
        m_blob.metadata()[U("key1")] = U("value1");
        m_blob.metadata()[U("key2")] = U("value2");
        m_blob.create(0, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto same_blob = m_container.get_page_blob_reference(m_blob.name());
        CHECK(same_blob.metadata().empty());
        same_blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2, same_blob.metadata().size());
        CHECK_UTF8_EQUAL(U("value1"), same_blob.metadata()[U("key1")]);
        CHECK_UTF8_EQUAL(U("value2"), same_blob.metadata()[U("key2")]);
    }

    TEST_FIXTURE(page_blob_test_base, page_blob_snapshot_metadata)
    {
        m_blob.metadata()[U("key1")] = U("value1");
        m_blob.metadata()[U("key2")] = U("value2");
        m_blob.create(0, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto snapshot1 = m_blob.create_snapshot(azure::storage::cloud_metadata(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2, snapshot1.metadata().size());
        CHECK_UTF8_EQUAL(U("value1"), snapshot1.metadata()[U("key1")]);
        CHECK_UTF8_EQUAL(U("value2"), snapshot1.metadata()[U("key2")]);

        azure::storage::cloud_page_blob snapshot1_clone(snapshot1.uri(), snapshot1.snapshot_time(), snapshot1.service_client().credentials());
        CHECK(snapshot1_clone.metadata().empty());
        snapshot1_clone.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2, snapshot1_clone.metadata().size());
        CHECK_UTF8_EQUAL(U("value1"), snapshot1_clone.metadata()[U("key1")]);
        CHECK_UTF8_EQUAL(U("value2"), snapshot1_clone.metadata()[U("key2")]);

        azure::storage::cloud_metadata snapshot_metadata;
        snapshot_metadata[U("key3")] = U("value1");
        snapshot_metadata[U("key4")] = U("value2");
        auto snapshot2 = m_blob.create_snapshot(snapshot_metadata, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2, snapshot1.metadata().size());
        CHECK_UTF8_EQUAL(U("value1"), snapshot2.metadata()[U("key3")]);
        CHECK_UTF8_EQUAL(U("value2"), snapshot2.metadata()[U("key4")]);

        azure::storage::cloud_page_blob snapshot2_clone(snapshot2.uri(), snapshot2.snapshot_time(), snapshot2.service_client().credentials());
        CHECK(snapshot2_clone.metadata().empty());
        snapshot2_clone.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2, snapshot2_clone.metadata().size());
        CHECK_UTF8_EQUAL(U("value1"), snapshot2_clone.metadata()[U("key3")]);
        CHECK_UTF8_EQUAL(U("value2"), snapshot2_clone.metadata()[U("key4")]);
    }

    TEST_FIXTURE(page_blob_test_base, page_blob_upload_maximum_execution_time)
    {
        std::chrono::seconds duration(10);

        std::vector<uint8_t> buffer;
        buffer.resize(2 * 1024 * 1024);

        azure::storage::blob_request_options options;
        options.set_maximum_execution_time(duration);
        options.set_stream_write_size_in_bytes(buffer.size() / 2);

        m_context.set_response_received([duration] (web::http::http_request&, const web::http::http_response&, azure::storage::operation_context)
        {
            std::this_thread::sleep_for(duration);
        });

        CHECK_THROW(m_blob.upload_from_stream(concurrency::streams::bytestream::open_istream(std::move(buffer)), azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);
        CHECK_EQUAL(2, m_context.request_results().size());

        m_context.set_response_received(std::function<void(web::http::http_request &, const web::http::http_response&, azure::storage::operation_context)>());
    }
}
