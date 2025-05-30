/**
 * @file document.cpp
 * @brief Implementation of the canopy::io::xml::document class for XML document parsing and validation.
 */

#include "io/xml/document.h"
#include <filesystem>
#include <libxml/xinclude.h>

namespace io::xml {

/**
 * @brief Constructs and parses an XML document from a file, with optional validation.
 *
 * This constructor loads and parses the XML file at the given path, resolving XInclude directives.
 * It also performs optional validation if a validator is provided.
 *
 * @param file_path Path to the XML file to parse.
 * @param validator Optional pointer to a validator for schema or DTD validation.
 *
 * @throws error Throws an error of type @ref error_type::io if the file cannot be read,
 *         @ref error_type::parse if the XML is malformed,
 *         or @ref error_type::x_include if XInclude processing fails.
 *
 * @note Uses libxml2's xmlReadFile and xmlXIncludeProcessFlags for parsing and XInclude resolution.
 * @note The document is managed by a unique_ptr with a custom deleter for safe resource management.
 *
 * @code
 * canopy::io::xml::document doc("file.xml");
 * @endcode
 */
document::document(const std::string &file_path, const validator *validator) {
    xmlResetLastError();
    doc_.reset(xmlReadFile(file_path.c_str(), nullptr, parser_options_));

    // Check for parsing errors and throw appropriate exceptions
    if (xmlErrorPtr xml_error = xmlGetLastError()) {
        if (xml_error->domain == XML_FROM_IO) {
            const std::string msg = std::string(xml_error->message) + file_path + std::to_string(errno);
            throw error(error_type::io, xml_error, msg);
        }
        throw error(error_type::parse, xml_error);
    }

    assert(doc_ && "Internal XML library failure.");

    // Process XInclude directives and check for errors
    if (xmlXIncludeProcessFlags(get(), parser_options_) < 0 || xmlGetLastError()) {
        throw error(error_type::x_include);
    }

    // Optionally validate the document if a validator is provided
    if (validator) {
        validator->validate(*this);
    }
}

/**
 * @brief Returns the root element of the XML document.
 *
 * Provides a lightweight wrapper around the root node, allowing further traversal or querying.
 *
 * @return The root element of the document.
 *
 * @code
 * auto root = doc.root();
 * std::cout << root.name() << std::endl;
 * @endcode
 */
element document::root() const noexcept {
    return element(reinterpret_cast<const xmlElement *>(xmlDocGetRootElement(doc_.get())));
}

} // namespace canopy::io::xml