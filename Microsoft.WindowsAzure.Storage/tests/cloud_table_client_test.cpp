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
        wa::storage::cloud_table_client client;

        CHECK(client.base_uri().primary_uri().is_empty());
        CHECK(client.base_uri().secondary_uri().is_empty());
        CHECK(client.credentials().is_anonymous());
        CHECK(client.default_request_options().payload_format() == wa::storage::table_payload_format::json);
    }

    TEST(TableClient_BaseUri)
    {
        wa::storage::storage_uri base_uri(web::http::uri(U("https://myaccount.table.core.windows.net")), web::http::uri(U("https://myaccount-secondary.table.core.windows.net")));

        wa::storage::cloud_table_client client(base_uri);

        CHECK(client.base_uri().primary_uri() == base_uri.primary_uri());
        CHECK(client.base_uri().secondary_uri() == base_uri.secondary_uri());
        CHECK(client.credentials().is_anonymous());
        CHECK(client.default_request_options().payload_format() == wa::storage::table_payload_format::json);
    }

    TEST(TableClient_BaseUriAndCredentials)
    {
        wa::storage::storage_uri base_uri(web::http::uri(U("https://myaccount.table.core.windows.net")), web::http::uri(U("https://myaccount-secondary.table.core.windows.net")));
        wa::storage::storage_credentials credentials(U("devstoreaccount1"), U("Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="));
        wa::storage::table_request_options default_request_options;

        wa::storage::cloud_table_client client(base_uri, credentials);

        CHECK(client.base_uri().primary_uri() == base_uri.primary_uri());
        CHECK(client.base_uri().secondary_uri() == base_uri.secondary_uri());
        CHECK(client.credentials().is_shared_key());
        CHECK(client.default_request_options().payload_format() == wa::storage::table_payload_format::json);
    }

    TEST(TableClient_BaseUriAndCredentialsAndDefaultRequestOptions)
    {
        wa::storage::storage_uri base_uri(web::http::uri(U("https://myaccount.table.core.windows.net")), web::http::uri(U("https://myaccount-secondary.table.core.windows.net")));
        wa::storage::storage_credentials credentials(U("devstoreaccount1"), U("Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="));
        wa::storage::table_request_options default_request_options;

        wa::storage::cloud_table_client client(base_uri, credentials, default_request_options);

        CHECK(client.base_uri().primary_uri() == base_uri.primary_uri());
        CHECK(client.base_uri().secondary_uri() == base_uri.secondary_uri());
        CHECK(client.credentials().is_shared_key());
        CHECK(client.default_request_options().payload_format() == wa::storage::table_payload_format::json);

        default_request_options.set_payload_format(wa::storage::table_payload_format::json_no_metadata);

        client = wa::storage::cloud_table_client(base_uri, credentials, default_request_options);

        CHECK(client.default_request_options().payload_format() == wa::storage::table_payload_format::json_no_metadata);
    }

    TEST(ListTables_Normal)
    {
        const int TABLE_COUNT = 5;

        wa::storage::cloud_table tables[TABLE_COUNT];
        bool is_found[TABLE_COUNT];
        for (int i = 0; i < TABLE_COUNT; ++i)
        {
            tables[i] = get_table();
            is_found[i] = false;
        }

        wa::storage::cloud_table_client client = get_table_client();

        utility::string_t prefix;
        wa::storage::table_request_options options;
        wa::storage::operation_context context;

        prefix = object_name_prefix;

        std::vector<wa::storage::cloud_table> results = client.list_tables(prefix, options, context);

        CHECK(results.size() >= TABLE_COUNT);

        for (std::vector<wa::storage::cloud_table>::const_iterator itr = results.cbegin(); itr != results.cend(); ++itr)
        {
            wa::storage::cloud_table table = *itr;

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

        for (int i = 0; i < TABLE_COUNT; ++i)
        {
            tables[i].delete_table();
        }
    }

    TEST(ListTables_Segmented)
    {
        const int TABLE_COUNT = 5;

        wa::storage::cloud_table tables[TABLE_COUNT];
        bool is_found[TABLE_COUNT];
        for (int i = 0; i < TABLE_COUNT; ++i)
        {
            tables[i] = get_table();
            is_found[i] = false;
        }

        wa::storage::cloud_table_client client = get_table_client();

        utility::string_t prefix;
        int max_results;
        wa::storage::continuation_token continuation_token;
        wa::storage::table_request_options options;
        wa::storage::operation_context context;

        prefix = object_name_prefix;
        max_results = 3;

        int segment_count = 0;
        wa::storage::table_result_segment result_segment;
        do
        {
            result_segment = client.list_tables_segmented(prefix, max_results, continuation_token, options, context);
            std::vector<wa::storage::cloud_table> results = result_segment.results();

            CHECK(results.size() >= 0);
            CHECK((int)results.size() <= max_results);

            for (std::vector<wa::storage::cloud_table>::const_iterator itr = results.cbegin(); itr != results.cend(); ++itr)
            {
                wa::storage::cloud_table table = *itr;

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

            ++segment_count;
            continuation_token = result_segment.continuation_token();
        }
        while (!continuation_token.empty());

        CHECK(segment_count > 1);

        for (int i = 0; i < TABLE_COUNT; ++i)
        {
            tables[i].delete_table();
        }
    }
}
