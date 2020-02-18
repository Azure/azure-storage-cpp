// -----------------------------------------------------------------------------------------
// <copyright file="cloud_append_blob_test.cpp" company="Microsoft">
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
#include "was/crc64.h"
#include "wascore/constants.h"

#pragma region Fixture

#pragma endregion

SUITE(Blob)
{
    TEST_FIXTURE(append_blob_test_base, append_block)
    {
        const size_t buffer_size = 16 * 1024;
        std::vector<uint8_t> buffer;
        buffer.resize(buffer_size);
        azure::storage::blob_request_options options;

        utility::string_t md5_header;
        utility::string_t crc64_header;
        m_context.set_sending_request([&md5_header, &crc64_header](web::http::http_request& request, azure::storage::operation_context)
        {
            if (!request.headers().match(web::http::header_names::content_md5, md5_header))
            {
                md5_header.clear();
            }
            if (!request.headers().match(azure::storage::protocol::ms_header_content_crc64, crc64_header))
            {
                crc64_header.clear();
            }
        });

        m_blob.create_or_replace(azure::storage::access_condition(), options, m_context);
        check_blob_no_stale_property(m_blob);

        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(false);
        for (uint16_t i = 0; i < 3; ++i)
        {
            fill_buffer(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            int64_t offset = m_blob.append_block(stream, utility::string_t(), azure::storage::access_condition(), options, m_context);
            CHECK_UTF8_EQUAL(utility::string_t(), md5_header);
            CHECK_UTF8_EQUAL(utility::string_t(), crc64_header);
            CHECK_EQUAL(i * buffer_size, offset);
            CHECK_EQUAL(i + 1, m_blob.properties().append_blob_committed_block_count());
        }

        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(false);
        for (uint16_t i = 3; i < 6; ++i)
        {
            auto md5 = fill_buffer_and_get_md5(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            int64_t offset = m_blob.append_block(stream, md5, azure::storage::access_condition(), options, m_context);
            CHECK_UTF8_EQUAL(md5, md5_header);
            CHECK_EQUAL(i * buffer_size, offset);
            CHECK_EQUAL(i + 1, m_blob.properties().append_blob_committed_block_count());
        }

        options.set_use_transactional_md5(true);
        options.set_use_transactional_crc64(false);
        for (uint16_t i = 6; i < 9; ++i)
        {
            auto md5 = fill_buffer_and_get_md5(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            int64_t offset = m_blob.append_block(stream, utility::string_t(), azure::storage::access_condition(), options, m_context);
            CHECK_UTF8_EQUAL(md5, md5_header);
            CHECK_EQUAL(i * buffer_size, offset);
            CHECK_EQUAL(i + 1, m_blob.properties().append_blob_committed_block_count());
        }

        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(true);
        for (uint16_t i = 9; i < 12; ++i)
        {
            auto crc64 = fill_buffer_and_get_crc64(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            int64_t offset = m_blob.append_block(stream, azure::storage::checksum_none, azure::storage::access_condition(), options, m_context);
            CHECK_UTF8_EQUAL(crc64, crc64_header);
            CHECK_EQUAL(i * buffer_size, offset);
            CHECK_EQUAL(i + 1, m_blob.properties().append_blob_committed_block_count());
        }

        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(false);
        for (uint16_t i = 12; i < 15; ++i)
        {
            auto crc64 = fill_buffer_and_get_crc64(buffer);
            uint64_t crc64_val = azure::storage::crc64(buffer.data(), buffer.size());
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            int64_t offset = m_blob.append_block(stream, crc64_val, azure::storage::access_condition(), options, m_context);
            CHECK_UTF8_EQUAL(crc64, crc64_header);
            CHECK_EQUAL(i * buffer_size, offset);
            CHECK_EQUAL(i + 1, m_blob.properties().append_blob_committed_block_count());
        }

        // block stream with length = 0
        options.set_use_transactional_md5(true);
        options.set_use_transactional_crc64(false);
        fill_buffer(buffer);
        auto stream1 = concurrency::streams::bytestream::open_istream(buffer);
        stream1.seek(buffer.size());
        CHECK_THROW(m_blob.append_block(stream1, utility::string_t(), azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);

        options.set_use_transactional_md5(true);
        options.set_use_transactional_crc64(false);
        fill_buffer(buffer);
        auto stream = concurrency::streams::bytestream::open_istream(buffer);
        CHECK_THROW(m_blob.append_block(stream, dummy_md5, azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);
        CHECK_UTF8_EQUAL(dummy_md5, md5_header);

        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(true);
        fill_buffer(buffer);
        stream = concurrency::streams::bytestream::open_istream(buffer);
        CHECK_THROW(m_blob.append_block(stream, dummy_crc64_val, azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);
        CHECK_UTF8_EQUAL(dummy_crc64, crc64_header);

        m_context.set_sending_request(std::function<void(web::http::http_request &, azure::storage::operation_context)>());
    }

    TEST_FIXTURE(append_blob_test_base, append_block_size)
    {
        const size_t buffer_size = 8 * 1024 * 1024;
        std::vector<uint8_t> buffer;
        buffer.reserve(buffer_size);

        azure::storage::blob_request_options options;
        m_blob.create_or_replace(azure::storage::access_condition(), options, m_context);
        check_blob_no_stale_property(m_blob);

        size_t sizes[] = { 1, 2, 1023, 1024, 4 * 1024, 1024 * 1024, azure::storage::protocol::max_append_block_size - 1, azure::storage::protocol::max_append_block_size };
        size_t invalid_sizes[] = { azure::storage::protocol::max_append_block_size + 1, 6 * 1024 * 1024, 8 * 1024 * 1024 };
        int64_t bytes_appended = 0;

        options.set_use_transactional_md5(true);
        options.set_use_transactional_crc64(false);
        for (size_t size : sizes)
        {
            buffer.resize(size);
            auto md5 = fill_buffer_and_get_md5(buffer, 0, size);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            int64_t offset = m_blob.append_block(stream, md5, azure::storage::access_condition(), options, m_context);
            CHECK_EQUAL(bytes_appended, offset);

            bytes_appended += size;
        }

        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(true);
        for (size_t size : sizes)
        {
            buffer.resize(size);
            fill_buffer(buffer, 0, size);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            int64_t offset = m_blob.append_block(stream, azure::storage::checksum_none, azure::storage::access_condition(), options, m_context);
            CHECK_EQUAL(bytes_appended, offset);

            bytes_appended += size;
        }

        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(false);
        for (size_t size : invalid_sizes)
        {
            buffer.resize(size);
            fill_buffer(buffer, 0, size);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            CHECK_THROW(m_blob.append_block(stream, azure::storage::checksum_none, azure::storage::access_condition(), options, m_context), std::invalid_argument);
        }
    }

    TEST_FIXTURE(append_blob_test_base, append_block_max_size_condition)
    {
        const size_t buffer_size =  64 * 1024;
        std::vector<uint8_t> buffer;
        buffer.resize(buffer_size);
        fill_buffer(buffer);

        azure::storage::blob_request_options options;
        options.set_use_transactional_md5(false);

        int64_t max_sizes1[] = {1, 1024, buffer_size - 1};
        for (int64_t max_size : max_sizes1)
        {
            m_blob.create_or_replace(azure::storage::access_condition(), options, m_context);
            check_blob_no_stale_property(m_blob);

            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            CHECK_THROW(m_blob.append_block(stream, utility::string_t(), azure::storage::access_condition::generate_if_max_size_less_than_or_equal_condition(max_size), options, m_context), azure::storage::storage_exception);
            m_blob.delete_blob();
        }

        int64_t max_sizes2[] = { buffer_size, 2 * buffer_size, std::numeric_limits<int64_t>::max() };
        for (int64_t max_size : max_sizes2)
        {
            m_blob.create_or_replace(azure::storage::access_condition(), options, m_context);
            check_blob_no_stale_property(m_blob);

            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            int64_t offset = m_blob.append_block(stream, utility::string_t(), azure::storage::access_condition::generate_if_max_size_less_than_or_equal_condition(max_size), options, m_context);
            CHECK_EQUAL(0, offset);
            m_blob.delete_blob();
        }

        int64_t blob_size = 0;
        int block_count = 0;
        m_blob.create_or_replace(azure::storage::access_condition(), options, m_context);
        check_blob_no_stale_property(m_blob);

        for (uint16_t i = 0; i < 3; ++i)
        {
            fill_buffer(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            int64_t offset = m_blob.append_block(stream, utility::string_t(), azure::storage::access_condition(), options, m_context);
            block_count++;
            CHECK_EQUAL(blob_size, offset);
            CHECK_EQUAL(block_count, m_blob.properties().append_blob_committed_block_count());
            blob_size += buffer.size();
        }

        int64_t max_sizes3[] = { 3 * buffer_size - 1, 3 * buffer_size, 3 * buffer_size + 1, 4 * buffer_size - 1 };
        for (int64_t max_size : max_sizes3)
        {
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            CHECK_THROW(m_blob.append_block(stream, utility::string_t(), azure::storage::access_condition::generate_if_max_size_less_than_or_equal_condition(max_size), options, m_context), azure::storage::storage_exception);
        }

        int64_t max_sizes4[] = { 4 * buffer_size, std::numeric_limits<int64_t>::max() };
        for (int64_t max_size : max_sizes4)
        {
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            int64_t offset = m_blob.append_block(stream, utility::string_t(), azure::storage::access_condition::generate_if_max_size_less_than_or_equal_condition(max_size), options, m_context);
            block_count++;
            CHECK_EQUAL(blob_size, offset);
            CHECK_EQUAL(block_count, m_blob.properties().append_blob_committed_block_count());
            blob_size += buffer.size();
        }

        m_blob.delete_blob();
    }

    TEST_FIXTURE(append_blob_test_base, append_block_append_position_condition)
    {
        const size_t buffer_size = 64 * 1024;
        std::vector<uint8_t> buffer;
        buffer.resize(buffer_size);

        azure::storage::blob_request_options options;
        options.set_use_transactional_md5(true);

        m_blob.create_or_replace(azure::storage::access_condition(), options, m_context);
        check_blob_no_stale_property(m_blob);

        int64_t invalid_appendpos[] = { 1, 2, buffer_size, buffer_size + 1, std::numeric_limits<int64_t>::max() };
        for (int64_t appendpos : invalid_appendpos)
        {
            auto md5 = fill_buffer_and_get_md5(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            CHECK_THROW(m_blob.append_block(stream, md5, azure::storage::access_condition::generate_if_append_position_equal_condition(appendpos), options, m_context), azure::storage::storage_exception);
        }

        for (int16_t i = 0; i < 3; i++)
        {
            auto md5 = fill_buffer_and_get_md5(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            int64_t offset = m_blob.append_block(stream, md5, azure::storage::access_condition::generate_if_append_position_equal_condition(i * buffer_size), options, m_context);
            CHECK_EQUAL(offset, i * buffer_size);
            CHECK_EQUAL(i + 1, m_blob.properties().append_blob_committed_block_count());
        }

        int64_t invalid_appendpos2[] = { buffer_size * 3 - 1, buffer_size * 3 + 1};
        for (int64_t appendpos : invalid_appendpos2)
        {
            auto md5 = fill_buffer_and_get_md5(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            CHECK_THROW(m_blob.append_block(stream, md5, azure::storage::access_condition::generate_if_append_position_equal_condition(appendpos), options, m_context), azure::storage::storage_exception);
        }

        auto md5 = fill_buffer_and_get_md5(buffer);
        auto stream = concurrency::streams::bytestream::open_istream(buffer);
        CHECK_EQUAL(3 * buffer_size, m_blob.append_block(stream, md5, azure::storage::access_condition::generate_if_append_position_equal_condition(3 * buffer_size), options, m_context));
        CHECK_EQUAL(4, m_blob.properties().append_blob_committed_block_count());

        m_blob.delete_blob();
    }

    TEST_FIXTURE(append_blob_test_base, append_blob_upload)
    {
        const size_t buffer_size = 6 * 1024 * 1024;
        azure::storage::blob_request_options options;
        options.set_use_transactional_md5(true);

        const size_t buffer_offsets[2] = { 0, 1024 };
        for (auto buffer_offset : buffer_offsets)
        {
            upload_and_download(m_blob, buffer_size, buffer_offset, 0, true, options, 3, false);
            m_blob.delete_blob();
            m_blob.properties().set_content_md5(utility::string_t());
        }
    }

    TEST_FIXTURE(append_blob_test_base, append_blob_upload_with_nonseekable)
    {
        const size_t buffer_size = 6 * 1024 * 1024;
        azure::storage::blob_request_options options;
        options.set_use_transactional_md5(true);

        const size_t buffer_offsets[2] = { 0, 1024 };
        for (auto buffer_offset : buffer_offsets)
        {
            upload_and_download(m_blob, buffer_size, buffer_offset, 0, false, options, 3, false);
            m_blob.delete_blob();
            m_blob.properties().set_content_md5(utility::string_t());
        }
    }

    TEST_FIXTURE(append_blob_test_base, append_blob_append)
    {
        const size_t file_buffer_size = 24 * 1024 * 1024 + 6;
        std::vector<uint8_t> file_buffer;
        file_buffer.resize(file_buffer_size);

        azure::storage::blob_request_options options;
        options.set_use_transactional_md5(false);

        utility::string_t md5_header;
        m_context.set_sending_request([&md5_header](web::http::http_request& request, azure::storage::operation_context)
        {
            if (!request.headers().match(web::http::header_names::content_md5, md5_header))
            {
                md5_header.clear();
            }
        });

        m_blob.create_or_replace(azure::storage::access_condition(), options, m_context);
        check_blob_no_stale_property(m_blob);

        int block_count = 0;

        // append stream (4M, 4M)
        const size_t buffer_offsets1[2] = { 0, 4 * 1024 * 1024};
        for (uint16_t i = 0; i < 2; ++i)
        {
            std::vector<uint8_t> buffer;
            buffer.resize(4 * 1024 * 1024);
            fill_buffer(buffer);
            std::copy(buffer.begin(), buffer.end(), file_buffer.begin() + buffer_offsets1[i]);

            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            azure::storage::access_condition condition = azure::storage::access_condition::generate_if_append_position_equal_condition(buffer_offsets1[i]);
            m_blob.append_from_stream(stream, condition, options, m_context);
            CHECK_UTF8_EQUAL(utility::string_t(), md5_header);

            block_count++;
            CHECK_EQUAL(block_count, m_blob.properties().append_blob_committed_block_count());
        }

        // append stream with length (2M, 2M, 2M)
        const size_t buffer_offsets2[3] = { 8 * 1024 * 1024,  10 * 1024 * 1024, 12 * 1024 * 1024 };
        for (uint16_t i = 0; i < 3; ++i)
        {
            std::vector<uint8_t> buffer;
            buffer.resize(4 * 1024 * 1024);
            fill_buffer(buffer);
            std::copy(buffer.begin(), buffer.begin() + 2 * 1024 * 1024, file_buffer.begin() + buffer_offsets2[i]);

            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            m_blob.append_from_stream(stream, 2 * 1024 * 1024, azure::storage::access_condition(), options, m_context);
            CHECK_UTF8_EQUAL(utility::string_t(), md5_header);

            block_count++;
            CHECK_EQUAL(block_count, m_blob.properties().append_blob_committed_block_count());
        }

        // append file (5M, 5M)
        const size_t buffer_offsets3[2] = { 14 * 1024 * 1024, 19 * 1024 * 1024 };
        for (uint16_t i = 0; i < 2; ++i)
        {
            std::vector<uint8_t> buffer;
            buffer.resize(5 * 1024 * 1024);
            fill_buffer(buffer);
            std::copy(buffer.begin(), buffer.end(), file_buffer.begin() + buffer_offsets3[i]);

            // create a temporary test file
            utility::string_t tmp_file_path = get_random_container_name(8);
            auto stream = concurrency::streams::file_stream<uint8_t>::open_ostream(tmp_file_path).get();
            stream.streambuf().putn_nocopy(buffer.data(), buffer.size()).wait();
            stream.close().wait();

            // append from file
            m_blob.append_from_file(tmp_file_path, azure::storage::access_condition(), options, m_context);

            // remote the temporary test file
            std::remove(utility::conversions::to_utf8string(tmp_file_path).c_str());

            block_count  += 2;
            CHECK_EQUAL(block_count, m_blob.properties().append_blob_committed_block_count());
        }

        // append text (1, 5)
        const size_t buffer_offsets4[2] = { 24 * 1024 * 1024, 24 * 1024 * 1024 + 1};
        {
            utility::string_t text1 = _XPLATSTR("1");
            std::string text1_copy = utility::conversions::to_utf8string(text1);
            std::copy(text1_copy.begin(), text1_copy.end(), file_buffer.begin() + buffer_offsets4[0]);
            m_blob.append_text(text1);
            block_count++;
            CHECK_EQUAL(block_count, m_blob.properties().append_blob_committed_block_count());

            utility::string_t text2 = _XPLATSTR("test2");
            std::string text2_copy = utility::conversions::to_utf8string(text2);
            std::copy(text2_copy.begin(), text2_copy.end(), file_buffer.begin() + buffer_offsets4[1]);
            m_blob.append_text(text2);
            block_count++;
            CHECK_EQUAL(block_count, m_blob.properties().append_blob_committed_block_count());
        }

        // download the blob
        concurrency::streams::container_buffer<std::vector<uint8_t>> downloaded_blob;
        m_blob.download_to_stream(downloaded_blob.create_ostream(), azure::storage::access_condition(), options, m_context);

        CHECK_ARRAY_EQUAL(file_buffer, downloaded_blob.collection(), (int)file_buffer.size());

        m_blob.delete_blob();

        m_context.set_sending_request(std::function<void(web::http::http_request &, azure::storage::operation_context)>());
    }

    TEST_FIXTURE(append_blob_test_base, append_blob_constructor)
    {
        m_blob.create_or_replace(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        check_blob_no_stale_property(m_blob);
        CHECK(!m_blob.properties().etag().empty());

        azure::storage::cloud_append_blob blob1(m_blob.uri());
        CHECK_UTF8_EQUAL(m_blob.name(), blob1.name());
        CHECK_UTF8_EQUAL(m_blob.uri().primary_uri().to_string(), blob1.uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.uri().secondary_uri().to_string(), blob1.uri().secondary_uri().to_string());
        CHECK(blob1.properties().etag().empty());

        azure::storage::cloud_blob blob2(m_blob);
        CHECK_UTF8_EQUAL(m_blob.name(), blob2.name());
        CHECK_UTF8_EQUAL(m_blob.uri().primary_uri().to_string(), blob2.uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.uri().secondary_uri().to_string(), blob2.uri().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.properties().etag(), blob2.properties().etag());

        azure::storage::cloud_append_blob blob3(blob2);
        CHECK_UTF8_EQUAL(m_blob.name(), blob3.name());
        CHECK_UTF8_EQUAL(m_blob.uri().primary_uri().to_string(), blob3.uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.uri().secondary_uri().to_string(), blob3.uri().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.properties().etag(), blob2.properties().etag());
    }

    TEST_FIXTURE(append_blob_test_base, append_blob_create)
    {
        auto same_blob = m_container.get_append_blob_reference(m_blob.name());
        CHECK(!same_blob.exists(azure::storage::blob_request_options(), m_context));
        CHECK_EQUAL(0U, same_blob.properties().size());
        CHECK(same_blob.properties().etag().empty());

        m_blob.create_or_replace(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        check_blob_no_stale_property(m_blob);
        CHECK_EQUAL(0U, m_blob.properties().size());
        CHECK(!m_blob.properties().etag().empty());
        CHECK(same_blob.exists(azure::storage::blob_request_options(), m_context));
        CHECK_EQUAL(0U, same_blob.properties().size());
        CHECK(!same_blob.properties().etag().empty());

        m_blob.delete_blob(azure::storage::delete_snapshots_option::none, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(!m_blob.exists(azure::storage::blob_request_options(), m_context));
        CHECK(!same_blob.exists(azure::storage::blob_request_options(), m_context));
        CHECK(!m_blob.properties().etag().empty());

        m_blob.create_or_replace(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        check_blob_no_stale_property(m_blob);
        CHECK_EQUAL(0U, m_blob.properties().size());
        CHECK(!m_blob.properties().etag().empty());
        CHECK(same_blob.exists(azure::storage::blob_request_options(), m_context));
        CHECK_EQUAL(0U, same_blob.properties().size());
        CHECK(!same_blob.properties().etag().empty());
    }

    TEST_FIXTURE(append_blob_test_base, append_blob_create_with_metadata)
    {
        m_blob.metadata()[_XPLATSTR("key1")] = _XPLATSTR("value1");
        m_blob.metadata()[_XPLATSTR("key2")] = _XPLATSTR("value2");
        m_blob.create_or_replace(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        check_blob_no_stale_property(m_blob);

        auto same_blob = m_container.get_append_blob_reference(m_blob.name());
        CHECK(same_blob.metadata().empty());
        same_blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2U, same_blob.metadata().size());
        CHECK_UTF8_EQUAL(_XPLATSTR("value1"), same_blob.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value2"), same_blob.metadata()[_XPLATSTR("key2")]);
    }

    TEST_FIXTURE(append_blob_test_base, append_blob_snapshot_metadata)
    {
        m_blob.metadata()[_XPLATSTR("key1")] = _XPLATSTR("value1");
        m_blob.metadata()[_XPLATSTR("key2")] = _XPLATSTR("value2");
        m_blob.create_or_replace(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        check_blob_no_stale_property(m_blob);

        auto snapshot1 = m_blob.create_snapshot(azure::storage::cloud_metadata(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        check_blob_no_stale_property(m_blob);
        CHECK_EQUAL(2U, snapshot1.metadata().size());
        CHECK_UTF8_EQUAL(_XPLATSTR("value1"), snapshot1.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value2"), snapshot1.metadata()[_XPLATSTR("key2")]);

        azure::storage::cloud_append_blob snapshot1_clone(snapshot1.uri(), snapshot1.snapshot_time(), snapshot1.service_client().credentials());
        CHECK(snapshot1_clone.metadata().empty());
        snapshot1_clone.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2U, snapshot1_clone.metadata().size());
        CHECK_UTF8_EQUAL(_XPLATSTR("value1"), snapshot1_clone.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value2"), snapshot1_clone.metadata()[_XPLATSTR("key2")]);

        azure::storage::cloud_metadata snapshot_metadata;
        snapshot_metadata[_XPLATSTR("key3")] = _XPLATSTR("value1");
        snapshot_metadata[_XPLATSTR("key4")] = _XPLATSTR("value2");
        auto snapshot2 = m_blob.create_snapshot(snapshot_metadata, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2U, snapshot1.metadata().size());
        check_blob_no_stale_property(m_blob);
        CHECK_UTF8_EQUAL(_XPLATSTR("value1"), snapshot2.metadata()[_XPLATSTR("key3")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value2"), snapshot2.metadata()[_XPLATSTR("key4")]);

        azure::storage::cloud_append_blob snapshot2_clone(snapshot2.uri(), snapshot2.snapshot_time(), snapshot2.service_client().credentials());
        CHECK(snapshot2_clone.metadata().empty());
        snapshot2_clone.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2U, snapshot2_clone.metadata().size());
        CHECK_UTF8_EQUAL(_XPLATSTR("value1"), snapshot2_clone.metadata()[_XPLATSTR("key3")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value2"), snapshot2_clone.metadata()[_XPLATSTR("key4")]);
    }

    TEST_FIXTURE(append_blob_test_base, append_blob_upload_max_size_condition)
    {
        const size_t buffer_size = 1024 * 1024;

        std::vector<uint8_t> buffer;
        buffer.resize(buffer_size);
        fill_buffer(buffer);
        concurrency::streams::istream stream = concurrency::streams::bytestream::open_istream(buffer);

        auto condition = azure::storage::access_condition::generate_if_max_size_less_than_or_equal_condition(512);
        CHECK_THROW(m_blob.upload_from_stream(stream, condition, azure::storage::blob_request_options(), m_context), std::invalid_argument);
    }

    TEST_FIXTURE(append_blob_test_base, append_block_stale_properties)
    {
        azure::storage::blob_request_options options;
        azure::storage::operation_context op = m_context;

        m_blob.create_or_replace(azure::storage::access_condition(), options, m_context);
        check_blob_no_stale_property(m_blob);
        auto lease_id = utility::uuid_to_string(utility::new_uuid());
        m_blob.acquire_lease(azure::storage::lease_time(std::chrono::seconds(60)), lease_id);
        m_blob.download_attributes(azure::storage::access_condition::generate_lease_condition(lease_id), options, op);
        m_blob.delete_blob(azure::storage::delete_snapshots_option::none, azure::storage::access_condition::generate_lease_condition(lease_id), options, op);

        m_blob.create_or_replace(azure::storage::access_condition(), options, m_context);
        check_blob_no_stale_property(m_blob);
        lease_id = utility::uuid_to_string(utility::new_uuid());
        m_blob.acquire_lease(azure::storage::lease_time(std::chrono::seconds(60)), lease_id);
        m_blob.download_attributes(azure::storage::access_condition::generate_lease_condition(lease_id), options, op);
        m_blob.delete_blob(azure::storage::delete_snapshots_option::none, azure::storage::access_condition::generate_lease_condition(lease_id), options, op);

        m_blob.create_or_replace(azure::storage::access_condition(), options, m_context);
        check_blob_no_stale_property(m_blob);
        lease_id = utility::uuid_to_string(utility::new_uuid());
        m_blob.acquire_lease(azure::storage::lease_time(std::chrono::seconds(60)), lease_id);
        m_blob.download_attributes(azure::storage::access_condition::generate_lease_condition(lease_id), options, op);
        m_blob.delete_blob(azure::storage::delete_snapshots_option::none, azure::storage::access_condition::generate_lease_condition(lease_id), options, op);
    }

    TEST_FIXTURE(append_blob_test_base, append_blob_create_delete_cancellation)
    {

        {
            // cancel the cancellation prior to the operation
            auto cancel_token_src = pplx::cancellation_token_source();
            cancel_token_src.cancel();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.create_or_replace_async(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
                task_result.get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL(OPERATION_CANCELED, ex_msg);
        }

        {
            // cancel the cancellation during the operation
            auto cancel_token_src = pplx::cancellation_token_source();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.create_or_replace_async(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
                std::this_thread::sleep_for(std::chrono::milliseconds(3)); //sleep for sometime before canceling the request and see result.
                cancel_token_src.cancel();
                task_result.get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL(OPERATION_CANCELED, ex_msg);
        }

        {
            auto cancel_token_src = pplx::cancellation_token_source();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.create_or_replace_async(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
                task_result.get();
                // cancel the cancellation after the operation
                cancel_token_src.cancel();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("", ex_msg);
        }

        {
            // cancel the cancellation prior to the operation
            auto cancel_token_src = pplx::cancellation_token_source();
            cancel_token_src.cancel();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.delete_blob_if_exists_async(azure::storage::delete_snapshots_option::include_snapshots, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
                task_result.get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL(OPERATION_CANCELED, ex_msg);
        }

        {
            // cancel the cancellation during the operation
            auto cancel_token_src = pplx::cancellation_token_source();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.delete_blob_if_exists_async(azure::storage::delete_snapshots_option::include_snapshots, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
                std::this_thread::sleep_for(std::chrono::milliseconds(3)); //sleep for sometime before canceling the request and see result.
                cancel_token_src.cancel();
                task_result.get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL(OPERATION_CANCELED, ex_msg);
        }

        {
            auto cancel_token_src = pplx::cancellation_token_source();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.delete_blob_if_exists_async(azure::storage::delete_snapshots_option::include_snapshots, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
                task_result.get();
                // cancel the cancellation after the operation
                cancel_token_src.cancel();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("", ex_msg);
        }

    }

    TEST_FIXTURE(append_blob_test_base, append_blob_create_delete_timeout)
    {

        {
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(1));

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.create_or_replace_async(azure::storage::access_condition(), options, m_context);
                task_result.get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("The client could not finish the operation within specified timeout.", ex_msg);
        }

        {
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(3));

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.create_or_replace_async(azure::storage::access_condition(), options, m_context);
                task_result.get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("The client could not finish the operation within specified timeout.", ex_msg);
        }

        {
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(10000));

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.create_or_replace_async(azure::storage::access_condition(), options, m_context);
                task_result.get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("", ex_msg);
        }

        {
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(1));

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.delete_blob_if_exists_async(azure::storage::delete_snapshots_option::include_snapshots, azure::storage::access_condition(), options, m_context);
                task_result.get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("The client could not finish the operation within specified timeout.", ex_msg);
        }

        {
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(3));

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.delete_blob_if_exists_async(azure::storage::delete_snapshots_option::include_snapshots, azure::storage::access_condition(), options, m_context);
                task_result.get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("The client could not finish the operation within specified timeout.", ex_msg);
        }

        {
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(10000));

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.delete_blob_if_exists_async(azure::storage::delete_snapshots_option::include_snapshots, azure::storage::access_condition(), options, m_context);
                task_result.get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("", ex_msg);
        }
    }

    TEST_FIXTURE(append_blob_test_base, append_blob_create_cancellation_timeout)
    {
        {
            //when cancellation first
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(100));
            auto cancel_token_src = pplx::cancellation_token_source();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.create_or_replace_async(azure::storage::access_condition(), options, m_context, cancel_token_src.get_token());
                cancel_token_src.cancel();
                task_result.get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL(OPERATION_CANCELED, ex_msg);
        }

        {
            //when timeout first
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(10));
            auto cancel_token_src = pplx::cancellation_token_source();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.create_or_replace_async(azure::storage::access_condition(), options, m_context, cancel_token_src.get_token());
                std::this_thread::sleep_for(std::chrono::milliseconds(30)); //sleep for sometime before canceling the request and see result.
                cancel_token_src.cancel();
                task_result.get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("The client could not finish the operation within specified timeout.", ex_msg);
        }
    }

    TEST_FIXTURE(append_blob_test_base, append_blob_open_read_write_cancellation)
    {
        std::vector<uint8_t> buffer;
        buffer.resize(4 * 1024 * 1024);
        fill_buffer(buffer);

        {
            // cancel the cancellation prior to the operation
            auto cancel_token_src = pplx::cancellation_token_source();
            cancel_token_src.cancel();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.open_write_async(true, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
                auto os = task_result.get();
                os.streambuf().putn_nocopy(buffer.data(), buffer.size()).wait();
                os.close().get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL(OPERATION_CANCELED, ex_msg);
        }

        {
            // cancel the cancellation prior to the operation and write to a canceled ostream.
            auto cancel_token_src = pplx::cancellation_token_source();
            cancel_token_src.cancel();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.open_write_async(true, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
                auto os = task_result.get();
                os.streambuf().putn_nocopy(buffer.data(), buffer.size()).wait();
                os.streambuf().putn_nocopy(buffer.data(), buffer.size()).wait();
                os.close().get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL(OPERATION_CANCELED, ex_msg);
        }

        {
            // cancel the cancellation during the operation
            auto cancel_token_src = pplx::cancellation_token_source();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.open_write_async(true, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
                auto os = task_result.get();
                os.streambuf().putn_nocopy(buffer.data(), buffer.size()).wait();
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); //sleep for sometime before canceling the request and see result.
                cancel_token_src.cancel();
                os.close().get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL(OPERATION_CANCELED, ex_msg);
        }

        {
            auto cancel_token_src = pplx::cancellation_token_source();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.open_write_async(true, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
                auto os = task_result.get();
                os.streambuf().putn_nocopy(buffer.data(), buffer.size()).wait();
                os.close().get();
                // cancel the cancellation after the operation
                cancel_token_src.cancel();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("", ex_msg);
        }

        m_blob.upload_from_stream(concurrency::streams::bytestream::open_istream(buffer));

        {
            auto cancel_token_src = pplx::cancellation_token_source();
            // cancel the cancellation prior to the operation
            cancel_token_src.cancel();

            std::string ex_msg;


            try
            {
                auto task_result = m_blob.open_read_async(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
                auto is = task_result.get();
                is.read().get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL(OPERATION_CANCELED, ex_msg);
        }

        {
            // cancel the cancellation during the operation
            auto cancel_token_src = pplx::cancellation_token_source();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.open_read_async(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); //sleep for sometime before canceling the request and see result.
                cancel_token_src.cancel();
                auto is = task_result.get();
                is.read().get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL(OPERATION_CANCELED, ex_msg);
        }

        {
            auto cancel_token_src = pplx::cancellation_token_source();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.open_read_async(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); //sleep for sometime before canceling the request and see result.
                auto is = task_result.get();
                is.read().get();
                // cancel the cancellation after the operation
                cancel_token_src.cancel();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("", ex_msg);
        }

    }

    TEST_FIXTURE(append_blob_test_base, append_blob_open_read_write_timeout)
    {
        std::vector<uint8_t> buffer;
        buffer.resize(4 * 1024 * 1024);
        fill_buffer(buffer);

        {
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(1));

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.open_write_async(true, azure::storage::access_condition(), options, m_context);
                auto os = task_result.get();
                os.streambuf().putn_nocopy(buffer.data(), buffer.size()).wait();
                os.close().get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("The client could not finish the operation within specified timeout.", ex_msg);
        }

        {
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(20));

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.open_write_async(true, azure::storage::access_condition(), options, m_context);
                auto os = task_result.get();
                os.streambuf().putn_nocopy(buffer.data(), buffer.size()).wait();
                os.close().get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("The client could not finish the operation within specified timeout.", ex_msg);
        }

        {
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::seconds(30));

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.open_write_async(true, azure::storage::access_condition(), options, m_context);
                auto os = task_result.get();
                os.streambuf().putn_nocopy(buffer.data(), buffer.size()).wait();
                os.close().get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("", ex_msg);
        }

        m_blob.upload_from_stream(concurrency::streams::bytestream::open_istream(buffer));

        {
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(1));

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.open_read_async(azure::storage::access_condition(), options, m_context);
                auto is = task_result.get();
                is.read().get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("The client could not finish the operation within specified timeout.", ex_msg);
        }

        {
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(20));

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.open_read_async(azure::storage::access_condition(), options, m_context);
                auto is = task_result.get();
                is.read().get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("The client could not finish the operation within specified timeout.", ex_msg);
        }

        {
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::seconds(30));

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.open_read_async(azure::storage::access_condition(), options, m_context);
                auto is = task_result.get();
                is.read().get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("", ex_msg);
        }
    }

    TEST_FIXTURE(append_blob_test_base, append_blob_open_read_write_cancellation_timeout)
    {
        std::vector<uint8_t> buffer;
        buffer.resize(4 * 1024 * 1024);
        fill_buffer(buffer);
        {
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(10));
            auto cancel_token_src = pplx::cancellation_token_source();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.open_write_async(true, azure::storage::access_condition(), options, m_context, cancel_token_src.get_token());
                auto os = task_result.get();
                os.streambuf().putn_nocopy(buffer.data(), buffer.size()).wait();
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); //sleep for sometime before canceling the request and see result.
                cancel_token_src.cancel();
                os.close().get();
            }
            catch (std::exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("The client could not finish the operation within specified timeout.", ex_msg);
        }
    }

    TEST_FIXTURE(append_blob_test_base, append_blob_concurrent_upload_cancellation_timeout)
    {
        utility::size64_t length = 260 * 1024 * 1024;
        std::vector<uint8_t> buffer;
        buffer.resize(length);
        fill_buffer(buffer);

        {
            auto cancel_token_src = pplx::cancellation_token_source();
            auto options = azure::storage::blob_request_options();
            options.set_parallelism_factor(4);
            options.set_maximum_execution_time(std::chrono::milliseconds(1000));
            // cancel the cancellation prior to the operation
            cancel_token_src.cancel();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.upload_from_stream_async(concurrency::streams::bytestream::open_istream(buffer), length, azure::storage::access_condition(), options, m_context, cancel_token_src.get_token());
                task_result.get();
            }
            catch (azure::storage::storage_exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL(OPERATION_CANCELED, ex_msg);
        }

        {
            auto cancel_token_src = pplx::cancellation_token_source();
            auto options = azure::storage::blob_request_options();
            options.set_parallelism_factor(4);
            options.set_maximum_execution_time(std::chrono::milliseconds(1000));

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.upload_from_stream_async(concurrency::streams::bytestream::open_istream(buffer), length, azure::storage::access_condition(), options, m_context, cancel_token_src.get_token());
                std::this_thread::sleep_for(std::chrono::milliseconds(300)); //sleep for sometime before canceling the request and see result.
                cancel_token_src.cancel();
                task_result.get();
            }
            catch (azure::storage::storage_exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL(OPERATION_CANCELED, ex_msg);
        }

        {
            auto options = azure::storage::blob_request_options();
            options.set_parallelism_factor(4);
            options.set_maximum_execution_time(std::chrono::milliseconds(1000));

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.upload_from_stream_async(concurrency::streams::bytestream::open_istream(buffer), length, azure::storage::access_condition(), options, m_context);
                task_result.get();
            }
            catch (azure::storage::storage_exception& e)
            {
                ex_msg = std::string(e.what());
            }

            CHECK_EQUAL("The client could not finish the operation within specified timeout.", ex_msg);
        }
    }

    TEST_FIXTURE(append_blob_test_base, append_blob_cpkv)
    {
        utility::size64_t length = 128 * 1024;
        std::vector<uint8_t> buffer(length);
        fill_buffer(buffer);
        auto empty_options = azure::storage::blob_request_options();
        auto cpk_options = azure::storage::blob_request_options();
        std::vector<uint8_t> key(32);
        fill_buffer(key);
        cpk_options.set_encryption_key(key);

        m_blob.create_or_replace(azure::storage::access_condition(), cpk_options, m_context);

        CHECK_THROW(m_blob.exists(empty_options, m_context), azure::storage::storage_exception);
        m_blob.exists(cpk_options, m_context);
        CHECK_THROW(m_blob.append_block(concurrency::streams::bytestream::open_istream(buffer), azure::storage::checksum_none, azure::storage::access_condition(), empty_options, m_context), azure::storage::storage_exception);
        m_blob.append_block(concurrency::streams::bytestream::open_istream(buffer), azure::storage::checksum_none, azure::storage::access_condition(), cpk_options, m_context);
        CHECK_THROW(m_blob.append_text(_XPLATSTR("Hello world"), azure::storage::access_condition(), empty_options, m_context), azure::storage::storage_exception);
        m_blob.append_text(_XPLATSTR("Hello world"), azure::storage::access_condition(), cpk_options, m_context);
    }
}
