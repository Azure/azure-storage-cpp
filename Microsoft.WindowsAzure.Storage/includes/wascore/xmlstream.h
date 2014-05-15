// -----------------------------------------------------------------------------------------
// <copyright file="xmlstream.h" company="Microsoft">
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

/***
* ==++==
*
* Copyright (c) Microsoft Corporation. All rights reserved. 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* ==--==
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* xmlstream.h
*
* This file contains xml stream implementation
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#pragma once

#include <string>
#include <strstream>
#include <intrin.h>
#include "cpprest/rawptrstream.h"
#include "cpprest/streams.h"

namespace azure { namespace storage { namespace core { namespace xml {

/// <summary>
/// A base implementation for IStream that returns E_NOTIMPL for all the methods
/// </summary>
class xmlstring_stream : public IStream
{
protected:
    xmlstring_stream()
        : m_refCount(1)
    {
    }

    virtual ~xmlstring_stream()
    {
    }

public:

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject)
    { 
        if (iid == __uuidof(IUnknown)
            || iid == __uuidof(IStream)
            || iid == __uuidof(ISequentialStream))
        {
            *ppvObject = static_cast<IStream*>(this);
            AddRef();
            return S_OK;
        } else
            return E_NOINTERFACE; 
    }

    virtual ULONG STDMETHODCALLTYPE AddRef(void) 
    { 
        return (ULONG)_InterlockedIncrement(&m_refCount); 
    }

    virtual ULONG STDMETHODCALLTYPE Release(void) 
    {
        ULONG res = (ULONG) _InterlockedDecrement(&m_refCount);
        if (res == 0) 
        {
            delete this;
        }
        return res;
    }

    // ISequentialStream Interface
public:

    virtual HRESULT STDMETHODCALLTYPE Read(void* , ULONG , ULONG* )
    {
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE Write(void const* , ULONG , ULONG* )
    {
        return E_NOTIMPL;
    }

    // IStream Interface
public:
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER)
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*,
        ULARGE_INTEGER*) 
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD)                                      
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE Revert(void)                                       
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)              
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)            
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE Clone(IStream **)                                  
    { 
        return E_NOTIMPL;   
    }

    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER , DWORD , ULARGE_INTEGER* )
    { 
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG* , DWORD ) 
    {
        return E_NOTIMPL;  
    }

private:

    volatile long m_refCount;
};

/// <summary>
/// An IStream implementation for reading xmlstrings
/// </summary>
class xmlstring_istream : public xmlstring_stream
{
public:

    static xmlstring_istream* create(concurrency::streams::istream stream)
    {
        return new xmlstring_istream(stream);
    }

    virtual HRESULT STDMETHODCALLTYPE Read(_Out_writes_ (cb) void* pv, _In_ ULONG cb, ULONG* pcbRead )
    {
        if (cb > 0)
        {
            // Synchronous for now.
            concurrency::streams::rawptr_buffer<uint8_t> buf((uint8_t *)pv, static_cast<std::streamsize>(cb));
            *pcbRead = (ULONG)m_stream.read(buf, static_cast<size_t>(cb)).get();
            return S_OK;
        }

        *pcbRead = cb;
        return S_OK;
    }

protected:

    xmlstring_istream(concurrency::streams::istream stream)
        : m_stream(stream)
    {
    }

    concurrency::streams::istream m_stream;
};

/// <summary>
/// An IStream implementation for writing xml strings
/// </summary>
class xmlstring_ostream : public xmlstring_stream
{
public:

    static xmlstring_ostream* create(std::ostream& stream)
    {
        return new xmlstring_ostream(stream);
    }

    virtual HRESULT STDMETHODCALLTYPE Write(void const* pv, ULONG cb, ULONG* pcbWritten)
    {
        // This method gets called when the XmlWriter is destroyed with cb == 0. We need this
        // check to ensure that we do not access the underlying stream after finalize is called.
        if (cb > 0)
        {
            const char * buf = (const char *) pv;
            std::string s(buf, cb);
            m_stream << s;
        }
        
        *pcbWritten = cb;
        return S_OK;
    }

protected:

    xmlstring_ostream(std::ostream& stream)
        : m_stream(stream)
    {
    }

    std::ostream& m_stream;
};

}}}} // namespace azure::storage::core::xml
