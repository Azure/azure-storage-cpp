// -----------------------------------------------------------------------------------------
// <copyright file="storage_exeption_test.cpp" company="Microsoft">
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
#include "table_test_base.h"
#include "check_macros.h"

SUITE(Core)
{
    TEST_FIXTURE(block_blob_test_base, storage_extended_error_verify_xml_with_details)
    {
        const size_t buffer_size = 8 * 1024;
        std::vector<uint8_t> buffer;
        buffer.resize(buffer_size);
        auto md5 = fill_buffer_and_get_md5(buffer);

        auto stream = concurrency::streams::bytestream::open_istream(buffer);
        m_blob.upload_block(get_block_id(0), stream, md5);

        try
        {
            stream.seek(1024, std::ios_base::beg);
            m_blob.upload_block(get_block_id(1), stream, md5);
            CHECK(false);
        }
        catch (azure::storage::storage_exception& ex)
        {
            CHECK_UTF8_EQUAL(_XPLATSTR("Md5Mismatch"), ex.result().extended_error().code());
            CHECK(!ex.result().extended_error().message().empty());
            CHECK(ex.result().extended_error().details().size() > 0);
        }
    }

    TEST_FIXTURE(table_service_test_base, storage_extended_error_verify_json_with_details)
    {
        utility::string_t table_name = get_table_name();
        azure::storage::cloud_table_client client = get_table_client();
        azure::storage::cloud_table table = client.get_table_reference(table_name);

        azure::storage::table_request_options options;
        azure::storage::operation_context context = m_context;
        try
        {
            table.delete_table(options, context);
            CHECK(false);
        }
        catch (const azure::storage::storage_exception& e)
        {
            CHECK(e.result().extended_error().code().compare(_XPLATSTR("ResourceNotFound")) == 0);
            CHECK(!e.result().extended_error().message().empty());
        }
    }

    TEST_FIXTURE(table_service_test_base, storage_extended_error_verify_xml_with_details)
    {
        utility::string_t table_name = get_table_name();
        azure::storage::cloud_table_client client = get_table_client();
        azure::storage::cloud_table table = client.get_table_reference(table_name);

        azure::storage::table_request_options options;
        azure::storage::operation_context context = m_context;
        context.set_sending_request([](web::http::http_request &r, azure::storage::operation_context) {
            r.headers()[_XPLATSTR("Accept")] = _XPLATSTR("application/xml");
        });

        try
        {
            table.exists(options, context);
            CHECK(false);
        }
        catch (const azure::storage::storage_exception& e)
        {
            CHECK(e.result().extended_error().code().compare(_XPLATSTR("MediaTypeNotSupported")) == 0);
            CHECK(!e.result().extended_error().message().empty());
        }
    }
}
