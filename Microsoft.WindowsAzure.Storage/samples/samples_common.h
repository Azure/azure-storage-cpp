// -----------------------------------------------------------------------------------------
// <copyright file="samples_common.h" company="Microsoft">
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

#include "was/common.h"


namespace azure { namespace storage { namespace samples {

    // TODO: Put your account name and account key here
    const utility::string_t storage_connection_string(_XPLATSTR("DefaultEndpointsProtocol=https;AccountName=myaccountname;AccountKey=myaccountkey"));

}}}  // namespace azure::storage::samples


class Sample
{
public:
    static const std::map<std::string, std::function<void()>>& samples()
    {
        return m_samples();
    }

protected:
    static void add_sample(std::string sample_name, std::function<void()> func)
    {
        m_samples().emplace(std::move(sample_name), std::move(func));
    }

private:
    static std::map<std::string, std::function<void()>>& m_samples()
    {
        static std::map<std::string, std::function<void()>> samples_instance;
        return samples_instance;
    }
};

#define SAMPLE(NAME, FUNCTION)                                                  \
void FUNCTION();                                                                \
                                                                                \
class Sample ## NAME : public Sample                                            \
{                                                                               \
public:                                                                         \
    Sample ## NAME()                                                            \
    {                                                                           \
        add_sample(#NAME, FUNCTION);                                            \
    }                                                                           \
};                                                                              \
namespace {                                                                     \
    Sample ## NAME Sample ## NAME_;                                             \
}
