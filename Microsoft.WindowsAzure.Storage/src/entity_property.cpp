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

namespace wa { namespace storage {

    // TODO: Move this back to the .h file after switching to Casablanca's datetime parsing
    const utility::datetime entity_property::datetime_value() const
    {
        if (m_property_type != edm_type::datetime)
        {
            throw std::runtime_error("The type of the entity property is not date/time.");
        }

        utility::datetime result = core::parse_datetime(m_value);
        if (!result.is_initialized())
        {
            throw std::runtime_error("An error occurred parsing the date/time.");
        }

        return result;
    }

    // TODO: Move this back to the .h file after switching to Casablanca's datetime parsing
    void entity_property::set_value_impl(const utility::datetime& value)
    {
        m_value = core::convert_to_string(value);
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
            utility::ostringstream_t buffer;
            // Two extra digits of precision are needed to ensure proper rounding
            buffer.precision(std::numeric_limits<double>::digits10 + 2);
            buffer << value;
            m_value = buffer.str();
        }
    }

}} // namespace wa::storage
