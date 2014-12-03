// -----------------------------------------------------------------------------------------
// <copyright file="streams.h" company="Microsoft">
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

#include "wascore/basic_types.h"
#include "streambuf.h"

namespace azure { namespace storage { namespace core {

    template<typename _CharType>
    class hash_wrapper_streambuf : public concurrency::streams::streambuf<_CharType>
    {
    public:
        hash_wrapper_streambuf()
            : concurrency::streams::streambuf<_CharType>()
        {
        }

        hash_wrapper_streambuf(concurrency::streams::streambuf<_CharType> inner_streambuf, hash_provider provider)
            : concurrency::streams::streambuf<_CharType>(std::make_shared<basic_hash_wrapper_streambuf<_CharType>>(inner_streambuf, provider))
        {
        }

        utility::size64_t total_written() const
        {
            const basic_hash_wrapper_streambuf<_CharType>* base = static_cast<basic_hash_wrapper_streambuf<_CharType>*>(Concurrency::streams::streambuf<_CharType>::get_base().get());
            return base->total_written();
        }

        utility::string_t hash() const
        {
            const basic_hash_wrapper_streambuf<_CharType>* base = static_cast<basic_hash_wrapper_streambuf<_CharType>*>(Concurrency::streams::streambuf<_CharType>::get_base().get());
            return base->hash();
        }
    };

}}} // namespace azure::storage::core
