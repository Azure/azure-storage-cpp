// -----------------------------------------------------------------------------------------
// <copyright file="table_query.cpp" company="Microsoft">
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

    const utility::string_t table_query::generate_filter_condition(const utility::string_t& property_name, const utility::string_t& comparison_operator, const utility::string_t& value)
    {
        utility::string_t modified_value = core::single_quote(value);
        return generate_filter_condition_impl(property_name, comparison_operator, modified_value);
    }

    const utility::string_t table_query::generate_filter_condition_impl(const utility::string_t& property_name, const utility::string_t& comparison_operator, const utility::string_t& value)
    {
        utility::string_t result;
        result.reserve(property_name.size() + comparison_operator.size() + value.size() + 2U);

        result.append(property_name);
        result.push_back(U(' '));
        result.append(comparison_operator);
        result.push_back(U(' '));
        result.append(value);

        return result;
    }

    const utility::string_t table_query::generate_filter_condition(const utility::string_t& property_name, const utility::string_t& comparison_operator, const std::vector<uint8_t>& value)
    {
        utility::string_t string_data_value = core::convert_to_string(value);

        utility::string_t string_value;
        string_value.reserve(string_data_value.size() + 3U);

        string_value.append(U("X'"));
        string_value.append(string_data_value);
        string_value.push_back(U('\''));

        return generate_filter_condition_impl(property_name, comparison_operator, string_value);
    }

    // TODO: Move this back to the .h file after switching to Casablanca's datetime parsing
    const utility::string_t table_query::generate_filter_condition(const utility::string_t& property_name, const utility::string_t& comparison_operator, const utility::datetime& value)
    {
        utility::string_t string_data_value = core::convert_to_string(value);

        utility::string_t string_value;
        string_value.reserve(string_data_value.size() + 10U);

        string_value.append(U("datetime'"));
        string_value.append(string_data_value);
        string_value.push_back(U('\''));

        return generate_filter_condition_impl(property_name, comparison_operator, string_value);
    }

}} // namespace wa::storage
