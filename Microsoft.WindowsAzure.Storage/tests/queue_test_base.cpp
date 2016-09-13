// -----------------------------------------------------------------------------------------
// <copyright file="queue_test_base.cpp" company="Microsoft">
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
#include "queue_test_base.h"
#include "check_macros.h"
#include "wascore/streams.h"

utility::string_t queue_service_test_base::queue_type_name = utility::string_t(_XPLATSTR("queue"));

azure::storage::cloud_queue_client queue_service_test_base::get_queue_client()
{
    return test_config::instance().account().create_cloud_queue_client();
}

utility::string_t queue_service_test_base::get_queue_name()
{
    return get_object_name(queue_type_name);
}

azure::storage::cloud_queue queue_service_test_base::get_queue(bool create)
{
    azure::storage::cloud_queue_client client = get_queue_client();
    utility::string_t queue_name = get_queue_name();
    azure::storage::cloud_queue queue = client.get_queue_reference(queue_name);
    if (create)
    {
        queue.create_if_not_exists();
    }
    return queue;
}
