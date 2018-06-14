// -----------------------------------------------------------------------------------------
// <copyright file="xmlhelpers.cpp" company="Microsoft">
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
* xmlhelpers.cpp
*
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#include "stdafx.h"
#include "wascore/xmlhelpers.h"

#ifdef _WIN32
#include "wascore/xmlstream.h"
#else
typedef int XmlNodeType;
#define XmlNodeType_Element xmlElementType::XML_ELEMENT_NODE
#define XmlNodeType_Text xmlElementType::XML_TEXT_NODE
#define XmlNodeType_EndElement xmlElementType::XML_ELEMENT_DECL
#define XmlNodeType_Whitespace xmlElementType::XML_DTD_NODE //? not align with tinyxml?
#endif

using namespace web;
using namespace utility;
using namespace concurrency;

namespace azure { namespace storage { namespace core { namespace xml {

    void xml_reader::initialize(streams::istream stream)
    {
#ifdef _WIN32
        HRESULT hr;
        CComPtr<IStream> pInputStream;
        pInputStream.Attach(xmlstring_istream::create(stream));

        if (pInputStream == nullptr)
        {
            auto error = ERROR_NOT_ENOUGH_MEMORY;
            log_error_message(_XPLATSTR("XML reader IStream creation failed"), error);
            throw utility::details::create_system_error(error);
        }

        if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**)&m_reader, NULL)))
        {

            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML reader CreateXmlReader failed"), error);
            throw utility::details::create_system_error(error);
        }

        if (FAILED(hr = m_reader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)))
        {

            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML reader SetProperty failed"), error);
            throw utility::details::create_system_error(error);
        }

        if (FAILED(hr = m_reader->SetInput(pInputStream)))
        {

            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML reader SetInput failed"), error);
            throw utility::details::create_system_error(error);
        }
#else
        concurrency::streams::stringstreambuf buffer;
        stream.read_to_end(buffer).get();
        m_data = buffer.collection();
        if (m_data.empty())
            m_reader.reset();
        else
            m_reader.reset(new xml_text_reader_wrapper(reinterpret_cast<const unsigned char*>(m_data.data()), static_cast<unsigned int>(m_data.size())));
#endif
    }

    xml_reader::parse_result xml_reader::parse()
    {
        if (m_streamDone) return xml_reader::parse_result::cannot_continue;
        // Set this to true each time the parse routine is invoked. Most derived readers will only invoke parse once.
        m_continueParsing = true;

        // read until there are no more nodes
#ifdef _WIN32
        HRESULT hr;
        XmlNodeType nodeType;
        while (m_continueParsing && S_OK == (hr = m_reader->Read(&nodeType)))
        {
#else
        if (m_reader == nullptr)
            return xml_reader::parse_result::cannot_continue; // no XML document to read

        while (m_continueParsing && m_reader->read())
        {
            auto nodeType = m_reader->get_node_type();
#endif
            switch (nodeType)
            {

            case XmlNodeType_Element:
            {
                auto name = get_current_element_name();
                m_elementStack.push_back(name);
                handle_begin_element(name);

#ifdef _WIN32
                if (m_reader->IsEmptyElement())
#else
                if (m_reader->is_empty_element())
#endif
                {
                    handle_end_element(name);
                    m_elementStack.pop_back();
                }
            }
                break;

            case XmlNodeType_Text:
            case XmlNodeType_Whitespace:
                if (m_elementStack.size()) {
                    handle_element(m_elementStack.back());
                }
                break;

            case XmlNodeType_EndElement:
                handle_end_element(m_elementStack.back());
                m_elementStack.pop_back();
                break;

            default:
                break;
            }
        }

        xml_reader::parse_result result = xml_reader::parse_result::can_continue;
        // If the loop was terminated because there was no more to read from the stream, set m_streamDone to true, so exit early
        // the next time parse is invoked.
        // if stream is not done, it means that the parsing is interuptted by pause().
        // if the element stack is not empty when the stream is done, it means that the xml is not complete.
        if (m_continueParsing)
        {
            m_streamDone = true;
            if (m_elementStack.empty())
            {
                result = xml_reader::parse_result::cannot_continue;
            }
            else
            {
                result = xml_reader::parse_result::xml_not_complete;
            }
        }
        return result;
    }

    utility::string_t xml_reader::get_parent_element_name(size_t pos)
    {
        if (m_elementStack.size() > pos + 1)
        {
            size_t currentDepth = m_elementStack.size() - 1;
            size_t parentDepth = currentDepth - 1;

            if (pos <= parentDepth)
            {
                return m_elementStack[parentDepth - pos];
            }
        }

        // return empty string
        return utility::string_t();
    }

    utility::string_t xml_reader::get_current_element_name()
    {
#ifdef _WIN32
        HRESULT hr;
        const wchar_t * pwszLocalName = NULL;

        if (FAILED(hr = m_reader->GetLocalName(&pwszLocalName, NULL)))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML reader GetLocalName failed"), error);
            throw utility::details::create_system_error(error);
        }
        return utility::string_t(pwszLocalName);
#else
        return utility::string_t(m_reader->get_local_name());
#endif
    }

    utility::string_t xml_reader::get_current_element_name_with_prefix()
    {
#ifdef _WIN32
        HRESULT hr;
        const wchar_t * pwszName = NULL;

        if (FAILED(hr = m_reader->GetQualifiedName(&pwszName, NULL)))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML reader GetQualified failed"), error);
            throw utility::details::create_system_error(error);
        }
        return utility::string_t(pwszName);
