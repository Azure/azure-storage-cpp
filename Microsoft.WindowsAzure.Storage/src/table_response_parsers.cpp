// -----------------------------------------------------------------------------------------
// <copyright file="table_response_parsers.cpp" company="Microsoft">
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
#include "wascore/protocol.h"
#include "was/common.h"

namespace wa { namespace storage { namespace protocol {

    /*
    class numeric_detector : public basic_ostream<char_t>
    {
    public:

        numeric_detector()
            : basic_ostream<char_t>(_Uninitialized::_Noinit)
        {
        }

        numeric_detector(const numeric_detector& other)
            : basic_ostream<char_t>(_Uninitialized::_Noinit), m_floating_point(other.m_floating_point)
        {
        }

        ~numeric_detector()
        {
        }

        numeric_detector operator<< (const int32_t& value)
        {
            m_floating_point = false;
            return *this;
        }

        numeric_detector operator<< (const double& value)
        {
            m_floating_point = true;
            return *this;
        }

        numeric_detector& operator= (const numeric_detector& other)
        {
            m_floating_point = other.m_floating_point;
        }

        bool is_floating_point() const
        {
            return m_floating_point;
        }

    private:

        bool m_floating_point;
    };
    */

    utility::string_t table_response_parsers::parse_etag(const web::http::http_response& response)
    {
        web::http::http_headers headers = response.headers();
        web::http::http_headers::const_iterator itr = headers.find(U("ETag"));

        utility::string_t etag;
        if (itr != headers.end())
        {
            etag = itr->second;
        }

        return etag;
    }

    continuation_token table_response_parsers::parse_continuation_token(const web::http::http_response& response, const request_result& result)
    {
        web::http::uri_builder builder;

        /*
        ostringstream_t next_marker;
        bool found_header = false;
        */

        web::http::http_headers headers = response.headers();

        web::http::http_headers::iterator next_partition_key_itr = headers.find(ms_header_continuation_next_partition_key);
        if (next_partition_key_itr != headers.end())
        {
            /*
            if (!found_header)
            {
                next_marker << U('&');
            }

            next_marker << table_service_next_partition_key << ...

            found_header = true;
            */

            builder.append_query(table_query_next_partition_key, next_partition_key_itr->second);
        }

        web::http::http_headers::iterator next_row_key_itr = headers.find(ms_header_continuation_next_row_key);
        if (next_row_key_itr != headers.end())
        {
            builder.append_query(table_query_next_row_key, next_row_key_itr->second);
        }

        web::http::http_headers::iterator next_table_name_itr = headers.find(ms_header_continuation_next_table_name);
        if (next_table_name_itr != headers.end())
        {
            builder.append_query(table_query_next_table_name, next_table_name_itr->second);
        }

        continuation_token continuation_token(builder.query());
        continuation_token.set_target_location(result.target_location());

        return continuation_token;
    }

