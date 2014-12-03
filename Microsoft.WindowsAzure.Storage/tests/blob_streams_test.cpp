// -----------------------------------------------------------------------------------------
// <copyright file="blob_streams_test.cpp" company="Microsoft">
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

size_t seek_read_and_compare(concurrency::streams::istream stream, std::vector<uint8_t> buffer_to_compare, utility::size64_t offset, size_t count, size_t expected_read_count)
{
    std::vector<uint8_t> buffer;
    buffer.resize(count);
    stream.seek(offset);
    auto read_count = stream.streambuf().getn(buffer.data(), count).get();
    CHECK_EQUAL(expected_read_count, read_count);
    CHECK_ARRAY_EQUAL(buffer_to_compare.data() + offset, buffer.data(), (int)read_count);
    return read_count;
}

SUITE(Blob)
{
    TEST_FIXTURE(block_blob_test_base, blob_read_stream_download)
    {
        azure::storage::blob_request_options options;
        options.set_stream_read_size_in_bytes(1 * 1024 * 1024);
        options.set_use_transactional_md5(true);

        std::vector<uint8_t> buffer;
        buffer.resize(3 * 1024 * 1024);
        fill_buffer_and_get_md5(buffer);
        m_blob.upload_from_stream(concurrency::streams::bytestream::open_istream(buffer), azure::storage::access_condition(), options, m_context);

        concurrency::streams::container_buffer<std::vector<uint8_t>> output_buffer;

        auto stream = m_blob.open_read(azure::storage::access_condition(), options, m_context);
        stream.read_to_end(output_buffer).wait();
        stream.close();

        CHECK_EQUAL(buffer.size(), output_buffer.collection().size());
        CHECK_ARRAY_EQUAL(buffer, output_buffer.collection(), (int)output_buffer.collection().size());
    }

    TEST_FIXTURE(block_blob_test_base, blob_read_stream_etag_lock)
    {
        azure::storage::blob_request_options options;
        options.set_stream_read_size_in_bytes(1 * 1024 * 1024);

        std::vector<uint8_t> buffer;
        buffer.resize(2 * 1024 * 1024);
        fill_buffer_and_get_md5(buffer);
        m_blob.upload_from_stream(concurrency::streams::bytestream::open_istream(buffer), azure::storage::access_condition(), options, m_context);

        auto stream = m_blob.open_read(azure::storage::access_condition(), options, m_context);
        m_blob.upload_metadata(azure::storage::access_condition(), options, m_context);
        CHECK_THROW(stream.read().wait(), azure::storage::storage_exception);
        stream.close();

        auto condition = azure::storage::access_condition::generate_if_match_condition(U("*"));
        stream = m_blob.open_read(condition, options, m_context);
        m_blob.upload_metadata(azure::storage::access_condition(), options, m_context);
        CHECK_THROW(stream.read().wait(), azure::storage::storage_exception);
        stream.close();
    }

    TEST_FIXTURE(block_blob_test_base, blob_read_stream_seek)
    {
        azure::storage::blob_request_options options;
        options.set_stream_read_size_in_bytes(2 * 1024 * 1024);

        std::vector<uint8_t> buffer;
        buffer.resize(3 * 1024 * 1024);
        fill_buffer_and_get_md5(buffer);
        m_blob.upload_from_stream(concurrency::streams::bytestream::open_istream(buffer), azure::storage::access_condition(), options, m_context);

        auto stream = m_blob.open_read(azure::storage::access_condition(), options, m_context);
        CHECK(stream.can_seek());

        // Because container create, blob upload, and HEAD are also in the request results,
        // number of requests should start with 3.
        size_t attempts = 3;

        size_t position = 0;
        CHECK_EQUAL(position, stream.tell());
        position += seek_read_and_compare(stream, buffer, position, 1024, 1024);
        attempts++;
        CHECK_EQUAL(position, stream.tell());
        position += seek_read_and_compare(stream, buffer, position, 512, 512);
        CHECK_EQUAL(position, stream.tell());
        position = buffer.size() - 128;
        stream.seek(position);
        CHECK_EQUAL(position, stream.tell());
        position += seek_read_and_compare(stream, buffer, position, 1024, 128);
        attempts++;
        CHECK_EQUAL(position, stream.tell());
        position = 4096;
        stream.seek(position);
        CHECK_EQUAL(position, stream.tell());
        position += seek_read_and_compare(stream, buffer, position, 1024, 1024);
        attempts++;
        CHECK_EQUAL(position, stream.tell());
        position += 4096;
        stream.seek(position);
        CHECK_EQUAL(position, stream.tell());
        position += seek_read_and_compare(stream, buffer, position, 1024, 1024);
        CHECK_EQUAL(position, stream.tell());
        position -= 4096;
        stream.seek(position);
        CHECK_EQUAL(position, stream.tell());
        position += seek_read_and_compare(stream, buffer, position, 128, 128);
        CHECK_EQUAL(position, stream.tell());
        position = 2 * 1024 * 1024 + 4096 - 512;
        stream.seek(position);
        CHECK_EQUAL(position, stream.tell());
        position += seek_read_and_compare(stream, buffer, position, 1024, 512);
        attempts++;
        CHECK_EQUAL(position, stream.tell());
        position += seek_read_and_compare(stream, buffer, position, 1024, 1024);
        CHECK_EQUAL(position, stream.tell());
        position -= 1024;
        stream.seek(position);
        CHECK_EQUAL(position, stream.tell());
        position += seek_read_and_compare(stream, buffer, position, 2048, 2048);
        CHECK_EQUAL(position, stream.tell());
        position = buffer.size() - 128;
        stream.seek(position);
        CHECK_EQUAL(position, stream.tell());
        position += seek_read_and_compare(stream, buffer, position, 1024, 128);
        CHECK_EQUAL(position, stream.tell());

        CHECK_EQUAL(attempts, m_context.request_results().size());
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_write_stream_seek)
    {
        auto stream = m_blob.open_write(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(!stream.can_seek());
        CHECK_EQUAL(concurrency::streams::ostream::traits::eof(), stream.seek(0));
    }

    TEST_FIXTURE(page_blob_test_base, page_blob_write_stream_seek_with_md5)
    {
        azure::storage::blob_request_options options;
        options.set_store_blob_content_md5(true);

        auto stream = m_blob.open_write(16 * 1024, azure::storage::access_condition(), options, m_context);
        CHECK(!stream.can_seek());
        CHECK_EQUAL(concurrency::streams::ostream::traits::eof(), stream.seek(0));
    }

    TEST_FIXTURE(page_blob_test_base, page_blob_write_stream_seek_without_md5)
    {
        std::vector<uint8_t> buffer;
        std::vector<uint8_t> final_blob_contents;
        final_blob_contents.resize(16 * 1024);

        azure::storage::blob_request_options options;
        options.set_store_blob_content_md5(false);

        auto stream = m_blob.open_write(final_blob_contents.size(), azure::storage::access_condition(), options, m_context);
        CHECK(stream.can_seek());

        // Because container create and blob create are also in the request results,
        // number of requests should start with 2.
        size_t attempts = 2;

        buffer.resize(1024);
        fill_buffer_and_get_md5(buffer);
        stream.streambuf().putn(buffer.data(), buffer.size()).wait();

        std::copy(buffer.begin(), buffer.end(), final_blob_contents.begin());

        stream.seek(5 * 1024);
        attempts++;

        fill_buffer_and_get_md5(buffer);
        stream.streambuf().putn(buffer.data(), buffer.size()).wait();

        std::copy(buffer.begin(), buffer.end(), final_blob_contents.begin() + 5 * 1024);

        fill_buffer_and_get_md5(buffer);
        stream.streambuf().putn(buffer.data(), buffer.size()).wait();

        std::copy(buffer.begin(), buffer.end(), final_blob_contents.begin() + 6 * 1024);

        stream.seek(512);
        attempts++;

        fill_buffer_and_get_md5(buffer);
        stream.streambuf().putn(buffer.data(), buffer.size()).wait();

        std::copy(buffer.begin(), buffer.end(), final_blob_contents.begin() + 512);

        stream.close().wait();
        attempts++;

        CHECK_EQUAL(attempts, m_context.request_results().size());

        concurrency::streams::container_buffer<std::vector<uint8_t>> downloaded_blob;
        m_blob.download_to_stream(downloaded_blob.create_ostream(), azure::storage::access_condition(), options, m_context);

        CHECK_ARRAY_EQUAL(final_blob_contents, downloaded_blob.collection(), (int)final_blob_contents.size());
    }

    TEST_FIXTURE(page_blob_test_base, existing_page_blob_write_stream)
    {
        CHECK_THROW(m_blob.open_write(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);

        azure::storage::blob_request_options options;
        options.set_store_blob_content_md5(true);

        std::vector<uint8_t> buffer;
        buffer.resize(16 * 1024);
        fill_buffer_and_get_md5(buffer);
        m_blob.upload_from_stream(concurrency::streams::bytestream::open_istream(buffer), azure::storage::access_condition(), options, m_context);

        CHECK_THROW(m_blob.open_write(azure::storage::access_condition(), options, m_context), std::logic_error);

        options.set_store_blob_content_md5(false);
        auto stream = m_blob.open_write(azure::storage::access_condition(), options, m_context);
        stream.seek(512);
        stream.streambuf().putn(buffer.data(), 512).wait();
        stream.close().wait();

        concurrency::streams::container_buffer<std::vector<uint8_t>> downloaded_blob;
        CHECK_THROW(m_blob.download_to_stream(downloaded_blob.create_ostream(), azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);

        downloaded_blob.seekpos(0, std::ios_base::out);
        options.set_disable_content_md5_validation(true);
        m_blob.download_to_stream(downloaded_blob.create_ostream(), azure::storage::access_condition(), options, m_context);

        CHECK_ARRAY_EQUAL(buffer.data(), downloaded_blob.collection().data(), 512);
        CHECK_ARRAY_EQUAL(buffer.data(), downloaded_blob.collection().data() + 512, 512);
        CHECK_ARRAY_EQUAL(buffer.data() + 1024, downloaded_blob.collection().data() + 1024, (int)(buffer.size()) - 1024);
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_write_stream_access_condition)
    {
        m_blob.upload_block_list(std::vector<azure::storage::block_list_item>(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto missing_blob = m_container.get_block_blob_reference(U("missing_blob1"));
        CHECK_THROW(missing_blob.open_write(azure::storage::access_condition::generate_if_match_condition(m_blob.properties().etag()), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);

        missing_blob = m_container.get_block_blob_reference(U("missing_blob2"));
        missing_blob.open_write(azure::storage::access_condition::generate_if_none_match_condition(m_blob.properties().etag()), azure::storage::blob_request_options(), m_context).close().wait();

        missing_blob = m_container.get_block_blob_reference(U("missing_blob3"));
        missing_blob.open_write(azure::storage::access_condition::generate_if_none_match_condition(U("*")), azure::storage::blob_request_options(), m_context).close().wait();

        missing_blob = m_container.get_block_blob_reference(U("missing_blob4"));
        missing_blob.open_write(azure::storage::access_condition::generate_if_modified_since_condition(m_blob.properties().last_modified() + utility::datetime::from_minutes(1)), azure::storage::blob_request_options(), m_context).close().wait();

        missing_blob = m_container.get_block_blob_reference(U("missing_blob5"));
        missing_blob.open_write(azure::storage::access_condition::generate_if_not_modified_since_condition(m_blob.properties().last_modified() - utility::datetime::from_minutes(1)), azure::storage::blob_request_options(), m_context).close().wait();

        m_blob.open_write(azure::storage::access_condition::generate_if_match_condition(m_blob.properties().etag()), azure::storage::blob_request_options(), m_context).close().wait();

        CHECK_THROW(m_blob.open_write(azure::storage::access_condition::generate_if_match_condition(missing_blob.properties().etag()), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);

        m_blob.open_write(azure::storage::access_condition::generate_if_none_match_condition(missing_blob.properties().etag()), azure::storage::blob_request_options(), m_context).close().wait();

        CHECK_THROW(m_blob.open_write(azure::storage::access_condition::generate_if_none_match_condition(m_blob.properties().etag()), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);

        auto stream = m_blob.open_write(azure::storage::access_condition::generate_if_none_match_condition(U("*")), azure::storage::blob_request_options(), m_context);
        CHECK_THROW(stream.close().wait(), azure::storage::storage_exception);

        m_blob.open_write(azure::storage::access_condition::generate_if_modified_since_condition(m_blob.properties().last_modified() - utility::datetime::from_minutes(1)), azure::storage::blob_request_options(), m_context).close().wait();

        CHECK_THROW(m_blob.open_write(azure::storage::access_condition::generate_if_modified_since_condition(m_blob.properties().last_modified() + utility::datetime::from_minutes(1)), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);

        m_blob.open_write(azure::storage::access_condition::generate_if_not_modified_since_condition(m_blob.properties().last_modified() + utility::datetime::from_minutes(1)), azure::storage::blob_request_options(), m_context).close().wait();

        CHECK_THROW(m_blob.open_write(azure::storage::access_condition::generate_if_not_modified_since_condition(m_blob.properties().last_modified() - utility::datetime::from_minutes(1)), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);

        stream = m_blob.open_write(azure::storage::access_condition::generate_if_match_condition(m_blob.properties().etag()), azure::storage::blob_request_options(), m_context);
        m_blob.upload_properties(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_THROW(stream.close().wait(), azure::storage::storage_exception);

        missing_blob = m_container.get_block_blob_reference(U("missing_blob6"));
        stream = missing_blob.open_write(azure::storage::access_condition::generate_if_none_match_condition(U("*")), azure::storage::blob_request_options(), m_context);
        missing_blob.upload_block_list(std::vector<azure::storage::block_list_item>(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_THROW(stream.close().wait(), azure::storage::storage_exception);

        stream = m_blob.open_write(azure::storage::access_condition::generate_if_not_modified_since_condition(m_blob.properties().last_modified()), azure::storage::blob_request_options(), m_context);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        m_blob.upload_properties(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_THROW(stream.close().wait(), azure::storage::storage_exception);
    }


    TEST_FIXTURE(page_blob_test_base, page_blob_write_stream_access_condition)
    {
        m_blob.create(0, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto missing_blob = m_container.get_page_blob_reference(U("missing_blob1"));
        CHECK_THROW(missing_blob.open_write(0, azure::storage::access_condition::generate_if_match_condition(m_blob.properties().etag()), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);

        missing_blob = m_container.get_page_blob_reference(U("missing_blob2"));
        missing_blob.open_write(0, azure::storage::access_condition::generate_if_none_match_condition(m_blob.properties().etag()), azure::storage::blob_request_options(), m_context).close().wait();

        missing_blob = m_container.get_page_blob_reference(U("missing_blob3"));
        missing_blob.open_write(0, azure::storage::access_condition::generate_if_none_match_condition(U("*")), azure::storage::blob_request_options(), m_context).close().wait();

        missing_blob = m_container.get_page_blob_reference(U("missing_blob4"));
        missing_blob.open_write(0, azure::storage::access_condition::generate_if_modified_since_condition(m_blob.properties().last_modified() + utility::datetime::from_minutes(1)), azure::storage::blob_request_options(), m_context).close().wait();

        missing_blob = m_container.get_page_blob_reference(U("missing_blob5"));
        missing_blob.open_write(0, azure::storage::access_condition::generate_if_not_modified_since_condition(m_blob.properties().last_modified() - utility::datetime::from_minutes(1)), azure::storage::blob_request_options(), m_context).close().wait();

        m_blob.open_write(0, azure::storage::access_condition::generate_if_match_condition(m_blob.properties().etag()), azure::storage::blob_request_options(), m_context).close().wait();

        CHECK_THROW(m_blob.open_write(0, azure::storage::access_condition::generate_if_match_condition(missing_blob.properties().etag()), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);

        m_blob.open_write(0, azure::storage::access_condition::generate_if_none_match_condition(missing_blob.properties().etag()), azure::storage::blob_request_options(), m_context).close().wait();

        CHECK_THROW(m_blob.open_write(0, azure::storage::access_condition::generate_if_none_match_condition(m_blob.properties().etag()), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);

        CHECK_THROW(m_blob.open_write(0, azure::storage::access_condition::generate_if_none_match_condition(U("*")), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);

        m_blob.open_write(0, azure::storage::access_condition::generate_if_modified_since_condition(m_blob.properties().last_modified() - utility::datetime::from_minutes(1)), azure::storage::blob_request_options(), m_context).close().wait();

        CHECK_THROW(m_blob.open_write(0, azure::storage::access_condition::generate_if_modified_since_condition(m_blob.properties().last_modified() + utility::datetime::from_minutes(1)), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);

        m_blob.open_write(0, azure::storage::access_condition::generate_if_not_modified_since_condition(m_blob.properties().last_modified() + utility::datetime::from_minutes(1)), azure::storage::blob_request_options(), m_context).close().wait();

        CHECK_THROW(m_blob.open_write(0, azure::storage::access_condition::generate_if_not_modified_since_condition(m_blob.properties().last_modified() - utility::datetime::from_minutes(1)), azure::storage::blob_request_options(), m_context), azure::storage::storage_exception);
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_write_stream_maximum_execution_time)
    {
        std::chrono::seconds duration(10);

        azure::storage::blob_request_options options;
        options.set_maximum_execution_time(duration);

        auto stream = m_blob.open_write(azure::storage::access_condition(), options, m_context);

        std::this_thread::sleep_for(duration);

        stream.write(static_cast<uint8_t>(0)).wait();
        stream.close().wait();
    }

    TEST_FIXTURE(page_blob_test_base, page_blob_write_stream_maximum_execution_time)
    {
        std::chrono::seconds duration(10);

        azure::storage::blob_request_options options;
        options.set_maximum_execution_time(duration);

        auto stream = m_blob.open_write(512, azure::storage::access_condition(), options, m_context);

        std::this_thread::sleep_for(duration);

        std::vector<uint8_t> buffer;
        buffer.resize(512);

        stream.streambuf().putn(buffer.data(), buffer.size()).wait();
        stream.close().wait();
    }

    TEST_FIXTURE(page_blob_test_base, existing_page_blob_write_stream_maximum_execution_time)
    {
        std::chrono::seconds duration(10);

        azure::storage::blob_request_options options;
        options.set_maximum_execution_time(duration);

        m_blob.create(512, azure::storage::access_condition(), options, m_context);
        auto stream = m_blob.open_write(azure::storage::access_condition(), options, m_context);

        std::this_thread::sleep_for(duration);

        std::vector<uint8_t> buffer;
        buffer.resize(512);

        stream.streambuf().putn(buffer.data(), buffer.size()).wait();
        stream.close().wait();
    }

    TEST_FIXTURE(block_blob_test_base, blob_read_stream_maximum_execution_time)
    {
        std::chrono::seconds duration(10);

        azure::storage::blob_request_options options;
        options.set_maximum_execution_time(duration);

        m_blob.upload_text(U("test"), azure::storage::access_condition(), options, m_context);
        auto stream = m_blob.open_read(azure::storage::access_condition(), options, m_context);

        std::this_thread::sleep_for(duration);

        stream.read().wait();
        stream.close().wait();
    }
}
