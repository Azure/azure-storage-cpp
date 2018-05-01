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
* xml_wrapper.h
*
* This file contains wrapper for libxml2
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#pragma once
#ifndef _XML_WRAPPER_H
#define _XML_WRAPPER_H

#ifndef _WIN32
#include <string>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

#include "wascore/basic_types.h"

namespace azure { namespace storage { namespace core { namespace xml {

    std::string xml_char_to_string(const xmlChar *xml_char);

    /// <summary>
    /// A class to wrap xmlTextReader of c library libxml2. This class provides abilities to read from xml format texts.
    /// </summary>
    class xml_text_reader_wrapper {
    public:
        xml_text_reader_wrapper(const unsigned char* buffer, unsigned int size);

        ~xml_text_reader_wrapper();

        /// <summary>
        /// Moves to the next node in the stream.
        /// </summary>
        /// <returns>true if the node was read successfully and false if there are no more nodes to read</returns>
        bool read();

        /// <summary>
        /// Gets the type of the current node.
        /// </summary>
        /// <returns>A integer that represent the type of the node</returns>
        unsigned get_node_type();

        /// <summary>
        /// Checks if current node is empty
        /// </summary>
        /// <returns>True if current node is empty and false if otherwise</returns>
        bool is_empty_element();

        /// <summary>
        /// Gets the local name of the node
        /// </summary>
        /// <returns>A string indicates the local name of this node</returns>
        std::string get_local_name();

        /// <summary>
        /// Gets the value of the node
        /// </summary>
        /// <returns>A string value of the node</returns>
        std::string get_value();

        /// <summary>
        /// Moves to the first attribute of the node.
        /// </summary>
        /// <returns>True if the move is successful, false if empty</returns>
        bool move_to_first_attribute();

        /// <summary>
        /// Moves to the next attribute of the node.
        /// </summary>
        /// <returns>True if the move is successful, false if empty</returns>
        bool move_to_next_attribute();

    private:
        xmlTextReaderPtr m_reader;
    };

    /// <summary>
    /// A class to wrap xmlNode of c library libxml2. This class provides abilities to create xml nodes.
    /// </summary>
    class xml_element_wrapper {
    public:
        xml_element_wrapper();

        ~xml_element_wrapper();

        xml_element_wrapper(xmlNode* node);

        /// <summary>
        /// Adds a child element to this node.
        /// </summary>
        /// <param name="name">The name of the child node.</param>
        /// <param name="prefix">The namespace prefix of the child node.</param>
        /// <returns>The created child node.</returns>
        xml_element_wrapper* add_child(const std::string& name, const std::string& prefix);

        /// <summary>
        /// Adds a namespace declaration to the node.
        /// </summary>
        /// <param name="uri">The namespace to associate with the prefix.</param>
        /// <param name="prefix">The namespace prefix</param>
        void set_namespace_declaration(const std::string& uri, const std::string& prefix);

        /// <summary>
        /// Set the namespace prefix
        /// </summary>
        /// <param name="prefix">name space prefix to be set</param>
        void set_namespace(const std::string& prefix);

        /// <summary>
        /// Sets the value of the attribute with this name (and prefix).
        /// </summary>
        /// <param name="name">The name of the attribute</param>
        /// <param name="value">The value of the attribute</param>
        /// <param name="prefix">The prefix of the attribute, this is optional.</param>
        void set_attribute(const std::string& name, const std::string& value, const std::string& prefix);

        /// <summary>
        /// Sets the text of the first text node. If there isn't a text node, add one and set it.
        /// </summary>
        /// <param name="text">The text to be set to the child node.</param>
        void set_child_text(const std::string& text);

        /// <summary>
        /// Frees the wrappers set in nod->_private
        /// </summary>
        /// <param name="node">The node to be freed.</param>
        /// <returns>A <see cref="pplx::task" /> object that represents the current operation.</returns>
        static void free_wrappers(xmlNode* node);

    private:
        xmlNode* m_ele;
    };

    /// <summary>
    /// A class to wrap xmlDoc of c library libxml2. This class provides abilities to create xml format texts from nodes.
    /// </summary>
    class xml_document_wrapper {
    public:
        xml_document_wrapper();

        ~xml_document_wrapper();

        /// <summary>
        /// Converts object to a string object.
        /// </summary>
        /// <returns>A std::string that contains the result</returns>
        std::string write_to_string();

        /// <summary>
        /// Creates the root node of the document.
        /// </summary>
        /// <param name="name">The name of the root node.</param>
        /// <param name="namespace_name">The namespace of the root node.</param>
        /// <param name="prefix">The namespace prefix of the root node.</param>
        /// <returns>A wrapper that contains the root node.</returns>
        xml_element_wrapper* create_root_node(const std::string& name, const std::string& namespace_name, const std::string& prefix);

        /// <summary>
        /// Gets the root node of the document.
        /// </summary>
        /// <returns>The root node of the document.</returns>
        xml_element_wrapper* get_root_node() const;

    private:
        xmlDocPtr m_doc;
    };
}}}};// namespace azure::storage::core::xml
#endif //#ifdef _WIN32

#endif //#ifndef _XML_WRAPPER_H