    table_entity table_response_parsers::parse_entity(const web::json::value& obj)
    {
        table_entity entity;

        // TODO: Choose a smart estimate of the number of properties to avoid rehashes
        /*
        if (obj.size() > 3)
        {
            entity.properties().reserve((obj.size() - 2U) / 2U);
        }
        */

        /*
        utility::string_t partition_key;
        utility::string_t row_key;
        utility::string_t etag;
        datetime timestamp;
        unordered_map<utility::string_t, entity_property> properties;
        */

        utility::string_t last_type_for_property;
        edm_type last_type;

        for (web::json::value::const_iterator property_itr = obj.cbegin(); property_itr != obj.cend(); ++property_itr)
        {
            utility::string_t property_name = property_itr->first.as_string();
            web::json::value property_obj = property_itr->second;

            if (property_name.compare(0, 6, U("odata.")) == 0)
            {
                // The object is a special OData value

                // TODO: if needed use: odata.type, odata.id, odata.editlink

                if (property_name.compare(6, property_name.size() - 6, U("etag")) == 0)
                {
                    // The object is the ETag
                    if (!property_obj.is_null() && property_obj.is_string() && entity.etag().empty())
                    {
                        entity.set_etag(property_obj.as_string());
                    }
                }
            }
            else if (property_name.size() > 11 && property_name.compare(property_name.size() - 11, 11, U("@odata.type")) == 0)
            {
                // The object is the type of a property

                if (!property_obj.is_null() && property_obj.is_string())
                {
                    last_type_for_property = property_name.substr(0, property_name.size() - 11);
                    last_type = get_property_type(property_obj.as_string());
                }
            }
            else
            {
                if (property_name.compare(U("PartitionKey")) == 0)
                {
                    // The object is the Partition Key

                    if (!property_obj.is_null() && property_obj.is_string() && entity.partition_key().empty())
                    {
                        entity.set_partition_key(property_obj.as_string());
                    }
                }
                else if (property_name.compare(U("RowKey")) == 0)
                {
                    // The object is the Row Key

                    if (!property_obj.is_null() && property_obj.is_string() && entity.row_key().empty())
                    {
                        entity.set_row_key(property_obj.as_string());
                    }
                }
                else if (property_name.compare(U("Timestamp")) == 0)
                {
                    // The object is the Timestamp

                    if (!property_obj.is_null() && property_obj.is_string() && !entity.timestamp().is_initialized())
                    {
                        utility::datetime timestamp = core::parse_datetime(property_obj.as_string());
                        entity.set_timestamp(timestamp);
                    }
                }
                else
                {
                    // The object is a regular property

                    entity_property entity_property;

                    // The type is set to String for consistency unless a specific EDM type was specified
                    if (!property_obj.is_null())
                    {
                        /*
                        entity_property.set_property_type(edm_type::string);
                        entity_property.set_is_null(true);
                        */

                        if (property_obj.is_boolean())
                        {
                            entity_property.set_value(property_obj.as_bool());
                        }
                        else if (property_obj.is_number())
                        {
                            /*
                            // This workaround is used to determine if the property is stored internally as an integer or floating point
                            numeric_detector detector;
                            property_obj.serialize(detector);
                            */

                            if (property_obj.is_integer())
                            {
                                entity_property.set_value(property_obj.as_integer());
                            }
                            else
                            {
                                entity_property.set_value(property_obj.as_double());
                            }
                        }
                        else
                        {
                            entity_property.set_value(property_obj.as_string());
                            if (property_name.compare(last_type_for_property) == 0)
                            {
                                entity_property.set_property_type(last_type);
                            }
                        }
                    }

                    /*
                    if (property_name.compare(last_type_for_property) == 0)
                    {
                        entity_property
                    }
                    else
                    {
                        // Treat all unknown property types as strings for consistency
                        entity_property.set_property_type(edm_type::string);

                        if (property_obj.is_null())
                        {
                            entity_property.set_is_null(true);
                        }
                        else
                        {
                            entity_property.set_value(property_obj.to_string());
                        }

                        switch (property_obj.type())
                        {
                        case web::json::value::value_type::Null:
                            entity_property.set_is_null(true);
                            break;

                        case web::json::value::value_type::Boolean:
                            entity_property = entity_property(property_obj.as_bool());
                            break;

                        case web::json::value::value_type::Number:
                            property_obj.as_double()
                            break;
                        }
                    }
                    */

                    entity.properties().insert(table_entity::property_type(property_name, entity_property));
                }
            }
        }

        return entity;
    }

