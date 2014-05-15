// -----------------------------------------------------------------------------------------
// <copyright file="cloud_table_client_test.cpp" company="Microsoft">
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
#include "test_helper.h"
#include "was/table.h"

SUITE(TableClient)
{
    TEST(TableClient_Empty)
    {
        azure::storage::cloud_table_client client;

        CHECK(client.base_uri().primary_uri().is_empty());
        CHECK(client.base_uri().secondary_uri().is_empty());
        CHECK(client.credentials().is_anonymous());
        CHECK(client.default_request_options().payload_format() == azure::storage::table_payload_format::json);
    }

    TEST(TableClient_BaseUri)
    {
        azure::storage::storage_uri base_uri(web::http::uri(U("https://myaccount.table.core.windows.net")), web::http::uri(U("https://myaccount-secondary.table.core.windows.net")));

        azure::storage::cloud_table_client client(base_uri);

        CHECK(client.base_uri().primary_uri() == base_uri.primary_uri());
        CHECK(client.base_uri().secondary_uri() == base_uri.secondary_uri());
        CHECK(client.credentials().is_anonymous());
        CHECK(client.default_request_options().payload_format() == azure::storage::table_payload_format::json);
    }

    TEST(TableClient_BaseUriAndCredentials)
    {
        azure::storage::storage_uri base_uri(web::http::uri(U("https://myaccount.table.core.windows.net")), web::http::uri(U("https://myaccount-secondary.table.core.windows.net")));
        azure::storage::storage_credentials credentials(U("devstoreaccount1"), U("Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="));
        azure::storage::table_request_options default_request_options;

        azure::storage::cloud_table_client client(base_uri, credentials);

        CHECK(client.base_uri().primary_uri() == base_uri.primary_uri());
        CHECK(client.base_uri().secondary_uri() == base_uri.secondary_uri());
        CHECK(client.credentials().is_shared_key());
        CHECK(client.default_request_options().payload_format() == azure::storage::table_payload_format::json);
    }

    TEST(TableClient_BaseUriAndCredentialsAndDefaultRequestOptions)
    {
        azure::storage::storage_uri base_uri(web::http::uri(U("https://myaccount.table.core.windows.net")), web::http::uri(U("https://myaccount-secondary.table.core.windows.net")));
        azure::storage::storage_credentials credentials(U("devstoreaccount1"), U("Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="));
        azure::storage::table_request_options default_request_options;

        azure::storage::cloud_table_client client(base_uri, credentials, default_request_options);

        CHECK(client.base_uri().primary_uri() == base_uri.primary_uri());
        CHECK(client.base_uri().secondary_uri() == base_uri.secondary_uri());
        CHECK(client.credentials().is_shared_key());
        CHECK(client.default_request_options().payload_format() == azure::storage::table_payload_format::json);

        default_request_options.set_payload_format(azure::storage::table_payload_format::json_no_metadata);

        client = azure::storage::cloud_table_client(base_uri, credentials, default_request_options);

        CHECK(client.default_request_options().payload_format() == azure::storage::table_payload_format::json_no_metadata);
    }

    TEST(ListTables_Normal)
    {
        const int TABLE_COUNT = 5;

        azure::storage::cloud_table tables[TABLE_COUNT];
        bool is_found[TABLE_COUNT];
        for (int i = 0; i < TABLE_COUNT; ++i)
        {
            tables[i] = get_table();
            is_found[i] = false;
        }

        azure::storage::cloud_table_client client = get_table_client();

        utility::string_t prefix;
        azure::storage::table_request_options options;
        azure::storage::operation_context context;

        prefix = object_name_prefix;

        std::vector<azure::storage::cloud_table> results = client.list_tables(prefix, options, context);

        CHECK(results.size() >= TABLE_COUNT);

        for (std::vector<azure::storage::cloud_table>::const_iterator itr = results.cbegin(); itr != results.cend(); ++itr)
        {
            azure::storage::cloud_table table = *itr;

            for (int i = 0; i < TABLE_COUNT; ++i)
            {
                if (table.name().compare(tables[i].name()) == 0)
                {
                    CHECK(!table.service_client().base_uri().primary_uri().is_empty());
                    CHECK(table.service_client().credentials().is_shared_key());
                    CHECK(!table.name().empty());
                    CHECK(!table.uri().primary_uri().is_empty());

                    is_found[i] = true;
                }
            }
        }

        CHECK(!context.client_request_id().empty());
        CHECK(context.start_time().is_initialized());
        CHECK(context.end_time().is_initialized());
        CHECK_EQUAL(1, context.request_results().size());
        CHECK(context.request_results()[0].is_response_available());
        CHECK(context.request_results()[0].start_time().is_initialized());
        CHECK(context.request_results()[0].end_time().is_initialized());
        CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
        CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
        CHECK(!context.request_results()[0].service_request_id().empty());
        CHECK(context.request_results()[0].request_date().is_initialized());
        CHECK(context.request_results()[0].content_md5().empty());
        CHECK(context.request_results()[0].etag().empty());
        CHECK(context.request_results()[0].extended_error().code().empty());
        CHECK(context.request_results()[0].extended_error().message().empty());
        CHECK(context.request_results()[0].extended_error().details().empty());

        for (int i = 0; i < TABLE_COUNT; ++i)
        {
            tables[i].delete_table();
        }
    }

    TEST(ListTables_Segmented)
    {
        const int TABLE_COUNT = 5;

        azure::storage::cloud_table tables[TABLE_COUNT];
        bool is_found[TABLE_COUNT];
        for (int i = 0; i < TABLE_COUNT; ++i)
        {
            tables[i] = get_table();
            is_found[i] = false;
        }

        azure::storage::cloud_table_client client = get_table_client();

        utility::string_t prefix;
        int max_results;
        azure::storage::continuation_token token;
        azure::storage::table_request_options options;
        azure::storage::operation_context context;

        prefix = object_name_prefix;
        max_results = 3;

        int segment_count = 0;
        azure::storage::table_result_segment result_segment;
        do
        {
            result_segment = client.list_tables_segmented(prefix, max_results, token, options, context);
            std::vector<azure::storage::cloud_table> results = result_segment.results();

            CHECK(results.size() >= 0);
            CHECK((int)results.size() <= max_results);

            for (std::vector<azure::storage::cloud_table>::const_iterator itr = results.cbegin(); itr != results.cend(); ++itr)
            {
                azure::storage::cloud_table table = *itr;

                for (int i = 0; i < TABLE_COUNT; ++i)
                {
                    if (table.name().compare(tables[i].name()) == 0)
                    {
                        CHECK(!table.service_client().base_uri().primary_uri().is_empty());
                        CHECK(table.service_client().credentials().is_shared_key());
                        CHECK(!table.name().empty());
                        CHECK(!table.uri().primary_uri().is_empty());

                        is_found[i] = true;
                    }
                }
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(segment_count + 1, context.request_results().size());
            CHECK(context.request_results()[segment_count].is_response_available());
            CHECK(context.request_results()[segment_count].start_time().is_initialized());
            CHECK(context.request_results()[segment_count].end_time().is_initialized());
            CHECK(context.request_results()[segment_count].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[segment_count].http_status_code());
            CHECK(!context.request_results()[segment_count].service_request_id().empty());
            CHECK(context.request_results()[segment_count].request_date().is_initialized());
            CHECK(context.request_results()[segment_count].content_md5().empty());
            CHECK(context.request_results()[segment_count].etag().empty());
            CHECK(context.request_results()[segment_count].extended_error().code().empty());
            CHECK(context.request_results()[segment_count].extended_error().message().empty());
            CHECK(context.request_results()[segment_count].extended_error().details().empty());

            ++segment_count;
            token = result_segment.continuation_token();
        }
        while (!token.empty());

        CHECK(segment_count > 1);

        for (int i = 0; i < TABLE_COUNT; ++i)
        {
            tables[i].delete_table();
        }
    }
}
