// -----------------------------------------------------------------------------------------
// <copyright file="cloud_table_test.cpp" company="Microsoft">
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
#include "table_test_base.h"
#include "was/table.h"
#include "was/storage_account.h"

// TODO: Consider making storage_account.h automatically included from blob.h/table.h/queue.h

std::vector<azure::storage::table_entity> execute_table_query(
    const azure::storage::cloud_table& table,
    const azure::storage::table_query& query,
    const azure::storage::table_request_options& options,
    azure::storage::operation_context context)
{
    std::vector<azure::storage::table_entity> results;
    azure::storage::table_query_iterator end_of_result;
    for (azure::storage::table_query_iterator iter = table.execute_query(query, options, context); iter != end_of_result; ++iter)
    {
        results.push_back(*iter);
    }

    return results;
}

SUITE(Table)
{
    TEST_FIXTURE(table_service_test_base, Table_Empty)
    {
        azure::storage::cloud_table table;

        CHECK(table.service_client().base_uri().primary_uri().is_empty());
        CHECK(table.service_client().base_uri().secondary_uri().is_empty());
        CHECK(table.service_client().credentials().is_anonymous());
        CHECK(table.name().empty());
        CHECK(table.uri().primary_uri().is_empty());
        CHECK(table.uri().secondary_uri().is_empty());
    }

    TEST_FIXTURE(table_service_test_base, Table_Uri)
    {
        azure::storage::storage_uri uri(web::http::uri(_XPLATSTR("https://myaccount.table.core.windows.net/mytable")), web::http::uri(_XPLATSTR("https://myaccount-secondary.table.core.windows.net/mytable")));

        azure::storage::cloud_table table1(uri);

        web::http::uri expected_primary_uri(_XPLATSTR("https://myaccount.table.core.windows.net"));
        web::http::uri expected_secondary_uri(_XPLATSTR("https://myaccount-secondary.table.core.windows.net"));

        CHECK(table1.service_client().base_uri().primary_uri() == expected_primary_uri);
        CHECK(table1.service_client().base_uri().secondary_uri() == expected_secondary_uri);
        CHECK(table1.service_client().credentials().is_anonymous());
        CHECK(table1.name().compare(_XPLATSTR("mytable")) == 0);
        CHECK(table1.uri().primary_uri() == uri.primary_uri());
        CHECK(table1.uri().secondary_uri() == uri.secondary_uri());

        utility::string_t sas_token(_XPLATSTR("se=2013-05-15T17%3A20%3A36Z&sig=mysignature&sp=raud&st=2013-05-15T16%3A20%3A36Z&sv=2012-02-12&tn=people"));

        azure::storage::storage_uri sas_uri(web::http::uri(_XPLATSTR("https://myaccount.table.core.windows.net/mytable?tn=people&sp=raud&sv=2012-02-12&se=2013-05-15T17%3A20%3A36Z&st=2013-05-15T16%3A20%3A36Z&sig=mysignature")), web::http::uri(_XPLATSTR("https://myaccount-secondary.table.core.windows.net/mytable?tn=people&sp=raud&sv=2012-02-12&se=2013-05-15T17%3A20%3A36Z&st=2013-05-15T16%3A20%3A36Z&sig=mysignature")));

        azure::storage::cloud_table table2(sas_uri);

        CHECK(table2.service_client().base_uri().primary_uri() == expected_primary_uri);
        CHECK(table2.service_client().base_uri().secondary_uri() == expected_secondary_uri);
        CHECK(table2.service_client().credentials().is_sas());
        CHECK(table2.service_client().credentials().sas_token() == sas_token);
        CHECK(table2.name().compare(_XPLATSTR("mytable")) == 0);
        CHECK(table2.uri().primary_uri() == uri.primary_uri());
        CHECK(table2.uri().secondary_uri() == uri.secondary_uri());
    }

    TEST_FIXTURE(table_service_test_base, Table_UriAndCredentials)
    {
        azure::storage::storage_uri uri(web::http::uri(_XPLATSTR("https://myaccount.table.core.windows.net/mytable")), web::http::uri(_XPLATSTR("https://myaccount-secondary.table.core.windows.net/mytable")));
        azure::storage::storage_credentials credentials(_XPLATSTR("devstoreaccount1"), _XPLATSTR("Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="));

        azure::storage::cloud_table table(uri, credentials);

        web::http::uri expected_primary_uri(_XPLATSTR("https://myaccount.table.core.windows.net"));
        web::http::uri expected_secondary_uri(_XPLATSTR("https://myaccount-secondary.table.core.windows.net"));

        CHECK(table.service_client().base_uri().primary_uri() == expected_primary_uri);
        CHECK(table.service_client().base_uri().secondary_uri() == expected_secondary_uri);
        CHECK(table.service_client().credentials().is_shared_key());
        CHECK(table.name().compare(_XPLATSTR("mytable")) == 0);
        CHECK(table.uri().primary_uri() == uri.primary_uri());
        CHECK(table.uri().secondary_uri() == uri.secondary_uri());

        utility::string_t sas_token(_XPLATSTR("sv=2012-02-12&tn=people&st=2013-05-15T16%3A20%3A36Z&se=2013-05-15T17%3A20%3A36Z&sp=raud&sig=mysignature"));
        utility::string_t invalid_sas_token(_XPLATSTR("sv=2012-02-12&tn=people&st=2013-05-15T16%3A20%3A36Z&se=2013-05-15T17%3A20%3A36Z&sp=raud&sig=invalid"));

        azure::storage::storage_uri sas_uri(web::http::uri(_XPLATSTR("https://myaccount.table.core.windows.net/mytable?tn=people&sp=raud&sv=2012-02-12&se=2013-05-15T17%3A20%3A36Z&st=2013-05-15T16%3A20%3A36Z&sig=mysignature")), web::http::uri(_XPLATSTR("https://myaccount-secondary.table.core.windows.net/mytable?tn=people&sp=raud&sv=2012-02-12&se=2013-05-15T17%3A20%3A36Z&st=2013-05-15T16%3A20%3A36Z&sig=mysignature")));
        azure::storage::storage_credentials sas_credentials(sas_token);

        CHECK_THROW(azure::storage::cloud_table(sas_uri, sas_credentials), std::invalid_argument);

        azure::storage::storage_credentials invalid_sas_credentials(invalid_sas_token);

        CHECK_THROW(azure::storage::cloud_table(sas_uri, invalid_sas_credentials), std::invalid_argument);

        CHECK_THROW(azure::storage::cloud_table(sas_uri, credentials), std::invalid_argument);
    }

    TEST_FIXTURE(table_service_test_base, EntityProperty_Binary)
    {
        std::vector<uint8_t> value = get_random_binary_data();
        azure::storage::entity_property property(value);

        CHECK(property.property_type() == azure::storage::edm_type::binary);
        CHECK(!property.is_null());
        CHECK_ARRAY_EQUAL(value, property.binary_value(), (int)value.size());

        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        CHECK(property.str().size() > 0);

        value = get_random_binary_data();
        property.set_value(value);

        CHECK(property.property_type() == azure::storage::edm_type::binary);
        CHECK(!property.is_null());
        CHECK_ARRAY_EQUAL(value, property.binary_value(), (int)value.size());

        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        CHECK(property.str().size() > 0);

        utility::string_t invalid_value(_XPLATSTR("ABCDEFG"));
        property.set_value(invalid_value);
        property.set_property_type(azure::storage::edm_type::binary);

        CHECK(property.property_type() == azure::storage::edm_type::binary);
        CHECK(!property.is_null());
        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK(property.str().size() > 0);

        invalid_value = utility::string_t(_XPLATSTR("ABCDEFG-"));
        property.set_value(invalid_value);
        property.set_property_type(azure::storage::edm_type::binary);

        CHECK(property.property_type() == azure::storage::edm_type::binary);
        CHECK(!property.is_null());
        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK(property.str().size() > 0);

        invalid_value = utility::string_t(_XPLATSTR("ABCDEFG:"));
        property.set_value(invalid_value);
        property.set_property_type(azure::storage::edm_type::binary);

        CHECK(property.property_type() == azure::storage::edm_type::binary);
        CHECK(!property.is_null());
        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK(property.str().size() > 0);
    }

    TEST_FIXTURE(table_service_test_base, EntityProperty_Boolean)
    {
        bool value = get_random_boolean();
        azure::storage::entity_property property(value);

        CHECK(property.property_type() == azure::storage::edm_type::boolean);
        CHECK(!property.is_null());
        CHECK(property.boolean_value() == value);

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        CHECK(property.str().size() > 0);

        value = get_random_boolean();
        property.set_value(value);

        CHECK(property.property_type() == azure::storage::edm_type::boolean);
        CHECK(!property.is_null());
        CHECK(property.boolean_value() == value);

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        CHECK(property.str().size() > 0);

        utility::string_t invalid_value(_XPLATSTR("ABCD"));
        property.set_value(invalid_value);
        property.set_property_type(azure::storage::edm_type::boolean);

        CHECK(property.property_type() == azure::storage::edm_type::boolean);
        CHECK(!property.is_null());
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK(property.str().size() > 0);

        invalid_value = utility::string_t(_XPLATSTR("yes"));
        property.set_value(invalid_value);
        property.set_property_type(azure::storage::edm_type::boolean);

        CHECK(property.property_type() == azure::storage::edm_type::boolean);
        CHECK(!property.is_null());
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK(property.str().size() > 0);

        invalid_value = utility::string_t(_XPLATSTR("0"));
        property.set_value(invalid_value);
        property.set_property_type(azure::storage::edm_type::boolean);

        CHECK(property.property_type() == azure::storage::edm_type::boolean);
        CHECK(!property.is_null());
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK(property.str().size() > 0);

        invalid_value = utility::string_t(_XPLATSTR(""));
        property.set_value(invalid_value);
        property.set_property_type(azure::storage::edm_type::boolean);

        CHECK(property.property_type() == azure::storage::edm_type::boolean);
        CHECK(!property.is_null());
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_EQUAL(0U, property.str().size());
    }

    TEST_FIXTURE(table_service_test_base, EntityProperty_DateTime)
    {
        utility::datetime value = get_random_datetime();
        azure::storage::entity_property property(value);

        CHECK(property.property_type() == azure::storage::edm_type::datetime);
        CHECK(!property.is_null());
        CHECK_CLOSE(value.to_interval(), property.datetime_value().to_interval(), 10000000);

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        CHECK(property.str().size() > 0);

        value = get_random_datetime();
        property.set_value(value);

        CHECK(property.property_type() == azure::storage::edm_type::datetime);
        CHECK(!property.is_null());
        CHECK_CLOSE(value.to_interval(), property.datetime_value().to_interval(), 10000000);

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        CHECK(property.str().size() > 0);
    }

    TEST_FIXTURE(table_service_test_base, EntityProperty_Double)
    {
        double value = get_random_double();
        azure::storage::entity_property property(value);

        CHECK(property.property_type() == azure::storage::edm_type::double_floating_point);
        CHECK(!property.is_null());
        CHECK(property.double_value() == value);

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        CHECK(property.str().size() > 0);

        value = get_random_double();
        property.set_value(value);

        CHECK(property.property_type() == azure::storage::edm_type::double_floating_point);
        CHECK(!property.is_null());
        CHECK(property.double_value() == value);

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        CHECK(property.str().size() > 0);

        value = std::numeric_limits<double>::quiet_NaN();
        property.set_value(value);

        CHECK(property.property_type() == azure::storage::edm_type::double_floating_point);
        CHECK(!property.is_null());
        CHECK(property.double_value() != property.double_value()); // Only NaN is defined to not equal itself
        CHECK(property.str().size() > 0);

        value = std::numeric_limits<double>::infinity();
        property.set_value(value);

        CHECK(property.property_type() == azure::storage::edm_type::double_floating_point);
        CHECK(!property.is_null());
        CHECK(property.double_value() == value);
        CHECK(property.str().size() > 0);

        value = -std::numeric_limits<double>::infinity();
        property.set_value(value);

        CHECK(property.property_type() == azure::storage::edm_type::double_floating_point);
        CHECK(!property.is_null());
        CHECK(property.double_value() == value);
        CHECK(property.str().size() > 0);

        utility::string_t invalid_value(_XPLATSTR("ABCD"));
        property.set_value(invalid_value);
        property.set_property_type(azure::storage::edm_type::double_floating_point);

        CHECK(property.property_type() == azure::storage::edm_type::double_floating_point);
        CHECK(!property.is_null());
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK(property.str().size() > 0);

        invalid_value = utility::string_t(_XPLATSTR("123.450ABCD"));
        property.set_value(invalid_value);
        property.set_property_type(azure::storage::edm_type::double_floating_point);

        CHECK(property.property_type() == azure::storage::edm_type::double_floating_point);
        CHECK(!property.is_null());
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK(property.str().size() > 0);
    }

    TEST_FIXTURE(table_service_test_base, EntityProperty_Guid)
    {
        utility::uuid value = get_random_guid();
        azure::storage::entity_property property(value);

        CHECK(property.property_type() == azure::storage::edm_type::guid);
        CHECK(!property.is_null());
        CHECK(utility::uuid_equal(property.guid_value(), value));

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        CHECK(property.str().size() > 0);

        value = get_random_guid();
        property.set_value(value);

        CHECK(property.property_type() == azure::storage::edm_type::guid);
        CHECK(!property.is_null());
        CHECK(utility::uuid_equal(property.guid_value(), value));

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        CHECK(property.str().size() > 0);

        utility::string_t invalid_value(_XPLATSTR("abcd"));
        property.set_value(invalid_value);
        property.set_property_type(azure::storage::edm_type::guid);

        CHECK(property.property_type() == azure::storage::edm_type::guid);
        CHECK(!property.is_null());
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK(property.str().size() > 0);

        invalid_value = utility::string_t(_XPLATSTR("01234567-89ab-cdef-gggg-012345abcdef"));
        property.set_value(invalid_value);
        property.set_property_type(azure::storage::edm_type::guid);

        CHECK(property.property_type() == azure::storage::edm_type::guid);
        CHECK(!property.is_null());
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK(property.str().size() > 0);

        invalid_value = utility::string_t(_XPLATSTR("01234567-89ab-cdef------012345abcdef"));
        property.set_value(invalid_value);
        property.set_property_type(azure::storage::edm_type::guid);

        CHECK(property.property_type() == azure::storage::edm_type::guid);
        CHECK(!property.is_null());
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK(property.str().size() > 0);
    }

    TEST_FIXTURE(table_service_test_base, EntityProperty_Int32)
    {
        int32_t value = get_random_int32();
        azure::storage::entity_property property(value);

        CHECK(property.property_type() == azure::storage::edm_type::int32);
        CHECK(!property.is_null());
        CHECK(property.int32_value() == value);

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        CHECK(property.str().size() > 0);

        value = get_random_int32();
        property.set_value(value);

        CHECK(property.property_type() == azure::storage::edm_type::int32);
        CHECK(!property.is_null());
        CHECK(property.int32_value() == value);

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        CHECK(property.str().size() > 0);

        utility::string_t invalid_value(_XPLATSTR("abcd"));
        property.set_value(invalid_value);
        property.set_property_type(azure::storage::edm_type::int32);

        CHECK(property.property_type() == azure::storage::edm_type::int32);
        CHECK(!property.is_null());
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK(property.str().size() > 0);

        invalid_value = utility::string_t(_XPLATSTR("01234567-89ab-cdef-gggg-012345abcdef"));
        property.set_value(invalid_value);
        property.set_property_type(azure::storage::edm_type::int32);

        CHECK(property.property_type() == azure::storage::edm_type::int32);
        CHECK(!property.is_null());
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK(property.str().size() > 0);
    }

    TEST_FIXTURE(table_service_test_base, EntityProperty_Int64)
    {
        int64_t value = get_random_int64();
        azure::storage::entity_property property(value);

        CHECK(property.property_type() == azure::storage::edm_type::int64);
        CHECK(!property.is_null());
        CHECK(property.int64_value() == value);

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        CHECK(property.str().size() > 0);

        value = get_random_int64();
        property.set_value(value);

        CHECK(property.property_type() == azure::storage::edm_type::int64);
        CHECK(!property.is_null());
        CHECK(property.int64_value() == value);

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        CHECK(property.str().size() > 0);
    }

    TEST_FIXTURE(table_service_test_base, EntityProperty_String)
    {
        {
            utility::string_t value = get_random_string();
            azure::storage::entity_property property(value);

            CHECK(property.property_type() == azure::storage::edm_type::string);
            CHECK(!property.is_null());
            CHECK(property.string_value().compare(value) == 0);

            CHECK_THROW(property.binary_value(), std::runtime_error);
            CHECK_THROW(property.boolean_value(), std::runtime_error);
            CHECK_THROW(property.datetime_value(), std::runtime_error);
            CHECK_THROW(property.double_value(), std::runtime_error);
            CHECK_THROW(property.guid_value(), std::runtime_error);
            CHECK_THROW(property.int32_value(), std::runtime_error);
            CHECK_THROW(property.int64_value(), std::runtime_error);

            CHECK(property.str().size() > 0);

            value = get_random_string();
            property.set_value(value);

            CHECK(property.property_type() == azure::storage::edm_type::string);
            CHECK(!property.is_null());
            CHECK(property.string_value().compare(value) == 0);

            CHECK_THROW(property.binary_value(), std::runtime_error);
            CHECK_THROW(property.boolean_value(), std::runtime_error);
            CHECK_THROW(property.datetime_value(), std::runtime_error);
            CHECK_THROW(property.double_value(), std::runtime_error);
            CHECK_THROW(property.guid_value(), std::runtime_error);
            CHECK_THROW(property.int32_value(), std::runtime_error);
            CHECK_THROW(property.int64_value(), std::runtime_error);

            CHECK(property.str().size() > 0);
        }

        {
            utility::string_t value = get_random_string();
            azure::storage::entity_property property(value.c_str());

            CHECK(property.property_type() == azure::storage::edm_type::string);
            CHECK(!property.is_null());
            CHECK(property.string_value().compare(value) == 0);

            CHECK_THROW(property.binary_value(), std::runtime_error);
            CHECK_THROW(property.boolean_value(), std::runtime_error);
            CHECK_THROW(property.datetime_value(), std::runtime_error);
            CHECK_THROW(property.double_value(), std::runtime_error);
            CHECK_THROW(property.guid_value(), std::runtime_error);
            CHECK_THROW(property.int32_value(), std::runtime_error);
            CHECK_THROW(property.int64_value(), std::runtime_error);

            CHECK(property.str().size() > 0);

            value = get_random_string();
            property.set_value(value.c_str());

            CHECK(property.property_type() == azure::storage::edm_type::string);
            CHECK(!property.is_null());
            CHECK(property.string_value().compare(value) == 0);

            CHECK_THROW(property.binary_value(), std::runtime_error);
            CHECK_THROW(property.boolean_value(), std::runtime_error);
            CHECK_THROW(property.datetime_value(), std::runtime_error);
            CHECK_THROW(property.double_value(), std::runtime_error);
            CHECK_THROW(property.guid_value(), std::runtime_error);
            CHECK_THROW(property.int32_value(), std::runtime_error);
            CHECK_THROW(property.int64_value(), std::runtime_error);

            CHECK(property.str().size() > 0);
        }
    }

    TEST_FIXTURE(table_service_test_base, EntityProperty_Null)
    {
        azure::storage::entity_property property;

        CHECK(property.property_type() == azure::storage::edm_type::string);
        CHECK(property.is_null());
        CHECK(property.string_value().empty());

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);

        property.set_is_null(false);

        CHECK(property.property_type() == azure::storage::edm_type::string);
        CHECK(!property.is_null());
        CHECK(property.string_value().empty());

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);

        property.set_is_null(true);

        CHECK(property.property_type() == azure::storage::edm_type::string);
        CHECK(property.is_null());
        CHECK(property.string_value().empty());

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);

        int32_t value = get_random_int32();
        property.set_value(value);

        CHECK(property.property_type() == azure::storage::edm_type::int32);
        CHECK(!property.is_null());
        CHECK(property.int32_value() == value);

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        property.set_is_null(true);

        CHECK(property.property_type() == azure::storage::edm_type::int32);
        CHECK(property.is_null());

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);
    }

    TEST_FIXTURE(table_service_test_base, Entity_PartitionKeyAndRowKey)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();

        azure::storage::table_entity entity(partition_key, row_key);

        CHECK(entity.partition_key().compare(partition_key) == 0);
        CHECK(entity.row_key().compare(row_key) == 0);
        CHECK(entity.etag().empty());
        CHECK(entity.properties().empty());
        CHECK(!entity.timestamp().is_initialized());

        partition_key = get_random_string();
        entity.set_partition_key(partition_key);

        CHECK(entity.partition_key().compare(partition_key) == 0);

        row_key = get_random_string();
        entity.set_row_key(row_key);

        CHECK(entity.row_key().compare(row_key) == 0);

        utility::string_t etag = get_random_string();
        entity.set_etag(etag);

        CHECK(entity.etag().compare(etag) == 0);

        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyA"), azure::storage::entity_property(get_random_boolean())));
        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyG"), azure::storage::entity_property(get_random_binary_data())));

        CHECK(entity.properties().size() == 2U);
    }

    TEST_FIXTURE(table_service_test_base, Entity_PartitionKeyAndRowKeyAndETagAndProperties)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();
        utility::string_t etag = get_random_string();
        azure::storage::table_entity::properties_type properties;

        properties.reserve(3);
        properties.insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyD"), azure::storage::entity_property(get_random_double())));
        properties.insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(get_random_string())));
        properties.insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyH"), azure::storage::entity_property(get_random_guid())));

        azure::storage::table_entity entity(partition_key, row_key, etag, properties);

        CHECK(entity.partition_key().compare(partition_key) == 0);
        CHECK(entity.row_key().compare(row_key) == 0);
        CHECK(entity.etag().compare(etag) == 0);
        CHECK(entity.properties().size() == 3U);
        CHECK(!entity.timestamp().is_initialized());

        partition_key = get_random_string();
        entity.set_partition_key(partition_key);

        CHECK(entity.partition_key().compare(partition_key) == 0);

        row_key = get_random_string();
        entity.set_row_key(row_key);

        CHECK(entity.row_key().compare(row_key) == 0);

        etag = get_random_string();
        entity.set_etag(etag);

        CHECK(entity.etag().compare(etag) == 0);

        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyA"), azure::storage::entity_property(get_random_boolean())));
        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyG"), azure::storage::entity_property(get_random_binary_data())));

        CHECK(entity.properties().size() == 5U);
    }

    TEST_FIXTURE(table_service_test_base, Operation_Delete)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();
        azure::storage::table_entity entity(partition_key, row_key);

        azure::storage::table_operation operation = azure::storage::table_operation::delete_entity(entity);

        CHECK(operation.entity().partition_key().compare(partition_key) == 0);
        CHECK(operation.entity().row_key().compare(row_key) == 0);
        CHECK(operation.entity().timestamp() == entity.timestamp());
        CHECK(operation.entity().etag().compare(entity.etag()) == 0);
        CHECK(operation.operation_type() == azure::storage::table_operation_type::delete_operation);
    }

    TEST_FIXTURE(table_service_test_base, Operation_Insert)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();
        azure::storage::table_entity entity(partition_key, row_key);

        azure::storage::table_operation operation = azure::storage::table_operation::insert_entity(entity);

        CHECK(operation.entity().partition_key().compare(partition_key) == 0);
        CHECK(operation.entity().row_key().compare(row_key) == 0);
        CHECK(operation.entity().timestamp() == entity.timestamp());
        CHECK(operation.entity().etag().compare(entity.etag()) == 0);
        CHECK(operation.operation_type() == azure::storage::table_operation_type::insert_operation);
    }

    TEST_FIXTURE(table_service_test_base, Operation_InsertOrMerge)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();
        azure::storage::table_entity entity(partition_key, row_key);

        azure::storage::table_operation operation = azure::storage::table_operation::insert_or_merge_entity(entity);

        CHECK(operation.entity().partition_key().compare(partition_key) == 0);
        CHECK(operation.entity().row_key().compare(row_key) == 0);
        CHECK(operation.entity().timestamp() == entity.timestamp());
        CHECK(operation.entity().etag().compare(entity.etag()) == 0);
        CHECK(operation.operation_type() == azure::storage::table_operation_type::insert_or_merge_operation);
    }

    TEST_FIXTURE(table_service_test_base, Operation_InsertOrReplace)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();
        azure::storage::table_entity entity(partition_key, row_key);

        azure::storage::table_operation operation = azure::storage::table_operation::insert_or_replace_entity(entity);

        CHECK(operation.entity().partition_key().compare(partition_key) == 0);
        CHECK(operation.entity().row_key().compare(row_key) == 0);
        CHECK(operation.entity().timestamp() == entity.timestamp());
        CHECK(operation.entity().etag().compare(entity.etag()) == 0);
        CHECK(operation.operation_type() == azure::storage::table_operation_type::insert_or_replace_operation);
    }

    TEST_FIXTURE(table_service_test_base, Operation_Merge)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();
        azure::storage::table_entity entity(partition_key, row_key);

        azure::storage::table_operation operation = azure::storage::table_operation::merge_entity(entity);

        CHECK(operation.entity().partition_key().compare(partition_key) == 0);
        CHECK(operation.entity().row_key().compare(row_key) == 0);
        CHECK(operation.entity().timestamp() == entity.timestamp());
        CHECK(operation.entity().etag().compare(entity.etag()) == 0);
        CHECK(operation.operation_type() == azure::storage::table_operation_type::merge_operation);
    }

    TEST_FIXTURE(table_service_test_base, Operation_Replace)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();
        azure::storage::table_entity entity(partition_key, row_key);

        azure::storage::table_operation operation = azure::storage::table_operation::replace_entity(entity);

        CHECK(operation.entity().partition_key().compare(partition_key) == 0);
        CHECK(operation.entity().row_key().compare(row_key) == 0);
        CHECK(operation.entity().timestamp() == entity.timestamp());
        CHECK(operation.entity().etag().compare(entity.etag()) == 0);
        CHECK(operation.operation_type() == azure::storage::table_operation_type::replace_operation);
    }

    TEST_FIXTURE(table_service_test_base, Operation_Retrieve)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();

        azure::storage::table_operation operation = azure::storage::table_operation::retrieve_entity(partition_key, row_key);

        CHECK(operation.entity().partition_key().compare(partition_key) == 0);
        CHECK(operation.entity().row_key().compare(row_key) == 0);
        CHECK(!operation.entity().timestamp().is_initialized());
        CHECK(operation.entity().etag().empty());
        CHECK(operation.operation_type() == azure::storage::table_operation_type::retrieve_operation);
    }

    TEST_FIXTURE(table_service_test_base, Query_Normal)
    {
        azure::storage::table_query query;

        CHECK_EQUAL(-1, query.take_count());
        CHECK(query.filter_string().empty());
        CHECK(query.select_columns().empty());

        int take_count = 25;
        query.set_take_count(take_count);

        CHECK_EQUAL(take_count, query.take_count());

        utility::string_t filter_string(_XPLATSTR("PartitionKey eq 'AAA' and RowKey lt 'BBB' and Timestamp ge datetime'2013-09-01T00:00:00Z'"));
        query.set_filter_string(filter_string);

        CHECK(query.filter_string().compare(filter_string) == 0);

        std::vector<utility::string_t> select_columns;
        select_columns.reserve(3);
        select_columns.push_back(_XPLATSTR("PropertyA"));
        select_columns.push_back(_XPLATSTR("PropertyB"));
        select_columns.push_back(_XPLATSTR("PropertyC"));
        query.set_select_columns(select_columns);

        CHECK_EQUAL(select_columns.size(), query.select_columns().size());
        CHECK(query.select_columns()[0].compare(_XPLATSTR("PropertyA")) == 0);
        CHECK(query.select_columns()[1].compare(_XPLATSTR("PropertyB")) == 0);
        CHECK(query.select_columns()[2].compare(_XPLATSTR("PropertyC")) == 0);
    }

    TEST_FIXTURE(table_service_test_base, TableRequestOptions_Normal)
    {
        azure::storage::table_request_options options;

        CHECK(options.payload_format() == azure::storage::table_payload_format::json);

        azure::storage::table_payload_format payload_format = azure::storage::table_payload_format::json_no_metadata;
        options.set_payload_format(payload_format);

        CHECK(options.payload_format() == payload_format);
    }

    TEST_FIXTURE(table_service_test_base, Table_CreateAndDelete)
    {
        utility::string_t table_name = get_table_name();
        azure::storage::cloud_table_client client = get_table_client();

        azure::storage::cloud_table table = client.get_table_reference(table_name);

        {
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            table.create(options, context);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            bool exists = table.exists(options, context);

            CHECK(exists);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
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
        }

        {
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            try
            {
                table.create(options, context);
                CHECK(false);
            }
            catch (const azure::storage::storage_exception& e)
            {
                CHECK_EQUAL(web::http::status_codes::Conflict, e.result().http_status_code());
                CHECK(e.result().extended_error().code().compare(_XPLATSTR("TableAlreadyExists")) == 0);
                CHECK(!e.result().extended_error().message().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::Conflict, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().compare(_XPLATSTR("TableAlreadyExists")) == 0);
            CHECK(!context.request_results()[0].extended_error().message().empty());
        }

        {
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            table.delete_table(options, context);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            bool exists = table.exists(options, context);

            CHECK(!exists);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NotFound, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            try
            {
                table.delete_table(options, context);
                CHECK(false);
            }
            catch (const azure::storage::storage_exception& e)
            {
                CHECK_EQUAL(web::http::status_codes::NotFound, e.result().http_status_code());
                CHECK(e.result().extended_error().code().compare(_XPLATSTR("ResourceNotFound")) == 0);
                CHECK(!e.result().extended_error().message().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NotFound, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().compare(_XPLATSTR("ResourceNotFound")) == 0);
            CHECK(!context.request_results()[0].extended_error().message().empty());
        }
    }

    TEST_FIXTURE(table_service_test_base, Table_CreateIfNotExistsAndDeleteIfExists)
    {
        utility::string_t table_name = get_table_name();
        azure::storage::cloud_table_client client = get_table_client();

        azure::storage::cloud_table table = client.get_table_reference(table_name);

        {
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            bool created = table.create_if_not_exists(options, context);

            CHECK(created);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(2U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NotFound, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
            CHECK(context.request_results()[1].is_response_available());
            CHECK(context.request_results()[1].start_time().is_initialized());
            CHECK(context.request_results()[1].end_time().is_initialized());
            CHECK(context.request_results()[1].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[1].http_status_code());
            CHECK(!context.request_results()[1].service_request_id().empty());
            CHECK(context.request_results()[1].request_date().is_initialized());
            CHECK(context.request_results()[1].content_md5().empty());
            CHECK(context.request_results()[1].etag().empty());
            CHECK(context.request_results()[1].extended_error().code().empty());
            CHECK(context.request_results()[1].extended_error().message().empty());
            CHECK(context.request_results()[1].extended_error().details().empty());
        }

        {
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            bool exists = table.exists(options, context);

            CHECK(exists);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
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
        }

        {
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            bool created = table.create_if_not_exists(options, context);

            CHECK(!created);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
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
        }

        {
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            bool deleted = table.delete_table_if_exists(options, context);

            CHECK(deleted);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(2U, context.request_results().size());
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
            CHECK(context.request_results()[1].is_response_available());
            CHECK(context.request_results()[1].start_time().is_initialized());
            CHECK(context.request_results()[1].end_time().is_initialized());
            CHECK(context.request_results()[1].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[1].http_status_code());
            CHECK(!context.request_results()[1].service_request_id().empty());
            CHECK(context.request_results()[1].request_date().is_initialized());
            CHECK(context.request_results()[1].content_md5().empty());
            CHECK(context.request_results()[1].etag().empty());
            CHECK(context.request_results()[1].extended_error().code().empty());
            CHECK(context.request_results()[1].extended_error().message().empty());
            CHECK(context.request_results()[1].extended_error().details().empty());
        }

        {
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            bool exists = table.exists(options, context);

            CHECK(!exists);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NotFound, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            bool deleted = table.delete_table_if_exists(options, context);

            CHECK(!deleted);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NotFound, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }
    }

    TEST_FIXTURE(table_service_test_base, Table_NotFound)
    {
        utility::string_t table_name = get_table_name();
        azure::storage::cloud_table_client client = get_table_client();

        azure::storage::cloud_table table = client.get_table_reference(table_name);

        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();

        int32_t int32_value = get_random_int32();
        utility::string_t string_value = get_random_string();

        azure::storage::table_entity entity(partition_key, row_key);

        entity.properties().reserve(2);
        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyA"), azure::storage::entity_property(int32_value)));
        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyB"), azure::storage::entity_property(string_value)));

        azure::storage::table_operation operation = azure::storage::table_operation::insert_entity(entity);
        azure::storage::table_request_options options;
        azure::storage::operation_context context;
        print_client_request_id(context, _XPLATSTR(""));

        try
        {
            table.execute(operation, options, context);
            CHECK(false);
        }
        catch (const azure::storage::storage_exception& e)
        {
            CHECK_EQUAL(web::http::status_codes::NotFound, e.result().http_status_code());
            CHECK(e.result().extended_error().code().compare(_XPLATSTR("TableNotFound")) == 0);
            CHECK(!e.result().extended_error().message().empty());
        }

        CHECK(!context.client_request_id().empty());
        CHECK(context.start_time().is_initialized());
        CHECK(context.end_time().is_initialized());
        CHECK_EQUAL(1U, context.request_results().size());
        CHECK(context.request_results()[0].is_response_available());
        CHECK(context.request_results()[0].start_time().is_initialized());
        CHECK(context.request_results()[0].end_time().is_initialized());
        CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
        CHECK_EQUAL(web::http::status_codes::NotFound, context.request_results()[0].http_status_code());
        CHECK(!context.request_results()[0].service_request_id().empty());
        CHECK(context.request_results()[0].request_date().is_initialized());
        CHECK(context.request_results()[0].content_md5().empty());
        CHECK(context.request_results()[0].etag().empty());
        CHECK(context.request_results()[0].extended_error().code().compare(_XPLATSTR("TableNotFound")) == 0);
        CHECK(!context.request_results()[0].extended_error().message().empty());
    }

    TEST_FIXTURE(table_service_test_base, EntityOperation_InsertAndDelete)
    {
        azure::storage::cloud_table table = get_table();

        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();

        bool boolean_value = get_random_boolean();
        int32_t int32_value = get_random_int32();
        int64_t int64_value = get_random_int64();
        double double_value = get_random_double();
        utility::string_t string_value = get_random_string();
        utility::datetime datetime_value = get_random_datetime();
        std::vector<uint8_t> binary_value = get_random_binary_data();
        utility::uuid guid_value = get_random_guid();

        {
            azure::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyA"), azure::storage::entity_property(boolean_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyB"), azure::storage::entity_property(int32_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyC"), azure::storage::entity_property(int64_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyD"), azure::storage::entity_property(double_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(string_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyF"), azure::storage::entity_property(datetime_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyG"), azure::storage::entity_property(binary_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyH"), azure::storage::entity_property(guid_value)));

            azure::storage::table_operation operation = azure::storage::table_operation::insert_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_operation operation = azure::storage::table_operation::retrieve_entity(partition_key, row_key);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(!result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(8U, result.entity().properties().size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyA")) != result.entity().properties().cend());
            CHECK_EQUAL(boolean_value, result.entity().properties().find(_XPLATSTR("PropertyA"))->second.boolean_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyB")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value, result.entity().properties().find(_XPLATSTR("PropertyB"))->second.int32_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyC")) != result.entity().properties().cend());
            CHECK_EQUAL(int64_value, result.entity().properties().find(_XPLATSTR("PropertyC"))->second.int64_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyD")) != result.entity().properties().cend());
            CHECK_EQUAL(double_value, result.entity().properties().find(_XPLATSTR("PropertyD"))->second.double_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE"))->second.string_value().compare(string_value) == 0);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF"))->second.datetime_value() == datetime_value);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyG")) != result.entity().properties().cend());
            CHECK_ARRAY_EQUAL(binary_value, result.entity().properties().find(_XPLATSTR("PropertyG"))->second.binary_value(), (int)binary_value.size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyH")) != result.entity().properties().cend());
            CHECK(utility::uuid_equal(result.entity().properties().find(_XPLATSTR("PropertyH"))->second.guid_value(), guid_value));

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyA"), azure::storage::entity_property(boolean_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyB"), azure::storage::entity_property(int32_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyC"), azure::storage::entity_property(int64_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyD"), azure::storage::entity_property(double_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(string_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyF"), azure::storage::entity_property(datetime_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyG"), azure::storage::entity_property(binary_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyH"), azure::storage::entity_property(guid_value)));

            azure::storage::table_operation operation = azure::storage::table_operation::insert_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            try
            {
                table.execute(operation, options, context);
                CHECK(false);
            }
            catch (const azure::storage::storage_exception& e)
            {
                CHECK_EQUAL(web::http::status_codes::Conflict, e.result().http_status_code());
                CHECK(e.result().extended_error().code().compare(_XPLATSTR("EntityAlreadyExists")) == 0);
                CHECK(!e.result().extended_error().message().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::Conflict, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().compare(_XPLATSTR("EntityAlreadyExists")) == 0);
            CHECK(!context.request_results()[0].extended_error().message().empty());
        }

        {
            //utility::string_t etag(_XPLATSTR("*"));

            azure::storage::table_entity entity(partition_key, row_key);
            //entity.set_etag(etag);

            azure::storage::table_operation operation = azure::storage::table_operation::delete_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(result.etag().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_operation operation = azure::storage::table_operation::retrieve_entity(partition_key, row_key);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(404, result.http_status_code());
            CHECK(result.etag().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NotFound, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_entity entity(partition_key, row_key);

            azure::storage::table_operation operation = azure::storage::table_operation::delete_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            try
            {
                table.execute(operation, options, context);
                CHECK(false);
            }
            catch (const azure::storage::storage_exception& e)
            {
                CHECK_EQUAL(web::http::status_codes::NotFound, e.result().http_status_code());
                CHECK(e.result().extended_error().code().compare(_XPLATSTR("ResourceNotFound")) == 0);
                CHECK(!e.result().extended_error().message().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NotFound, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().compare(_XPLATSTR("ResourceNotFound")) == 0);
            CHECK(!context.request_results()[0].extended_error().message().empty());
        }

        table.delete_table();
    }

    TEST_FIXTURE(table_service_test_base, EntityOperation_InsertAndMerge)
    {
        azure::storage::cloud_table table = get_table();

        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();

        bool boolean_value = get_random_boolean();
        int32_t int32_value = get_random_int32();
        int64_t int64_value = get_random_int64();
        double double_value = get_random_double();
        utility::string_t string_value = get_random_string();
        utility::datetime datetime_value = get_random_datetime();
        std::vector<uint8_t> binary_value = get_random_binary_data();
        utility::uuid guid_value = get_random_guid();

        {
            azure::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyA"), azure::storage::entity_property(boolean_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyB"), azure::storage::entity_property(int32_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyC"), azure::storage::entity_property(int64_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyD"), azure::storage::entity_property(double_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(string_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyF"), azure::storage::entity_property(datetime_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyG"), azure::storage::entity_property(binary_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyH"), azure::storage::entity_property(guid_value)));

            azure::storage::table_operation operation = azure::storage::table_operation::insert_or_merge_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_operation operation = azure::storage::table_operation::retrieve_entity(partition_key, row_key);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(!result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(8U, result.entity().properties().size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyA")) != result.entity().properties().cend());
            CHECK_EQUAL(boolean_value, result.entity().properties().find(_XPLATSTR("PropertyA"))->second.boolean_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyB")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value, result.entity().properties().find(_XPLATSTR("PropertyB"))->second.int32_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyC")) != result.entity().properties().cend());
            CHECK_EQUAL(int64_value, result.entity().properties().find(_XPLATSTR("PropertyC"))->second.int64_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyD")) != result.entity().properties().cend());
            CHECK_EQUAL(double_value, result.entity().properties().find(_XPLATSTR("PropertyD"))->second.double_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE"))->second.string_value().compare(string_value) == 0);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF"))->second.datetime_value() == datetime_value);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyG")) != result.entity().properties().cend());
            CHECK_ARRAY_EQUAL(binary_value, result.entity().properties().find(_XPLATSTR("PropertyG"))->second.binary_value(), (int)binary_value.size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyH")) != result.entity().properties().cend());
            CHECK(utility::uuid_equal(result.entity().properties().find(_XPLATSTR("PropertyH"))->second.guid_value(), guid_value));

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        int64_value = get_random_int64();
        double_value = get_random_double();
        string_value = get_random_string();
        datetime_value = get_random_datetime();
        binary_value = get_random_binary_data();
        guid_value = get_random_guid();

        int32_t int32_value2 = get_random_int32();
        int32_t int32_value3 = get_random_int32();

        {
            azure::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyC"), azure::storage::entity_property(int64_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyD"), azure::storage::entity_property(double_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(string_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyF"), azure::storage::entity_property(datetime_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyG"), azure::storage::entity_property(binary_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyH"), azure::storage::entity_property(guid_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyI"), azure::storage::entity_property(int32_value2)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyJ"), azure::storage::entity_property(int32_value3)));

            azure::storage::table_operation operation = azure::storage::table_operation::insert_or_merge_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_operation operation = azure::storage::table_operation::retrieve_entity(partition_key, row_key);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(!result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(10U, result.entity().properties().size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyA")) != result.entity().properties().cend());
            CHECK_EQUAL(boolean_value, result.entity().properties().find(_XPLATSTR("PropertyA"))->second.boolean_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyB")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value, result.entity().properties().find(_XPLATSTR("PropertyB"))->second.int32_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyC")) != result.entity().properties().cend());
            CHECK_EQUAL(int64_value, result.entity().properties().find(_XPLATSTR("PropertyC"))->second.int64_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyD")) != result.entity().properties().cend());
            CHECK_EQUAL(double_value, result.entity().properties().find(_XPLATSTR("PropertyD"))->second.double_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE"))->second.string_value().compare(string_value) == 0);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF"))->second.datetime_value() == datetime_value);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyG")) != result.entity().properties().cend());
            CHECK_ARRAY_EQUAL(binary_value, result.entity().properties().find(_XPLATSTR("PropertyG"))->second.binary_value(), (int)binary_value.size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyH")) != result.entity().properties().cend());
            CHECK(utility::uuid_equal(result.entity().properties().find(_XPLATSTR("PropertyH"))->second.guid_value(), guid_value));
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyI")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value2, result.entity().properties().find(_XPLATSTR("PropertyI"))->second.int32_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyJ")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value3, result.entity().properties().find(_XPLATSTR("PropertyJ"))->second.int32_value());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        int64_value = get_random_int64();
        double_value = get_random_double();
        string_value = get_random_string();
        datetime_value = get_random_datetime();
        binary_value = get_random_binary_data();
        guid_value = get_random_guid();

        int32_t int32_value4 = get_random_int32();
        int32_t int32_value5 = get_random_int32();

        {
            azure::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyC"), azure::storage::entity_property(int64_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyD"), azure::storage::entity_property(double_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(string_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyF"), azure::storage::entity_property(datetime_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyG"), azure::storage::entity_property(binary_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyH"), azure::storage::entity_property(guid_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyK"), azure::storage::entity_property(int32_value4)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyL"), azure::storage::entity_property(int32_value5)));

            azure::storage::table_operation operation = azure::storage::table_operation::merge_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_operation operation = azure::storage::table_operation::retrieve_entity(partition_key, row_key);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(!result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(12U, result.entity().properties().size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyA")) != result.entity().properties().cend());
            CHECK_EQUAL(boolean_value, result.entity().properties().find(_XPLATSTR("PropertyA"))->second.boolean_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyB")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value, result.entity().properties().find(_XPLATSTR("PropertyB"))->second.int32_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyC")) != result.entity().properties().cend());
            CHECK_EQUAL(int64_value, result.entity().properties().find(_XPLATSTR("PropertyC"))->second.int64_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyD")) != result.entity().properties().cend());
            CHECK_EQUAL(double_value, result.entity().properties().find(_XPLATSTR("PropertyD"))->second.double_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE"))->second.string_value().compare(string_value) == 0);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF"))->second.datetime_value() == datetime_value);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyG")) != result.entity().properties().cend());
            CHECK_ARRAY_EQUAL(binary_value, result.entity().properties().find(_XPLATSTR("PropertyG"))->second.binary_value(), (int)binary_value.size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyH")) != result.entity().properties().cend());
            CHECK(utility::uuid_equal(result.entity().properties().find(_XPLATSTR("PropertyH"))->second.guid_value(), guid_value));
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyI")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value2, result.entity().properties().find(_XPLATSTR("PropertyI"))->second.int32_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyJ")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value3, result.entity().properties().find(_XPLATSTR("PropertyJ"))->second.int32_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyK")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value4, result.entity().properties().find(_XPLATSTR("PropertyK"))->second.int32_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyL")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value5, result.entity().properties().find(_XPLATSTR("PropertyL"))->second.int32_value());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_entity entity(partition_key, row_key);

            azure::storage::table_operation operation = azure::storage::table_operation::delete_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(result.etag().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        int64_value = get_random_int64();
        double_value = get_random_double();
        string_value = get_random_string();
        datetime_value = get_random_datetime();
        binary_value = get_random_binary_data();
        guid_value = get_random_guid();

        int32_t int32_value6 = get_random_int32();
        int32_t int32_value7 = get_random_int32();

        {
            azure::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyC"), azure::storage::entity_property(int64_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyD"), azure::storage::entity_property(double_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(string_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyF"), azure::storage::entity_property(datetime_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyG"), azure::storage::entity_property(binary_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyH"), azure::storage::entity_property(guid_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyM"), azure::storage::entity_property(int32_value6)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyN"), azure::storage::entity_property(int32_value7)));

            azure::storage::table_operation operation = azure::storage::table_operation::merge_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            try
            {
                table.execute(operation, options, context);
                CHECK(false);
            }
            catch (const azure::storage::storage_exception& e)
            {
                CHECK_EQUAL(web::http::status_codes::NotFound, e.result().http_status_code());
                CHECK(e.result().extended_error().code().compare(_XPLATSTR("ResourceNotFound")) == 0);
                CHECK(!e.result().extended_error().message().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NotFound, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().compare(_XPLATSTR("ResourceNotFound")) == 0);
            CHECK(!context.request_results()[0].extended_error().message().empty());
        }

        table.delete_table();
    }

    TEST_FIXTURE(table_service_test_base, EntityOperation_InsertAndReplace)
    {
        azure::storage::cloud_table table = get_table();

        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();

        bool boolean_value = get_random_boolean();
        int32_t int32_value = get_random_int32();
        int64_t int64_value = get_random_int64();
        double double_value = get_random_double();
        utility::string_t string_value = get_random_string();
        utility::datetime datetime_value = get_random_datetime();
        std::vector<uint8_t> binary_value = get_random_binary_data();
        utility::uuid guid_value = get_random_guid();

        {
            azure::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyA"), azure::storage::entity_property(boolean_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyB"), azure::storage::entity_property(int32_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyC"), azure::storage::entity_property(int64_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyD"), azure::storage::entity_property(double_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(string_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyF"), azure::storage::entity_property(datetime_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyG"), azure::storage::entity_property(binary_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyH"), azure::storage::entity_property(guid_value)));

            azure::storage::table_operation operation = azure::storage::table_operation::insert_or_replace_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_operation operation = azure::storage::table_operation::retrieve_entity(partition_key, row_key);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(!result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(8U, result.entity().properties().size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyA")) != result.entity().properties().cend());
            CHECK_EQUAL(boolean_value, result.entity().properties().find(_XPLATSTR("PropertyA"))->second.boolean_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyB")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value, result.entity().properties().find(_XPLATSTR("PropertyB"))->second.int32_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyC")) != result.entity().properties().cend());
            CHECK_EQUAL(int64_value, result.entity().properties().find(_XPLATSTR("PropertyC"))->second.int64_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyD")) != result.entity().properties().cend());
            CHECK_EQUAL(double_value, result.entity().properties().find(_XPLATSTR("PropertyD"))->second.double_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE"))->second.string_value().compare(string_value) == 0);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF"))->second.datetime_value() == datetime_value);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyG")) != result.entity().properties().cend());
            CHECK_ARRAY_EQUAL(binary_value, result.entity().properties().find(_XPLATSTR("PropertyG"))->second.binary_value(), (int)binary_value.size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyH")) != result.entity().properties().cend());
            CHECK(utility::uuid_equal(result.entity().properties().find(_XPLATSTR("PropertyH"))->second.guid_value(), guid_value));

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        int64_value = get_random_int64();
        double_value = get_random_double();
        string_value = get_random_string();
        datetime_value = get_random_datetime();
        binary_value = get_random_binary_data();
        guid_value = get_random_guid();

        int32_t int32_value2 = get_random_int32();
        int32_t int32_value3 = get_random_int32();

        {
            azure::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyC"), azure::storage::entity_property(int64_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyD"), azure::storage::entity_property(double_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(string_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyF"), azure::storage::entity_property(datetime_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyG"), azure::storage::entity_property(binary_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyH"), azure::storage::entity_property(guid_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyI"), azure::storage::entity_property(int32_value2)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyJ"), azure::storage::entity_property(int32_value3)));

            azure::storage::table_operation operation = azure::storage::table_operation::insert_or_replace_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_operation operation = azure::storage::table_operation::retrieve_entity(partition_key, row_key);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(!result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(8U, result.entity().properties().size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyC")) != result.entity().properties().cend());
            CHECK_EQUAL(int64_value, result.entity().properties().find(_XPLATSTR("PropertyC"))->second.int64_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyD")) != result.entity().properties().cend());
            CHECK_EQUAL(double_value, result.entity().properties().find(_XPLATSTR("PropertyD"))->second.double_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE"))->second.string_value().compare(string_value) == 0);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF"))->second.datetime_value() == datetime_value);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyG")) != result.entity().properties().cend());
            CHECK_ARRAY_EQUAL(binary_value, result.entity().properties().find(_XPLATSTR("PropertyG"))->second.binary_value(), (int)binary_value.size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyH")) != result.entity().properties().cend());
            CHECK(utility::uuid_equal(result.entity().properties().find(_XPLATSTR("PropertyH"))->second.guid_value(), guid_value));
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyI")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value2, result.entity().properties().find(_XPLATSTR("PropertyI"))->second.int32_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyJ")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value3, result.entity().properties().find(_XPLATSTR("PropertyJ"))->second.int32_value());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        int64_value = get_random_int64();
        double_value = get_random_double();
        string_value = get_random_string();
        datetime_value = get_random_datetime();
        binary_value = get_random_binary_data();
        guid_value = get_random_guid();

        int32_t int32_value4 = get_random_int32();
        int32_t int32_value5 = get_random_int32();

        {
            azure::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyC"), azure::storage::entity_property(int64_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyD"), azure::storage::entity_property(double_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(string_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyF"), azure::storage::entity_property(datetime_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyG"), azure::storage::entity_property(binary_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyH"), azure::storage::entity_property(guid_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyK"), azure::storage::entity_property(int32_value4)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyL"), azure::storage::entity_property(int32_value5)));

            azure::storage::table_operation operation = azure::storage::table_operation::replace_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_operation operation = azure::storage::table_operation::retrieve_entity(partition_key, row_key);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(!result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(8U, result.entity().properties().size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyC")) != result.entity().properties().cend());
            CHECK_EQUAL(int64_value, result.entity().properties().find(_XPLATSTR("PropertyC"))->second.int64_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyD")) != result.entity().properties().cend());
            CHECK_EQUAL(double_value, result.entity().properties().find(_XPLATSTR("PropertyD"))->second.double_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE"))->second.string_value().compare(string_value) == 0);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF"))->second.datetime_value() == datetime_value);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyG")) != result.entity().properties().cend());
            CHECK_ARRAY_EQUAL(binary_value, result.entity().properties().find(_XPLATSTR("PropertyG"))->second.binary_value(), (int)binary_value.size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyH")) != result.entity().properties().cend());
            CHECK(utility::uuid_equal(result.entity().properties().find(_XPLATSTR("PropertyH"))->second.guid_value(), guid_value));
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyK")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value4, result.entity().properties().find(_XPLATSTR("PropertyK"))->second.int32_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyL")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value5, result.entity().properties().find(_XPLATSTR("PropertyL"))->second.int32_value());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_entity entity(partition_key, row_key);

            azure::storage::table_operation operation = azure::storage::table_operation::delete_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(result.etag().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        int64_value = get_random_int64();
        double_value = get_random_double();
        string_value = get_random_string();
        datetime_value = get_random_datetime();
        binary_value = get_random_binary_data();
        guid_value = get_random_guid();

        int32_t int32_value6 = get_random_int32();
        int32_t int32_value7 = get_random_int32();

        {
            azure::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyC"), azure::storage::entity_property(int64_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyD"), azure::storage::entity_property(double_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(string_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyF"), azure::storage::entity_property(datetime_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyG"), azure::storage::entity_property(binary_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyH"), azure::storage::entity_property(guid_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyM"), azure::storage::entity_property(int32_value6)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyN"), azure::storage::entity_property(int32_value7)));

            azure::storage::table_operation operation = azure::storage::table_operation::replace_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            try
            {
                table.execute(operation, options, context);
                CHECK(false);
            }
            catch (const azure::storage::storage_exception& e)
            {
                CHECK_EQUAL(web::http::status_codes::NotFound, e.result().http_status_code());
                CHECK(e.result().extended_error().code().compare(_XPLATSTR("ResourceNotFound")) == 0);
                CHECK(!e.result().extended_error().message().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NotFound, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().compare(_XPLATSTR("ResourceNotFound")) == 0);
            CHECK(!context.request_results()[0].extended_error().message().empty());
        }

        table.delete_table();
    }

    TEST_FIXTURE(table_service_test_base, EntityOperation_Timeout)
    {
        azure::storage::cloud_table table = get_table();

        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();

        bool boolean_value = get_random_boolean();
        int32_t int32_value = get_random_int32();
        int64_t int64_value = get_random_int64();
        double double_value = get_random_double();
        utility::string_t string_value = get_random_string();
        utility::datetime datetime_value = get_random_datetime();
        std::vector<uint8_t> binary_value = get_random_binary_data();
        utility::uuid guid_value = get_random_guid();

        {
            azure::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyA"), azure::storage::entity_property(boolean_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyB"), azure::storage::entity_property(int32_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyC"), azure::storage::entity_property(int64_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyD"), azure::storage::entity_property(double_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(string_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyF"), azure::storage::entity_property(datetime_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyG"), azure::storage::entity_property(binary_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyH"), azure::storage::entity_property(guid_value)));

            azure::storage::table_operation operation = azure::storage::table_operation::insert_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            options.set_server_timeout(std::chrono::seconds(20));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_operation operation = azure::storage::table_operation::retrieve_entity(partition_key, row_key);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            options.set_server_timeout(std::chrono::seconds(20));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(!result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(8U, result.entity().properties().size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyA")) != result.entity().properties().cend());
            CHECK_EQUAL(boolean_value, result.entity().properties().find(_XPLATSTR("PropertyA"))->second.boolean_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyB")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value, result.entity().properties().find(_XPLATSTR("PropertyB"))->second.int32_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyC")) != result.entity().properties().cend());
            CHECK_EQUAL(int64_value, result.entity().properties().find(_XPLATSTR("PropertyC"))->second.int64_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyD")) != result.entity().properties().cend());
            CHECK_EQUAL(double_value, result.entity().properties().find(_XPLATSTR("PropertyD"))->second.double_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE"))->second.string_value().compare(string_value) == 0);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF"))->second.datetime_value() == datetime_value);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyG")) != result.entity().properties().cend());
            CHECK_ARRAY_EQUAL(binary_value, result.entity().properties().find(_XPLATSTR("PropertyG"))->second.binary_value(), (int)binary_value.size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyH")) != result.entity().properties().cend());
            CHECK(utility::uuid_equal(result.entity().properties().find(_XPLATSTR("PropertyH"))->second.guid_value(), guid_value));

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        table.delete_table();
    }

    TEST_FIXTURE(table_service_test_base, EntityOperation_InvalidValueType)
    {
        azure::storage::cloud_table table = get_table();

        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();

        {
            azure::storage::table_entity entity(partition_key, row_key);

            azure::storage::entity_property bad_property = azure::storage::entity_property(get_random_int32());
            bad_property.set_property_type(azure::storage::edm_type::datetime);

            entity.properties().reserve(1);
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("BadProperty"), bad_property));

            azure::storage::table_operation operation = azure::storage::table_operation::insert_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            try
            {
                table.execute(operation, options, context);
                CHECK(false);
            }
            catch (const azure::storage::storage_exception& e)
            {
                CHECK_EQUAL(web::http::status_codes::BadRequest, e.result().http_status_code());
                CHECK(e.result().extended_error().code().compare(_XPLATSTR("InvalidInput")) == 0);
                CHECK(!e.result().extended_error().message().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::BadRequest, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().compare(_XPLATSTR("InvalidInput")) == 0);
            CHECK(!context.request_results()[0].extended_error().message().empty());
        }

        table.delete_table();
    }

    TEST_FIXTURE(table_service_test_base, EntityOperation_DoubleSpecialValues)
    {
        azure::storage::cloud_table table = get_table();

        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();

        double nan_value = std::numeric_limits<double>::quiet_NaN();
        double infinity_value = std::numeric_limits<double>::infinity();
        double negative_infinity_value = -std::numeric_limits<double>::infinity();
        double negative_zero = -0.0;
        double whole_number = 123.0;
        double positive_exponent_value = 1.23e308;
        double negative_exponent_value = 2.34e-308;
        double denormalized_value = 1.0e-308;
        double zero_value = 0.0;
        double one_value = 1.0;

        {
            azure::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(10);
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyA"), azure::storage::entity_property(nan_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyB"), azure::storage::entity_property(infinity_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyC"), azure::storage::entity_property(negative_infinity_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyD"), azure::storage::entity_property(negative_zero)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(whole_number)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyF"), azure::storage::entity_property(positive_exponent_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyG"), azure::storage::entity_property(negative_exponent_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyH"), azure::storage::entity_property(denormalized_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyI"), azure::storage::entity_property(zero_value)));
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyJ"), azure::storage::entity_property(one_value)));

            azure::storage::table_operation operation = azure::storage::table_operation::insert_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());
        }

        {
            azure::storage::table_operation operation = azure::storage::table_operation::retrieve_entity(partition_key, row_key);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(!result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(10U, result.entity().properties().size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyA")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyA"))->second.property_type() == azure::storage::edm_type::double_floating_point);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyA"))->second.double_value() != result.entity().properties().find(_XPLATSTR("PropertyA"))->second.double_value()); // Only NaN is defined to not equal itself
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyB")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyB"))->second.property_type() == azure::storage::edm_type::double_floating_point);
            CHECK_EQUAL(infinity_value, result.entity().properties().find(_XPLATSTR("PropertyB"))->second.double_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyC")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyC"))->second.property_type() == azure::storage::edm_type::double_floating_point);
            CHECK_EQUAL(negative_infinity_value, result.entity().properties().find(_XPLATSTR("PropertyC"))->second.double_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyD")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyD"))->second.property_type() == azure::storage::edm_type::double_floating_point);
            // TODO: Investigate why the service doesn't handle -0.0 correctly (also investigate if other client libraries can handle -0.0)
            //CHECK_EQUAL(negative_infinity_value, 1.0 / result.entity().properties().find(_XPLATSTR("PropertyD"))->second.double_value()); // 1.0 / -0.0 == -Infinity
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyE"))->second.property_type() == azure::storage::edm_type::double_floating_point);
            CHECK_EQUAL(whole_number, result.entity().properties().find(_XPLATSTR("PropertyE"))->second.double_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyF"))->second.property_type() == azure::storage::edm_type::double_floating_point);
            CHECK_EQUAL(positive_exponent_value, result.entity().properties().find(_XPLATSTR("PropertyF"))->second.double_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyG")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyG"))->second.property_type() == azure::storage::edm_type::double_floating_point);
            CHECK_EQUAL(negative_exponent_value, result.entity().properties().find(_XPLATSTR("PropertyG"))->second.double_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyH")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyH"))->second.property_type() == azure::storage::edm_type::double_floating_point);
            CHECK_EQUAL(denormalized_value, result.entity().properties().find(_XPLATSTR("PropertyH"))->second.double_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyI")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyI"))->second.property_type() == azure::storage::edm_type::double_floating_point);
            CHECK_EQUAL(zero_value, result.entity().properties().find(_XPLATSTR("PropertyI"))->second.double_value());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyJ")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyJ"))->second.property_type() == azure::storage::edm_type::double_floating_point);
            CHECK_EQUAL(one_value, result.entity().properties().find(_XPLATSTR("PropertyJ"))->second.double_value());
        }

        table.delete_table();
    }

    TEST_FIXTURE(table_service_test_base, Casablanca_DoubleJsonParsing)
    {
        for (int i = 0; i < 50000; ++i)
        {
            std::vector<std::pair<utility::string_t, web::json::value>> fields;
            fields.reserve(1);

            double double_value = get_random_double();
            web::json::value property_value = web::json::value(double_value);
            fields.push_back(std::make_pair(_XPLATSTR("DoubleProperty"), std::move(property_value)));

            // Test if the Casablanca JSON serialization and parsing can round-trip a double value
            web::json::value input_document = web::json::value::object(fields);
            utility::string_t message = input_document.serialize();
            web::json::value output_document = web::json::value::parse(message);
            
            CHECK(output_document.is_object());
            CHECK(output_document.as_object().find(_XPLATSTR("DoubleProperty")) != output_document.as_object().cend());
            auto num = output_document.as_object().find(_XPLATSTR("DoubleProperty"))->second;
            CHECK_EQUAL(web::json::value::value_type::Number, num.type());
            // Casablanca cannot take doubles like 0.0, 1.0, 2.00 to be double value, so if a number is seen as not a double but a integer, need to check that it is a whole number
            CHECK(num.is_double() || (round(num.as_double()) == num.as_double()));
            CHECK_EQUAL(double_value, num.as_double());
        }
    }

    TEST_FIXTURE(table_service_test_base, EntityBatch_Normal)
    {
        const int BATCH_SIZE = 3;

        azure::storage::cloud_table table = get_table();

        utility::string_t partition_key = get_random_string();
        utility::string_t row_keys[BATCH_SIZE];

        for (int i = 0; i < BATCH_SIZE; ++i)
        {
            row_keys[i] = get_random_string();
        }

        int32_t int32_value = get_random_int32();
        utility::string_t string_value = get_random_string();

        {
            azure::storage::table_batch_operation operation;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                azure::storage::table_entity entity(partition_key, row_keys[i]);

                entity.properties().reserve(2);
                entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyA"), azure::storage::entity_property(int32_value)));
                entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyB"), azure::storage::entity_property(string_value)));

                operation.insert_entity(entity);
            }

            std::vector<azure::storage::table_result> results = table.execute_batch(operation, options, context);

            CHECK_EQUAL((size_t)BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                CHECK(results[i].entity().partition_key().empty());
                CHECK(results[i].entity().row_key().empty());
                CHECK(!results[i].entity().timestamp().is_initialized());
                CHECK(results[i].entity().etag().empty());
                CHECK_EQUAL(0U, results[i].entity().properties().size());
                CHECK_EQUAL(204, results[i].http_status_code());
                CHECK(!results[i].etag().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::Accepted, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_query query;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            utility::string_t filter_string = azure::storage::table_query::generate_filter_condition(_XPLATSTR("PartitionKey"), azure::storage::query_comparison_operator::equal, partition_key);
            query.set_filter_string(filter_string);

            std::vector<azure::storage::table_entity> results = execute_table_query(table, query, options, context);

            CHECK_EQUAL((size_t)BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                CHECK_EQUAL(2U, results[i].properties().size());
                CHECK_EQUAL(int32_value, results[i].properties()[_XPLATSTR("PropertyA")].int32_value());
                CHECK(string_value.compare(results[i].properties()[_XPLATSTR("PropertyB")].string_value()) == 0);
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
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
        }

        int32_t int32_value2 = get_random_int32();
        utility::string_t string_value2 = get_random_string();

        {
            azure::storage::table_batch_operation operation;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                azure::storage::table_entity entity(partition_key, row_keys[i]);

                entity.properties().reserve(3);
                entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyA"), azure::storage::entity_property(int32_value2)));
                entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyB"), azure::storage::entity_property(string_value)));
                entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyC"), azure::storage::entity_property(string_value2)));

                operation.insert_or_merge_entity(entity);
            }

            std::vector<azure::storage::table_result> results = table.execute_batch(operation, options, context);

            CHECK_EQUAL((size_t)BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                CHECK(results[i].entity().partition_key().empty());
                CHECK(results[i].entity().row_key().empty());
                CHECK(!results[i].entity().timestamp().is_initialized());
                CHECK(results[i].entity().etag().empty());
                CHECK_EQUAL(0U, results[i].entity().properties().size());
                CHECK_EQUAL(204, results[i].http_status_code());
                CHECK(!results[i].etag().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::Accepted, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_query query;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            utility::string_t filter_string = azure::storage::table_query::generate_filter_condition(_XPLATSTR("PartitionKey"), azure::storage::query_comparison_operator::equal, partition_key);
            query.set_filter_string(filter_string);

            std::vector<azure::storage::table_entity> results = execute_table_query(table, query, options, context);

            CHECK_EQUAL((size_t)BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                CHECK_EQUAL(3U, results[i].properties().size());
                CHECK_EQUAL(int32_value2, results[i].properties()[_XPLATSTR("PropertyA")].int32_value());
                CHECK(string_value.compare(results[i].properties()[_XPLATSTR("PropertyB")].string_value()) == 0);
                CHECK(string_value2.compare(results[i].properties()[_XPLATSTR("PropertyC")].string_value()) == 0);
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
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
        }

        int32_t int32_value3 = get_random_int32();
        utility::string_t string_value3 = get_random_string();

        {
            azure::storage::table_batch_operation operation;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                azure::storage::table_entity entity(partition_key, row_keys[i]);

                entity.properties().reserve(3);
                entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyA"), azure::storage::entity_property(int32_value3)));
                entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyB"), azure::storage::entity_property(string_value)));
                entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyD"), azure::storage::entity_property(string_value3)));

                operation.insert_or_replace_entity(entity);
            }

            std::vector<azure::storage::table_result> results = table.execute_batch(operation, options, context);

            CHECK_EQUAL((size_t)BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                CHECK(results[i].entity().partition_key().empty());
                CHECK(results[i].entity().row_key().empty());
                CHECK(!results[i].entity().timestamp().is_initialized());
                CHECK(results[i].entity().etag().empty());
                CHECK_EQUAL(0U, results[i].entity().properties().size());
                CHECK_EQUAL(204, results[i].http_status_code());
                CHECK(!results[i].etag().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::Accepted, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_query query;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            utility::string_t filter_string = azure::storage::table_query::generate_filter_condition(_XPLATSTR("PartitionKey"), azure::storage::query_comparison_operator::equal, partition_key);
            query.set_filter_string(filter_string);

            std::vector<azure::storage::table_entity> results = execute_table_query(table, query, options, context);

            CHECK_EQUAL((size_t)BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                CHECK_EQUAL(3U, results[i].properties().size());
                CHECK_EQUAL(int32_value3, results[i].properties()[_XPLATSTR("PropertyA")].int32_value());
                CHECK(string_value.compare(results[i].properties()[_XPLATSTR("PropertyB")].string_value()) == 0);
                CHECK(string_value3.compare(results[i].properties()[_XPLATSTR("PropertyD")].string_value()) == 0);
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
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
        }

        int32_t int32_value4 = get_random_int32();
        utility::string_t string_value4 = get_random_string();

        {
            azure::storage::table_batch_operation operation;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                azure::storage::table_entity entity(partition_key, row_keys[i]);

                entity.properties().reserve(2);
                entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyB"), azure::storage::entity_property(string_value4)));
                entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(int32_value4)));

                operation.replace_entity(entity);
            }

            std::vector<azure::storage::table_result> results = table.execute_batch(operation, options, context);

            CHECK_EQUAL((size_t)BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                CHECK(results[i].entity().partition_key().empty());
                CHECK(results[i].entity().row_key().empty());
                CHECK(!results[i].entity().timestamp().is_initialized());
                CHECK(results[i].entity().etag().empty());
                CHECK_EQUAL(0U, results[i].entity().properties().size());
                CHECK_EQUAL(204, results[i].http_status_code());
                CHECK(!results[i].etag().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::Accepted, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_query query;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            utility::string_t filter_string = azure::storage::table_query::generate_filter_condition(_XPLATSTR("PartitionKey"), azure::storage::query_comparison_operator::equal, partition_key);
            query.set_filter_string(filter_string);

            std::vector<azure::storage::table_entity> results = execute_table_query(table, query, options, context);

            CHECK_EQUAL((size_t)BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                CHECK_EQUAL(2U, results[i].properties().size());
                CHECK(string_value4.compare(results[i].properties()[_XPLATSTR("PropertyB")].string_value()) == 0);
                CHECK_EQUAL(int32_value4, results[i].properties()[_XPLATSTR("PropertyE")].int32_value());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
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
        }

        int32_t int32_value5 = get_random_int32();
        utility::string_t string_value5 = get_random_string();

        {
            azure::storage::table_batch_operation operation;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                azure::storage::table_entity entity(partition_key, row_keys[i]);

                entity.properties().reserve(2);
                entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(int32_value5)));
                entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyF"), azure::storage::entity_property(string_value5)));

                operation.merge_entity(entity);
            }

            std::vector<azure::storage::table_result> results = table.execute_batch(operation, options, context);

            CHECK_EQUAL((size_t)BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                CHECK(results[i].entity().partition_key().empty());
                CHECK(results[i].entity().row_key().empty());
                CHECK(!results[i].entity().timestamp().is_initialized());
                CHECK(results[i].entity().etag().empty());
                CHECK_EQUAL(0U, results[i].entity().properties().size());
                CHECK_EQUAL(204, results[i].http_status_code());
                CHECK(!results[i].etag().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::Accepted, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_query query;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            utility::string_t filter_string = azure::storage::table_query::generate_filter_condition(_XPLATSTR("PartitionKey"), azure::storage::query_comparison_operator::equal, partition_key);
            query.set_filter_string(filter_string);

            std::vector<azure::storage::table_entity> results = execute_table_query(table, query, options, context);

            CHECK_EQUAL((size_t)BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                CHECK_EQUAL(3U, results[i].properties().size());
                CHECK(string_value4.compare(results[i].properties()[_XPLATSTR("PropertyB")].string_value()) == 0);
                CHECK_EQUAL(int32_value5, results[i].properties()[_XPLATSTR("PropertyE")].int32_value());
                CHECK(string_value5.compare(results[i].properties()[_XPLATSTR("PropertyF")].string_value()) == 0);
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
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
        }

        {
            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                azure::storage::table_batch_operation operation;
                azure::storage::table_request_options options;
                azure::storage::operation_context context;
                print_client_request_id(context, _XPLATSTR(""));

                operation.retrieve_entity(partition_key, row_keys[i]);

                std::vector<azure::storage::table_result> results = table.execute_batch(operation, options, context);

                CHECK_EQUAL(1U, results.size());

                CHECK(operation.operations()[0].entity().partition_key().compare(results[0].entity().partition_key()) == 0);
                CHECK(operation.operations()[0].entity().row_key().compare(results[0].entity().row_key()) == 0);
                CHECK(!results[0].entity().etag().empty());
                CHECK(results[0].entity().timestamp().is_initialized());
                CHECK_EQUAL(3U, results[0].entity().properties().size());
                CHECK_EQUAL(200, results[0].http_status_code());
                CHECK(!results[0].etag().empty());

                CHECK(!context.client_request_id().empty());
                CHECK(context.start_time().is_initialized());
                CHECK(context.end_time().is_initialized());
                CHECK_EQUAL(1U, context.request_results().size());
                CHECK(context.request_results()[0].is_response_available());
                CHECK(context.request_results()[0].start_time().is_initialized());
                CHECK(context.request_results()[0].end_time().is_initialized());
                CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
                CHECK_EQUAL(web::http::status_codes::Accepted, context.request_results()[0].http_status_code());
                CHECK(!context.request_results()[0].service_request_id().empty());
                CHECK(context.request_results()[0].request_date().is_initialized());
                CHECK(context.request_results()[0].content_md5().empty());
                CHECK(context.request_results()[0].etag().empty());
                CHECK(context.request_results()[0].extended_error().code().empty());
                CHECK(context.request_results()[0].extended_error().message().empty());
                CHECK(context.request_results()[0].extended_error().details().empty());
            }
        }

        {
            azure::storage::table_batch_operation operation;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                azure::storage::table_entity entity(partition_key, row_keys[i]);

                operation.delete_entity(entity);
            }

            std::vector<azure::storage::table_result> results = table.execute_batch(operation, options, context);

            CHECK_EQUAL((size_t)BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                CHECK(results[i].entity().partition_key().empty());
                CHECK(results[i].entity().row_key().empty());
                CHECK(!results[i].entity().timestamp().is_initialized());
                CHECK(results[i].entity().etag().empty());
                CHECK_EQUAL(0U, results[i].entity().properties().size());
                CHECK_EQUAL(204, results[i].http_status_code());
                CHECK(results[i].etag().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::Accepted, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                azure::storage::table_batch_operation operation;
                azure::storage::table_request_options options;
                azure::storage::operation_context context;
                print_client_request_id(context, _XPLATSTR(""));

                operation.retrieve_entity(partition_key, row_keys[i]);

                std::vector<azure::storage::table_result> results = table.execute_batch(operation, options, context);

                CHECK_EQUAL(1U, results.size());

                CHECK(results[0].entity().partition_key().empty());
                CHECK(results[0].entity().row_key().empty());
                CHECK(results[0].entity().etag().empty());
                CHECK(!results[0].entity().timestamp().is_initialized());
                CHECK_EQUAL(0U, results[0].entity().properties().size());
                CHECK_EQUAL(404, results[0].http_status_code());
                CHECK(results[0].etag().empty());

                CHECK(!context.client_request_id().empty());
                CHECK(context.start_time().is_initialized());
                CHECK(context.end_time().is_initialized());
                CHECK_EQUAL(1U, context.request_results().size());
                CHECK(context.request_results()[0].is_response_available());
                CHECK(context.request_results()[0].start_time().is_initialized());
                CHECK(context.request_results()[0].end_time().is_initialized());
                CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
                CHECK_EQUAL(web::http::status_codes::Accepted, context.request_results()[0].http_status_code());
                CHECK(!context.request_results()[0].service_request_id().empty());
                CHECK(context.request_results()[0].request_date().is_initialized());
                CHECK(context.request_results()[0].content_md5().empty());
                CHECK(context.request_results()[0].etag().empty());
                CHECK(context.request_results()[0].extended_error().code().empty());
                CHECK(context.request_results()[0].extended_error().message().empty());
                CHECK(context.request_results()[0].extended_error().details().empty());
            }
        }

        table.delete_table();
    }

    TEST_FIXTURE(table_service_test_base, EntityBatch_InvalidInput)
    {
        const int BATCH_SIZE = 3;

        azure::storage::cloud_table table = get_table();

        utility::string_t partition_key = get_random_string();

        {
            azure::storage::table_batch_operation operation;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::entity_property bad_property = azure::storage::entity_property(get_random_int32());
            bad_property.set_property_type(azure::storage::edm_type::datetime);

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                azure::storage::table_entity entity(partition_key, get_random_string());

                entity.properties().reserve(1);
                entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("BadProperty"), bad_property));

                operation.insert_entity(entity);
            }

            try
            {
                table.execute_batch(operation, options, context);
                CHECK(false);
            }
            catch (const azure::storage::storage_exception& e)
            {
                CHECK_EQUAL(web::http::status_codes::BadRequest, e.result().http_status_code());
                CHECK(e.result().extended_error().code().compare(_XPLATSTR("InvalidInput")) == 0);
                CHECK(!e.result().extended_error().message().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::BadRequest, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().compare(_XPLATSTR("InvalidInput")) == 0);
            CHECK(!context.request_results()[0].extended_error().message().empty());
        }

        {
            azure::storage::table_batch_operation operation;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            utility::string_t bad_row_key = _XPLATSTR("bad//key");

            operation.retrieve_entity(partition_key, bad_row_key);

            try
            {
                table.execute_batch(operation, options, context);
                CHECK(false);
            }
            catch (const azure::storage::storage_exception& e)
            {
                CHECK_EQUAL(web::http::status_codes::BadRequest, e.result().http_status_code());
                CHECK(e.result().extended_error().code().compare(_XPLATSTR("InvalidInput")) == 0);
                CHECK(!e.result().extended_error().message().empty());
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::BadRequest, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().compare(_XPLATSTR("InvalidInput")) == 0);
            CHECK(!context.request_results()[0].extended_error().message().empty());
        }

        table.delete_table();
    }

    TEST_FIXTURE(table_service_test_base, EntityBatch_PartitionKeyMismatch)
    {
        azure::storage::cloud_table table = get_table(false);

        utility::string_t partition_key1 = get_random_string();
        utility::string_t row_key1 = get_random_string();
        utility::string_t partition_key2 = get_random_string();
        utility::string_t row_key2 = get_random_string();

        azure::storage::table_batch_operation operation;
        azure::storage::table_request_options options;
        azure::storage::operation_context context;
        print_client_request_id(context, _XPLATSTR(""));

        azure::storage::table_entity entity1(partition_key1, row_key1);
        operation.insert_entity(entity1);

        azure::storage::table_entity entity2(partition_key2, row_key2);
        operation.insert_entity(entity2);

        CHECK_THROW(table.execute_batch(operation, options, context), std::invalid_argument);
    }

    TEST_FIXTURE(table_service_test_base, EntityBatch_MultipleRetrieve)
    {
        azure::storage::cloud_table table = get_table(false);

        utility::string_t partition_key = get_random_string();
        utility::string_t row_key1 = get_random_string();
        utility::string_t row_key2 = get_random_string();

        azure::storage::table_batch_operation operation;
        azure::storage::table_request_options options;
        azure::storage::operation_context context;
        print_client_request_id(context, _XPLATSTR(""));

        operation.retrieve_entity(partition_key, row_key1);
        operation.retrieve_entity(partition_key, row_key2);

        CHECK_THROW(table.execute_batch(operation, options, context), std::invalid_argument);
    }

    TEST_FIXTURE(table_service_test_base, EntityBatch_RetrieveMixture)
    {
        azure::storage::cloud_table table = get_table(false);

        utility::string_t partition_key = get_random_string();
        utility::string_t row_key1 = get_random_string();
        utility::string_t row_key2 = get_random_string();

        azure::storage::table_batch_operation operation;
        azure::storage::table_request_options options;
        azure::storage::operation_context context;
        print_client_request_id(context, _XPLATSTR(""));

        operation.retrieve_entity(partition_key, row_key1);

        azure::storage::table_entity entity2(partition_key, row_key2);
        operation.insert_entity(entity2);

        CHECK_THROW(table.execute_batch(operation, options, context), std::invalid_argument);
    }

    TEST_FIXTURE(table_service_test_base, EntityQuery_Normal)
    {
        azure::storage::cloud_table table = get_table();

        utility::string_t partition_key1 = get_random_string();
        utility::string_t partition_key2 = get_random_string();

        utility::datetime datetime_value = utility::datetime::from_string(_XPLATSTR("2013-01-02T03:04:05.1234567Z"), utility::datetime::ISO_8601);

        {
            for (int partition = 1; partition <= 2; ++partition)
            {
                utility::string_t partition_key = partition == 1 ? partition_key1 : partition_key2;

                for (int row1 = 0; row1 < 26; ++row1)
                {
                    azure::storage::table_batch_operation operation;

                    for (int row2 = 0; row2 < 26; ++row2)
                    {
                        utility::string_t row_key = get_string((utility::char_t)('a' + row1), (utility::char_t)('a' + row2));

                        azure::storage::table_entity entity(partition_key, row_key);

                        entity.properties().reserve(8);
                        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyA"), azure::storage::entity_property(get_random_boolean())));
                        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyB"), azure::storage::entity_property(get_random_int32())));
                        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyC"), azure::storage::entity_property(get_random_int64())));
                        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyD"), azure::storage::entity_property(get_random_double())));
                        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(get_random_string())));
                        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyF"), azure::storage::entity_property(get_random_datetime())));
                        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyG"), azure::storage::entity_property(get_random_binary_data())));
                        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyH"), azure::storage::entity_property(get_random_guid())));

                        operation.insert_entity(entity);
                    }

                    std::vector<azure::storage::table_result> results = table.execute_batch(operation);

                    for (std::vector<azure::storage::table_result>::const_iterator itr = results.cbegin(); itr != results.cend(); ++itr)
                    {
                        azure::storage::table_result result = *itr;

                        CHECK(result.entity().partition_key().empty());
                        CHECK(result.entity().row_key().empty());
                        CHECK(!result.entity().timestamp().is_initialized());
                        CHECK(result.entity().etag().empty());
                        CHECK_EQUAL(204, result.http_status_code());
                        CHECK(!result.etag().empty());
                    }
                }
            }
        }

        utility::string_t uuid_string = _XPLATSTR("12345678-abcd-efab-cdef-1234567890ab");
        utility::uuid uuid_to_use = utility::string_to_uuid(uuid_string);
        utility::string_t converted_uuid_string = utility::uuid_to_string(uuid_to_use);

        // Explicit checks to ensure that UUIDs behave as we would expect per platform
        // After this check, we can jsut use converted_uuid_string.
#ifdef _WIN32
        CHECK(uuid_string.compare(converted_uuid_string) == 0);
#else
        utility::string_t capital_uuid_string = _XPLATSTR("12345678-ABCD-EFAB-CDEF-1234567890AB");
        CHECK((capital_uuid_string).compare(converted_uuid_string) == 0);
#endif // _WIN32

        {
            azure::storage::table_query query;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            int take_count = 10;
            query.set_take_count(take_count);

            CHECK_EQUAL(take_count, query.take_count());

            utility::string_t filter_string = azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PartitionKey"), azure::storage::query_comparison_operator::equal, partition_key1), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("RowKey"), azure::storage::query_comparison_operator::greater_than_or_equal, _XPLATSTR("k"))), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("RowKey"), azure::storage::query_comparison_operator::less_than, _XPLATSTR("n"))), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("Timestamp"), azure::storage::query_comparison_operator::greater_than_or_equal, utility::datetime::from_string(_XPLATSTR("2013-09-01T00:00:00Z"), utility::datetime::ISO_8601))), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyA"), azure::storage::query_comparison_operator::not_equal, false)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyB"), azure::storage::query_comparison_operator::not_equal, 1234567890)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyC"), azure::storage::query_comparison_operator::not_equal, (int64_t)1234567890123456789LL)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyD"), azure::storage::query_comparison_operator::not_equal, 9.1234567890123456789)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyE"), azure::storage::query_comparison_operator::not_equal, _XPLATSTR("ABCDE12345"))),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyF"), azure::storage::query_comparison_operator::not_equal, datetime_value)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyG"), azure::storage::query_comparison_operator::not_equal, std::vector<uint8_t>(10, 'X'))),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyH"), azure::storage::query_comparison_operator::not_equal, uuid_to_use));
            query.set_filter_string(filter_string);

            utility::string_t expected_filter_string = utility::string_t(_XPLATSTR("(((((((((((PartitionKey eq '")) + partition_key1 + utility::string_t(_XPLATSTR("') and (RowKey ge 'k')) and (RowKey lt 'n')) and (Timestamp ge datetime'2013-09-01T00:00:00Z')) and (PropertyA ne false)) and (PropertyB ne 1234567890)) and (PropertyC ne 1234567890123456789L)) and (PropertyD ne 9.1234567890123461)) and (PropertyE ne 'ABCDE12345')) and (PropertyF ne datetime'2013-01-02T03:04:05.1234567Z')) and (PropertyG ne X'58585858585858585858')) and (PropertyH ne guid'") + converted_uuid_string + _XPLATSTR("')"));
            CHECK(filter_string.compare(expected_filter_string) == 0);

            std::vector<utility::string_t> select_columns;
            select_columns.reserve(9);
            select_columns.push_back(_XPLATSTR("PropertyA"));
            select_columns.push_back(_XPLATSTR("PropertyB"));
            select_columns.push_back(_XPLATSTR("PropertyC"));
            select_columns.push_back(_XPLATSTR("PropertyD"));
            select_columns.push_back(_XPLATSTR("PropertyE"));
            select_columns.push_back(_XPLATSTR("PropertyF"));
            select_columns.push_back(_XPLATSTR("PropertyG"));
            select_columns.push_back(_XPLATSTR("PropertyH"));
            select_columns.push_back(_XPLATSTR("PropertyX"));
            query.set_select_columns(select_columns);

            options.set_payload_format(azure::storage::table_payload_format::json_full_metadata);

            std::vector<azure::storage::table_entity> results = execute_table_query(table, query, options, context);

            CHECK(results.size() > 0);
            CHECK_EQUAL(take_count, (int)results.size());

            for (std::vector<azure::storage::table_entity>::const_iterator entity_iterator = results.cbegin(); entity_iterator != results.cend(); ++entity_iterator)
            {
                azure::storage::table_entity entity = *entity_iterator;

                CHECK(!entity.partition_key().empty());
                CHECK(!entity.row_key().empty());
                CHECK(entity.timestamp().is_initialized());
                CHECK(!entity.etag().empty());

                azure::storage::table_entity::properties_type properties = entity.properties();

                CHECK_EQUAL(9U, properties.size());

                for (azure::storage::table_entity::properties_type::const_iterator property_it = properties.cbegin(); property_it != properties.cend(); ++property_it)
                {
                    const utility::string_t& property_name = property_it->first;
                    const azure::storage::entity_property& property = property_it->second;

                    CHECK(!property_name.empty());
                    CHECK(property.is_null() || !property.str().empty());
                }
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK(context.request_results().size() > 0);
            for (std::vector<azure::storage::request_result>::size_type i = 0; i < context.request_results().size(); ++i)
            {
                CHECK(context.request_results()[i].is_response_available());
                CHECK(context.request_results()[i].start_time().is_initialized());
                CHECK(context.request_results()[i].end_time().is_initialized());
                CHECK(context.request_results()[i].target_location() != azure::storage::storage_location::unspecified);
                CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[i].http_status_code());
                CHECK(!context.request_results()[i].service_request_id().empty());
                CHECK(context.request_results()[i].request_date().is_initialized());
                CHECK(context.request_results()[i].content_md5().empty());
                CHECK(context.request_results()[i].etag().empty());
                CHECK(context.request_results()[i].extended_error().code().empty());
                CHECK(context.request_results()[i].extended_error().message().empty());
                CHECK(context.request_results()[i].extended_error().details().empty());
            }
        }

        {
            azure::storage::table_query query;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            int take_count = 10;
            query.set_take_count(take_count);

            CHECK_EQUAL(take_count, query.take_count());

            utility::string_t filter_string = azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PartitionKey"), azure::storage::query_comparison_operator::equal, partition_key1), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("RowKey"), azure::storage::query_comparison_operator::greater_than_or_equal, _XPLATSTR("k"))), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("RowKey"), azure::storage::query_comparison_operator::less_than, _XPLATSTR("n"))), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("Timestamp"), azure::storage::query_comparison_operator::greater_than_or_equal, utility::datetime::from_string(_XPLATSTR("2013-09-01T00:00:00Z"), utility::datetime::ISO_8601))), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyA"), azure::storage::query_comparison_operator::not_equal, false)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyB"), azure::storage::query_comparison_operator::not_equal, 1234567890)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyC"), azure::storage::query_comparison_operator::not_equal, (int64_t)1234567890123456789LL)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyD"), azure::storage::query_comparison_operator::not_equal, 9.1234567890123456789)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyE"), azure::storage::query_comparison_operator::not_equal, _XPLATSTR("ABCDE12345"))),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyF"), azure::storage::query_comparison_operator::not_equal, datetime_value)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyG"), azure::storage::query_comparison_operator::not_equal, std::vector<uint8_t>(10, 'X'))),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyH"), azure::storage::query_comparison_operator::not_equal, uuid_to_use));
            query.set_filter_string(filter_string);

            utility::string_t expected_filter_string = utility::string_t(_XPLATSTR("(((((((((((PartitionKey eq '")) + partition_key1 + utility::string_t(_XPLATSTR("') and (RowKey ge 'k')) and (RowKey lt 'n')) and (Timestamp ge datetime'2013-09-01T00:00:00Z')) and (PropertyA ne false)) and (PropertyB ne 1234567890)) and (PropertyC ne 1234567890123456789L)) and (PropertyD ne 9.1234567890123461)) and (PropertyE ne 'ABCDE12345')) and (PropertyF ne datetime'2013-01-02T03:04:05.1234567Z')) and (PropertyG ne X'58585858585858585858')) and (PropertyH ne guid'") + converted_uuid_string + _XPLATSTR("')"));
            CHECK(filter_string.compare(expected_filter_string) == 0);

            std::vector<utility::string_t> select_columns;
            select_columns.reserve(9);
            select_columns.push_back(_XPLATSTR("PropertyA"));
            select_columns.push_back(_XPLATSTR("PropertyB"));
            select_columns.push_back(_XPLATSTR("PropertyC"));
            select_columns.push_back(_XPLATSTR("PropertyD"));
            select_columns.push_back(_XPLATSTR("PropertyE"));
            select_columns.push_back(_XPLATSTR("PropertyF"));
            select_columns.push_back(_XPLATSTR("PropertyG"));
            select_columns.push_back(_XPLATSTR("PropertyH"));
            select_columns.push_back(_XPLATSTR("PropertyX"));
            query.set_select_columns(select_columns);

            options.set_payload_format(azure::storage::table_payload_format::json_full_metadata);

            std::vector<azure::storage::table_entity> results = execute_table_query(table, query, options, context);

            CHECK(results.size() > 0);

            for (std::vector<azure::storage::table_entity>::const_iterator entity_iterator = results.cbegin(); entity_iterator != results.cend(); ++entity_iterator)
            {
                azure::storage::table_entity entity = *entity_iterator;

                CHECK(!entity.partition_key().empty());
                CHECK(!entity.row_key().empty());
                CHECK(entity.timestamp().is_initialized());
                CHECK(!entity.etag().empty());

                azure::storage::table_entity::properties_type properties = entity.properties();

                CHECK_EQUAL(9U, properties.size());

                for (azure::storage::table_entity::properties_type::const_iterator property_it = properties.cbegin(); property_it != properties.cend(); ++property_it)
                {
                    const utility::string_t& property_name = property_it->first;
                    const azure::storage::entity_property& property = property_it->second;

                    CHECK(!property_name.empty());
                    CHECK(property.is_null() || !property.str().empty());
                }
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK(context.request_results().size() > 0);
            for (std::vector<azure::storage::request_result>::size_type i = 0; i < context.request_results().size(); ++i)
            {
                CHECK(context.request_results()[i].is_response_available());
                CHECK(context.request_results()[i].start_time().is_initialized());
                CHECK(context.request_results()[i].end_time().is_initialized());
                CHECK(context.request_results()[i].target_location() != azure::storage::storage_location::unspecified);
                CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[i].http_status_code());
                CHECK(!context.request_results()[i].service_request_id().empty());
                CHECK(context.request_results()[i].request_date().is_initialized());
                CHECK(context.request_results()[i].content_md5().empty());
                CHECK(context.request_results()[i].etag().empty());
                CHECK(context.request_results()[i].extended_error().code().empty());
                CHECK(context.request_results()[i].extended_error().message().empty());
                CHECK(context.request_results()[i].extended_error().details().empty());
            }
        }

        table.delete_table();
    }

    TEST_FIXTURE(table_service_test_base, EntityQuery_Segmented)
    {
        azure::storage::cloud_table table = get_table();

        utility::string_t partition_key1 = get_random_string();
        utility::string_t partition_key2 = get_random_string();

        utility::datetime datetime_value;
        datetime_value = datetime_value + 130015694451234567; // 2013-01-02T03:04:05.1234567Z

        {
            for (int partition = 1; partition <= 2; ++partition)
            {
                utility::string_t partition_key = partition == 1 ? partition_key1 : partition_key2;

                for (int row1 = 0; row1 < 26; ++row1)
                {
                    azure::storage::table_batch_operation operation;

                    for (int row2 = 0; row2 < 26; ++row2)
                    {
                        utility::string_t row_key = get_string((utility::char_t)('a' + row1), (utility::char_t)('a' + row2));

                        azure::storage::table_entity entity(partition_key, row_key);

                        entity.properties().reserve(8);
                        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyA"), azure::storage::entity_property(get_random_boolean())));
                        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyB"), azure::storage::entity_property(get_random_int32())));
                        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyC"), azure::storage::entity_property(get_random_int64())));
                        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyD"), azure::storage::entity_property(get_random_double())));
                        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyE"), azure::storage::entity_property(get_random_string())));
                        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyF"), azure::storage::entity_property(get_random_datetime())));
                        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyG"), azure::storage::entity_property(get_random_binary_data())));
                        entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyH"), azure::storage::entity_property(get_random_guid())));

                        operation.insert_entity(entity);
                    }

                    std::vector<azure::storage::table_result> results = table.execute_batch(operation);

                    for (std::vector<azure::storage::table_result>::const_iterator itr = results.cbegin(); itr != results.cend(); ++itr)
                    {
                        azure::storage::table_result result = *itr;

                        CHECK(result.entity().partition_key().empty());
                        CHECK(result.entity().row_key().empty());
                        CHECK(!result.entity().timestamp().is_initialized());
                        CHECK(result.entity().etag().empty());
                        CHECK_EQUAL(204, result.http_status_code());
                        CHECK(!result.etag().empty());
                    }
                }
            }
        }

        utility::string_t uuid_string = _XPLATSTR("12345678-abcd-efab-cdef-1234567890ab");
        utility::uuid uuid_to_use = utility::string_to_uuid(uuid_string);
        utility::string_t converted_uuid_string = utility::uuid_to_string(uuid_to_use);

        {
            azure::storage::table_query query;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            int take_count = 10;
            query.set_take_count(take_count);

            CHECK_EQUAL(take_count, query.take_count());

            utility::string_t filter_string = azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PartitionKey"), azure::storage::query_comparison_operator::equal, partition_key1), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("RowKey"), azure::storage::query_comparison_operator::greater_than_or_equal, _XPLATSTR("k"))), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("RowKey"), azure::storage::query_comparison_operator::less_than, _XPLATSTR("n"))), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("Timestamp"), azure::storage::query_comparison_operator::greater_than_or_equal, utility::datetime::from_string(_XPLATSTR("2013-09-01T00:00:00Z"), utility::datetime::ISO_8601))), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyA"), azure::storage::query_comparison_operator::not_equal, false)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyB"), azure::storage::query_comparison_operator::not_equal, 1234567890)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyC"), azure::storage::query_comparison_operator::not_equal, (int64_t)1234567890123456789LL)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyD"), azure::storage::query_comparison_operator::not_equal, 9.1234567890123456789)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyE"), azure::storage::query_comparison_operator::not_equal, _XPLATSTR("ABCDE12345"))),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyF"), azure::storage::query_comparison_operator::not_equal, datetime_value)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyG"), azure::storage::query_comparison_operator::not_equal, std::vector<uint8_t>(10, 'X'))),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyH"), azure::storage::query_comparison_operator::not_equal, uuid_to_use));
            query.set_filter_string(filter_string);

            utility::string_t expected_filter_string = utility::string_t(_XPLATSTR("(((((((((((PartitionKey eq '")) + partition_key1 + utility::string_t(_XPLATSTR("') and (RowKey ge 'k')) and (RowKey lt 'n')) and (Timestamp ge datetime'2013-09-01T00:00:00Z')) and (PropertyA ne false)) and (PropertyB ne 1234567890)) and (PropertyC ne 1234567890123456789L)) and (PropertyD ne 9.1234567890123461)) and (PropertyE ne 'ABCDE12345')) and (PropertyF ne datetime'2013-01-02T03:04:05.1234567Z')) and (PropertyG ne X'58585858585858585858')) and (PropertyH ne guid'") + converted_uuid_string + _XPLATSTR("')"));
            CHECK(filter_string.compare(expected_filter_string) == 0);

            std::vector<utility::string_t> select_columns;
            select_columns.reserve(9);
            select_columns.push_back(_XPLATSTR("PropertyA"));
            select_columns.push_back(_XPLATSTR("PropertyB"));
            select_columns.push_back(_XPLATSTR("PropertyC"));
            select_columns.push_back(_XPLATSTR("PropertyD"));
            select_columns.push_back(_XPLATSTR("PropertyE"));
            select_columns.push_back(_XPLATSTR("PropertyF"));
            select_columns.push_back(_XPLATSTR("PropertyG"));
            select_columns.push_back(_XPLATSTR("PropertyH"));
            select_columns.push_back(_XPLATSTR("PropertyX"));
            query.set_select_columns(select_columns);

            options.set_payload_format(azure::storage::table_payload_format::json_full_metadata);

            std::vector<azure::storage::table_entity> results = execute_table_query(table, query, options, context);

            CHECK(results.size() > 0);
            CHECK_EQUAL(take_count, (int)results.size());

            for (std::vector<azure::storage::table_entity>::const_iterator entity_iterator = results.cbegin(); entity_iterator != results.cend(); ++entity_iterator)
            {
                azure::storage::table_entity entity = *entity_iterator;

                CHECK(!entity.partition_key().empty());
                CHECK(!entity.row_key().empty());
                CHECK(entity.timestamp().is_initialized());
                CHECK(!entity.etag().empty());

                azure::storage::table_entity::properties_type properties = entity.properties();

                CHECK_EQUAL(9U, properties.size());

                for (azure::storage::table_entity::properties_type::const_iterator property_it = properties.cbegin(); property_it != properties.cend(); ++property_it)
                {
                    const utility::string_t& property_name = property_it->first;
                    const azure::storage::entity_property& property = property_it->second;

                    CHECK(!property_name.empty());
                    CHECK(property.is_null() || !property.str().empty());
                }
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK(context.request_results().size() > 0);
            for (std::vector<azure::storage::request_result>::size_type i = 0; i < context.request_results().size(); ++i)
            {
                CHECK(context.request_results()[i].is_response_available());
                CHECK(context.request_results()[i].start_time().is_initialized());
                CHECK(context.request_results()[i].end_time().is_initialized());
                CHECK(context.request_results()[i].target_location() != azure::storage::storage_location::unspecified);
                CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[i].http_status_code());
                CHECK(!context.request_results()[i].service_request_id().empty());
                CHECK(context.request_results()[i].request_date().is_initialized());
                CHECK(context.request_results()[i].content_md5().empty());
                CHECK(context.request_results()[i].etag().empty());
                CHECK(context.request_results()[i].extended_error().code().empty());
                CHECK(context.request_results()[i].extended_error().message().empty());
                CHECK(context.request_results()[i].extended_error().details().empty());
            }
        }

        {
            azure::storage::table_query query;
            azure::storage::continuation_token token;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            int take_count = 10;
            query.set_take_count(take_count);

            CHECK_EQUAL(take_count, query.take_count());

            utility::string_t filter_string = azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::combine_filter_conditions(
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PartitionKey"), azure::storage::query_comparison_operator::equal, partition_key1), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("RowKey"), azure::storage::query_comparison_operator::greater_than_or_equal, _XPLATSTR("k"))), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("RowKey"), azure::storage::query_comparison_operator::less_than, _XPLATSTR("n"))), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("Timestamp"), azure::storage::query_comparison_operator::greater_than_or_equal, utility::datetime::from_string(_XPLATSTR("2013-09-01T00:00:00Z"), utility::datetime::ISO_8601))), 
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyA"), azure::storage::query_comparison_operator::not_equal, false)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyB"), azure::storage::query_comparison_operator::not_equal, 1234567890)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyC"), azure::storage::query_comparison_operator::not_equal, (int64_t)1234567890123456789LL)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyD"), azure::storage::query_comparison_operator::not_equal, 9.1234567890123456789)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyE"), azure::storage::query_comparison_operator::not_equal, _XPLATSTR("ABCDE12345"))),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyF"), azure::storage::query_comparison_operator::not_equal, datetime_value)),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyG"), azure::storage::query_comparison_operator::not_equal, std::vector<uint8_t>(10, 'X'))),
                azure::storage::query_logical_operator::op_and, 
                azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyH"), azure::storage::query_comparison_operator::not_equal, uuid_to_use));
            query.set_filter_string(filter_string);

            utility::string_t expected_filter_string = utility::string_t(_XPLATSTR("(((((((((((PartitionKey eq '")) + partition_key1 + utility::string_t(_XPLATSTR("') and (RowKey ge 'k')) and (RowKey lt 'n')) and (Timestamp ge datetime'2013-09-01T00:00:00Z')) and (PropertyA ne false)) and (PropertyB ne 1234567890)) and (PropertyC ne 1234567890123456789L)) and (PropertyD ne 9.1234567890123461)) and (PropertyE ne 'ABCDE12345')) and (PropertyF ne datetime'2013-01-02T03:04:05.1234567Z')) and (PropertyG ne X'58585858585858585858')) and (PropertyH ne guid'") + converted_uuid_string + _XPLATSTR("')"));
            CHECK(filter_string.compare(expected_filter_string) == 0);

            std::vector<utility::string_t> select_columns;
            select_columns.reserve(9);
            select_columns.push_back(_XPLATSTR("PropertyA"));
            select_columns.push_back(_XPLATSTR("PropertyB"));
            select_columns.push_back(_XPLATSTR("PropertyC"));
            select_columns.push_back(_XPLATSTR("PropertyD"));
            select_columns.push_back(_XPLATSTR("PropertyE"));
            select_columns.push_back(_XPLATSTR("PropertyF"));
            select_columns.push_back(_XPLATSTR("PropertyG"));
            select_columns.push_back(_XPLATSTR("PropertyH"));
            select_columns.push_back(_XPLATSTR("PropertyX"));
            query.set_select_columns(select_columns);

            options.set_payload_format(azure::storage::table_payload_format::json_full_metadata);

            size_t segment_count = 0;
            azure::storage::table_query_segment query_segment;
            do
            {
                query_segment = table.execute_query_segmented(query, token, options, context);
                std::vector<azure::storage::table_entity> results = query_segment.results();

                CHECK((int)results.size() <= take_count);

                for (std::vector<azure::storage::table_entity>::const_iterator entity_iterator = results.cbegin(); entity_iterator != results.cend(); ++entity_iterator)
                {
                    azure::storage::table_entity entity = *entity_iterator;

                    CHECK(!entity.partition_key().empty());
                    CHECK(!entity.row_key().empty());
                    CHECK(entity.timestamp().is_initialized());
                    CHECK(!entity.etag().empty());

                    azure::storage::table_entity::properties_type properties = entity.properties();

                    CHECK_EQUAL(9U, properties.size());

                    for (azure::storage::table_entity::properties_type::const_iterator property_it = properties.cbegin(); property_it != properties.cend(); ++property_it)
                    {
                        const utility::string_t& property_name = property_it->first;
                        const azure::storage::entity_property& property = property_it->second;

                        CHECK(!property_name.empty());
                        CHECK(property.is_null() || !property.str().empty());
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
                token = query_segment.continuation_token();
            }
            while (!token.empty());

            CHECK(segment_count > 1);
        }

        table.delete_table();
    }

    TEST_FIXTURE(table_service_test_base, EntityQuery_Empty)
    {
        azure::storage::cloud_table table = get_table();

        utility::string_t partition_key = get_random_string();

        azure::storage::table_query query;
        azure::storage::table_request_options options;
        azure::storage::operation_context context;
        print_client_request_id(context, _XPLATSTR(""));

        utility::string_t filter_string = azure::storage::table_query::generate_filter_condition(_XPLATSTR("PartitionKey"), azure::storage::query_comparison_operator::equal, partition_key);
        query.set_filter_string(filter_string);

        std::vector<azure::storage::table_entity> results = execute_table_query(table, query, options, context);

        CHECK_EQUAL(0U, results.size());

        for (std::vector<azure::storage::table_entity>::const_iterator entity_iterator = results.cbegin(); entity_iterator != results.cend(); ++entity_iterator)
        {
            CHECK(false);
        }

        CHECK(!context.client_request_id().empty());
        CHECK(context.start_time().is_initialized());
        CHECK(context.end_time().is_initialized());
        CHECK_EQUAL(1U, context.request_results().size());
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

        table.delete_table();
    }

    TEST_FIXTURE(table_service_test_base, EntityQuery_InvalidInput)
    {
        azure::storage::cloud_table table = get_table();

        utility::string_t partition_key = get_random_string();

        azure::storage::table_query query;
        azure::storage::table_request_options options;
        azure::storage::operation_context context;
        print_client_request_id(context, _XPLATSTR(""));

        // An invalid filter string because PartitionKey is not a numeric type
        utility::string_t filter_string = (_XPLATSTR("PartitionKey eq 12345"));
        query.set_filter_string(filter_string);

        try
        {
            execute_table_query(table, query, options, context);
            CHECK(false);
        }
        catch (const azure::storage::storage_exception& e)
        {
            CHECK_EQUAL(web::http::status_codes::BadRequest, e.result().http_status_code());
            CHECK(e.result().extended_error().code().compare(_XPLATSTR("InvalidInput")) == 0);
            CHECK(!e.result().extended_error().message().empty());
        }

        CHECK(!context.client_request_id().empty());
        CHECK(context.start_time().is_initialized());
        CHECK(context.end_time().is_initialized());
        CHECK_EQUAL(1U, context.request_results().size());
        CHECK(context.request_results()[0].is_response_available());
        CHECK(context.request_results()[0].start_time().is_initialized());
        CHECK(context.request_results()[0].end_time().is_initialized());
        CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
        CHECK_EQUAL(web::http::status_codes::BadRequest, context.request_results()[0].http_status_code());
        CHECK(!context.request_results()[0].service_request_id().empty());
        CHECK(context.request_results()[0].request_date().is_initialized());
        CHECK(context.request_results()[0].content_md5().empty());
        CHECK(context.request_results()[0].etag().empty());
        CHECK(context.request_results()[0].extended_error().code().compare(_XPLATSTR("InvalidInput")) == 0);
        CHECK(!context.request_results()[0].extended_error().message().empty());

        table.delete_table();
    }

    TEST_FIXTURE(table_service_test_base, EntityQuery_UriEncoding)
    {
        azure::storage::cloud_table table = get_table();

        utility::string_t partition_key = get_random_string();
        utility::string_t row_key1 = get_random_string();
        utility::string_t row_key2 = get_random_string();

        utility::string_t property_value(_XPLATSTR("@$%^, +\"' /?:=&#"));

        {
            azure::storage::table_batch_operation operation;

            azure::storage::table_entity entity1(partition_key, row_key1);
            entity1.properties().reserve(1);
            entity1.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("TextProperty"), azure::storage::entity_property(_XPLATSTR("Normal text"))));
            operation.insert_entity(entity1);

            azure::storage::table_entity entity2(partition_key, row_key2);
            entity2.properties().reserve(1);
            entity2.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("TextProperty"), azure::storage::entity_property(property_value)));
            operation.insert_entity(entity2);

            std::vector<azure::storage::table_result> results = table.execute_batch(operation);

            for (std::vector<azure::storage::table_result>::const_iterator it = results.cbegin(); it != results.cend(); ++it)
            {
                azure::storage::table_result result = *it;

                CHECK(result.entity().partition_key().empty());
                CHECK(result.entity().row_key().empty());
                CHECK(!result.entity().timestamp().is_initialized());
                CHECK(result.entity().etag().empty());
                CHECK_EQUAL(204, result.http_status_code());
                CHECK(!result.etag().empty());
            }
        }

        {
            azure::storage::table_query query;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            utility::string_t filter_string = azure::storage::table_query::generate_filter_condition(_XPLATSTR("TextProperty"), azure::storage::query_comparison_operator::equal, property_value);
            query.set_filter_string(filter_string);

            utility::string_t expected_filter_string(_XPLATSTR("TextProperty eq '@$%^, +\"'' /?:=&#'"));
            CHECK(filter_string.compare(expected_filter_string) == 0);

            options.set_payload_format(azure::storage::table_payload_format::json_full_metadata);

            std::vector<azure::storage::table_entity> results = execute_table_query(table, query, options, context);

            CHECK_EQUAL(1U, results.size());

            for (std::vector<azure::storage::table_entity>::const_iterator entity_iterator = results.cbegin(); entity_iterator != results.cend(); ++entity_iterator)
            {
                azure::storage::table_entity entity = *entity_iterator;

                CHECK(!entity.partition_key().empty());
                CHECK(!entity.row_key().empty());
                CHECK(entity.timestamp().is_initialized());
                CHECK(!entity.etag().empty());

                azure::storage::table_entity::properties_type properties = entity.properties();

                CHECK_EQUAL(1U, properties.size());

                for (azure::storage::table_entity::properties_type::const_iterator property_it = properties.cbegin(); property_it != properties.cend(); ++property_it)
                {
                    const utility::string_t& property_name = property_it->first;
                    const azure::storage::entity_property& property = property_it->second;

                    CHECK(property_name.compare(_XPLATSTR("TextProperty")) == 0);
                    CHECK(property.property_type() == azure::storage::edm_type::string);
                    CHECK(!property.is_null());
                    CHECK(property.string_value().compare(property_value) == 0);
                }
            }

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
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
        }

        table.delete_table();
    }

    TEST_FIXTURE(table_service_test_base, Table_Permissions)
    {
        azure::storage::cloud_table table = get_table();

        utility::string_t policy_name1 = _XPLATSTR("policy1");
        utility::string_t policy_name2 = _XPLATSTR("policy2");

        utility::datetime start = utility::datetime::utc_now() - utility::datetime::from_minutes(5U);
        utility::datetime expiry = start + utility::datetime::from_hours(2U);

        uint8_t permission1 = azure::storage::table_shared_access_policy::permissions::read | azure::storage::table_shared_access_policy::permissions::add;
        uint8_t permission2 = azure::storage::table_shared_access_policy::permissions::read | azure::storage::table_shared_access_policy::permissions::update;

        azure::storage::table_shared_access_policy policy1(start, expiry, permission1);
        azure::storage::table_shared_access_policy policy2(start, expiry, permission2);

        {
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_permissions permissions = table.download_permissions(options, context);

            CHECK(permissions.policies().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
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
        }

        {
            azure::storage::table_permissions permissions;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::shared_access_policies<azure::storage::table_shared_access_policy> policies;
            policies.insert(std::make_pair(policy_name1, policy1));
            policies.insert(std::make_pair(policy_name2, policy2));

            permissions.set_policies(policies);
            table.upload_permissions(permissions, options, context);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_permissions permissions = table.download_permissions(options, context);

            CHECK_EQUAL(2U, permissions.policies().size());
            CHECK_EQUAL(permission1, permissions.policies()[policy_name1].permission());
            CHECK(permissions.policies()[policy_name1].start().is_initialized());
            CHECK(permissions.policies()[policy_name1].expiry().is_initialized());
            CHECK_EQUAL(permission2, permissions.policies()[policy_name2].permission());
            CHECK(permissions.policies()[policy_name2].start().is_initialized());
            CHECK(permissions.policies()[policy_name2].expiry().is_initialized());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
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
        }

        {
            azure::storage::table_permissions permissions;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::shared_access_policies<azure::storage::table_shared_access_policy> policies;

            permissions.set_policies(policies);
            table.upload_permissions(permissions, options, context);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_permissions permissions = table.download_permissions(options, context);

            CHECK(permissions.policies().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
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
        }

        {
            utility::string_t policy_name = _XPLATSTR("policy3");

            utility::datetime truncated_start;
            truncated_start = truncated_start + start.to_interval() / 1000ULL * 1000ULL;
            utility::datetime truncated_expiry;
            truncated_expiry = truncated_expiry + expiry.to_interval() / 1000ULL * 1000ULL;
            uint8_t permission = azure::storage::table_shared_access_policy::permissions::read;

            azure::storage::table_shared_access_policy policy(truncated_start, truncated_expiry, permission);

            azure::storage::shared_access_policies<azure::storage::table_shared_access_policy> policies;
            policies.insert(std::make_pair(policy_name, policy));

            azure::storage::table_permissions permissions;

            permissions.set_policies(policies);
            table.upload_permissions(permissions);
        }
    }

    TEST_FIXTURE(table_service_test_base, Table_SharedAccessSignature)
    {
        azure::storage::cloud_table table1 = get_table();

        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();
        int32_t property_value = get_random_int32();

        {
            azure::storage::table_entity entity(partition_key, row_key);
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("MyProperty"), azure::storage::entity_property(property_value)));
            azure::storage::table_operation insert_operation = azure::storage::table_operation::insert_entity(entity);
            azure::storage::table_result result = table1.execute(insert_operation);
        }

        {
            utility::datetime start_date = utility::datetime::utc_now() - utility::datetime::from_minutes(5U);
            utility::datetime expiry_date = start_date + utility::datetime::from_hours(2U);

            azure::storage::table_shared_access_policy policy;
            policy.set_permissions(azure::storage::table_shared_access_policy::permissions::read);
            policy.set_start(start_date);
            policy.set_expiry(expiry_date);

            const azure::storage::storage_uri& uri = table1.uri();
            utility::string_t sas_token = table1.get_shared_access_signature(policy, utility::string_t(), partition_key, row_key, partition_key, row_key);
            azure::storage::storage_credentials credentials(sas_token);
            azure::storage::cloud_table table2(uri, credentials);

            azure::storage::table_operation retrieve_operation = azure::storage::table_operation::retrieve_entity(partition_key, row_key);
            azure::storage::table_result result = table2.execute(retrieve_operation);

            CHECK_EQUAL(200, result.http_status_code());
            CHECK_EQUAL(1U, result.entity().properties().size());
            CHECK(result.entity().properties().find(_XPLATSTR("MyProperty")) != result.entity().properties().cend());
            CHECK_EQUAL(property_value, result.entity().properties().find(_XPLATSTR("MyProperty"))->second.int32_value());

            azure::storage::table_entity entity(partition_key, row_key);
            azure::storage::table_operation delete_operation = azure::storage::table_operation::delete_entity(entity);

            CHECK_THROW(table2.execute(delete_operation), azure::storage::storage_exception);
        }

        {
            // Verify the time format sent to the server is valid when the fractional seconds component of the time ends with some zeros
            utility::datetime start_date;
            start_date = start_date + utility::datetime::utc_now().to_interval() / 1000ULL * 1000ULL - utility::datetime::from_minutes(5U);
            utility::datetime expiry_date = start_date + utility::datetime::from_hours(2U);

            azure::storage::table_shared_access_policy policy;
            policy.set_permissions(azure::storage::table_shared_access_policy::permissions::read);
            policy.set_start(start_date);
            policy.set_expiry(expiry_date);

            const azure::storage::storage_uri& uri = table1.uri();
            utility::string_t sas_token = table1.get_shared_access_signature(policy, utility::string_t(), partition_key, row_key, partition_key, row_key);
            azure::storage::storage_credentials credentials(sas_token);
            azure::storage::cloud_table table2(uri, credentials);

            azure::storage::table_operation retrieve_operation = azure::storage::table_operation::retrieve_entity(partition_key, row_key);
            azure::storage::table_result result = table2.execute(retrieve_operation);

            CHECK_EQUAL(200, result.http_status_code());
            CHECK_EQUAL(1U, result.entity().properties().size());
            CHECK(result.entity().properties().find(_XPLATSTR("MyProperty")) != result.entity().properties().cend());
            CHECK_EQUAL(property_value, result.entity().properties().find(_XPLATSTR("MyProperty"))->second.int32_value());
        }
    }

    TEST_FIXTURE(table_service_test_base, Table_TruncatedDateTime)
    {
        azure::storage::cloud_table table = get_table();

        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();

        utility::datetime truncated_value;
        truncated_value = truncated_value + get_random_datetime().to_interval() / 1000ULL * 1000ULL;

        {
            azure::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(1);
            entity.properties().insert(azure::storage::table_entity::property_type(_XPLATSTR("PropertyA"), azure::storage::entity_property(truncated_value)));

            azure::storage::table_operation operation = azure::storage::table_operation::insert_entity(entity);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::NoContent, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_operation operation = azure::storage::table_operation::retrieve_entity(partition_key, row_key);
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            azure::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(!result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(1U, result.entity().properties().size());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyA")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyA"))->second.property_type() == azure::storage::edm_type::datetime);
            CHECK(result.entity().properties().find(_XPLATSTR("PropertyA"))->second.datetime_value() == truncated_value);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
            CHECK(context.request_results()[0].is_response_available());
            CHECK(context.request_results()[0].start_time().is_initialized());
            CHECK(context.request_results()[0].end_time().is_initialized());
            CHECK(context.request_results()[0].target_location() != azure::storage::storage_location::unspecified);
            CHECK_EQUAL(web::http::status_codes::OK, context.request_results()[0].http_status_code());
            CHECK(!context.request_results()[0].service_request_id().empty());
            CHECK(context.request_results()[0].request_date().is_initialized());
            CHECK(context.request_results()[0].content_md5().empty());
            CHECK(!context.request_results()[0].etag().empty());
            CHECK(context.request_results()[0].extended_error().code().empty());
            CHECK(context.request_results()[0].extended_error().message().empty());
            CHECK(context.request_results()[0].extended_error().details().empty());
        }

        {
            azure::storage::table_query query;
            azure::storage::table_request_options options;
            azure::storage::operation_context context;
            print_client_request_id(context, _XPLATSTR(""));

            utility::string_t filter_string = azure::storage::table_query::generate_filter_condition(_XPLATSTR("PropertyA"), azure::storage::query_comparison_operator::equal, truncated_value);
            query.set_filter_string(filter_string);

            std::vector<azure::storage::table_entity> results = execute_table_query(table, query, options, context);

            CHECK_EQUAL(1U, results.size());

            CHECK(results[0].partition_key().compare(partition_key) == 0);
            CHECK(results[0].row_key().compare(row_key) == 0);
            CHECK(results[0].timestamp().is_initialized());
            CHECK(!results[0].etag().empty());

            CHECK_EQUAL(1U, results[0].properties().size());
            CHECK(results[0].properties().find(_XPLATSTR("PropertyA")) != results[0].properties().cend());
            CHECK(results[0].properties().find(_XPLATSTR("PropertyA"))->second.property_type() == azure::storage::edm_type::datetime);
            CHECK(results[0].properties().find(_XPLATSTR("PropertyA"))->second.datetime_value() == truncated_value);

            CHECK(!context.client_request_id().empty());
            CHECK(context.start_time().is_initialized());
            CHECK(context.end_time().is_initialized());
            CHECK_EQUAL(1U, context.request_results().size());
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
        }

        table.delete_table();
    }
}