#else
        throw std::runtime_error("Not implemented");
#endif
    }

    utility::string_t xml_reader::get_current_element_text()
    {
#ifdef _WIN32
        HRESULT hr;
        const wchar_t * pwszValue;

        if (FAILED(hr = m_reader->GetValue(&pwszValue, NULL)))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML reader GetValue failed"), error);
            throw utility::details::create_system_error(error);
        }

        return utility::string_t(pwszValue);
#else
        return utility::string_t(m_reader->get_value());
#endif
    }

    bool xml_reader::move_to_first_attribute()
    {
#ifdef _WIN32
        HRESULT hr;
        if (FAILED(hr = m_reader->MoveToFirstAttribute()))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML reader MoveToFirstAttribute failed"), error);
            throw utility::details::create_system_error(error);
        }
        return (hr == S_OK);
#else
        return m_reader->move_to_first_attribute();
#endif
    }

    bool xml_reader::move_to_next_attribute()
    {
#ifdef _WIN32
        HRESULT hr;
        if (FAILED(hr = m_reader->MoveToNextAttribute()))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML reader MoveToNextAttribute failed"), error);
            throw utility::details::create_system_error(error);
        }
        return (hr == S_OK);
#else
        return m_reader->move_to_next_attribute();
#endif
    }

    void xml_writer::initialize(std::ostream& stream)
    {
#ifdef _WIN32
        HRESULT hr;
        CComPtr<IStream> pStream;
        pStream.Attach(xmlstring_ostream::create(stream));

        if (pStream == nullptr)
        {
            auto error = ERROR_NOT_ENOUGH_MEMORY;
            log_error_message(_XPLATSTR("XML writer IStream creation failed"), error);
            throw utility::details::create_system_error(error);
        }

        if (FAILED(hr = CreateXmlWriter(__uuidof(IXmlWriter), (void**)&m_writer, NULL)))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML writer CreateXmlWriter failed"), error);
            throw utility::details::create_system_error(error);
        }

        if (FAILED(hr = m_writer->SetOutput(pStream)))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML writer SetOutput failed"), error);
            throw utility::details::create_system_error(error);
        }

        if (FAILED(hr = m_writer->SetProperty(XmlWriterProperty_Indent, TRUE)))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML writer SetProperty failed"), error);
            throw utility::details::create_system_error(error);
        }

        if (FAILED(hr = m_writer->WriteStartDocument(XmlStandalone_Omit)))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML writer WriteStartDocument failed"), error);
            throw utility::details::create_system_error(error);
        }
#else // LINUX
        m_document.reset(new xml_document_wrapper());
        m_elementStack = std::stack<xml_element_wrapper*>();
        m_stream = &stream;
#endif
    }

    void xml_writer::finalize()
    {
#ifdef _WIN32
        HRESULT hr;

        if (FAILED(hr = m_writer->WriteEndDocument()))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML writer WriteEndDocument failed"), error);
            throw utility::details::create_system_error(error);
        }
        if (FAILED(hr = m_writer->Flush()))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML writer Flush failed"), error);
            throw utility::details::create_system_error(error);
        }
#else // LINUX
        auto result = m_document->write_to_string();
        *m_stream << reinterpret_cast<const char *>(result.c_str());

