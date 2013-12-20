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
#include "test_helper.h"
#include "was/table.h"

SUITE(Table)
{
    /*
    utility::string_t property_null(bool is_null)
    {
        if (is_null)
        {
            return utility::string_t(U("null, "));
        }

        return utility::string_t();
    }

    utility::string_t property_type_name(edm_type property_type)
    {
        switch (property_type)
        {
        case edm_type::binary:
            return utility::string_t(U("binary"));

        case edm_type::boolean:
            return utility::string_t(U("boolean"));

        case edm_type::datetime:
            return utility::string_t(U("datetime"));

        case edm_type::double_floating_point:
            return utility::string_t(U("double"));

        case edm_type::guid:
            return utility::string_t(U("guid"));

        case edm_type::int32:
            return utility::string_t(U("int32"));

        case edm_type::int64:
            return utility::string_t(U("int64"));

        case edm_type::string:
            return utility::string_t(U("string"));

        }

        return utility::string_t(U("Unknown"));
    }

    utility::string_t random_string()
    {
        const int SIZE = 10;
        utility::string_t result;
        result.reserve(SIZE);
        for (int i = 0; i < SIZE; ++i)
        {
            result.push_back(U('0') + rand() % SIZE);
        }
        return result;
    }

    void print(const table_entity& entity)
    {
        utility::stringstream_t log;
        log << U("PK: ") << entity.partition_key() << U(", ") << U("FK: ") << entity.row_key() << U(", ") << U("TS: ") << entity.timestamp().to_string();

        unordered_map<utility::string_t, wa::storage::entity_property> properties = entity.properties();
        for (unordered_map<utility::string_t, wa::storage::entity_property>::const_iterator propertyIterator = properties.cbegin(); propertyIterator != properties.cend(); ++propertyIterator)
        {
            utility::string_t property_name = propertyIterator->first;
            entity_property property = propertyIterator->second;
            log << U(", ") << property_name << U(" (") << property_null(property.is_null()) << property_type_name(property.property_type()) << U("): ") << property.str();

            if (property.property_type() == edm_type::double_floating_point)
            {
                double x = property.double_value();
            }
        }

        log << endl;
        ucout << log.str();
    }

    void print(const table_result& result)
    {
        utility::stringstream_t log;
        log << U("Status: ") << result.http_status_code() << U("ETag: ") << result.etag() << endl;
        ucout << log.str();

        print(result.entity());
    }
    */

    /*
    utility::uuid_t get_guid() const
    {
        UUID new_id;
        UuidCreate(&new_id);

        utility::uuid_t result;
        memcpy(result.data, &new_id, 16);

        return result;
    }
    */

    TEST(Table_Empty)
    {
        wa::storage::cloud_table table;

        CHECK(table.service_client().base_uri().primary_uri().is_empty());
        CHECK(table.service_client().base_uri().secondary_uri().is_empty());
        CHECK(table.service_client().credentials().is_anonymous());
        CHECK(table.name().empty());
        CHECK(table.uri().primary_uri().is_empty());
        CHECK(table.uri().secondary_uri().is_empty());
    }

    TEST(Table_Uri)
    {
        wa::storage::storage_uri uri(web::http::uri(U("https://myaccount.table.core.windows.net/mytable")), web::http::uri(U("https://myaccount-secondary.table.core.windows.net/mytable")));

        wa::storage::cloud_table table(uri);

        web::http::uri expected_primary_uri(U("https://myaccount.table.core.windows.net"));
        web::http::uri expected_secondary_uri(U("https://myaccount-secondary.table.core.windows.net"));

        CHECK(table.service_client().base_uri().primary_uri() == expected_primary_uri);
        CHECK(table.service_client().base_uri().secondary_uri() == expected_secondary_uri);
        CHECK(table.service_client().credentials().is_anonymous());
        CHECK(table.name().compare(U("mytable")) == 0);
        CHECK(table.uri().primary_uri() == uri.primary_uri());
        CHECK(table.uri().secondary_uri() == uri.secondary_uri());
    }

    TEST(Table_UriAndCredentials)
    {
        wa::storage::storage_uri uri(web::http::uri(U("https://myaccount.table.core.windows.net/mytable")), web::http::uri(U("https://myaccount-secondary.table.core.windows.net/mytable")));
        wa::storage::storage_credentials credentials(U("devstoreaccount1"), U("Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="));

        wa::storage::cloud_table table(uri, credentials);

        web::http::uri expected_primary_uri(U("https://myaccount.table.core.windows.net"));
        web::http::uri expected_secondary_uri(U("https://myaccount-secondary.table.core.windows.net"));

        CHECK(table.service_client().base_uri().primary_uri() == expected_primary_uri);
        CHECK(table.service_client().base_uri().secondary_uri() == expected_secondary_uri);
        CHECK(table.service_client().credentials().is_shared_key());
        CHECK(table.name().compare(U("mytable")) == 0);
        CHECK(table.uri().primary_uri() == uri.primary_uri());
        CHECK(table.uri().secondary_uri() == uri.secondary_uri());
    }

    /*
    TEST(Table_ClientAndUri)
    {
        wa::storage::storage_uri base_uri(web::http::uri(U("https://myaccount.table.core.windows.net")), web::http::uri(U("https://myaccount-secondary.table.core.windows.net")));
        wa::storage::storage_credentials credentials(U("devstoreaccount1"), U("Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw=="));
        wa::storage::cloud_table_client client(base_uri, credentials);
        utility::string_t table_name(U("mytable"));
        wa::storage::storage_uri uri(web::http::uri(U("https://myaccount.table.core.windows.net/mytable")), web::http::uri(U("https://myaccount-secondary.table.core.windows.net/mytable")));

        wa::storage::cloud_table table(client, table_name, uri);

        CHECK(table.service_client().base_uri().primary_uri() == base_uri.primary_uri());
        CHECK(table.service_client().base_uri().secondary_uri() == base_uri.secondary_uri());
        CHECK(table.service_client().credentials().is_shared_key());
        CHECK(table.name().compare(U("mytable")) == 0);
        CHECK(table.uri().primary_uri() == uri.primary_uri());
        CHECK(table.uri().secondary_uri() == uri.secondary_uri());
    }
    */

    TEST(EntityProperty_Binary)
    {
        std::vector<uint8_t> value = get_random_binary_data();
        wa::storage::entity_property property(value);

        CHECK(property.property_type() == wa::storage::edm_type::binary);
        CHECK(!property.is_null());
        CHECK_ARRAY_EQUAL(value, property.binary_value(), value.size());

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

        CHECK(property.property_type() == wa::storage::edm_type::binary);
        CHECK(!property.is_null());
        CHECK_ARRAY_EQUAL(value, property.binary_value(), value.size());

        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.guid_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        CHECK(property.str().size() > 0);
    }

    TEST(EntityProperty_Boolean)
    {
        bool value = get_random_boolean();
        wa::storage::entity_property property(value);

        CHECK(property.property_type() == wa::storage::edm_type::boolean);
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

        CHECK(property.property_type() == wa::storage::edm_type::boolean);
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
    }

    TEST(EntityProperty_DateTime)
    {
        utility::datetime value = get_random_datetime();
        wa::storage::entity_property property(value);

        CHECK(property.property_type() == wa::storage::edm_type::datetime);
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

        CHECK(property.property_type() == wa::storage::edm_type::datetime);
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

    TEST(EntityProperty_Double)
    {
        double value = get_random_double();
        wa::storage::entity_property property(value);

        CHECK(property.property_type() == wa::storage::edm_type::double_floating_point);
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

        CHECK(property.property_type() == wa::storage::edm_type::double_floating_point);
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

        CHECK(property.property_type() == wa::storage::edm_type::double_floating_point);
        CHECK(!property.is_null());
        CHECK(property.double_value() != property.double_value()); // Only NaN is defined to not equal itself
        CHECK(property.str().size() > 0);

        value = std::numeric_limits<double>::infinity();
        property.set_value(value);

        CHECK(property.property_type() == wa::storage::edm_type::double_floating_point);
        CHECK(!property.is_null());
        CHECK(property.double_value() == value);
        CHECK(property.str().size() > 0);

        value = -std::numeric_limits<double>::infinity();
        property.set_value(value);

        CHECK(property.property_type() == wa::storage::edm_type::double_floating_point);
        CHECK(!property.is_null());
        CHECK(property.double_value() == value);
        CHECK(property.str().size() > 0);
    }

    TEST(EntityProperty_Guid)
    {
        utility::uuid value = get_random_guid();
        wa::storage::entity_property property(value);

        CHECK(property.property_type() == wa::storage::edm_type::guid);
        CHECK(!property.is_null());
        CHECK(property.guid_value() == value);

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

        CHECK(property.property_type() == wa::storage::edm_type::guid);
        CHECK(!property.is_null());
        CHECK(property.guid_value() == value);

        CHECK_THROW(property.binary_value(), std::runtime_error);
        CHECK_THROW(property.boolean_value(), std::runtime_error);
        CHECK_THROW(property.datetime_value(), std::runtime_error);
        CHECK_THROW(property.double_value(), std::runtime_error);
        CHECK_THROW(property.int32_value(), std::runtime_error);
        CHECK_THROW(property.int64_value(), std::runtime_error);
        CHECK_THROW(property.string_value(), std::runtime_error);

        CHECK(property.str().size() > 0);
    }

    TEST(EntityProperty_Int32)
    {
        int32_t value = get_random_int32();
        wa::storage::entity_property property(value);

        CHECK(property.property_type() == wa::storage::edm_type::int32);
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

        CHECK(property.property_type() == wa::storage::edm_type::int32);
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
    }

    TEST(EntityProperty_Int64)
    {
        int64_t value = get_random_int64();
        wa::storage::entity_property property(value);

        CHECK(property.property_type() == wa::storage::edm_type::int64);
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

        CHECK(property.property_type() == wa::storage::edm_type::int64);
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

    TEST(EntityProperty_String)
    {
        utility::string_t value = get_random_string();
        wa::storage::entity_property property(value);

        CHECK(property.property_type() == wa::storage::edm_type::string);
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

        CHECK(property.property_type() == wa::storage::edm_type::string);
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

    TEST(EntityProperty_Null)
    {
        wa::storage::entity_property property;

        CHECK(property.property_type() == wa::storage::edm_type::string);
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

        CHECK(property.property_type() == wa::storage::edm_type::string);
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

        CHECK(property.property_type() == wa::storage::edm_type::string);
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

        CHECK(property.property_type() == wa::storage::edm_type::int32);
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

        CHECK(property.property_type() == wa::storage::edm_type::int32);
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

    TEST(Entity_PartitionKeyAndRowKey)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();

        wa::storage::table_entity entity(partition_key, row_key);

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

        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyA"), wa::storage::entity_property(get_random_boolean())));
        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyG"), wa::storage::entity_property(get_random_binary_data())));

        CHECK(entity.properties().size() == 2U);
    }

    TEST(Entity_PartitionKeyAndRowKeyAndETagAndProperties)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();
        utility::string_t etag = get_random_string();
        wa::storage::table_entity::properties_type properties;

        properties.reserve(3);
        properties.insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(get_random_double())));
        properties.insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(get_random_string())));
        properties.insert(wa::storage::table_entity::property_type(U("PropertyH"), wa::storage::entity_property(get_random_guid())));

        wa::storage::table_entity entity(partition_key, row_key, etag, properties);

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

        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyA"), wa::storage::entity_property(get_random_boolean())));
        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyG"), wa::storage::entity_property(get_random_binary_data())));

        CHECK(entity.properties().size() == 5U);
    }

    TEST(Operation_Delete)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();
        wa::storage::table_entity entity(partition_key, row_key);

        wa::storage::table_operation operation = wa::storage::table_operation::delete_entity(entity);

        CHECK(operation.entity().partition_key().compare(partition_key) == 0);
        CHECK(operation.entity().row_key().compare(row_key) == 0);
        CHECK(operation.entity().timestamp() == entity.timestamp());
        CHECK(operation.entity().etag().compare(entity.etag()) == 0);
        CHECK(operation.operation_type() == wa::storage::table_operation_type::delete_operation);
    }

    TEST(Operation_Insert)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();
        wa::storage::table_entity entity(partition_key, row_key);

        wa::storage::table_operation operation = wa::storage::table_operation::insert_entity(entity);

        CHECK(operation.entity().partition_key().compare(partition_key) == 0);
        CHECK(operation.entity().row_key().compare(row_key) == 0);
        CHECK(operation.entity().timestamp() == entity.timestamp());
        CHECK(operation.entity().etag().compare(entity.etag()) == 0);
        CHECK(operation.operation_type() == wa::storage::table_operation_type::insert_operation);
    }

    TEST(Operation_InsertOrMerge)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();
        wa::storage::table_entity entity(partition_key, row_key);

        wa::storage::table_operation operation = wa::storage::table_operation::insert_or_merge_entity(entity);

        CHECK(operation.entity().partition_key().compare(partition_key) == 0);
        CHECK(operation.entity().row_key().compare(row_key) == 0);
        CHECK(operation.entity().timestamp() == entity.timestamp());
        CHECK(operation.entity().etag().compare(entity.etag()) == 0);
        CHECK(operation.operation_type() == wa::storage::table_operation_type::insert_or_merge_operation);
    }

    TEST(Operation_InsertOrReplace)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();
        wa::storage::table_entity entity(partition_key, row_key);

        wa::storage::table_operation operation = wa::storage::table_operation::insert_or_replace_entity(entity);

        CHECK(operation.entity().partition_key().compare(partition_key) == 0);
        CHECK(operation.entity().row_key().compare(row_key) == 0);
        CHECK(operation.entity().timestamp() == entity.timestamp());
        CHECK(operation.entity().etag().compare(entity.etag()) == 0);
        CHECK(operation.operation_type() == wa::storage::table_operation_type::insert_or_replace_operation);
    }

    TEST(Operation_Merge)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();
        wa::storage::table_entity entity(partition_key, row_key);

        wa::storage::table_operation operation = wa::storage::table_operation::merge_entity(entity);

        CHECK(operation.entity().partition_key().compare(partition_key) == 0);
        CHECK(operation.entity().row_key().compare(row_key) == 0);
        CHECK(operation.entity().timestamp() == entity.timestamp());
        CHECK(operation.entity().etag().compare(entity.etag()) == 0);
        CHECK(operation.operation_type() == wa::storage::table_operation_type::merge_operation);
    }

    TEST(Operation_Replace)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();
        wa::storage::table_entity entity(partition_key, row_key);

        wa::storage::table_operation operation = wa::storage::table_operation::replace_entity(entity);

        CHECK(operation.entity().partition_key().compare(partition_key) == 0);
        CHECK(operation.entity().row_key().compare(row_key) == 0);
        CHECK(operation.entity().timestamp() == entity.timestamp());
        CHECK(operation.entity().etag().compare(entity.etag()) == 0);
        CHECK(operation.operation_type() == wa::storage::table_operation_type::replace_operation);
    }

    TEST(Operation_Retrieve)
    {
        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();

        wa::storage::table_operation operation = wa::storage::table_operation::retrieve_entity(partition_key, row_key);

        CHECK(operation.entity().partition_key().compare(partition_key) == 0);
        CHECK(operation.entity().row_key().compare(row_key) == 0);
        CHECK(!operation.entity().timestamp().is_initialized());
        CHECK(operation.entity().etag().empty());
        CHECK(operation.operation_type() == wa::storage::table_operation_type::retrieve_operation);
    }

    TEST(Query_Normal)
    {
        wa::storage::table_query query;

        CHECK_EQUAL(-1, query.take_count());
        CHECK(query.filter_string().empty());
        CHECK(query.select_columns().empty());

        int take_count = 25;
        query.set_take_count(take_count);

        CHECK_EQUAL(take_count, query.take_count());

        utility::string_t filter_string(U("PartitionKey eq 'AAA' and RowKey lt 'BBB' and Timestamp ge datetime'2013-09-01T00:00:00Z'"));
        query.set_filter_string(filter_string);

        CHECK(query.filter_string().compare(filter_string) == 0);

        std::vector<utility::string_t> select_columns;
        select_columns.reserve(3);
        select_columns.push_back(U("PropertyA"));
        select_columns.push_back(U("PropertyB"));
        select_columns.push_back(U("PropertyC"));
        query.set_select_columns(select_columns);

        CHECK_EQUAL(select_columns.size(), query.select_columns().size());
        CHECK(query.select_columns()[0].compare(U("PropertyA")) == 0);
        CHECK(query.select_columns()[1].compare(U("PropertyB")) == 0);
        CHECK(query.select_columns()[2].compare(U("PropertyC")) == 0);
    }

    TEST(TableRequestOptions_Normal)
    {
        wa::storage::table_request_options options;

        CHECK(options.payload_format() == wa::storage::table_payload_format::json);

        wa::storage::table_payload_format payload_format = wa::storage::table_payload_format::json_no_metadata;
        options.set_payload_format(payload_format);

        CHECK(options.payload_format() == payload_format);
    }

    TEST(Table_CreateAndDelete)
    {
        utility::string_t table_name = get_table_name();
        wa::storage::cloud_table_client client = get_table_client();

        wa::storage::cloud_table table = client.get_table_reference(table_name);

        {
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            table.create(options, context);
        }

        {
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            bool exists = table.exists(options, context);

            CHECK(exists);
        }

        {
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            CHECK_THROW(table.create(options, context), wa::storage::storage_exception);
        }

        {
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            table.delete_table(options, context);
        }

        {
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            bool exists = table.exists(options, context);

            CHECK(!exists);
        }

        {
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            CHECK_THROW(table.delete_table(options, context), wa::storage::storage_exception);
        }
    }

    TEST(Table_CreateIfNotExistsAndDeleteIfExists)
    {
        utility::string_t table_name = get_table_name();
        wa::storage::cloud_table_client client = get_table_client();

        wa::storage::cloud_table table = client.get_table_reference(table_name);

        {
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            bool created = table.create_if_not_exists(options, context);

            CHECK(created);
        }

        {
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            bool exists = table.exists(options, context);

            CHECK(exists);
        }

        {
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            bool created = table.create_if_not_exists(options, context);

            CHECK(!created);
        }

        {
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            bool deleted = table.delete_table_if_exists(options, context);

            CHECK(deleted);
        }

        {
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            bool exists = table.exists(options, context);

            CHECK(!exists);
        }

        {
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            bool deleted = table.delete_table_if_exists(options, context);

            CHECK(!deleted);
        }
    }

    /*
    class foo : public web::json::details::_Number
    {
    public:
        foo()
            : web::json::details::_Number(123.4567890123456789)
        {
        }

        std::basic_string<utility::char_t> bar()
        {
            std::basic_string<utility::char_t> stream;
            format(stream);
            return stream;
        }
    };

    TEST(FooTest)
    {
        foo x;
        x.bar();
    }
    */

    TEST(EntityOperation_InsertAndDelete)
    {
        wa::storage::cloud_table table = get_table();

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
            wa::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyA"), wa::storage::entity_property(boolean_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyB"), wa::storage::entity_property(int32_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(int64_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(double_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(string_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyF"), wa::storage::entity_property(datetime_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyG"), wa::storage::entity_property(binary_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyH"), wa::storage::entity_property(guid_value)));

            wa::storage::table_operation operation = wa::storage::table_operation::insert_entity(entity);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());
        }

        {
            wa::storage::table_operation operation = wa::storage::table_operation::retrieve_entity(partition_key, row_key);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(8, result.entity().properties().size());
            CHECK(result.entity().properties().find(U("PropertyA")) != result.entity().properties().cend());
            CHECK_EQUAL(boolean_value, result.entity().properties().find(U("PropertyA"))->second.boolean_value());
            CHECK(result.entity().properties().find(U("PropertyB")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value, result.entity().properties().find(U("PropertyB"))->second.int32_value());
            CHECK(result.entity().properties().find(U("PropertyC")) != result.entity().properties().cend());
            CHECK_EQUAL(int64_value, result.entity().properties().find(U("PropertyC"))->second.int64_value());
            CHECK(result.entity().properties().find(U("PropertyD")) != result.entity().properties().cend());
            CHECK_EQUAL(double_value, result.entity().properties().find(U("PropertyD"))->second.double_value());
            CHECK(result.entity().properties().find(U("PropertyE")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(U("PropertyE"))->second.string_value().compare(string_value) == 0);
            CHECK(result.entity().properties().find(U("PropertyF")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(U("PropertyF"))->second.datetime_value() == datetime_value);
            CHECK(result.entity().properties().find(U("PropertyG")) != result.entity().properties().cend());
            CHECK_ARRAY_EQUAL(binary_value, result.entity().properties().find(U("PropertyG"))->second.binary_value(), binary_value.size());
            CHECK(result.entity().properties().find(U("PropertyH")) != result.entity().properties().cend());
            CHECK(utility::uuid_equal(result.entity().properties().find(U("PropertyH"))->second.guid_value(), guid_value));
        }

        {
            wa::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyA"), wa::storage::entity_property(boolean_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyB"), wa::storage::entity_property(int32_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(int64_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(double_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(string_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyF"), wa::storage::entity_property(datetime_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyG"), wa::storage::entity_property(binary_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyH"), wa::storage::entity_property(guid_value)));

            wa::storage::table_operation operation = wa::storage::table_operation::insert_entity(entity);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            CHECK_THROW(table.execute(operation, options, context), wa::storage::storage_exception);
        }

        {
            //utility::string_t etag(U("*"));

            wa::storage::table_entity entity(partition_key, row_key);
            //entity.set_etag(etag);

            wa::storage::table_operation operation = wa::storage::table_operation::delete_entity(entity);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(result.etag().empty());
        }

        {
            wa::storage::table_operation operation = wa::storage::table_operation::retrieve_entity(partition_key, row_key);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(404, result.http_status_code());
            CHECK(result.etag().empty());
        }

        {
            wa::storage::table_entity entity(partition_key, row_key);

            wa::storage::table_operation operation = wa::storage::table_operation::delete_entity(entity);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            CHECK_THROW(table.execute(operation, options, context), wa::storage::storage_exception);
        }

        table.delete_table();
    }

    TEST(EntityOperation_InsertAndMerge)
    {
        wa::storage::cloud_table table = get_table();

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
            wa::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyA"), wa::storage::entity_property(boolean_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyB"), wa::storage::entity_property(int32_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(int64_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(double_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(string_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyF"), wa::storage::entity_property(datetime_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyG"), wa::storage::entity_property(binary_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyH"), wa::storage::entity_property(guid_value)));

            wa::storage::table_operation operation = wa::storage::table_operation::insert_or_merge_entity(entity);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());
        }

        {
            wa::storage::table_operation operation = wa::storage::table_operation::retrieve_entity(partition_key, row_key);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(8, result.entity().properties().size());
            CHECK(result.entity().properties().find(U("PropertyA")) != result.entity().properties().cend());
            CHECK_EQUAL(boolean_value, result.entity().properties().find(U("PropertyA"))->second.boolean_value());
            CHECK(result.entity().properties().find(U("PropertyB")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value, result.entity().properties().find(U("PropertyB"))->second.int32_value());
            CHECK(result.entity().properties().find(U("PropertyC")) != result.entity().properties().cend());
            CHECK_EQUAL(int64_value, result.entity().properties().find(U("PropertyC"))->second.int64_value());
            CHECK(result.entity().properties().find(U("PropertyD")) != result.entity().properties().cend());
            CHECK_EQUAL(double_value, result.entity().properties().find(U("PropertyD"))->second.double_value());
            CHECK(result.entity().properties().find(U("PropertyE")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(U("PropertyE"))->second.string_value().compare(string_value) == 0);
            CHECK(result.entity().properties().find(U("PropertyF")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(U("PropertyF"))->second.datetime_value() == datetime_value);
            CHECK(result.entity().properties().find(U("PropertyG")) != result.entity().properties().cend());
            CHECK_ARRAY_EQUAL(binary_value, result.entity().properties().find(U("PropertyG"))->second.binary_value(), binary_value.size());
            CHECK(result.entity().properties().find(U("PropertyH")) != result.entity().properties().cend());
            CHECK(utility::uuid_equal(result.entity().properties().find(U("PropertyH"))->second.guid_value(), guid_value));
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
            wa::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(int64_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(double_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(string_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyF"), wa::storage::entity_property(datetime_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyG"), wa::storage::entity_property(binary_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyH"), wa::storage::entity_property(guid_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyI"), wa::storage::entity_property(int32_value2)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyJ"), wa::storage::entity_property(int32_value3)));

            wa::storage::table_operation operation = wa::storage::table_operation::insert_or_merge_entity(entity);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());
        }

        {
            wa::storage::table_operation operation = wa::storage::table_operation::retrieve_entity(partition_key, row_key);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(10, result.entity().properties().size());
            CHECK(result.entity().properties().find(U("PropertyA")) != result.entity().properties().cend());
            CHECK_EQUAL(boolean_value, result.entity().properties().find(U("PropertyA"))->second.boolean_value());
            CHECK(result.entity().properties().find(U("PropertyB")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value, result.entity().properties().find(U("PropertyB"))->second.int32_value());
            CHECK(result.entity().properties().find(U("PropertyC")) != result.entity().properties().cend());
            CHECK_EQUAL(int64_value, result.entity().properties().find(U("PropertyC"))->second.int64_value());
            CHECK(result.entity().properties().find(U("PropertyD")) != result.entity().properties().cend());
            CHECK_EQUAL(double_value, result.entity().properties().find(U("PropertyD"))->second.double_value());
            CHECK(result.entity().properties().find(U("PropertyE")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(U("PropertyE"))->second.string_value().compare(string_value) == 0);
            CHECK(result.entity().properties().find(U("PropertyF")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(U("PropertyF"))->second.datetime_value() == datetime_value);
            CHECK(result.entity().properties().find(U("PropertyG")) != result.entity().properties().cend());
            CHECK_ARRAY_EQUAL(binary_value, result.entity().properties().find(U("PropertyG"))->second.binary_value(), binary_value.size());
            CHECK(result.entity().properties().find(U("PropertyH")) != result.entity().properties().cend());
            CHECK(utility::uuid_equal(result.entity().properties().find(U("PropertyH"))->second.guid_value(), guid_value));
            CHECK(result.entity().properties().find(U("PropertyI")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value2, result.entity().properties().find(U("PropertyI"))->second.int32_value());
            CHECK(result.entity().properties().find(U("PropertyJ")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value3, result.entity().properties().find(U("PropertyJ"))->second.int32_value());
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
            wa::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(int64_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(double_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(string_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyF"), wa::storage::entity_property(datetime_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyG"), wa::storage::entity_property(binary_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyH"), wa::storage::entity_property(guid_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyK"), wa::storage::entity_property(int32_value4)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyL"), wa::storage::entity_property(int32_value5)));

            wa::storage::table_operation operation = wa::storage::table_operation::merge_entity(entity);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());
        }

        {
            wa::storage::table_operation operation = wa::storage::table_operation::retrieve_entity(partition_key, row_key);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(12, result.entity().properties().size());
            CHECK(result.entity().properties().find(U("PropertyA")) != result.entity().properties().cend());
            CHECK_EQUAL(boolean_value, result.entity().properties().find(U("PropertyA"))->second.boolean_value());
            CHECK(result.entity().properties().find(U("PropertyB")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value, result.entity().properties().find(U("PropertyB"))->second.int32_value());
            CHECK(result.entity().properties().find(U("PropertyC")) != result.entity().properties().cend());
            CHECK_EQUAL(int64_value, result.entity().properties().find(U("PropertyC"))->second.int64_value());
            CHECK(result.entity().properties().find(U("PropertyD")) != result.entity().properties().cend());
            CHECK_EQUAL(double_value, result.entity().properties().find(U("PropertyD"))->second.double_value());
            CHECK(result.entity().properties().find(U("PropertyE")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(U("PropertyE"))->second.string_value().compare(string_value) == 0);
            CHECK(result.entity().properties().find(U("PropertyF")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(U("PropertyF"))->second.datetime_value() == datetime_value);
            CHECK(result.entity().properties().find(U("PropertyG")) != result.entity().properties().cend());
            CHECK_ARRAY_EQUAL(binary_value, result.entity().properties().find(U("PropertyG"))->second.binary_value(), binary_value.size());
            CHECK(result.entity().properties().find(U("PropertyH")) != result.entity().properties().cend());
            CHECK(utility::uuid_equal(result.entity().properties().find(U("PropertyH"))->second.guid_value(), guid_value));
            CHECK(result.entity().properties().find(U("PropertyI")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value2, result.entity().properties().find(U("PropertyI"))->second.int32_value());
            CHECK(result.entity().properties().find(U("PropertyJ")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value3, result.entity().properties().find(U("PropertyJ"))->second.int32_value());
            CHECK(result.entity().properties().find(U("PropertyK")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value4, result.entity().properties().find(U("PropertyK"))->second.int32_value());
            CHECK(result.entity().properties().find(U("PropertyL")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value5, result.entity().properties().find(U("PropertyL"))->second.int32_value());
        }

        {
            wa::storage::table_entity entity(partition_key, row_key);

            wa::storage::table_operation operation = wa::storage::table_operation::delete_entity(entity);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(result.etag().empty());
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
            wa::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(int64_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(double_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(string_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyF"), wa::storage::entity_property(datetime_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyG"), wa::storage::entity_property(binary_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyH"), wa::storage::entity_property(guid_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyM"), wa::storage::entity_property(int32_value6)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyN"), wa::storage::entity_property(int32_value7)));

            wa::storage::table_operation operation = wa::storage::table_operation::merge_entity(entity);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            CHECK_THROW(table.execute(operation, options, context), wa::storage::storage_exception);
        }

        table.delete_table();
    }

    TEST(EntityOperation_InsertAndReplace)
    {
        wa::storage::cloud_table table = get_table();

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
            wa::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyA"), wa::storage::entity_property(boolean_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyB"), wa::storage::entity_property(int32_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(int64_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(double_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(string_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyF"), wa::storage::entity_property(datetime_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyG"), wa::storage::entity_property(binary_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyH"), wa::storage::entity_property(guid_value)));

            wa::storage::table_operation operation = wa::storage::table_operation::insert_or_replace_entity(entity);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());
        }

        {
            wa::storage::table_operation operation = wa::storage::table_operation::retrieve_entity(partition_key, row_key);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(8, result.entity().properties().size());
            CHECK(result.entity().properties().find(U("PropertyA")) != result.entity().properties().cend());
            CHECK_EQUAL(boolean_value, result.entity().properties().find(U("PropertyA"))->second.boolean_value());
            CHECK(result.entity().properties().find(U("PropertyB")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value, result.entity().properties().find(U("PropertyB"))->second.int32_value());
            CHECK(result.entity().properties().find(U("PropertyC")) != result.entity().properties().cend());
            CHECK_EQUAL(int64_value, result.entity().properties().find(U("PropertyC"))->second.int64_value());
            CHECK(result.entity().properties().find(U("PropertyD")) != result.entity().properties().cend());
            CHECK_EQUAL(double_value, result.entity().properties().find(U("PropertyD"))->second.double_value());
            CHECK(result.entity().properties().find(U("PropertyE")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(U("PropertyE"))->second.string_value().compare(string_value) == 0);
            CHECK(result.entity().properties().find(U("PropertyF")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(U("PropertyF"))->second.datetime_value() == datetime_value);
            CHECK(result.entity().properties().find(U("PropertyG")) != result.entity().properties().cend());
            CHECK_ARRAY_EQUAL(binary_value, result.entity().properties().find(U("PropertyG"))->second.binary_value(), binary_value.size());
            CHECK(result.entity().properties().find(U("PropertyH")) != result.entity().properties().cend());
            CHECK(utility::uuid_equal(result.entity().properties().find(U("PropertyH"))->second.guid_value(), guid_value));
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
            wa::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(int64_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(double_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(string_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyF"), wa::storage::entity_property(datetime_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyG"), wa::storage::entity_property(binary_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyH"), wa::storage::entity_property(guid_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyI"), wa::storage::entity_property(int32_value2)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyJ"), wa::storage::entity_property(int32_value3)));

            wa::storage::table_operation operation = wa::storage::table_operation::insert_or_replace_entity(entity);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());
        }

        {
            wa::storage::table_operation operation = wa::storage::table_operation::retrieve_entity(partition_key, row_key);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(8, result.entity().properties().size());
            CHECK(result.entity().properties().find(U("PropertyC")) != result.entity().properties().cend());
            CHECK_EQUAL(int64_value, result.entity().properties().find(U("PropertyC"))->second.int64_value());
            CHECK(result.entity().properties().find(U("PropertyD")) != result.entity().properties().cend());
            CHECK_EQUAL(double_value, result.entity().properties().find(U("PropertyD"))->second.double_value());
            CHECK(result.entity().properties().find(U("PropertyE")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(U("PropertyE"))->second.string_value().compare(string_value) == 0);
            CHECK(result.entity().properties().find(U("PropertyF")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(U("PropertyF"))->second.datetime_value() == datetime_value);
            CHECK(result.entity().properties().find(U("PropertyG")) != result.entity().properties().cend());
            CHECK_ARRAY_EQUAL(binary_value, result.entity().properties().find(U("PropertyG"))->second.binary_value(), binary_value.size());
            CHECK(result.entity().properties().find(U("PropertyH")) != result.entity().properties().cend());
            CHECK(utility::uuid_equal(result.entity().properties().find(U("PropertyH"))->second.guid_value(), guid_value));
            CHECK(result.entity().properties().find(U("PropertyI")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value2, result.entity().properties().find(U("PropertyI"))->second.int32_value());
            CHECK(result.entity().properties().find(U("PropertyJ")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value3, result.entity().properties().find(U("PropertyJ"))->second.int32_value());
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
            wa::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(int64_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(double_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(string_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyF"), wa::storage::entity_property(datetime_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyG"), wa::storage::entity_property(binary_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyH"), wa::storage::entity_property(guid_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyK"), wa::storage::entity_property(int32_value4)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyL"), wa::storage::entity_property(int32_value5)));

            wa::storage::table_operation operation = wa::storage::table_operation::replace_entity(entity);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());
        }

        {
            wa::storage::table_operation operation = wa::storage::table_operation::retrieve_entity(partition_key, row_key);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(8, result.entity().properties().size());
            CHECK(result.entity().properties().find(U("PropertyC")) != result.entity().properties().cend());
            CHECK_EQUAL(int64_value, result.entity().properties().find(U("PropertyC"))->second.int64_value());
            CHECK(result.entity().properties().find(U("PropertyD")) != result.entity().properties().cend());
            CHECK_EQUAL(double_value, result.entity().properties().find(U("PropertyD"))->second.double_value());
            CHECK(result.entity().properties().find(U("PropertyE")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(U("PropertyE"))->second.string_value().compare(string_value) == 0);
            CHECK(result.entity().properties().find(U("PropertyF")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(U("PropertyF"))->second.datetime_value() == datetime_value);
            CHECK(result.entity().properties().find(U("PropertyG")) != result.entity().properties().cend());
            CHECK_ARRAY_EQUAL(binary_value, result.entity().properties().find(U("PropertyG"))->second.binary_value(), binary_value.size());
            CHECK(result.entity().properties().find(U("PropertyH")) != result.entity().properties().cend());
            CHECK(utility::uuid_equal(result.entity().properties().find(U("PropertyH"))->second.guid_value(), guid_value));
            CHECK(result.entity().properties().find(U("PropertyK")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value4, result.entity().properties().find(U("PropertyK"))->second.int32_value());
            CHECK(result.entity().properties().find(U("PropertyL")) != result.entity().properties().cend());
            CHECK_EQUAL(int32_value5, result.entity().properties().find(U("PropertyL"))->second.int32_value());
        }

        {
            wa::storage::table_entity entity(partition_key, row_key);

            wa::storage::table_operation operation = wa::storage::table_operation::delete_entity(entity);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(result.etag().empty());
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
            wa::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(8);
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(int64_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(double_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(string_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyF"), wa::storage::entity_property(datetime_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyG"), wa::storage::entity_property(binary_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyH"), wa::storage::entity_property(guid_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyM"), wa::storage::entity_property(int32_value6)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyN"), wa::storage::entity_property(int32_value7)));

            wa::storage::table_operation operation = wa::storage::table_operation::replace_entity(entity);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            CHECK_THROW(table.execute(operation, options, context), wa::storage::storage_exception);
        }

        table.delete_table();
    }

    TEST(EntityOperation_DoubleSpecialValues)
    {
        wa::storage::cloud_table table = get_table();

        utility::string_t partition_key = get_random_string();
        utility::string_t row_key = get_random_string();

        double nan_value = std::numeric_limits<double>::quiet_NaN();
        double infinity_value = std::numeric_limits<double>::infinity();
        double negative_infinity_value = -std::numeric_limits<double>::infinity();
        double negative_zero = -0.0;
        double round_number = 123.0;

        {
            wa::storage::table_entity entity(partition_key, row_key);

            entity.properties().reserve(4);
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyA"), wa::storage::entity_property(nan_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyB"), wa::storage::entity_property(infinity_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(negative_infinity_value)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(negative_zero)));
            entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(round_number)));

            wa::storage::table_operation operation = wa::storage::table_operation::insert_entity(entity);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(!result.etag().empty());
        }

        {
            wa::storage::table_operation operation = wa::storage::table_operation::retrieve_entity(partition_key, row_key);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().compare(partition_key) == 0);
            CHECK(result.entity().row_key().compare(row_key) == 0);
            CHECK(result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(200, result.http_status_code());
            CHECK(!result.etag().empty());

            CHECK_EQUAL(5, result.entity().properties().size());
            CHECK(result.entity().properties().find(U("PropertyA")) != result.entity().properties().cend());
            CHECK(result.entity().properties().find(U("PropertyA"))->second.double_value() != result.entity().properties().find(U("PropertyA"))->second.double_value()); // Only NaN is defined to not equal itself
            CHECK(result.entity().properties().find(U("PropertyB")) != result.entity().properties().cend());
            CHECK_EQUAL(infinity_value, result.entity().properties().find(U("PropertyB"))->second.double_value());
            CHECK(result.entity().properties().find(U("PropertyC")) != result.entity().properties().cend());
            CHECK_EQUAL(negative_infinity_value, result.entity().properties().find(U("PropertyC"))->second.double_value());

            // TODO: Handle -0.0 correctly (also investigate that the service and other client libraries can handle -0.0)
            /*
            CHECK(result.entity().properties().find(U("PropertyD")) != result.entity().properties().cend());
            CHECK_EQUAL(0.0, result.entity().properties().find(U("PropertyD"))->second.double_value());
            CHECK_EQUAL(negative_infinity_value, 1.0 / result.entity().properties().find(U("PropertyD"))->second.double_value()); // 1.0 / -0.0 == -Infinity
            */

            CHECK(result.entity().properties().find(U("PropertyE")) != result.entity().properties().cend());
            CHECK_EQUAL(round_number, result.entity().properties().find(U("PropertyE"))->second.double_value());
        }

        {
            wa::storage::table_entity entity(partition_key, row_key);

            wa::storage::table_operation operation = wa::storage::table_operation::delete_entity(entity);
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_result result = table.execute(operation, options, context);

            CHECK(result.entity().partition_key().empty());
            CHECK(result.entity().row_key().empty());
            CHECK(!result.entity().timestamp().is_initialized());
            CHECK(result.entity().etag().empty());
            CHECK_EQUAL(204, result.http_status_code());
            CHECK(result.etag().empty());
        }

        table.delete_table();
    }

    TEST(EntityBatch_Normal)
    {
        const int BATCH_SIZE = 3;

        wa::storage::cloud_table table = get_table();

        utility::string_t partition_key = get_random_string();
        utility::string_t row_keys[BATCH_SIZE];

        for (int i = 0; i < BATCH_SIZE; ++i)
        {
            row_keys[i] = get_random_string();
        }

        int32_t int32_value = get_random_int32();
        utility::string_t string_value = get_random_string();

        {
            wa::storage::table_batch_operation operation;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                wa::storage::table_entity entity(partition_key, row_keys[i]);

                entity.properties().reserve(2);
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyA"), wa::storage::entity_property(int32_value)));
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyB"), wa::storage::entity_property(string_value)));

                operation.insert_entity(entity);
            }

            std::vector<wa::storage::table_result> results = table.execute_batch(operation, options, context);

            CHECK_EQUAL(BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                /*
                CHECK(operation.operations()[i].entity().partition_key().compare(results[i].entity().partition_key()) == 0);
                CHECK(operation.operations()[i].entity().row_key().compare(results[i].entity().row_key()) == 0);
                */
                CHECK(results[i].entity().partition_key().empty());
                CHECK(results[i].entity().row_key().empty());
                CHECK(!results[i].entity().timestamp().is_initialized());
                CHECK(results[i].entity().etag().empty());
                CHECK_EQUAL(0U, results[i].entity().properties().size());
                CHECK_EQUAL(204, results[i].http_status_code());
                CHECK(!results[i].etag().empty());
            }
        }

        {
            wa::storage::table_query query;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            utility::string_t filter_string = wa::storage::table_query::generate_filter_condition(U("PartitionKey"), wa::storage::query_comparison_operator::equal, partition_key);
            query.set_filter_string(filter_string);

            std::vector<wa::storage::table_entity> results = table.execute_query(query, options, context);

            CHECK_EQUAL(BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                CHECK_EQUAL(2, results[i].properties().size());
                CHECK_EQUAL(int32_value, results[i].properties()[U("PropertyA")].int32_value());
                CHECK(string_value.compare(results[i].properties()[U("PropertyB")].string_value()) == 0);
            }
        }

        int32_t int32_value2 = get_random_int32();
        utility::string_t string_value2 = get_random_string();

        {
            wa::storage::table_batch_operation operation;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                wa::storage::table_entity entity(partition_key, row_keys[i]);

                entity.properties().reserve(3);
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyA"), wa::storage::entity_property(int32_value2)));
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyB"), wa::storage::entity_property(string_value)));
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(string_value2)));

                operation.insert_or_merge_entity(entity);
            }

            std::vector<wa::storage::table_result> results = table.execute_batch(operation, options, context);

            CHECK_EQUAL(BATCH_SIZE, results.size());

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
        }

        {
            wa::storage::table_query query;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            utility::string_t filter_string = wa::storage::table_query::generate_filter_condition(U("PartitionKey"), wa::storage::query_comparison_operator::equal, partition_key);
            query.set_filter_string(filter_string);

            std::vector<wa::storage::table_entity> results = table.execute_query(query, options, context);

            CHECK_EQUAL(BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                CHECK_EQUAL(3, results[i].properties().size());
                CHECK_EQUAL(int32_value2, results[i].properties()[U("PropertyA")].int32_value());
                CHECK(string_value.compare(results[i].properties()[U("PropertyB")].string_value()) == 0);
                CHECK(string_value2.compare(results[i].properties()[U("PropertyC")].string_value()) == 0);
            }
        }

        int32_t int32_value3 = get_random_int32();
        utility::string_t string_value3 = get_random_string();

        {
            wa::storage::table_batch_operation operation;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                wa::storage::table_entity entity(partition_key, row_keys[i]);

                entity.properties().reserve(3);
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyA"), wa::storage::entity_property(int32_value3)));
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyB"), wa::storage::entity_property(string_value)));
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(string_value3)));

                operation.insert_or_replace_entity(entity);
            }

            std::vector<wa::storage::table_result> results = table.execute_batch(operation, options, context);

            CHECK_EQUAL(BATCH_SIZE, results.size());

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
        }

        {
            wa::storage::table_query query;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            utility::string_t filter_string = wa::storage::table_query::generate_filter_condition(U("PartitionKey"), wa::storage::query_comparison_operator::equal, partition_key);
            query.set_filter_string(filter_string);

            std::vector<wa::storage::table_entity> results = table.execute_query(query, options, context);

            CHECK_EQUAL(BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                CHECK_EQUAL(3, results[i].properties().size());
                CHECK_EQUAL(int32_value3, results[i].properties()[U("PropertyA")].int32_value());
                CHECK(string_value.compare(results[i].properties()[U("PropertyB")].string_value()) == 0);
                CHECK(string_value3.compare(results[i].properties()[U("PropertyD")].string_value()) == 0);
            }
        }

        int32_t int32_value4 = get_random_int32();
        utility::string_t string_value4 = get_random_string();

        {
            wa::storage::table_batch_operation operation;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                wa::storage::table_entity entity(partition_key, row_keys[i]);

                entity.properties().reserve(2);
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyB"), wa::storage::entity_property(string_value4)));
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(int32_value4)));

                operation.replace_entity(entity);
            }

            std::vector<wa::storage::table_result> results = table.execute_batch(operation, options, context);

            CHECK_EQUAL(BATCH_SIZE, results.size());

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
        }

        {
            wa::storage::table_query query;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            utility::string_t filter_string = wa::storage::table_query::generate_filter_condition(U("PartitionKey"), wa::storage::query_comparison_operator::equal, partition_key);
            query.set_filter_string(filter_string);

            std::vector<wa::storage::table_entity> results = table.execute_query(query, options, context);

            CHECK_EQUAL(BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                CHECK_EQUAL(2, results[i].properties().size());
                CHECK(string_value4.compare(results[i].properties()[U("PropertyB")].string_value()) == 0);
                CHECK_EQUAL(int32_value4, results[i].properties()[U("PropertyE")].int32_value());
            }
        }

        int32_t int32_value5 = get_random_int32();
        utility::string_t string_value5 = get_random_string();

        {
            wa::storage::table_batch_operation operation;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                wa::storage::table_entity entity(partition_key, row_keys[i]);

                entity.properties().reserve(2);
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(int32_value5)));
                entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyF"), wa::storage::entity_property(string_value5)));

                operation.merge_entity(entity);
            }

            std::vector<wa::storage::table_result> results = table.execute_batch(operation, options, context);

            CHECK_EQUAL(BATCH_SIZE, results.size());

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
        }

        {
            wa::storage::table_query query;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            utility::string_t filter_string = wa::storage::table_query::generate_filter_condition(U("PartitionKey"), wa::storage::query_comparison_operator::equal, partition_key);
            query.set_filter_string(filter_string);

            std::vector<wa::storage::table_entity> results = table.execute_query(query, options, context);

            CHECK_EQUAL(BATCH_SIZE, results.size());

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                CHECK_EQUAL(3, results[i].properties().size());
                CHECK(string_value4.compare(results[i].properties()[U("PropertyB")].string_value()) == 0);
                CHECK_EQUAL(int32_value5, results[i].properties()[U("PropertyE")].int32_value());
                CHECK(string_value5.compare(results[i].properties()[U("PropertyF")].string_value()) == 0);
            }
        }

        {
            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                wa::storage::table_batch_operation operation;
                wa::storage::table_request_options options;
                wa::storage::operation_context context;

                operation.retrieve_entity(partition_key, row_keys[i]);

                std::vector<wa::storage::table_result> results = table.execute_batch(operation, options, context);

                CHECK_EQUAL(1, results.size());

                CHECK(operation.operations()[0].entity().partition_key().compare(results[0].entity().partition_key()) == 0);
                CHECK(operation.operations()[0].entity().row_key().compare(results[0].entity().row_key()) == 0);
                CHECK(!results[0].entity().etag().empty());
                CHECK(results[0].entity().timestamp().is_initialized());
                CHECK_EQUAL(3U, results[0].entity().properties().size());
                CHECK_EQUAL(200, results[0].http_status_code());
                CHECK(!results[0].etag().empty());
            }
        }

        {
            wa::storage::table_batch_operation operation;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                wa::storage::table_entity entity(partition_key, row_keys[i]);

                operation.delete_entity(entity);
            }

            std::vector<wa::storage::table_result> results = table.execute_batch(operation, options, context);

            CHECK_EQUAL(BATCH_SIZE, results.size());

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
        }

        {
            for (int i = 0; i < BATCH_SIZE; ++i)
            {
                wa::storage::table_batch_operation operation;
                wa::storage::table_request_options options;
                wa::storage::operation_context context;

                operation.retrieve_entity(partition_key, row_keys[i]);

                std::vector<wa::storage::table_result> results = table.execute_batch(operation, options, context);

                CHECK_EQUAL(1, results.size());

                CHECK(results[0].entity().partition_key().empty());
                CHECK(results[0].entity().row_key().empty());
                CHECK(results[0].entity().etag().empty());
                CHECK(!results[0].entity().timestamp().is_initialized());
                CHECK_EQUAL(0U, results[0].entity().properties().size());
                CHECK_EQUAL(404, results[0].http_status_code());
                CHECK(results[0].etag().empty());
            }
        }

        table.delete_table();
    }

    TEST(EntityBatch_PartitionKeyMismatch)
    {
        wa::storage::cloud_table table = get_table(false);

        utility::string_t partition_key1 = get_random_string();
        utility::string_t row_key1 = get_random_string();
        utility::string_t partition_key2 = get_random_string();
        utility::string_t row_key2 = get_random_string();

        wa::storage::table_batch_operation operation;
        wa::storage::table_request_options options;
        wa::storage::operation_context context;

        wa::storage::table_entity entity1(partition_key1, row_key1);
        operation.insert_entity(entity1);

        wa::storage::table_entity entity2(partition_key2, row_key2);
        operation.insert_entity(entity2);

        CHECK_THROW(table.execute_batch(operation, options, context), std::invalid_argument);
    }

    TEST(EntityBatch_MultipleRetrieve)
    {
        wa::storage::cloud_table table = get_table(false);

        utility::string_t partition_key = get_random_string();
        utility::string_t row_key1 = get_random_string();
        utility::string_t row_key2 = get_random_string();

        wa::storage::table_batch_operation operation;
        wa::storage::table_request_options options;
        wa::storage::operation_context context;

        operation.retrieve_entity(partition_key, row_key1);
        operation.retrieve_entity(partition_key, row_key2);

        CHECK_THROW(table.execute_batch(operation, options, context), std::invalid_argument);
    }

    TEST(EntityBatch_RetrieveMixture)
    {
        wa::storage::cloud_table table = get_table(false);

        utility::string_t partition_key = get_random_string();
        utility::string_t row_key1 = get_random_string();
        utility::string_t row_key2 = get_random_string();

        wa::storage::table_batch_operation operation;
        wa::storage::table_request_options options;
        wa::storage::operation_context context;

        operation.retrieve_entity(partition_key, row_key1);

        wa::storage::table_entity entity2(partition_key, row_key2);
        operation.insert_entity(entity2);

        CHECK_THROW(table.execute_batch(operation, options, context), std::invalid_argument);
    }

    TEST(EntityQuery_Normal)
    {
        wa::storage::cloud_table table = get_table();

        utility::string_t partition_key1 = get_random_string();
        utility::string_t partition_key2 = get_random_string();

        {
            for (int partition = 1; partition <= 2; ++partition)
            {
                utility::string_t partition_key = partition == 1 ? partition_key1 : partition_key2;

                for (int row1 = 0; row1 < 26; ++row1)
                {
                    wa::storage::table_batch_operation operation;

                    for (int row2 = 0; row2 < 26; ++row2)
                    {
                        utility::string_t row_key = get_string('a' + row1, 'a' + row2);

                        wa::storage::table_entity entity(partition_key, row_key);

                        entity.properties().reserve(8);
                        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyA"), wa::storage::entity_property(get_random_boolean())));
                        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyB"), wa::storage::entity_property(get_random_int32())));
                        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(get_random_int64())));
                        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(get_random_double())));
                        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(get_random_string())));
                        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyF"), wa::storage::entity_property(get_random_datetime())));
                        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyG"), wa::storage::entity_property(get_random_binary_data())));
                        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyH"), wa::storage::entity_property(get_random_guid())));

                        operation.insert_entity(entity);
                    }

                    std::vector<wa::storage::table_result> results = table.execute_batch(operation);

                    for (std::vector<wa::storage::table_result>::const_iterator itr = results.cbegin(); itr != results.cend(); ++itr)
                    {
                        wa::storage::table_result result = *itr;

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

        {
            wa::storage::table_query query;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            int take_count = 10;
            query.set_take_count(take_count);

            CHECK_EQUAL(take_count, query.take_count());

            utility::string_t filter_string = wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::generate_filter_condition(U("PartitionKey"), wa::storage::query_comparison_operator::equal, partition_key1), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("RowKey"), wa::storage::query_comparison_operator::greater_than_or_equal, U("k"))), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("RowKey"), wa::storage::query_comparison_operator::less_than, U("n"))), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("Timestamp"), wa::storage::query_comparison_operator::greater_than_or_equal, utility::datetime::from_string(U("2013-09-01T00:00:00Z"), utility::datetime::ISO_8601))), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyA"), wa::storage::query_comparison_operator::not_equal, false)),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyB"), wa::storage::query_comparison_operator::not_equal, 1234567890)),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyC"), wa::storage::query_comparison_operator::not_equal, 1234567890123456789LL)),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyD"), wa::storage::query_comparison_operator::not_equal, 9.1234567890123456789)),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyE"), wa::storage::query_comparison_operator::not_equal, U("ABCDE12345"))),
                wa::storage::query_logical_operator::and, 
                // TODO: Add fractional seconds
                wa::storage::table_query::generate_filter_condition(U("PropertyF"), wa::storage::query_comparison_operator::not_equal, utility::datetime::from_string(U("2013-01-02T03:04:05Z"), utility::datetime::date_format::ISO_8601))),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyG"), wa::storage::query_comparison_operator::not_equal, std::vector<uint8_t>(10, 'X'))),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyH"), wa::storage::query_comparison_operator::not_equal, utility::string_to_uuid(U("12345678-abcd-efab-cdef-1234567890ab"))));
            query.set_filter_string(filter_string);

            utility::string_t expected_filter_string = utility::string_t(U("(((((((((((PartitionKey eq '")) + partition_key1 + utility::string_t(U("') and (RowKey ge 'k')) and (RowKey lt 'n')) and (Timestamp ge datetime'2013-09-01T00:00:00Z')) and (PropertyA ne false)) and (PropertyB ne 1234567890)) and (PropertyC ne 1234567890123456789L)) and (PropertyD ne 9.1234567890123461)) and (PropertyE ne 'ABCDE12345')) and (PropertyF ne datetime'2013-01-02T03:04:05Z')) and (PropertyG ne X'58585858585858585858')) and (PropertyH ne guid'12345678-abcd-efab-cdef-1234567890ab')"));
            CHECK(filter_string.compare(expected_filter_string) == 0);

            std::vector<utility::string_t> select_columns;
            select_columns.reserve(9);
            select_columns.push_back(U("PropertyA"));
            select_columns.push_back(U("PropertyB"));
            select_columns.push_back(U("PropertyC"));
            select_columns.push_back(U("PropertyD"));
            select_columns.push_back(U("PropertyE"));
            select_columns.push_back(U("PropertyF"));
            select_columns.push_back(U("PropertyG"));
            select_columns.push_back(U("PropertyH"));
            select_columns.push_back(U("PropertyX"));
            query.set_select_columns(select_columns);

            options.set_payload_format(wa::storage::table_payload_format::json_full_metadata);

            std::vector<wa::storage::table_entity> results = table.execute_query(query, options, context);

            CHECK(results.size() > 0);
            CHECK((int)results.size() > take_count);

            for (std::vector<wa::storage::table_entity>::const_iterator entity_iterator = results.cbegin(); entity_iterator != results.cend(); ++entity_iterator)
            {
                wa::storage::table_entity entity = *entity_iterator;

                CHECK(!entity.partition_key().empty());
                CHECK(!entity.row_key().empty());
                CHECK(entity.timestamp().is_initialized());
                CHECK(!entity.etag().empty());

                wa::storage::table_entity::properties_type properties = entity.properties();

                CHECK_EQUAL(9, properties.size());

                for (wa::storage::table_entity::properties_type::const_iterator propertyIterator = properties.cbegin(); propertyIterator != properties.cend(); ++propertyIterator)
                {
                    utility::string_t property_name = propertyIterator->first;
                    wa::storage::entity_property property = propertyIterator->second;

                    CHECK(!property_name.empty());
                    CHECK(property.is_null() || !property.str().empty());
                }
            }
        }

        {
            wa::storage::table_query query;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            int take_count = 10;
            query.set_take_count(take_count);

            CHECK_EQUAL(take_count, query.take_count());

            utility::string_t filter_string = wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::generate_filter_condition(U("PartitionKey"), wa::storage::query_comparison_operator::equal, partition_key1), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("RowKey"), wa::storage::query_comparison_operator::greater_than_or_equal, U("k"))), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("RowKey"), wa::storage::query_comparison_operator::less_than, U("n"))), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("Timestamp"), wa::storage::query_comparison_operator::greater_than_or_equal, utility::datetime::from_string(U("2013-09-01T00:00:00Z"), utility::datetime::ISO_8601))), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyA"), wa::storage::query_comparison_operator::not_equal, false)),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyB"), wa::storage::query_comparison_operator::not_equal, 1234567890)),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyC"), wa::storage::query_comparison_operator::not_equal, 1234567890123456789LL)),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyD"), wa::storage::query_comparison_operator::not_equal, 9.1234567890123456789)),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyE"), wa::storage::query_comparison_operator::not_equal, U("ABCDE12345"))),
                wa::storage::query_logical_operator::and, 
                // TODO: Add fractional seconds
                wa::storage::table_query::generate_filter_condition(U("PropertyF"), wa::storage::query_comparison_operator::not_equal, utility::datetime::from_string(U("2013-01-02T03:04:05Z"), utility::datetime::date_format::ISO_8601))),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyG"), wa::storage::query_comparison_operator::not_equal, std::vector<uint8_t>(10, 'X'))),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyH"), wa::storage::query_comparison_operator::not_equal, utility::string_to_uuid(U("12345678-abcd-efab-cdef-1234567890ab"))));
            query.set_filter_string(filter_string);

            utility::string_t expected_filter_string = utility::string_t(U("(((((((((((PartitionKey eq '")) + partition_key1 + utility::string_t(U("') and (RowKey ge 'k')) and (RowKey lt 'n')) and (Timestamp ge datetime'2013-09-01T00:00:00Z')) and (PropertyA ne false)) and (PropertyB ne 1234567890)) and (PropertyC ne 1234567890123456789L)) and (PropertyD ne 9.1234567890123461)) and (PropertyE ne 'ABCDE12345')) and (PropertyF ne datetime'2013-01-02T03:04:05Z')) and (PropertyG ne X'58585858585858585858')) and (PropertyH ne guid'12345678-abcd-efab-cdef-1234567890ab')"));
            CHECK(filter_string.compare(expected_filter_string) == 0);

            std::vector<utility::string_t> select_columns;
            select_columns.reserve(9);
            select_columns.push_back(U("PropertyA"));
            select_columns.push_back(U("PropertyB"));
            select_columns.push_back(U("PropertyC"));
            select_columns.push_back(U("PropertyD"));
            select_columns.push_back(U("PropertyE"));
            select_columns.push_back(U("PropertyF"));
            select_columns.push_back(U("PropertyG"));
            select_columns.push_back(U("PropertyH"));
            select_columns.push_back(U("PropertyX"));
            query.set_select_columns(select_columns);

            options.set_payload_format(wa::storage::table_payload_format::json_full_metadata);

            std::vector<wa::storage::table_entity> results = table.execute_query(query, options, context);

            CHECK(results.size() > 0);

            for (std::vector<wa::storage::table_entity>::const_iterator entity_iterator = results.cbegin(); entity_iterator != results.cend(); ++entity_iterator)
            {
                wa::storage::table_entity entity = *entity_iterator;

                CHECK(!entity.partition_key().empty());
                CHECK(!entity.row_key().empty());
                CHECK(entity.timestamp().is_initialized());
                CHECK(!entity.etag().empty());

                wa::storage::table_entity::properties_type properties = entity.properties();

                CHECK_EQUAL(9, properties.size());

                for (wa::storage::table_entity::properties_type::const_iterator propertyIterator = properties.cbegin(); propertyIterator != properties.cend(); ++propertyIterator)
                {
                    utility::string_t property_name = propertyIterator->first;
                    wa::storage::entity_property property = propertyIterator->second;

                    CHECK(!property_name.empty());
                    CHECK(property.is_null() || !property.str().empty());
                }
            }
        }

        table.delete_table();
    }

    TEST(EntityQuery_Segmented)
    {
        wa::storage::cloud_table table = get_table();

        utility::string_t partition_key1 = get_random_string();
        utility::string_t partition_key2 = get_random_string();

        {
            for (int partition = 1; partition <= 2; ++partition)
            {
                utility::string_t partition_key = partition == 1 ? partition_key1 : partition_key2;

                for (int row1 = 0; row1 < 26; ++row1)
                {
                    wa::storage::table_batch_operation operation;

                    for (int row2 = 0; row2 < 26; ++row2)
                    {
                        utility::string_t row_key = get_string('a' + row1, 'a' + row2);

                        wa::storage::table_entity entity(partition_key, row_key);

                        entity.properties().reserve(8);
                        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyA"), wa::storage::entity_property(get_random_boolean())));
                        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyB"), wa::storage::entity_property(get_random_int32())));
                        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyC"), wa::storage::entity_property(get_random_int64())));
                        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyD"), wa::storage::entity_property(get_random_double())));
                        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyE"), wa::storage::entity_property(get_random_string())));
                        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyF"), wa::storage::entity_property(get_random_datetime())));
                        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyG"), wa::storage::entity_property(get_random_binary_data())));
                        entity.properties().insert(wa::storage::table_entity::property_type(U("PropertyH"), wa::storage::entity_property(get_random_guid())));

                        operation.insert_entity(entity);
                    }

                    std::vector<wa::storage::table_result> results = table.execute_batch(operation);

                    for (std::vector<wa::storage::table_result>::const_iterator itr = results.cbegin(); itr != results.cend(); ++itr)
                    {
                        wa::storage::table_result result = *itr;

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

        {
            wa::storage::table_query query;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            int take_count = 10;
            query.set_take_count(take_count);

            CHECK_EQUAL(take_count, query.take_count());

            utility::string_t filter_string = wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::generate_filter_condition(U("PartitionKey"), wa::storage::query_comparison_operator::equal, partition_key1), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("RowKey"), wa::storage::query_comparison_operator::greater_than_or_equal, U("k"))), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("RowKey"), wa::storage::query_comparison_operator::less_than, U("n"))), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("Timestamp"), wa::storage::query_comparison_operator::greater_than_or_equal, utility::datetime::from_string(U("2013-09-01T00:00:00Z"), utility::datetime::ISO_8601))), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyA"), wa::storage::query_comparison_operator::not_equal, false)),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyB"), wa::storage::query_comparison_operator::not_equal, 1234567890)),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyC"), wa::storage::query_comparison_operator::not_equal, 1234567890123456789LL)),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyD"), wa::storage::query_comparison_operator::not_equal, 9.1234567890123456789)),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyE"), wa::storage::query_comparison_operator::not_equal, U("ABCDE12345"))),
                wa::storage::query_logical_operator::and, 
                // TODO: Add fractional seconds
                wa::storage::table_query::generate_filter_condition(U("PropertyF"), wa::storage::query_comparison_operator::not_equal, utility::datetime::from_string(U("2013-01-02T03:04:05Z"), utility::datetime::date_format::ISO_8601))),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyG"), wa::storage::query_comparison_operator::not_equal, std::vector<uint8_t>(10, 'X'))),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyH"), wa::storage::query_comparison_operator::not_equal, utility::string_to_uuid(U("12345678-abcd-efab-cdef-1234567890ab"))));
            query.set_filter_string(filter_string);

            utility::string_t expected_filter_string = utility::string_t(U("(((((((((((PartitionKey eq '")) + partition_key1 + utility::string_t(U("') and (RowKey ge 'k')) and (RowKey lt 'n')) and (Timestamp ge datetime'2013-09-01T00:00:00Z')) and (PropertyA ne false)) and (PropertyB ne 1234567890)) and (PropertyC ne 1234567890123456789L)) and (PropertyD ne 9.1234567890123461)) and (PropertyE ne 'ABCDE12345')) and (PropertyF ne datetime'2013-01-02T03:04:05Z')) and (PropertyG ne X'58585858585858585858')) and (PropertyH ne guid'12345678-abcd-efab-cdef-1234567890ab')"));
            CHECK(filter_string.compare(expected_filter_string) == 0);

            std::vector<utility::string_t> select_columns;
            select_columns.reserve(9);
            select_columns.push_back(U("PropertyA"));
            select_columns.push_back(U("PropertyB"));
            select_columns.push_back(U("PropertyC"));
            select_columns.push_back(U("PropertyD"));
            select_columns.push_back(U("PropertyE"));
            select_columns.push_back(U("PropertyF"));
            select_columns.push_back(U("PropertyG"));
            select_columns.push_back(U("PropertyH"));
            select_columns.push_back(U("PropertyX"));
            query.set_select_columns(select_columns);

            options.set_payload_format(wa::storage::table_payload_format::json_full_metadata);

            std::vector<wa::storage::table_entity> results = table.execute_query(query, options, context);

            CHECK(results.size() > 0);
            CHECK((int)results.size() > take_count);

            for (std::vector<wa::storage::table_entity>::const_iterator entity_iterator = results.cbegin(); entity_iterator != results.cend(); ++entity_iterator)
            {
                wa::storage::table_entity entity = *entity_iterator;

                CHECK(!entity.partition_key().empty());
                CHECK(!entity.row_key().empty());
                CHECK(entity.timestamp().is_initialized());
                CHECK(!entity.etag().empty());

                wa::storage::table_entity::properties_type properties = entity.properties();

                CHECK_EQUAL(9, properties.size());

                for (wa::storage::table_entity::properties_type::const_iterator propertyIterator = properties.cbegin(); propertyIterator != properties.cend(); ++propertyIterator)
                {
                    utility::string_t property_name = propertyIterator->first;
                    wa::storage::entity_property property = propertyIterator->second;

                    CHECK(!property_name.empty());
                    CHECK(property.is_null() || !property.str().empty());
                }
            }
        }

        {
            wa::storage::table_query query;
            wa::storage::continuation_token continuation_token;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            int take_count = 10;
            query.set_take_count(take_count);

            CHECK_EQUAL(take_count, query.take_count());

            utility::string_t filter_string = wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::combine_filter_conditions(
                wa::storage::table_query::generate_filter_condition(U("PartitionKey"), wa::storage::query_comparison_operator::equal, partition_key1), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("RowKey"), wa::storage::query_comparison_operator::greater_than_or_equal, U("k"))), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("RowKey"), wa::storage::query_comparison_operator::less_than, U("n"))), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("Timestamp"), wa::storage::query_comparison_operator::greater_than_or_equal, utility::datetime::from_string(U("2013-09-01T00:00:00Z"), utility::datetime::ISO_8601))), 
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyA"), wa::storage::query_comparison_operator::not_equal, false)),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyB"), wa::storage::query_comparison_operator::not_equal, 1234567890)),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyC"), wa::storage::query_comparison_operator::not_equal, 1234567890123456789LL)),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyD"), wa::storage::query_comparison_operator::not_equal, 9.1234567890123456789)),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyE"), wa::storage::query_comparison_operator::not_equal, U("ABCDE12345"))),
                wa::storage::query_logical_operator::and, 
                // TODO: Add fractional seconds
                wa::storage::table_query::generate_filter_condition(U("PropertyF"), wa::storage::query_comparison_operator::not_equal, utility::datetime::from_string(U("2013-01-02T03:04:05Z"), utility::datetime::date_format::ISO_8601))),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyG"), wa::storage::query_comparison_operator::not_equal, std::vector<uint8_t>(10, 'X'))),
                wa::storage::query_logical_operator::and, 
                wa::storage::table_query::generate_filter_condition(U("PropertyH"), wa::storage::query_comparison_operator::not_equal, utility::string_to_uuid(U("12345678-abcd-efab-cdef-1234567890ab"))));
            query.set_filter_string(filter_string);

            utility::string_t expected_filter_string = utility::string_t(U("(((((((((((PartitionKey eq '")) + partition_key1 + utility::string_t(U("') and (RowKey ge 'k')) and (RowKey lt 'n')) and (Timestamp ge datetime'2013-09-01T00:00:00Z')) and (PropertyA ne false)) and (PropertyB ne 1234567890)) and (PropertyC ne 1234567890123456789L)) and (PropertyD ne 9.1234567890123461)) and (PropertyE ne 'ABCDE12345')) and (PropertyF ne datetime'2013-01-02T03:04:05Z')) and (PropertyG ne X'58585858585858585858')) and (PropertyH ne guid'12345678-abcd-efab-cdef-1234567890ab')"));
            CHECK(filter_string.compare(expected_filter_string) == 0);

            std::vector<utility::string_t> select_columns;
            select_columns.reserve(9);
            select_columns.push_back(U("PropertyA"));
            select_columns.push_back(U("PropertyB"));
            select_columns.push_back(U("PropertyC"));
            select_columns.push_back(U("PropertyD"));
            select_columns.push_back(U("PropertyE"));
            select_columns.push_back(U("PropertyF"));
            select_columns.push_back(U("PropertyG"));
            select_columns.push_back(U("PropertyH"));
            select_columns.push_back(U("PropertyX"));
            query.set_select_columns(select_columns);

            options.set_payload_format(wa::storage::table_payload_format::json_full_metadata);

            int segment_count = 0;
            wa::storage::table_query_segment query_segment;
            do
            {
                query_segment = table.execute_query_segmented(query, continuation_token, options, context);
                std::vector<wa::storage::table_entity> results = query_segment.results();

                CHECK(results.size() > 0);
                CHECK((int)results.size() <= take_count);

                for (std::vector<wa::storage::table_entity>::const_iterator entity_iterator = results.cbegin(); entity_iterator != results.cend(); ++entity_iterator)
                {
                    wa::storage::table_entity entity = *entity_iterator;

                    CHECK(!entity.partition_key().empty());
                    CHECK(!entity.row_key().empty());
                    CHECK(entity.timestamp().is_initialized());
                    CHECK(!entity.etag().empty());

                    wa::storage::table_entity::properties_type properties = entity.properties();

                    CHECK_EQUAL(9, properties.size());

                    for (wa::storage::table_entity::properties_type::const_iterator propertyIterator = properties.cbegin(); propertyIterator != properties.cend(); ++propertyIterator)
                    {
                        utility::string_t property_name = propertyIterator->first;
                        wa::storage::entity_property property = propertyIterator->second;

                        CHECK(!property_name.empty());
                        CHECK(property.is_null() || !property.str().empty());
                    }
                }

                ++segment_count;
                continuation_token = query_segment.continuation_token();
            }
            while (!continuation_token.empty());

            CHECK(segment_count > 1);
        }

        table.delete_table();
    }

    TEST(Table_Permissions)
    {
        wa::storage::cloud_table table = get_table();

        utility::string_t policy_name1 = U("policy1");
        utility::string_t policy_name2 = U("policy2");

        uint8_t permission1 = wa::storage::table_shared_access_policy::permissions::read | wa::storage::table_shared_access_policy::permissions::add;
        uint8_t permission2 = wa::storage::table_shared_access_policy::permissions::read | wa::storage::table_shared_access_policy::permissions::update;

        wa::storage::table_shared_access_policy policy1(utility::datetime::utc_now(), utility::datetime::utc_now(), permission1);
        wa::storage::table_shared_access_policy policy2(utility::datetime::utc_now(), utility::datetime::utc_now(), permission2);

        {
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_permissions permissions = table.download_permissions(options, context);

            CHECK(permissions.policies().empty());
        }

        {
            wa::storage::table_permissions permissions;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::shared_access_policies<wa::storage::table_shared_access_policy> policies;
            policies.insert(std::pair<utility::string_t, wa::storage::table_shared_access_policy>(policy_name1, policy1));
            policies.insert(std::pair<utility::string_t, wa::storage::table_shared_access_policy>(policy_name2, policy2));

            permissions.set_policies(policies);
            table.upload_permissions(permissions, options, context);
        }

        {
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_permissions permissions = table.download_permissions(options, context);

            CHECK_EQUAL(2U, permissions.policies().size());
            CHECK_EQUAL(permission1, permissions.policies()[policy_name1].permission());
            CHECK(permissions.policies()[policy_name1].start().is_initialized());
            CHECK(permissions.policies()[policy_name1].expiry().is_initialized());
            CHECK_EQUAL(permission2, permissions.policies()[policy_name2].permission());
            CHECK(permissions.policies()[policy_name2].start().is_initialized());
            CHECK(permissions.policies()[policy_name2].expiry().is_initialized());
        }

        {
            wa::storage::table_permissions permissions;
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::shared_access_policies<wa::storage::table_shared_access_policy> policies;

            permissions.set_policies(policies);
            table.upload_permissions(permissions, options, context);
        }

        {
            wa::storage::table_request_options options;
            wa::storage::operation_context context;

            wa::storage::table_permissions permissions = table.download_permissions(options, context);

            CHECK(permissions.policies().empty());
        }
    }
}
