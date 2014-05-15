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
#include "resources.h"

namespace azure { namespace storage { namespace core {

    template<typename _CharType>
    class basic_istreambuf : public concurrency::streams::details::streambuf_state_manager<_CharType>
    {
    public:

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
    class basic_splitter_streambuf : public basic_ostreambuf<_CharType>
    {
    public:

        basic_splitter_streambuf(concurrency::streams::streambuf<_CharType> streambuf1, concurrency::streams::streambuf<_CharType> streambuf2)
            : basic_ostreambuf<_CharType>(), m_streambuf1(streambuf1), m_streambuf2(streambuf2), m_total_written(0)
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
            auto streambuf2 = m_streambuf2;
            return m_streambuf1.sync().then([streambuf2] () mutable -> pplx::task<bool>
            {
                return streambuf2.sync().then([] () -> bool
                {
                    return true;
                });
            });
        }

        pplx::task<void> _close_write()
        {
            basic_ostreambuf<_CharType>::_close_write();
            auto streambuf2 = m_streambuf2;
            return m_streambuf1.close(std::ios_base::out).then([streambuf2] () mutable -> pplx::task<void>
            {
                return streambuf2.close(std::ios_base::out);
            });
        }

        pplx::task<int_type> _putc(char_type ch)
        {
            m_total_written++;
            auto streambuf2 = m_streambuf2;
            return m_streambuf1.putc(ch).then([streambuf2, ch] (int_type result1) mutable -> pplx::task<int_type>
            {
                return streambuf2.putc(ch).then([result1] (int_type result2) -> int_type
                {
                    if (result1 != result2)
                    {
                        throw;
                    }

                    return result1;
                });
            });
        }

        pplx::task<size_t> _putn(const char_type* ptr, size_t count)
        {
            m_total_written += count;
            auto streambuf2 = m_streambuf2;
            return m_streambuf1.putn(ptr, count).then([streambuf2, ptr, count] (size_t result1) mutable -> pplx::task<size_t>
            {
                return streambuf2.putn(ptr, count).then([result1] (size_t result2) -> size_t
                {
                    if (result1 != result2)
                    {
                        throw;
                    }

                    return result1;
                });
            });
        }

        utility::size64_t total_written() const
        {
            return m_total_written;
        }

    private:

        concurrency::streams::streambuf<_CharType> m_streambuf1;
        concurrency::streams::streambuf<_CharType> m_streambuf2;
        utility::size64_t m_total_written;
    };

    template<typename _CharType>
    class basic_null_streambuf : public basic_ostreambuf<_CharType>
    {
    public:

        basic_null_streambuf()
            : basic_ostreambuf<_CharType>()
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
            return (utility::size64_t)0;
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
            return pplx::task_from_result<bool>(true);
        }

        pplx::task<void> _close_write()
        {
            return basic_ostreambuf<_CharType>::_close_write();
        }

        pplx::task<int_type> _putc(char_type ch)
        {
            return pplx::task_from_result<int_type>((int_type)ch);
        }

        pplx::task<size_t> _putn(const char_type* ptr, size_t count)
        {
            UNREFERENCED_PARAMETER(ptr);
            return pplx::task_from_result<size_t>(count);
        }
    };

    class basic_hash_streambuf : public basic_ostreambuf<concurrency::streams::ostream::traits::char_type>
    {
    public:

        basic_hash_streambuf()
            : basic_ostreambuf<concurrency::streams::ostream::traits::char_type>()
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
            return (utility::size64_t)0;
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
            return pplx::task_from_result<bool>(true);
        }

        const std::vector<unsigned char>& hash() const
        {
            if (is_open())
            {
                throw std::logic_error(protocol::error_hash_on_closed_streambuf);
            }

            return m_hash;
        }

    protected:

        void store_and_throw(std::exception_ptr e)
        {
            m_currentException = e;
            std::rethrow_exception(e);
        }
        
        std::vector<unsigned char> m_hash;
    };

}}} // namespace azure::storage::core