#endif
    }

    void xml_writer::write_start_element_with_prefix(const utility::string_t& elementPrefix, const utility::string_t& elementName, const utility::string_t& namespaceName)
    {
#ifdef _WIN32
        HRESULT hr;
        if (FAILED(hr = m_writer->WriteStartElement(elementPrefix.c_str(), elementName.c_str(), namespaceName.empty() ? NULL : namespaceName.c_str())))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML writer WriteStartElement with prefix failed"), error);
            throw utility::details::create_system_error(error);
        }
#else 
        if (m_elementStack.empty())
        {
            m_elementStack.push(m_document->create_root_node(elementName, namespaceName, elementPrefix));
        }
        else
        {
            m_elementStack.push(m_elementStack.top()->add_child(elementName, elementPrefix));
            if (!namespaceName.empty())
            {
                m_elementStack.top()->set_namespace_declaration(namespaceName, elementPrefix);
            }
        }
#endif
    }

    void xml_writer::write_start_element(const utility::string_t& elementName, const utility::string_t& namespaceName)
    {
#ifdef _WIN32
        HRESULT hr;

        if (FAILED(hr = m_writer->WriteStartElement(NULL, elementName.c_str(), namespaceName.empty() ? NULL : namespaceName.c_str())))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML writer WriteStartElement failed"), error);
            throw utility::details::create_system_error(error);
        }
#else 
        write_start_element_with_prefix(_XPLATSTR(""), elementName, namespaceName);
#endif
    }

    void xml_writer::write_end_element()
    {
#ifdef _WIN32
        HRESULT hr;

        if (FAILED(hr = m_writer->WriteEndElement()))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML writer WriteEndElement failed"), error);
            throw utility::details::create_system_error(error);
        }
#else 
        m_elementStack.pop();
#endif
    }

    void xml_writer::write_full_end_element()
    {
#ifdef _WIN32
        HRESULT hr;

        if (FAILED(hr = m_writer->WriteFullEndElement()))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML writer WriteFullEndElement failed"), error);
            throw utility::details::create_system_error(error);
        }
#else
        throw std::runtime_error("Not implemented");
#endif
    }

    void xml_writer::write_string(const utility::string_t& str)
    {
#ifdef _WIN32
        HRESULT hr;

        if (FAILED(hr = m_writer->WriteString(str.c_str())))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML writer WriteString failed"), error);
            throw utility::details::create_system_error(error);
        }
#else
        UNREFERENCED_PARAMETER(str);
        throw std::runtime_error("Not implemented");
#endif
    }


    void xml_writer::write_attribute_string(const utility::string_t& prefix, const utility::string_t& name, const utility::string_t& namespaceUri, const utility::string_t& value)
    {
#ifdef _WIN32
        HRESULT hr;

        if (FAILED(hr = m_writer->WriteAttributeString(prefix.empty() ? NULL : prefix.c_str(),
            name.empty() ? NULL : name.c_str(),
            namespaceUri.empty() ? NULL : namespaceUri.c_str(),
            value.empty() ? NULL : value.c_str())))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML writer WriteAttributeString failed"), error);
            throw utility::details::create_system_error(error);
        }
#else
        UNREFERENCED_PARAMETER(namespaceUri);
        if (prefix == _XPLATSTR("xmlns"))
        {
            m_elementStack.top()->set_namespace_declaration(
                value, name
                );
        }
        else
        {
            m_elementStack.top()->set_attribute(
                name,
                value,
                prefix);
        }
#endif
    }

    void xml_writer::write_element(const utility::string_t& elementName, const utility::string_t& value)
    {
#ifdef _WIN32
        HRESULT hr;

        if (FAILED(hr = m_writer->WriteElementString(NULL, elementName.c_str(), NULL, value.c_str())))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML writer WriteElementString failed"), error);
            throw utility::details::create_system_error(error);
        }
#else // LINUX
        write_element_with_prefix(_XPLATSTR(""), elementName, value);
#endif
    }

    void xml_writer::write_element_with_prefix(const utility::string_t& prefix, const utility::string_t& elementName, const utility::string_t& value)
    {
#ifdef _WIN32
        HRESULT hr;

        if (FAILED(hr = m_writer->WriteElementString(prefix.c_str(), elementName.c_str(), NULL, value.c_str())))
        {
            auto error = GetLastError();
            log_error_message(_XPLATSTR("XML writer WriteElementStringWithPrefix failed"), error);
            throw utility::details::create_system_error(error);
        }
#else
        write_start_element_with_prefix(prefix, elementName);
        m_elementStack.top()->set_child_text(value);
        write_end_element();
#endif
    }

}
        }
    }
} // namespace azure::storage::core::xml