    std::vector<table_result> table_response_parsers::parse_batch_results(Concurrency::streams::stringstreambuf& response_buffer, bool is_query, size_t batch_size)
    {
        std::vector<table_result> batch_result;
        batch_result.reserve(batch_size);

        std::string& response_body = response_buffer.collection();

        // TODO: Make this Casablanca code more robust

        // More strings we will search for during the course of parsing
        std::string http_begin("HTTP"), etag_header("ETag: "), crlf("\r\n"), crlfcrlf("\r\n\r\n"), space(" ");

        if (is_query)
        {
            // Find the HTTP status line
            size_t response_begin = response_body.find(http_begin);

            // Find the status code within that line
            size_t status_code_begin = response_body.find(space, response_begin) + space.size();
            size_t status_code_end = response_body.find(space, status_code_begin);
            std::string status_code_string = response_body.substr(status_code_begin, status_code_end - status_code_begin);

            // Extract the status code as an integer
            int status_code = utility::conversions::scan_string<int>(utility::conversions::to_string_t(status_code_string));

            // Acceptable codes are 'Created' and 'NoContent'
            if (status_code == web::http::status_codes::OK || status_code == web::http::status_codes::Created || status_code == web::http::status_codes::Accepted || status_code == web::http::status_codes::NoContent || status_code == web::http::status_codes::PartialContent || status_code == web::http::status_codes::NotFound)
            {
                table_result result;
                result.set_http_status_code(status_code);

                // The operation succeeded. Try to find an ETag: header if it exists. Delete operations will not have an etag header
                // so copy the list of headers into a substring and search within the list, lest we cross a changeset boundary.
                size_t headersBegin = response_body.find(crlf, status_code_end) + crlf.size();
                size_t headersEnd = response_body.find(crlfcrlf, headersBegin) + crlf.size();
                std::string headers = response_body.substr(headersBegin, headersEnd - headersBegin);

                utility::string_t etag_value;

                size_t etagBegin = headers.find(etag_header);
                if (etagBegin != std::string::npos)
                {
                    etagBegin += etag_header.size();
                    size_t etagEnd = headers.find(crlf, etagBegin);
                    std::string etag = headers.substr(etagBegin, etagEnd - etagBegin);
                    etag_value = utility::conversions::to_string_t(etag);

                    result.set_etag(etag_value);
                }

                size_t content_begin_offset = response_body.find(crlfcrlf, status_code_end) + crlfcrlf.size();
                size_t content_end_offset = response_body.find("\r\n--batchresponse", content_begin_offset) + crlf.size();

                std::string content_string = response_body.substr(content_begin_offset, content_end_offset - content_begin_offset);

                web::json::value obj = web::json::value::parse(utility::conversions::to_string_t(content_string));
                table_entity entity = parse_entity(obj);
                entity.set_etag(etag_value);
                result.set_entity(entity);

                batch_result.push_back(result);
            }
            else
            {
                // An operation failed. Find the start of the XML block that contains information about the error
                size_t content_begin_offset = response_body.find(crlfcrlf, status_code_end) + crlfcrlf.size();
                size_t content_end_offset = response_body.find("\r\n--batchresponse", content_begin_offset) + crlf.size();

                std::string error_string = response_body.substr(content_begin_offset, content_end_offset - content_begin_offset);

                throw storage_exception(utility::conversions::to_utf8string(error_string));
            }
        }
        else
        {
            // Strings we will search for during the course of parsing
            std::string content_type("Content-Type"), boundary_begin("boundary=");

            // Find the Content-Type header for the batch - it contains the changeset marker right after the occurence of "boundary="
            size_t content_typeOffset = response_body.find(content_type);
            size_t changeset_begin_offset = response_body.find(boundary_begin, content_typeOffset) + boundary_begin.size();
            size_t changeset_end_offset = response_body.find(crlf, changeset_begin_offset);

            std::string changeset_marker_prefix = "--" + response_body.substr(changeset_begin_offset, changeset_end_offset - changeset_begin_offset);
            std::string changeset_begin_marker = changeset_marker_prefix + crlf;

            size_t result_index = SIZE_MAX;

            // Loop through the response looking for changeset begin markers.
            size_t next_changeset = response_body.find(changeset_begin_marker, changeset_end_offset);

            while(next_changeset != std::string::npos)
            {
                ++result_index;
                // Find the HTTP status line
                size_t response_begin = response_body.find(http_begin, next_changeset + changeset_begin_marker.size());

                // Find the status code within that line
                size_t status_code_begin = response_body.find(space, response_begin) + space.size();
                size_t status_code_end = response_body.find(space, status_code_begin);
                std::string status_code_string = response_body.substr(status_code_begin, status_code_end - status_code_begin);

                // Extract the status code as an integer
                int status_code = utility::conversions::scan_string<int>(utility::conversions::to_string_t(status_code_string));

                // Acceptable codes are 'Created' and 'NoContent'
                if (status_code == web::http::status_codes::OK || status_code == web::http::status_codes::Created || status_code == web::http::status_codes::Accepted || status_code == web::http::status_codes::NoContent || status_code == web::http::status_codes::PartialContent)
                {
                    table_result result;
                    result.set_http_status_code(status_code);

                    // The operation succeeded. Try to find an ETag: header if it exists. Delete operations will not have an etag header
                    // so copy the list of headers into a substring and search within the list, lest we cross a changeset boundary.
                    size_t headersBegin = response_body.find(crlf, status_code_end) + crlf.size();
                    size_t headersEnd = response_body.find(crlfcrlf, headersBegin) + crlf.size();
                    std::string headers = response_body.substr(headersBegin, headersEnd - headersBegin);

                    size_t etagBegin = headers.find(etag_header);
                    if (etagBegin != std::string::npos)
                    {
                        etagBegin += etag_header.size();
                        size_t etagEnd = headers.find(crlf, etagBegin);
                        std::string etag = headers.substr(etagBegin, etagEnd - etagBegin);

                        result.set_etag(utility::conversions::to_string_t(etag));
                    }

                    batch_result.push_back(result);

                    next_changeset = response_body.find(changeset_begin_marker, headersEnd);
                }
                else
                {
                    // An operation failed. Find the start of the XML block that contains information about the error
                    size_t content_begin_offset = response_body.find(crlfcrlf, status_code_end) + crlfcrlf.size();
                    size_t content_end_offset = response_body.find(changeset_marker_prefix, content_begin_offset);

                    std::string error_string = response_body.substr(content_begin_offset, content_end_offset - content_begin_offset);

                    throw storage_exception(utility::conversions::to_utf8string(error_string));
                }
            }
        }

        return batch_result;
    }

