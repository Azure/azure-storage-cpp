// -----------------------------------------------------------------------------------------
// <copyright file="xmlhelpers.h" company="Microsoft">
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
* xmlhelpers.h
*
* This file contains xml parsing helper routines
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#pragma once
#ifndef _XMLHELPERS_H
#define _XMLHELPERS_H
#ifdef _WIN32
#include <atlbase.h>
#include <xmllite.h>
#else
#include "wascore/xml_wrapper.h"
#include <stack>
#endif 

#include <string>
#include "cpprest/details/basic_types.h"
#include "was/core.h"

namespace azure { namespace storage { namespace core { namespace xml {

/// <summary>
/// XML reader based on xmlllite
/// </summary>
class xml_reader
{
public:

    /// <summary>
    /// An enumeration describing result of the parse() operation.
    /// </summary>
    enum parse_result
    {
        /// <summary>
        /// Parsed is finished and cannot be continued.
        /// </summary>
        cannot_continue = 0,

        /// <summary>
        /// Parse is paused and can be continued.
        /// </summary>
        can_continue = 1,

        /// <summary>
        /// Exited because XML is not complete.
        /// </summary>
        xml_not_complete = 2,
    };

    virtual ~xml_reader() {}

    /// <summary>
    /// Parse the given xml string/stream. Return value indicates if
    /// the parsing was successful.
    /// </summary>
    parse_result parse();

protected:

    xml_reader() : m_continueParsing(true), m_streamDone(false)
    {
    }

    xml_reader(concurrency::streams::istream stream) : m_continueParsing(true), m_streamDone(false)
    {
        initialize(stream);
    }

    /// <summary>
    /// Callback for handling the start of an element.
    /// </summary>
    virtual void handle_begin_element(const utility::string_t&)
    {
    }

    /// <summary>
    /// Callback for handling the element text.
    /// </summary>
    virtual void handle_element(const utility::string_t& )
    {
    }

    /// <summary>
    /// Callback for handling the end of an element.
    /// </summary>
    virtual void handle_end_element(const utility::string_t& )
    {
    }

    /// <summary>
    /// Logs an error from processing XML
    /// </summary>
    virtual void log_error_message(const utility::string_t& message, unsigned long error = 0)
    {
        UNREFERENCED_PARAMETER(message);
        UNREFERENCED_PARAMETER(error);
    }

    /// <summary>
    /// Returns the parent element name
    /// </summary>
    utility::string_t get_parent_element_name(size_t pos = 0);

    /// <summary>
    /// Returns the current element name
    /// </summary>
    utility::string_t get_current_element_name();

    /// <summary>
    /// Returns the current element name with the prefix if any. 
    /// </summary>
    utility::string_t get_current_element_name_with_prefix();

    /// <summary>
    /// Returns the current element value
    /// </summary>
    utility::string_t get_current_element_text();

    /// <summary>
    /// Moves to the first attribute in the node
    /// </summary>
    bool move_to_first_attribute();

    /// <summary>
    /// Moves to the next attribute in the node
    /// </summary>
    bool move_to_next_attribute();

    /// <summary>
    /// Extracts the current element value into the provided type
    /// </summary>
    template <class T>
    void extract_current_element(T& value)
    {
        utility::istringstream_t iss(get_current_element_text());
        iss >> value;
    }

    /// <summary>
    /// Initialize the reader
    /// </summary>
    void initialize(concurrency::streams::istream stream);

    /// <summary>
    /// Can be called by the derived classes in the handle_* routines, to cause the parse routine to exit early,
    /// in order to capture records as they are parsed. Parsing is resumed by invoking the parse method again.
    /// </summary>
    void pause() { m_continueParsing = false; }

#ifdef _WIN32
    CComPtr<IXmlReader> m_reader;
#else
    std::shared_ptr<xml_text_reader_wrapper> m_reader;
    std::string m_data;
#endif 

    std::vector<utility::string_t> m_elementStack;
    bool m_continueParsing;
    bool m_streamDone;
};

/// <summary>
/// XML writer based on xmlllite
/// </summary>
class xml_writer
{
public:

    virtual ~xml_writer() {}

protected:
    xml_writer()
    {
    }

    /// <summary>
    /// Initialize the writer
    /// </summary>
    void initialize(std::ostream& stream);

    /// <summary>
    /// Finalize the writer
    /// </summary>
    void finalize();

    /// <summary>
    /// Write the start element tag
    /// </summary>
    void write_start_element(const utility::string_t& elementName, const utility::string_t& namespaceName = _XPLATSTR(""));

    /// <summary>
    /// Writes the start element tag with a prefix
    /// </summary>
    void write_start_element_with_prefix(const utility::string_t& elementPrefix, const utility::string_t& elementName,
                                         const utility::string_t& namespaceName = _XPLATSTR(""));

    /// <summary>
    /// Write the end element tag for the current element
    /// </summary>
    void write_end_element();

    /// <summary>
    /// Write the full end element tag for the current element
    /// </summary>
    void write_full_end_element();

    /// <summary>
    /// Write an element including the name and text.
    /// </summary>
    template<class T>
    void write_element(const utility::string_t& elementName, T value)
    {
        write_element(elementName, convert_to_string(value));
    }

    /// <summary>
    /// Write an element including the name and text.
    /// </summary>
    void write_element(const utility::string_t& elementName, const utility::string_t& value);

    /// <summary>
    /// Write an element including the prefix, name and text.
    /// </summary>
    void write_element_with_prefix(const utility::string_t& prefix, const utility::string_t& elementName, const utility::string_t& value);

    /// <summary>
    /// Write raw data
    /// </summary>
    void write_raw(const utility::string_t& data);

    /// <summary>
    /// Write a string
    /// </summary>
    void write_string(const utility::string_t& string);

    /// <summary>
    /// Write an attribute string with a prefix
    /// </summary>
    void write_attribute_string(const utility::string_t& prefix, const utility::string_t& name,
                                const utility::string_t& namespaceUri, const utility::string_t& value);


    /// <summary>
    /// Logs an error from processing XML
    /// </summary>
    virtual void log_error_message(const utility::string_t& message, unsigned long error = 0)
    {
        UNREFERENCED_PARAMETER(message);
        UNREFERENCED_PARAMETER(error);
    }
private:
#ifdef _WIN32
    CComPtr<IXmlWriter> m_writer;
#else // LINUX
    std::shared_ptr<xml_document_wrapper> m_document;
    std::stack<xml_element_wrapper*> m_elementStack;
    std::ostream * m_stream;
#endif
};

}}}} // namespace azure::storage::core::xml

#endif
