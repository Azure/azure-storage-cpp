// -----------------------------------------------------------------------------------------
// <copyright file="entity_property.cpp" company="Microsoft">
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
#include "was/table.h"
#include "wascore/util.h"
#include "wascore/resources.h"

namespace azure { namespace storage {

    std::vector<uint8_t> entity_property::binary_value() const
    {
        if (m_property_type != edm_type::binary)
        {
            throw std::runtime_error(protocol::error_entity_property_not_binary);
        }

        return std::vector<uint8_t>(utility::conversions::from_base64(m_value));
    }

    bool entity_property::boolean_value() const
    {
        if (m_property_type != edm_type::boolean)
        {
            throw std::runtime_error(protocol::error_entity_property_not_boolean);
        }

        if (m_value.compare(_XPLATSTR("false")) == 0)
        {
            return false;
        }
        else if (m_value.compare(_XPLATSTR("true")) == 0)
        {
            return true;
        }
        else
        {
            throw std::runtime_error(protocol::error_parse_boolean);
        }
    }

    utility::datetime entity_property::datetime_value() const
    {
        if (m_property_type != edm_type::datetime)
        {
            throw std::runtime_error(protocol::error_entity_property_not_datetime);
        }

        utility::datetime result = utility::datetime::from_string(m_value, utility::datetime::ISO_8601);
        if (!result.is_initialized())
        {
            throw std::runtime_error(protocol::error_parse_datetime);
        }

        return result;
    }

    double entity_property::double_value() const
    {
        if (m_property_type != edm_type::double_floating_point)
        {
            throw std::runtime_error(protocol::error_entity_property_not_double);
        }

        if (m_value.compare(protocol::double_not_a_number) == 0)
        {
            return std::numeric_limits<double>::quiet_NaN();
        }
        else if (m_value.compare(protocol::double_infinity) == 0)
        {
            return std::numeric_limits<double>::infinity();
        }
        else if (m_value.compare(protocol::double_negative_infinity) == 0)
        {
            return -std::numeric_limits<double>::infinity();
        }

        double result;
        utility::istringstream_t buffer(m_value);
        buffer >> result;

        if (buffer.fail() || !buffer.eof())
        {
            throw std::runtime_error(protocol::error_parse_double);
        }

        return result;
    }

    utility::uuid entity_property::guid_value() const
    {
        if (m_property_type != edm_type::guid)
        {
            throw std::runtime_error(protocol::error_entity_property_not_guid);
        }

        utility::uuid result = utility::string_to_uuid(m_value);
        return result;
    }

    int32_t entity_property::int32_value() const
    {
        if (m_property_type != edm_type::int32)
        {
            throw std::runtime_error(protocol::error_entity_property_not_int32);
        }

        int32_t result;
        utility::istringstream_t buffer(m_value);
        buffer >> result;

        if (buffer.fail() || !buffer.eof())
        {
            throw std::runtime_error(protocol::error_parse_int32);
        }

        return result;
    }

    int64_t entity_property::int64_value() const
    {
        if (m_property_type != edm_type::int64)
        {
            throw std::runtime_error(protocol::error_entity_property_not_int64);
        }

        int64_t result;
        utility::istringstream_t buffer(m_value);
        buffer >> result;
        return result;
    }

    utility::string_t entity_property::string_value() const
    {
        if (m_property_type != edm_type::string)
        {
            throw std::runtime_error(protocol::error_entity_property_not_string);
        }

        return m_value;
    }

    void entity_property::set_value_impl(double value)
    {
        if (core::is_nan(value))
        {
            m_value = protocol::double_not_a_number;
        }
        else if (value == std::numeric_limits<double>::infinity())
        {
            m_value = protocol::double_infinity;
        }
        else if (value == -std::numeric_limits<double>::infinity())
        {
            m_value = protocol::double_negative_infinity;
        }
        else
        {
            m_value = core::convert_to_string(value);
        }
    }

    void entity_property::set_value_impl(int32_t value)
    {
        m_value = core::convert_to_string(value);
    }

    void entity_property::set_value_impl(int64_t value)
    {
        m_value = core::convert_to_string(value);
    }

}} // namespace azure::storage
