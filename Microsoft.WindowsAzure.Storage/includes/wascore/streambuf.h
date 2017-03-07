// -----------------------------------------------------------------------------------------
// <copyright file="streambuf.h" company="Microsoft">
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

#include "cpprest/streams.h"

#include "wascore/basic_types.h"
#include "hashing.h"

namespace azure { namespace storage { namespace core {

    template<typename _CharType>
    class basic_istreambuf : public concurrency::streams::details::streambuf_state_manager<_CharType>
    {
    public:
        typedef typename concurrency::streams::details::streambuf_state_manager<_CharType>::traits traits;
        typedef typename concurrency::streams::details::streambuf_state_manager<_CharType>::int_type int_type;
        typedef typename concurrency::streams::details::streambuf_state_manager<_CharType>::pos_type pos_type;
        typedef typename concurrency::streams::details::streambuf_state_manager<_CharType>::off_type off_type;

        basic_istreambuf()
            : concurrency::streams::details::streambuf_state_manager<_CharType>(std::ios_base::in)
        {
        }

        pplx::task<int_type> _putc(_CharType ch)
        {
            UNREFERENCED_PARAMETER(ch);
            return pplx::task_from_result(traits::eof());
        }

        pplx::task<size_t> _putn(const _CharType *ptr, size_t count)
        {
            UNREFERENCED_PARAMETER(ptr);
            UNREFERENCED_PARAMETER(count);
            return pplx::task_from_result((size_t)0);
        }

        pplx::task<bool> _sync()
        {
            return pplx::task_from_result(false);
        }

        _CharType* _alloc(_In_ size_t count)
        {
            UNREFERENCED_PARAMETER(count);
            return nullptr;
        }

        void _commit(_In_ size_t count)
        {
            UNREFERENCED_PARAMETER(count);
        }
    };

    template<typename _CharType>
    class basic_ostreambuf : public concurrency::streams::details::streambuf_state_manager<_CharType>
    {
    public:
        typedef typename concurrency::streams::details::streambuf_state_manager<_CharType>::traits traits;
        typedef typename concurrency::streams::details::streambuf_state_manager<_CharType>::int_type int_type;
        typedef typename concurrency::streams::details::streambuf_state_manager<_CharType>::pos_type pos_type;
        typedef typename concurrency::streams::details::streambuf_state_manager<_CharType>::off_type off_type;

        basic_ostreambuf()
            : concurrency::streams::details::streambuf_state_manager<_CharType>(std::ios_base::out)
        {
        }

        size_t in_avail() const
        {
            return 0;
        }

        pplx::task<int_type> _bumpc()
        {
            return pplx::task_from_result(traits::eof());
        }

        int_type _sbumpc()
        {
            return traits::eof();
        }

        pplx::task<int_type> _getc()
        {
            return pplx::task_from_result(traits::eof());
        }

        int_type _sgetc()
        {
            return traits::eof();
        }

        pplx::task<int_type> _nextc()
        {
            return pplx::task_from_result(traits::eof());
        }

        pplx::task<int_type> _ungetc()
        {
            return pplx::task_from_result(traits::eof());
        }

        pplx::task<size_t> _getn(_Out_writes_(count) _CharType *ptr, _In_ size_t count)
        {
            UNREFERENCED_PARAMETER(ptr);
            UNREFERENCED_PARAMETER(count);
            return pplx::task_from_result((size_t)0);
        }

        size_t _scopy(_Out_writes_(count) _CharType *ptr, _In_ size_t count)
        {
            UNREFERENCED_PARAMETER(ptr);
            UNREFERENCED_PARAMETER(count);
            return 0;
        }

        bool acquire(_Out_ _CharType*& ptr, _Out_ size_t& count)
        {
            UNREFERENCED_PARAMETER(ptr);
            UNREFERENCED_PARAMETER(count);
            return false;
        }

        void release(_Out_writes_(count) _CharType* ptr, _In_ size_t count)
        {
            UNREFERENCED_PARAMETER(ptr);
            UNREFERENCED_PARAMETER(count);
        }
    };

    template<typename _CharType>
    class basic_hash_wrapper_streambuf : public basic_ostreambuf<_CharType>
    {
    public:
        typedef _CharType char_type;
        typedef typename basic_ostreambuf<_CharType>::traits traits;
        typedef typename basic_ostreambuf<_CharType>::int_type int_type;
        typedef typename basic_ostreambuf<_CharType>::pos_type pos_type;
        typedef typename basic_ostreambuf<_CharType>::off_type off_type;

        basic_hash_wrapper_streambuf(concurrency::streams::streambuf<_CharType> inner_streambuf, hash_provider provider)
            : basic_ostreambuf<_CharType>(), m_inner_streambuf(inner_streambuf), m_hash_provider(provider), m_total_written(0)
        {
        }

        bool can_seek() const
        {
            return false;
        }

        bool has_size() const
        {
            return false;
        }

        utility::size64_t size() const
        {
            return 0;
        }

        size_t buffer_size(std::ios_base::openmode direction) const
        {
            UNREFERENCED_PARAMETER(direction);
            return (size_t)0;
        }

        void set_buffer_size(size_t size, std::ios_base::openmode direction)
        {
            UNREFERENCED_PARAMETER(size);
            UNREFERENCED_PARAMETER(direction);
        }

        pos_type getpos(std::ios_base::openmode direction) const
        {
            UNREFERENCED_PARAMETER(direction);
            return (pos_type)traits::eof();
        }

        pos_type seekoff(off_type offset, std::ios_base::seekdir way, std::ios_base::openmode direction)
        {
            UNREFERENCED_PARAMETER(offset);
            UNREFERENCED_PARAMETER(way);
            UNREFERENCED_PARAMETER(direction);
            return (pos_type)traits::eof();
        }

        pos_type seekpos(pos_type pos, std::ios_base::openmode direction)
        {
            UNREFERENCED_PARAMETER(pos);
            UNREFERENCED_PARAMETER(direction);
            return (pos_type)traits::eof();
        }

        char_type* _alloc(_In_ size_t count)
        {
            UNREFERENCED_PARAMETER(count);
            return nullptr;
        }

        void _commit(_In_ size_t count)
        {
            UNREFERENCED_PARAMETER(count);
            // no-op, as hash streams do not support alloc/commit
        }

        pplx::task<bool> _sync()
        {
            return m_inner_streambuf.sync().then([]() -> bool
            {
                return true;
            });
        }

        pplx::task<void> _close_write()
        {
            m_hash_provider.close();
            return m_inner_streambuf.close(std::ios_base::out);
        }

        pplx::task<int_type> _putc(char_type ch)
        {
            return m_inner_streambuf.putc(ch).then([this, ch](int_type ch_written) -> int_type 
            {
                ++m_total_written;
                m_hash_provider.write(&ch, 1);
                return ch_written;
            });
        }

        pplx::task<size_t> _putn(const char_type* ptr, size_t count)
        {
            return m_inner_streambuf.putn_nocopy(ptr, count).then([this, ptr](size_t count) -> size_t
            {
                m_total_written += count;
                m_hash_provider.write(ptr, count);
                return count;
            });
        }

        utility::string_t hash() const
        {
            return m_hash_provider.hash();
        }

        utility::size64_t total_written() const
        {
            return m_total_written;
        }

    private:

        concurrency::streams::streambuf<_CharType> m_inner_streambuf;
        hash_provider m_hash_provider;
        utility::size64_t m_total_written;
    };

}}} // namespace azure::storage::core
