// -----------------------------------------------------------------------------------------
// <copyright file="cloud_queue_message.cpp" company="Microsoft">
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
#include "was/queue.h"

namespace azure { namespace storage {

    const std::chrono::seconds max_time_to_live(7 * 24 * 60 * 60);

    void cloud_queue_message::update_message_info(const cloud_queue_message& message_metadata)
    {
        m_id = message_metadata.m_id;
        m_insertion_time = message_metadata.m_insertion_time;
        m_expiration_time = message_metadata.m_expiration_time;
        m_pop_receipt = message_metadata.m_pop_receipt;
        m_next_visible_time = message_metadata.m_next_visible_time;
    }

}} // namespace azure::storage
