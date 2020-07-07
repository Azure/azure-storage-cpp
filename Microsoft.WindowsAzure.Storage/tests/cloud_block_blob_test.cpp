// -----------------------------------------------------------------------------------------
// <copyright file="cloud_block_blob_test.cpp" company="Microsoft">
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
#include "cpprest/rawptrstream.h"
#include "was/crc64.h"
#include "wascore/constants.h"
#include "wascore/util.h"

#pragma region Fixture

utility::string_t block_blob_test_base::get_block_id(uint16_t block_index)
{
    return utility::conversions::to_base64(block_index);
}

void block_blob_test_base::check_block_list_equal(const std::vector<azure::storage::block_list_item>& committed_put_block_list, const std::vector<azure::storage::block_list_item>& uncommitted_put_block_list)
{
    {
        auto get_block_list = m_blob.download_block_list(azure::storage::block_listing_filter::committed, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(committed_put_block_list.size(), get_block_list.size());
        for (size_t i = 0; i < committed_put_block_list.size(); ++i)
        {
            CHECK_UTF8_EQUAL(committed_put_block_list[i].id(), get_block_list[i].id());
            CHECK(azure::storage::block_list_item::block_mode::committed == get_block_list[i].mode());
        }
    }

    {
        auto get_block_list = m_blob.download_block_list(azure::storage::block_listing_filter::uncommitted, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(uncommitted_put_block_list.size(), get_block_list.size());
        for (size_t i = 0; i < uncommitted_put_block_list.size(); ++i)
        {
            CHECK_UTF8_EQUAL(uncommitted_put_block_list[i].id(), get_block_list[i].id());
            CHECK(azure::storage::block_list_item::block_mode::uncommitted == get_block_list[i].mode());
        }
    }

    {
        auto get_block_list = m_blob.download_block_list(azure::storage::block_listing_filter::all, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(committed_put_block_list.size() + uncommitted_put_block_list.size(), get_block_list.size());
        for (size_t i = 0; i < committed_put_block_list.size(); ++i)
        {
            CHECK_UTF8_EQUAL(committed_put_block_list[i].id(), get_block_list[i].id());
            CHECK(azure::storage::block_list_item::block_mode::committed == get_block_list[i].mode());
        }

        for (size_t i = 0; i < uncommitted_put_block_list.size(); ++i)
        {
            CHECK_UTF8_EQUAL(uncommitted_put_block_list[i].id(), get_block_list[committed_put_block_list.size() + i].id());
            CHECK(azure::storage::block_list_item::block_mode::uncommitted == get_block_list[committed_put_block_list.size() + i].mode());
        }
    }
}

static std::string INTENDED_ERR_MSG = "Intended exception from currupted_ostreambuf.";
static std::string ARCHIVE_BLOB_IN_STANDARD_ACCOUNT_ERR_MSG = "Blob access tier is supported only for Blob Storage accounts.";
static std::string REHYDRATE_CANNOT_SET_TO_ARCHIVE_ERR_MSG = "This operation is not permitted because the blob is being rehydrated.";
static std::string ACTIVE_LEASE_ERROR_MESSAGE = "There is currently a lease on the blob and no lease ID was specified in the request.";

template<typename _CharType>
class currupted_ostreambuf : public Concurrency::streams::details::basic_rawptr_buffer<_CharType>
{
public:
    currupted_ostreambuf(bool keep_writable, int recover_on_nretries)
        : Concurrency::streams::details::basic_rawptr_buffer<_CharType>(), m_keepwritable(keep_writable), m_recover_on_nretries(recover_on_nretries)
    { }

    pplx::task<size_t> _putn(const _CharType* ptr, size_t count) override
    {
        UNREFERENCED_PARAMETER(ptr);
        try
        {
            ++m_call_count;
            if (m_call_count < m_recover_on_nretries + 1)
            {
                throw azure::storage::storage_exception(INTENDED_ERR_MSG);
            }
            return pplx::task_from_result(count);
        }
        catch (...)
        {
            return pplx::task_from_exception<size_t>(std::current_exception());
        }
    }

    bool can_write() const override
    {
        return m_keepwritable
            ? true
            : Concurrency::streams::details::basic_rawptr_buffer<_CharType>::can_write();
    }

    int call_count() const
    {
        return m_call_count;
    }

private:
    int m_call_count = 0;
    int m_recover_on_nretries = 0;
    bool m_keepwritable = false;
};

template <typename _CharType>
class currupted_stream
{
public:
    typedef _CharType char_type;
    typedef currupted_ostreambuf<_CharType> buffer_type;

    static concurrency::streams::basic_ostream<char_type> open_ostream(bool keep_writable, int recover_on_nretries)
    {
        return concurrency::streams::basic_ostream<char_type>(concurrency::streams::streambuf<char_type>(std::make_shared<buffer_type>(keep_writable, recover_on_nretries)));
    }
};

#pragma endregion

SUITE(Blob)
{
    TEST_FIXTURE(block_blob_test_base, block_upload)
    {
        std::vector<uint8_t> buffer;
        buffer.resize(16 * 1024);
        azure::storage::blob_request_options options;
        std::vector<azure::storage::block_list_item> uncommitted_blocks;
        std::vector<azure::storage::block_list_item> committed_blocks;

        utility::string_t md5_header;
        utility::string_t crc64_header;
        m_context.set_sending_request([&md5_header, &crc64_header] (web::http::http_request& request, azure::storage::operation_context)
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

        uint16_t block_id_counter = 0;

        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(false);
        for (uint16_t i = 0; i < 3; ++i)
        {
            fill_buffer(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            auto block_id = get_block_id(block_id_counter++);
            uncommitted_blocks.push_back(azure::storage::block_list_item(block_id));
            m_blob.upload_block(block_id, stream, utility::string_t(), azure::storage::access_condition(), options, m_context);
            CHECK_UTF8_EQUAL(utility::string_t(), md5_header);
        }

        check_block_list_equal(committed_blocks, uncommitted_blocks);
        std::copy(uncommitted_blocks.begin(), uncommitted_blocks.end(), std::back_inserter(committed_blocks));
        m_blob.upload_block_list(committed_blocks, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        uncommitted_blocks.clear();

        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(false);
        for (uint16_t i = 0; i < 3; ++i)
        {
            auto md5 = fill_buffer_and_get_md5(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            auto block_id = get_block_id(block_id_counter++);
            uncommitted_blocks.push_back(azure::storage::block_list_item(block_id));
            m_blob.upload_block(block_id, stream, md5, azure::storage::access_condition(), options, m_context);
            CHECK_UTF8_EQUAL(md5, md5_header);
        }

        check_block_list_equal(committed_blocks, uncommitted_blocks);
        std::copy(uncommitted_blocks.begin(), uncommitted_blocks.end(), std::back_inserter(committed_blocks));
        m_blob.upload_block_list(committed_blocks, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        uncommitted_blocks.clear();

        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(false);
        for (uint16_t i = 0; i < 3; ++i)
        {
            auto crc64 = fill_buffer_and_get_crc64(buffer);
            uint64_t crc64_val = azure::storage::crc64(buffer.data(), buffer.size());
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            auto block_id = get_block_id(block_id_counter++);
            uncommitted_blocks.push_back(azure::storage::block_list_item(block_id));
            m_blob.upload_block(block_id, stream, crc64_val, azure::storage::access_condition(), options, m_context);
            CHECK_UTF8_EQUAL(crc64, crc64_header);
        }

        check_block_list_equal(committed_blocks, uncommitted_blocks);
        std::copy(uncommitted_blocks.begin(), uncommitted_blocks.end(), std::back_inserter(committed_blocks));
        m_blob.upload_block_list(committed_blocks, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        uncommitted_blocks.clear();

        options.set_use_transactional_md5(true);
        options.set_use_transactional_crc64(false);
        for (uint16_t i = 0; i < 3; ++i)
        {
            auto md5 = fill_buffer_and_get_md5(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            auto block_id = get_block_id(block_id_counter++);
            uncommitted_blocks.push_back(azure::storage::block_list_item(block_id));
            m_blob.upload_block(block_id, stream, utility::string_t(), azure::storage::access_condition(), options, m_context);
            CHECK_UTF8_EQUAL(md5, md5_header);
        }

        check_block_list_equal(committed_blocks, uncommitted_blocks);
        std::copy(uncommitted_blocks.begin(), uncommitted_blocks.end(), std::back_inserter(committed_blocks));
        m_blob.upload_block_list(committed_blocks, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        uncommitted_blocks.clear();

        options.set_use_transactional_md5(true);
        options.set_use_transactional_crc64(false);
        for (uint16_t i = 0; i < 3; ++i)
        {
            auto md5 = fill_buffer_and_get_md5(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            auto block_id = get_block_id(block_id_counter++);
            uncommitted_blocks.push_back(azure::storage::block_list_item(block_id));
            m_blob.upload_block(block_id, stream, azure::storage::checksum_none, azure::storage::access_condition(), options, m_context);
            CHECK_UTF8_EQUAL(md5, md5_header);
        }

        check_block_list_equal(committed_blocks, uncommitted_blocks);
        std::copy(uncommitted_blocks.begin(), uncommitted_blocks.end(), std::back_inserter(committed_blocks));
        m_blob.upload_block_list(committed_blocks, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        uncommitted_blocks.clear();

        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(true);
        for (uint16_t i = 0; i < 3; ++i)
        {
            auto crc64 = fill_buffer_and_get_crc64(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            auto block_id = get_block_id(block_id_counter++);
            uncommitted_blocks.push_back(azure::storage::block_list_item(block_id));
            m_blob.upload_block(block_id, stream, azure::storage::checksum_none, azure::storage::access_condition(), options, m_context);
            CHECK_UTF8_EQUAL(crc64, crc64_header);
        }

        check_block_list_equal(committed_blocks, uncommitted_blocks);
        std::copy(uncommitted_blocks.begin(), uncommitted_blocks.end(), std::back_inserter(committed_blocks));
        m_blob.upload_block_list(committed_blocks, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        uncommitted_blocks.clear();

        {
            options.set_use_transactional_md5(true);
            options.set_use_transactional_crc64(false);
            fill_buffer(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            CHECK_THROW(m_blob.upload_block(get_block_id(0), stream, dummy_md5, azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);
            CHECK_UTF8_EQUAL(dummy_md5, md5_header);
        }
        {
            options.set_use_transactional_md5(false);
            options.set_use_transactional_crc64(true);
            fill_buffer(buffer);
            auto stream = concurrency::streams::bytestream::open_istream(buffer);
            CHECK_THROW(m_blob.upload_block(get_block_id(0), stream, dummy_crc64_val, azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);
            CHECK_UTF8_EQUAL(dummy_crc64, crc64_header);
        }

        check_block_list_equal(committed_blocks, uncommitted_blocks);

        m_context.set_sending_request(std::function<void(web::http::http_request &, azure::storage::operation_context)>());
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_upload)
    {
        const size_t size = 6 * 1024 * 1024;
        azure::storage::blob_request_options options;
        options.set_store_blob_content_md5(false);

        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(false);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 1, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(true);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 1, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_use_transactional_crc64(false);
        options.set_use_transactional_md5(true);
        options.set_store_blob_content_md5(true);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 1, true), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_single_blob_upload_threshold_in_bytes(4 * 1024 * 1024);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 3, true), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_store_blob_content_md5(false);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 3, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_stream_write_size_in_bytes(1 * 1024 * 1024);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 7, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_use_transactional_md5(false);
        options.set_single_blob_upload_threshold_in_bytes(6 * 1024 * 1024);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 1, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_parallelism_factor(4);
        options.set_use_transactional_md5(true);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 7, false), 4);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_parallelism_factor(8);
        options.set_store_blob_content_md5(true);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 7, true), 6);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_store_blob_content_md5(false);
        options.set_use_transactional_md5(false);
        options.set_parallelism_factor(1);
        options.set_http_buffer_size(512 * 1024);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 1, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_stream_write_size_in_bytes(4 * 1024 * 1024);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 1, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_store_blob_content_md5(true);
        options.set_single_blob_upload_threshold_in_bytes(32 * 1024 * 1024);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 1, true), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_parallelism_factor(4);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 3, true), 2);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_store_blob_content_md5(false);
        options.set_parallelism_factor(1);
        options.set_stream_write_size_in_bytes(1 * 1024 * 1024);
        options.set_single_blob_upload_threshold_in_bytes(6 * 1024 * 1024);
        options.set_http_buffer_size(0);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 1, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_stream_write_size_in_bytes(4 * 1024 * 1024);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 1, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_store_blob_content_md5(true);
        options.set_single_blob_upload_threshold_in_bytes(32 * 1024 * 1024);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 1, true), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_parallelism_factor(4);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, true, options, 3, true), 2);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_upload_with_nonseekable)
    {
        const size_t size = 6 * 1024 * 1024;
        azure::storage::blob_request_options options;
        options.set_store_blob_content_md5(false);

        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(false);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, false, options, 3, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(true);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, false, options, 3, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_use_transactional_md5(true);
        options.set_use_transactional_crc64(false);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, false, options, 3, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_store_blob_content_md5(true);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, false, options, 3, true), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_single_blob_upload_threshold_in_bytes(4 * 1024 * 1024);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, false, options, 3, true), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_store_blob_content_md5(false);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, false, options, 3, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_stream_write_size_in_bytes(1 * 1024 * 1024);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, false, options, 7, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_single_blob_upload_threshold_in_bytes(6 * 1024 * 1024);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, false, options, 7, false), 1);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_parallelism_factor(4);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, false, options, 7, false), 4);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());

        options.set_parallelism_factor(8);
        options.set_store_blob_content_md5(true);
        check_parallelism(upload_and_download(m_blob, size, 0, 0, false, options, 7, true), 6);
        m_blob.delete_blob();
        m_blob.properties().set_content_md5(utility::string_t());
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_upload_with_size)
    {
        const size_t buffer_size = 6 * 1024 * 1024;
        const size_t blob_size = 4 * 1024 * 1024;

        const size_t buffer_offsets[2] = { 0, 1024 };
        for (auto buffer_offset : buffer_offsets)
        {
            azure::storage::blob_request_options options;
            options.set_stream_write_size_in_bytes(blob_size);
            options.set_store_blob_content_md5(false);
            options.set_parallelism_factor(1);

            options.set_use_transactional_md5(false);
            options.set_use_transactional_crc64(false);
            check_parallelism(upload_and_download(m_blob, buffer_size, buffer_offset, blob_size, true, options, 1, false), 1);
            m_blob.delete_blob();
            m_blob.properties().set_content_md5(utility::string_t());

            options.set_use_transactional_md5(false);
            options.set_use_transactional_crc64(true);
            check_parallelism(upload_and_download(m_blob, buffer_size, buffer_offset, blob_size, true, options, 1, false), 1);
            m_blob.delete_blob();
            m_blob.properties().set_content_md5(utility::string_t());

            options.set_use_transactional_crc64(false);
            options.set_use_transactional_md5(true);
            options.set_store_blob_content_md5(true);
            check_parallelism(upload_and_download(m_blob, buffer_size, buffer_offset, blob_size, true, options, 1, true), 1);
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
            check_parallelism(upload_and_download(m_blob, buffer_size, buffer_offset, blob_size, true, options, 5, true), 4);
            m_blob.delete_blob();
            m_blob.properties().set_content_md5(utility::string_t());
        }
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_upload_with_size_with_nonseekable)
    {
        const size_t buffer_size = 6 * 1024 * 1024;
        const size_t blob_size = 4 * 1024 * 1024;

        const size_t buffer_offsets[2] = { 0, 1024 };
        for (auto buffer_offset : buffer_offsets)
        {
            azure::storage::blob_request_options options;
            options.set_stream_write_size_in_bytes(blob_size);
            options.set_store_blob_content_md5(false);
            options.set_parallelism_factor(1);

            options.set_use_transactional_md5(false);
            options.set_use_transactional_crc64(false);
            check_parallelism(upload_and_download(m_blob, buffer_size, buffer_offset, blob_size, false, options, 1, false), 1);
            m_blob.delete_blob();
            m_blob.properties().set_content_md5(utility::string_t());

            options.set_use_transactional_md5(false);
            options.set_use_transactional_crc64(true);
            check_parallelism(upload_and_download(m_blob, buffer_size, buffer_offset, blob_size, false, options, 1, false), 1);
            m_blob.delete_blob();
            m_blob.properties().set_content_md5(utility::string_t());

            options.set_use_transactional_crc64(false);
            options.set_use_transactional_md5(true);
            options.set_store_blob_content_md5(true);
            check_parallelism(upload_and_download(m_blob, buffer_size, buffer_offset, blob_size, false, options, 1, true), 1);
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
            check_parallelism(upload_and_download(m_blob, buffer_size, buffer_offset, blob_size, false, options, 5, true), 4);
            m_blob.delete_blob();
            m_blob.properties().set_content_md5(utility::string_t());
        }
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_upload_with_invalid_size)
    {
        const size_t buffer_size = 2 * 1024 * 1024;
        azure::storage::blob_request_options options;
        options.set_store_blob_content_md5(false);
        CHECK_THROW(upload_and_download(m_blob, buffer_size, 0, buffer_size + 1, true, options, 0, false), std::invalid_argument);
        CHECK_THROW(upload_and_download(m_blob, buffer_size, 0, buffer_size + 1, false, options, 0, false), std::invalid_argument);
        CHECK_THROW(upload_and_download(m_blob, buffer_size, 1024, buffer_size - 1023, true, options, 0, false), std::invalid_argument);
        CHECK_THROW(upload_and_download(m_blob, buffer_size, 1024, buffer_size - 1023, false, options, 0, false), std::invalid_argument);
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_text_upload)
    {
        azure::storage::blob_request_options options;
        options.set_store_blob_content_md5(true);

        utility::string_t text;
        m_blob.upload_text(text, azure::storage::access_condition(), options, m_context);
        CHECK(m_blob.download_text(azure::storage::access_condition(), options, m_context).empty());

        text = _XPLATSTR("test");
        m_blob.upload_text(text, azure::storage::access_condition(), options, m_context);
        CHECK_UTF8_EQUAL(text, m_blob.download_text(azure::storage::access_condition(), options, m_context));

        m_blob.properties().set_content_md5(dummy_md5);
        m_blob.upload_properties();
        options.set_retry_policy(azure::storage::no_retry_policy());
        CHECK_THROW(m_blob.download_text(azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_file_upload)
    {
        azure::storage::blob_request_options options;
        options.set_store_blob_content_md5(true);

        utility::string_t md5_header;
        m_context.set_sending_request([&md5_header] (web::http::http_request& request, azure::storage::operation_context)
        {
            if (!request.headers().match(azure::storage::protocol::ms_header_blob_content_md5, md5_header))
            {
                md5_header.clear();
            }
        });

        temp_file file(1000);
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
        CHECK_ARRAY_EQUAL(original_file_buffer.collection(), downloaded_file_buffer.collection(), (int) downloaded_file_buffer.collection().size());

        m_blob.properties().set_content_md5(dummy_md5);
        m_blob.upload_properties();
        options.set_retry_policy(azure::storage::no_retry_policy());
        CHECK_THROW(m_blob.download_to_file(file2.path(), azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_constructor)
    {
        m_blob.upload_block_list(std::vector<azure::storage::block_list_item>(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK(!m_blob.properties().etag().empty());

        azure::storage::cloud_block_blob blob1(m_blob.uri());
        CHECK_UTF8_EQUAL(m_blob.name(), blob1.name());
        CHECK_UTF8_EQUAL(m_blob.uri().primary_uri().to_string(), blob1.uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.uri().secondary_uri().to_string(), blob1.uri().secondary_uri().to_string());
        CHECK(blob1.properties().etag().empty());

        azure::storage::cloud_blob blob2(m_blob);
        CHECK_UTF8_EQUAL(m_blob.name(), blob2.name());
        CHECK_UTF8_EQUAL(m_blob.uri().primary_uri().to_string(), blob2.uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.uri().secondary_uri().to_string(), blob2.uri().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.properties().etag(), blob2.properties().etag());

        azure::storage::cloud_block_blob blob3(blob2);
        CHECK_UTF8_EQUAL(m_blob.name(), blob3.name());
        CHECK_UTF8_EQUAL(m_blob.uri().primary_uri().to_string(), blob3.uri().primary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.uri().secondary_uri().to_string(), blob3.uri().secondary_uri().to_string());
        CHECK_UTF8_EQUAL(m_blob.properties().etag(), blob2.properties().etag());
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_block_list_with_metadata)
    {
        m_blob.metadata()[_XPLATSTR("key1")] = _XPLATSTR("value1");
        m_blob.metadata()[_XPLATSTR("key2")] = _XPLATSTR("value2");
        m_blob.upload_block_list(std::vector<azure::storage::block_list_item>());

        auto same_blob = m_container.get_block_blob_reference(m_blob.name());
        CHECK(same_blob.metadata().empty());
        same_blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2U, same_blob.metadata().size());
        CHECK_UTF8_EQUAL(_XPLATSTR("value1"), same_blob.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value2"), same_blob.metadata()[_XPLATSTR("key2")]);
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_block_list_use_transactional_checksum)
    {
        m_blob.properties().set_content_type(_XPLATSTR("text/plain; charset=utf-8"));

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

        std::vector<azure::storage::block_list_item> blocks;
        for (uint16_t i = 0; i < 10; i++)
        {
            auto id = get_block_id(i);
            auto utf8_body = utility::conversions::to_utf8string(azure::storage::core::convert_to_string(i));
            auto stream = concurrency::streams::bytestream::open_istream(std::move(utf8_body));
            m_blob.upload_block(id, stream, utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
            blocks.push_back(azure::storage::block_list_item(id));
        }

        azure::storage::blob_request_options options;
        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(false);
        m_blob.upload_block_list(blocks, azure::storage::access_condition(), options, m_context);
        CHECK_UTF8_EQUAL(utility::string_t(), md5_header);
        CHECK_UTF8_EQUAL(_XPLATSTR("0123456789"), m_blob.download_text(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context));

        options.set_use_transactional_md5(true);
        options.set_use_transactional_crc64(false);
        m_blob.upload_block_list(blocks, azure::storage::access_condition(), options, m_context);
        CHECK(!md5_header.empty());
        CHECK_UTF8_EQUAL(m_context.request_results().back().content_md5(), md5_header);
        CHECK_UTF8_EQUAL(_XPLATSTR("0123456789"), m_blob.download_text(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context));

        options.set_use_transactional_md5(false);
        options.set_use_transactional_crc64(true);
        m_blob.upload_block_list(blocks, azure::storage::access_condition(), options, m_context);
        CHECK(!crc64_header.empty());
        CHECK_UTF8_EQUAL(m_context.request_results().back().content_crc64(), crc64_header);
        CHECK_UTF8_EQUAL(_XPLATSTR("0123456789"), m_blob.download_text(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context));

        m_context.set_sending_request(std::function<void(web::http::http_request &, azure::storage::operation_context)>());
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_put_blob_with_metadata)
    {
        m_blob.metadata()[_XPLATSTR("key1")] = _XPLATSTR("value1");
        m_blob.metadata()[_XPLATSTR("key2")] = _XPLATSTR("value2");
        m_blob.upload_text(utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto same_blob = m_container.get_block_blob_reference(m_blob.name());
        CHECK(same_blob.metadata().empty());
        same_blob.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2U, same_blob.metadata().size());
        CHECK_UTF8_EQUAL(_XPLATSTR("value1"), same_blob.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value2"), same_blob.metadata()[_XPLATSTR("key2")]);
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_upload_invalid_options)
    {
        azure::storage::blob_request_options options;
        options.set_store_blob_content_md5(false);
        options.set_use_transactional_md5(true);

        CHECK_THROW(m_blob.upload_text(utility::string_t(), azure::storage::access_condition(), options, m_context), std::invalid_argument);
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_snapshot_metadata)
    {
        m_blob.metadata()[_XPLATSTR("key1")] = _XPLATSTR("value1");
        m_blob.metadata()[_XPLATSTR("key2")] = _XPLATSTR("value2");
        m_blob.upload_text(_XPLATSTR("1"));

        auto snapshot1 = m_blob.create_snapshot(azure::storage::cloud_metadata(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2U, snapshot1.metadata().size());
        CHECK_UTF8_EQUAL(_XPLATSTR("value1"), snapshot1.metadata()[_XPLATSTR("key1")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value2"), snapshot1.metadata()[_XPLATSTR("key2")]);

        azure::storage::cloud_block_blob snapshot1_clone(snapshot1.uri(), snapshot1.snapshot_time(), snapshot1.service_client().credentials());
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
        CHECK_UTF8_EQUAL(_XPLATSTR("value1"), snapshot2.metadata()[_XPLATSTR("key3")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value2"), snapshot2.metadata()[_XPLATSTR("key4")]);

        azure::storage::cloud_block_blob snapshot2_clone(snapshot2.uri(), snapshot2.snapshot_time(), snapshot2.service_client().credentials());
        CHECK(snapshot2_clone.metadata().empty());
        snapshot2_clone.download_attributes(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_EQUAL(2U, snapshot2_clone.metadata().size());
        CHECK_UTF8_EQUAL(_XPLATSTR("value1"), snapshot2_clone.metadata()[_XPLATSTR("key3")]);
        CHECK_UTF8_EQUAL(_XPLATSTR("value2"), snapshot2_clone.metadata()[_XPLATSTR("key4")]);
    }

    TEST_FIXTURE(block_blob_test_base, block_reordering)
    {
        m_blob.properties().set_content_type(_XPLATSTR("text/plain; charset=utf-8"));

        std::vector<azure::storage::block_list_item> blocks;
        for (uint16_t i = 0; i < 10; i++)
        {
            auto id = get_block_id(i);
            auto utf8_body = utility::conversions::to_utf8string(azure::storage::core::convert_to_string(i));
            auto stream = concurrency::streams::bytestream::open_istream(std::move(utf8_body));
            m_blob.upload_block(id, stream, utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
            blocks.push_back(azure::storage::block_list_item(id));
        }

        m_blob.upload_block_list(blocks, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_UTF8_EQUAL(_XPLATSTR("0123456789"), m_blob.download_text(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context));

        blocks.erase(blocks.begin());
        m_blob.upload_block_list(blocks, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_UTF8_EQUAL(_XPLATSTR("123456789"), m_blob.download_text(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context));

        auto iter = blocks.begin();
        std::advance(iter, 3);
        blocks.erase(iter);
        m_blob.upload_block_list(blocks, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_UTF8_EQUAL(_XPLATSTR("12356789"), m_blob.download_text(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context));

        auto id = get_block_id(4);
        auto utf8_body = utility::conversions::to_utf8string(azure::storage::core::convert_to_string(4));
        auto stream = concurrency::streams::bytestream::open_istream(std::move(utf8_body));
        m_blob.upload_block(id, stream, utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        blocks.insert(blocks.begin(), azure::storage::block_list_item(id));
        m_blob.upload_block_list(blocks, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_UTF8_EQUAL(_XPLATSTR("412356789"), m_blob.download_text(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context));

        blocks.push_back(azure::storage::block_list_item(id));
        m_blob.upload_block_list(blocks, azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);
        CHECK_UTF8_EQUAL(_XPLATSTR("4123567894"), m_blob.download_text(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context));
    }

    TEST_FIXTURE(block_blob_test_base, list_uncommitted_blobs)
    {
        std::vector<uint8_t> buffer;
        buffer.resize(16 * 1024);
        fill_buffer(buffer);
        auto stream = concurrency::streams::bytestream::open_istream(buffer);
        auto ucblob = m_container.get_block_blob_reference(_XPLATSTR("ucblob"));
        ucblob.upload_block(get_block_id(0), stream, utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        m_blob.upload_text(utility::string_t(), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context);

        auto blobs = list_all_blobs(utility::string_t(), azure::storage::blob_listing_details::none, 0, azure::storage::blob_request_options());
        CHECK_EQUAL(1U, blobs.size());

        blobs = list_all_blobs(utility::string_t(), azure::storage::blob_listing_details::uncommitted_blobs, 0, azure::storage::blob_request_options());
        CHECK_EQUAL(2U, blobs.size());
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_upload_maximum_execution_time)
    {
        std::chrono::seconds duration(10);

        std::vector<uint8_t> buffer;
        buffer.resize(2 * 1024 * 1024);

        azure::storage::blob_request_options options;
        options.set_maximum_execution_time(duration);
        options.set_stream_write_size_in_bytes(buffer.size() / 2);
        options.set_single_blob_upload_threshold_in_bytes(buffer.size() / 2);

        m_context.set_response_received([duration] (web::http::http_request&, const web::http::http_response&, azure::storage::operation_context)
        {
            std::this_thread::sleep_for(duration);
        });

        CHECK_THROW(m_blob.upload_from_stream(concurrency::streams::bytestream::open_istream(std::move(buffer)), azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);
        CHECK_EQUAL(2U, m_context.request_results().size());

        m_context.set_response_received(std::function<void(web::http::http_request &, const web::http::http_response&, azure::storage::operation_context)>());
    }

    TEST_FIXTURE(block_blob_test_base, large_block_blob)
    {
        std::vector<uint8_t> buffer;
        buffer.resize(12 * 1024 * 1024);

        azure::storage::blob_request_options options;
        CHECK_THROW(options.set_single_blob_upload_threshold_in_bytes(5001 * 1024 * 1024ULL), std::invalid_argument);
        CHECK_THROW(options.set_stream_write_size_in_bytes(4001 * 1024 * 1024ULL), std::invalid_argument);

        m_blob.upload_from_stream(concurrency::streams::bytestream::open_istream(buffer), azure::storage::access_condition(), options, m_context);
        CHECK_EQUAL(2U, m_context.request_results().size()); // CreateContainer + PutBlob

        options.set_single_blob_upload_threshold_in_bytes(buffer.size() / 2);
        m_blob.upload_from_stream(concurrency::streams::bytestream::open_istream(buffer), azure::storage::access_condition(), options, m_context);
        CHECK_EQUAL(6U, m_context.request_results().size()); // PutBlock * 3 + PutBlockList

        options.set_stream_write_size_in_bytes(6 * 1024 * 1024);
        m_blob.upload_from_stream(concurrency::streams::bytestream::open_istream(buffer), azure::storage::access_condition(), options, m_context);
        CHECK_EQUAL(9U, m_context.request_results().size()); // PutBlock * 2 + PutBlockList
    }

    // Validate retry of download_range_to_stream_async.
    TEST_FIXTURE(block_blob_test_base, block_blob_retry)
    {
        std::vector<uint8_t> buffer;
        buffer.resize(1024);

        azure::storage::blob_request_options options;
        // attempt to retry one more time by default
        options.set_retry_policy(azure::storage::linear_retry_policy(std::chrono::seconds(1), 1));
        m_blob.upload_from_stream(concurrency::streams::bytestream::open_istream(buffer), azure::storage::access_condition(), options, m_context);

        Concurrency::streams::basic_ostream<uint8_t> target;
        pplx::task<void> task;

        // Validate no retry when stream is closed by Casablanca.
        {
            std::exception actual;
            target = currupted_stream<uint8_t>::open_ostream(false, 1);
            task = m_blob.download_range_to_stream_async(target, 0, 100, azure::storage::access_condition(), options, azure::storage::operation_context());
            CHECK_STORAGE_EXCEPTION(task.get(), INTENDED_ERR_MSG);
            CHECK_EQUAL(1, static_cast<const currupted_ostreambuf<uint8_t>*>(target.streambuf().get_base().get())->call_count());
        }

        // Validate exception will be propagated correctly even retry failed.
        {
            std::exception actual;
            target = currupted_stream<uint8_t>::open_ostream(true, 2);
            task = m_blob.download_range_to_stream_async(target, 0, 100, azure::storage::access_condition(), options, azure::storage::operation_context());
            CHECK_STORAGE_EXCEPTION(task.get(), INTENDED_ERR_MSG);
            CHECK_EQUAL(2, static_cast<const currupted_ostreambuf<uint8_t>*>(target.streambuf().get_base().get())->call_count());
        }

        // Validate no exception thrown when retry success.
        {
            target = currupted_stream<uint8_t>::open_ostream(true, 1);
            task = m_blob.download_range_to_stream_async(target, 0, 100, azure::storage::access_condition(), options, azure::storage::operation_context());
            CHECK_NOTHROW(task.get());
            CHECK_EQUAL(2, static_cast<const currupted_ostreambuf<uint8_t>*>(target.streambuf().get_base().get())->call_count());
        }
    }

    // Validate set standard blob tier for block blob on standard account.
    TEST_FIXTURE(block_blob_test_base, block_blob_standard_tier)
    {
        // preparation
        azure::storage::blob_request_options options;
        m_blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), options, m_context);

        // test can convert hot->cool or cool->hot.
        m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::cool, azure::storage::access_condition(), options, azure::storage::operation_context());
        // validate local has been updated.
        CHECK(azure::storage::standard_blob_tier::cool == m_blob.properties().standard_blob_tier());
        // validate server has been updated
        m_blob.download_attributes();
        CHECK(azure::storage::standard_blob_tier::cool == m_blob.properties().standard_blob_tier());
        m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::hot, azure::storage::access_condition(), options, azure::storage::operation_context());
        // validate local has been updated.
        CHECK(azure::storage::standard_blob_tier::hot == m_blob.properties().standard_blob_tier());
        // validate server has been updated
        m_blob.download_attributes();
        CHECK(azure::storage::standard_blob_tier::hot == m_blob.properties().standard_blob_tier());

        // test standard storage can set archive.
        m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::archive, azure::storage::access_condition(), options, azure::storage::operation_context());
        // validate server has been updated
        m_blob.download_attributes();
        CHECK(azure::storage::standard_blob_tier::archive == m_blob.properties().standard_blob_tier());
    }

    // Validate set standard blob tier for block blob on blob storage account.
    TEST_FIXTURE(premium_block_blob_test_base, block_blob_premium_tier)
    {
        // preparation
        azure::storage::blob_request_options options;
        m_blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), options, m_context);

        // test can convert hot->cool or cool->hot.
        m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::cool, azure::storage::access_condition(), options, azure::storage::operation_context());
        // validate local has been updated.
        CHECK(azure::storage::standard_blob_tier::cool == m_blob.properties().standard_blob_tier());
        // validate server has been updated
        m_blob.download_attributes();
        CHECK(azure::storage::standard_blob_tier::cool == m_blob.properties().standard_blob_tier());
        m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::hot, azure::storage::access_condition(), options, azure::storage::operation_context());
        // validate local has been updated.
        CHECK(azure::storage::standard_blob_tier::hot == m_blob.properties().standard_blob_tier());
        // validate server has been updated
        m_blob.download_attributes();
        CHECK(azure::storage::standard_blob_tier::hot == m_blob.properties().standard_blob_tier());

        // test premium storage can set archive.
        m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::archive, azure::storage::access_condition(), options, azure::storage::operation_context());
        // validate local has been updated.
        CHECK(azure::storage::standard_blob_tier::archive == m_blob.properties().standard_blob_tier());
        // validate server has been updated
        m_blob.download_attributes();
        CHECK(azure::storage::standard_blob_tier::archive == m_blob.properties().standard_blob_tier());

        // test archive storage can set back to archive.
        m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::archive, azure::storage::access_condition(), options, azure::storage::operation_context());
        // validate local has been changed.
        CHECK(azure::storage::standard_blob_tier::archive == m_blob.properties().standard_blob_tier());
        // validate server has been changed
        m_blob.download_attributes();
        CHECK(azure::storage::standard_blob_tier::archive == m_blob.properties().standard_blob_tier());

        // test archive storage can set back to cool.
        m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::cool, azure::storage::access_condition(), options, azure::storage::operation_context());
        // validate local has been not been updated.
        CHECK(azure::storage::standard_blob_tier::cool == m_blob.properties().standard_blob_tier());
        // validate server has been archive information
        m_blob.download_attributes();
        CHECK(azure::storage::archive_status::rehydrate_pending_to_cool == m_blob.properties().archive_status());
        //validate cannot set back to archive immediately
        CHECK_STORAGE_EXCEPTION(m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::archive, azure::storage::access_condition(), options, azure::storage::operation_context()), REHYDRATE_CANNOT_SET_TO_ARCHIVE_ERR_MSG);
    }

    TEST_FIXTURE(premium_block_blob_test_base, block_blob_premium_tier_with_lease)
    {
        // preparation
        azure::storage::blob_request_options options;
        m_blob.upload_text(_XPLATSTR("test"), azure::storage::access_condition(), options, m_context);

        // acquire a lease
        auto lease_id = m_blob.acquire_lease(azure::storage::lease_time(), _XPLATSTR(""));

        // set the acquired lease to access condition.
        azure::storage::access_condition condition;
        condition.set_lease_id(lease_id);

        // test can convert hot->cool or cool->hot.
        m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::cool, condition, options, azure::storage::operation_context());
        // validate local has been updated.
        CHECK(azure::storage::standard_blob_tier::cool == m_blob.properties().standard_blob_tier());
        // validate server has been updated
        m_blob.download_attributes();
        CHECK(azure::storage::standard_blob_tier::cool == m_blob.properties().standard_blob_tier());
        m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::hot, condition, options, azure::storage::operation_context());
        // validate local has been updated.
        CHECK(azure::storage::standard_blob_tier::hot == m_blob.properties().standard_blob_tier());
        // validate server has been updated
        m_blob.download_attributes();
        CHECK(azure::storage::standard_blob_tier::hot == m_blob.properties().standard_blob_tier());

        // test premium storage can set archive.
        m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::archive, condition, options, azure::storage::operation_context());
        // validate local has been updated.
        CHECK(azure::storage::standard_blob_tier::archive == m_blob.properties().standard_blob_tier());
        // validate server has been updated
        m_blob.download_attributes();
        CHECK(azure::storage::standard_blob_tier::archive == m_blob.properties().standard_blob_tier());

        // test archive storage can set back to archive.
        m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::archive, condition, options, azure::storage::operation_context());
        // validate local has been changed.
        CHECK(azure::storage::standard_blob_tier::archive == m_blob.properties().standard_blob_tier());
        // validate server has been changed
        m_blob.download_attributes();
        CHECK(azure::storage::standard_blob_tier::archive == m_blob.properties().standard_blob_tier());

        // test archive storage can set back to cool.
        m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::cool, condition, options, azure::storage::operation_context());
        // validate local has been not been updated.
        CHECK(azure::storage::standard_blob_tier::cool == m_blob.properties().standard_blob_tier());
        // validate server has been archive information
        m_blob.download_attributes();
        CHECK(azure::storage::archive_status::rehydrate_pending_to_cool == m_blob.properties().archive_status());
        //validate cannot set back to archive immediately
        CHECK_STORAGE_EXCEPTION(m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::archive, condition, options, azure::storage::operation_context()), REHYDRATE_CANNOT_SET_TO_ARCHIVE_ERR_MSG);
        CHECK(azure::storage::archive_status::rehydrate_pending_to_cool == m_blob.properties().archive_status());

        // validate no lease id would report failure.
        CHECK_STORAGE_EXCEPTION(m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::archive, azure::storage::access_condition(), options, azure::storage::operation_context()), ACTIVE_LEASE_ERROR_MESSAGE);
        CHECK(azure::storage::archive_status::rehydrate_pending_to_cool == m_blob.properties().archive_status());
    }

    TEST_FIXTURE(block_blob_test_base, block_blob_create_delete_cancellation)
    {

        {
            // cancel the cancellation prior to the operation
            auto cancel_token_src = pplx::cancellation_token_source();
            cancel_token_src.cancel();
            std::string ex_msg;

            try
            {
                auto task_result = m_blob.upload_text_async(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
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
                auto task_result = m_blob.upload_text_async(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
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
                auto task_result = m_blob.upload_text_async(_XPLATSTR("test"), azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
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

    TEST_FIXTURE(block_blob_test_base, block_blob_create_delete_timeout)
    {

        {
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(1));

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.upload_text_async(_XPLATSTR("test"), azure::storage::access_condition(), options, m_context);
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
                auto task_result = m_blob.upload_text_async(_XPLATSTR("test"), azure::storage::access_condition(), options, m_context);
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
                auto task_result = m_blob.upload_text_async(_XPLATSTR("test"), azure::storage::access_condition(), options, m_context);
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

    TEST_FIXTURE(block_blob_test_base, block_blob_create_delete_cancellation_timeout)
    {
        {
            //when cancellation first
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(100));
            auto cancel_token_src = pplx::cancellation_token_source();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.delete_blob_if_exists_async(azure::storage::delete_snapshots_option::include_snapshots, azure::storage::access_condition(), options, m_context, cancel_token_src.get_token());
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
            options.set_maximum_execution_time(std::chrono::milliseconds(3));
            auto cancel_token_src = pplx::cancellation_token_source();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.delete_blob_if_exists_async(azure::storage::delete_snapshots_option::include_snapshots, azure::storage::access_condition(), options, m_context, cancel_token_src.get_token());
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

    TEST_FIXTURE(block_blob_test_base, block_blob_open_read_write_cancellation)
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
                auto task_result = m_blob.open_write_async(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
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
                auto task_result = m_blob.open_write_async(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
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
                auto task_result = m_blob.open_write_async(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
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
                auto task_result = m_blob.open_write_async(azure::storage::access_condition(), azure::storage::blob_request_options(), m_context, cancel_token_src.get_token());
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

    TEST_FIXTURE(block_blob_test_base, block_blob_open_read_write_timeout)
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
                auto task_result = m_blob.open_write_async(azure::storage::access_condition(), options, m_context);
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
                auto task_result = m_blob.open_write_async(azure::storage::access_condition(), options, m_context);
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
            options.set_maximum_execution_time(std::chrono::seconds(20));

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.open_write_async(azure::storage::access_condition(), options, m_context);
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

    TEST_FIXTURE(block_blob_test_base, block_blob_open_read_write_cancellation_timeout)
    {
        std::vector<uint8_t> buffer;
        buffer.resize(4 * 1024 * 1024);
        fill_buffer(buffer);

        {
            //when cancellation first
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(100));
            auto cancel_token_src = pplx::cancellation_token_source();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.open_write_async(azure::storage::access_condition(), options, m_context, cancel_token_src.get_token());
                auto os = task_result.get();
                cancel_token_src.cancel();
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
            //when timeout first
            auto options = azure::storage::blob_request_options();
            options.set_maximum_execution_time(std::chrono::milliseconds(10));
            auto cancel_token_src = pplx::cancellation_token_source();

            std::string ex_msg;

            try
            {
                auto task_result = m_blob.open_write_async(azure::storage::access_condition(), options, m_context, cancel_token_src.get_token());
                auto os = task_result.get();
                os.streambuf().putn_nocopy(buffer.data(), buffer.size()).wait();
                std::this_thread::sleep_for(std::chrono::milliseconds(30)); //sleep for sometime before canceling the request and see result.
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

    TEST_FIXTURE(block_blob_test_base, block_blob_concurrent_upload_cancellation_timeout)
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

    TEST_FIXTURE(block_blob_test_base, block_blob_single_upload_cancellation_timeout)
    {
        utility::size64_t length = 128 * 1024 * 1024;
        std::vector<uint8_t> buffer;
        buffer.resize(length);
        fill_buffer(buffer);

        {
            auto cancel_token_src = pplx::cancellation_token_source();
            auto options = azure::storage::blob_request_options();
            options.set_single_blob_upload_threshold_in_bytes(length * 2);
            options.set_maximum_execution_time(std::chrono::milliseconds(500));
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
            options.set_single_blob_upload_threshold_in_bytes(length * 2);
            options.set_maximum_execution_time(std::chrono::milliseconds(500));

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
            options.set_single_blob_upload_threshold_in_bytes(length * 2);
            options.set_maximum_execution_time(std::chrono::milliseconds(500));

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

    TEST_FIXTURE(block_blob_test_base, block_blob_cpkv)
    {
        utility::size64_t length = 128 * 1024;
        std::vector<uint8_t> buffer(length);
        fill_buffer(buffer);
        auto empty_options = azure::storage::blob_request_options();
        auto cpk_options = azure::storage::blob_request_options();
        std::vector<uint8_t> key(32);
        fill_buffer(key);
        cpk_options.set_encryption_key(key);

        m_blob.upload_from_stream(concurrency::streams::bytestream::open_istream(buffer), length, azure::storage::access_condition(), cpk_options, m_context);

        for (const auto& options : { empty_options, cpk_options })
        {
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;
            concurrency::streams::ostream download_stream(download_buffer);
            if (options.encryption_key().empty())
            {
                CHECK_THROW(m_blob.download_to_stream(download_stream, azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);
            }
            else
            {
                m_blob.download_to_stream(download_stream, azure::storage::access_condition(), options, m_context);
                CHECK(!m_blob.properties().encryption_key_sha256().empty());
                CHECK(buffer == download_buffer.collection());
            }
        }

        auto binary_data = get_random_binary_data();
        binary_data.resize(10);
        utility::string_t block_id = utility::conversions::to_base64(binary_data);
        m_blob.upload_block(block_id, concurrency::streams::bytestream::open_istream(buffer), azure::storage::checksum_none, azure::storage::access_condition(), cpk_options, m_context);
        std::vector<azure::storage::block_list_item> blocks_id;
        blocks_id.emplace_back(block_id);
        m_blob.upload_block_list(blocks_id, azure::storage::access_condition(), cpk_options, m_context);

        for (const auto& options : { empty_options, cpk_options })
        {
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;
            concurrency::streams::ostream download_stream(download_buffer);
            if (options.encryption_key().empty())
            {
                CHECK_THROW(m_blob.download_to_stream(download_stream, azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);
            }
            else
            {
                m_blob.download_to_stream(download_stream, azure::storage::access_condition(), options, m_context);
                CHECK(!m_blob.properties().encryption_key_sha256().empty());
                CHECK(buffer == download_buffer.collection());
            }
        }

        m_blob.properties().set_content_type(_XPLATSTR("application/octet-stream"));
        m_blob.properties().set_content_language(_XPLATSTR("en-US"));
        CHECK_THROW(m_blob.upload_metadata(azure::storage::access_condition(), empty_options, m_context), azure::storage::storage_exception);
        CHECK_THROW(m_blob.download_attributes(azure::storage::access_condition(), empty_options, m_context), azure::storage::storage_exception);
        m_blob.upload_properties(azure::storage::access_condition(), empty_options, m_context);
        m_blob.upload_metadata(azure::storage::access_condition(), cpk_options, m_context);
        m_blob.download_attributes(azure::storage::access_condition(), cpk_options, m_context);

        CHECK_THROW(m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::cool, azure::storage::access_condition(), empty_options, m_context), azure::storage::storage_exception);
        CHECK_THROW(m_blob.set_standard_blob_tier(azure::storage::standard_blob_tier::cool, azure::storage::access_condition(), cpk_options, m_context), azure::storage::storage_exception);

        CHECK_THROW(m_blob.create_snapshot(azure::storage::cloud_metadata(), azure::storage::access_condition(), empty_options, m_context), azure::storage::storage_exception);
        auto snapshot_blob = m_blob.create_snapshot(azure::storage::cloud_metadata(), azure::storage::access_condition(), cpk_options, m_context);
        CHECK(snapshot_blob.is_snapshot());
        for (const auto& options : { empty_options, cpk_options })
        {
            concurrency::streams::container_buffer<std::vector<uint8_t>> download_buffer;
            concurrency::streams::ostream download_stream(download_buffer);
            if (options.encryption_key().empty())
            {
                CHECK_THROW(snapshot_blob.download_to_stream(download_stream, azure::storage::access_condition(), options, m_context), azure::storage::storage_exception);
            }
            else
            {
                snapshot_blob.download_to_stream(download_stream, azure::storage::access_condition(), options, m_context);
                CHECK(!snapshot_blob.properties().encryption_key_sha256().empty());
                CHECK(buffer == download_buffer.collection());
            }
        }
    }
}