    std::vector<table_entity> table_response_parsers::parse_query_results(const web::json::value& obj)
    {
        std::vector<table_entity> result;

        if (obj.is_object())
        {
            /*
            for (web::json::value::const_iterator obj_itr = obj.cbegin(); obj_itr != obj.cend(); ++obj_itr)
            {
            }
            */

            web::json::value entities_obj = obj.get(U("value"));
            if (!entities_obj.is_null() && entities_obj.is_array())
            {
                for (web::json::value::const_iterator entity_itr = entities_obj.cbegin(); entity_itr != entities_obj.cend(); ++entity_itr)
                {
                    web::json::value entity_obj = entity_itr->second;
                    if (!entity_obj.is_null() && entity_obj.is_object() && entity_obj.size() > 0)
                    {
                        table_entity entity = parse_entity(entity_obj);
                        result.push_back(entity);
                    }
                }
            }
        }

        return result;
    }

    edm_type table_response_parsers::get_property_type(const utility::string_t& type_name)
    {
        if (type_name.compare(U("Edm.Binary")) == 0)
        {
            return edm_type::binary;
        }
        else if (type_name.compare(U("Edm.Boolean")) == 0)
        {
            return edm_type::boolean;
        }
        else if (type_name.compare(U("Edm.DateTime")) == 0)
        {
            return edm_type::datetime;
        }
        else if (type_name.compare(U("Edm.Double")) == 0)
        {
            return edm_type::double_floating_point;
        }
        else if (type_name.compare(U("Edm.Guid")) == 0)
        {
            return edm_type::guid;
        }
        else if (type_name.compare(U("Edm.Int32")) == 0)
        {
            return edm_type::int32;
        }
        else if (type_name.compare(U("Edm.Int64")) == 0)
        {
            return edm_type::int64;
        }
        else
        {
            return edm_type::string;
        }
    }

}}} // namespace wa::storage::protocol
