// -----------------------------------------------------------------------------------------
// <copyright file="test_helper.h" company="Microsoft">
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

#pragma once
#include "was/table.h"
#include "was/queue.h"

extern utility::string_t object_name_prefix;

utility::string_t get_string(utility::char_t value1, utility::char_t value2);
void initialize_random();
bool get_random_boolean();
int32_t get_random_int32();
int64_t get_random_int64();
double get_random_double();
utility::string_t get_random_string();
utility::datetime get_random_datetime();
std::vector<uint8_t> get_random_binary_data();
utility::uuid get_random_guid();
wa::storage::storage_credentials get_credentials();
wa::storage::cloud_table_client get_table_client();
utility::string_t get_table_name();
wa::storage::cloud_table get_table(bool create = true);
wa::storage::cloud_queue_client get_queue_client();
utility::string_t get_queue_name();
wa::storage::cloud_queue get_queue(bool create = true);
