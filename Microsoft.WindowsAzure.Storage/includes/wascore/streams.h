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

#ifdef WIN32
#include "hash_windows.h"
#else
#include "hash_linux.h"
#endif

namespace wa { namespace storage { namespace core {

    template<typename _CharType>
    class splitter_streambuf : public concurrency::streams::streambuf<_CharType>
    {
    public:
        splitter_streambuf()
            : concurrency::streams::streambuf<_CharType>()
        {
        }

        splitter_streambuf(concurrency::streams::streambuf<_CharType> streambuf1, concurrency::streams::streambuf<_CharType> streambuf2)
            : concurrency::streams::streambuf<_CharType>(std::make_shared<basic_splitter_streambuf<_CharType>>(streambuf1, streambuf2))
        {
        }

        utility::size64_t total_written() const
        {
            auto base = static_cast<basic_splitter_streambuf<_CharType>*>(get_base().get());
            return base->total_written();
        }
    };

    template<typename _CharType>
    class null_streambuf : public concurrency::streams::streambuf<_CharType>
    {
    public:
        null_streambuf()
            : concurrency::streams::streambuf<_CharType>(std::make_shared<basic_null_streambuf<_CharType>>())
        {
        }
    };

    class hash_streambuf : public concurrency::streams::streambuf<basic_hash_streambuf::char_type>
    {
    public:
        hash_streambuf()
            : concurrency::streams::streambuf<basic_hash_streambuf::char_type>()
        {
        }

        hash_streambuf(_In_ const std::shared_ptr<basic_hash_streambuf> &ptr)
            : concurrency::streams::streambuf<basic_hash_streambuf::char_type>(ptr)
        {
        }

        const std::vector<unsigned char>& hash() const
        {
            auto base = static_cast<basic_hash_streambuf*>(get_base().get());
            return base->hash();
        }
    };

    class hash_hmac_sha256_streambuf : public hash_streambuf
    {
    public:
        hash_hmac_sha256_streambuf(std::vector<unsigned char> key)
            : hash_streambuf(std::make_shared<basic_hash_hmac_sha256_streambuf>(std::move(key)))
        {
        }
    };

    class hash_md5_streambuf : public hash_streambuf
    {
    public:
        hash_md5_streambuf()
            : hash_streambuf(std::make_shared<basic_hash_md5_streambuf>())
        {
        }
    };

}}} // namespace wa::storage::core
