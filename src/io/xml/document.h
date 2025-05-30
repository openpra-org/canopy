#pragma once

/**
 * @file document.h
 * @brief Defines the canopy::io::xml::document class for XML document parsing and validation.
 *
 * This file provides the interface for the document class, which encapsulates XML parsing,
 * XInclude resolution, and optional validation using libxml2.
 */

#include <memory>
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "io/xml/validator.h"
#include "io/xml/element.h"

namespace canopy::io::xml {

class validator;

/**
 * @class document
 * @brief Represents an XML document, providing parsing, validation, and access to the root element.
 *
 * This class encapsulates the parsing of XML files using libxml2, with optional validation and XInclude resolution.
 * It manages the lifetime of the underlying `xmlDoc` pointer and provides safe access to the document's root element.
 *
 * Typical usage:
 * @code
 * #include "io/xml/document.h"
 *
 * canopy::io::xml::document doc("example.xml");
 * auto root_elem = doc.root();
 * std::cout << root_elem.name() << std::endl;
 * @endcode
 */
class document {
public:
    /**
     * @brief Constructs and parses an XML document from a file, optionally validating it.
     *
     * This constructor loads and parses the XML file at the given path, resolving XInclude directives.
     * If a validator is provided, the document is validated accordingly.
     *
     * @param file_path Path to the XML file to parse.
     * @param validator Optional pointer to a validator for schema or DTD validation.
     *
     * @throws std::runtime_error if parsing or validation fails.
     *
     * @code
     * canopy::io::xml::validator v("schema.xsd");
     * canopy::io::xml::document doc("file.xml", &v);
     * @endcode
     */
    explicit document(const std::string& file_path, const validator* validator = nullptr);

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
    [[nodiscard]] element root() const noexcept;

    /**
     * @brief Provides const low-level access to the underlying libxml2 document pointer.
     *
     * Intended for advanced use cases such as custom validation or direct libxml2 manipulation.
     *
     * @return Const pointer to the underlying xmlDoc.
     *
     * @warning Use with care; direct manipulation may invalidate the document state.
     */
    [[nodiscard]] const xmlDoc* get() const noexcept { return doc_.get(); }

    /**
     * @brief Provides mutable low-level access to the underlying libxml2 document pointer.
     *
     * Intended for advanced use cases such as custom validation or direct libxml2 manipulation.
     *
     * @return Mutable pointer to the underlying xmlDoc.
     *
     * @warning Use with care; direct manipulation may invalidate the document state.
     */
    [[nodiscard]] xmlDoc* get() noexcept { return doc_.get(); }

    /**
     * @brief Default parser options for libxml2.
     *
     * These options enable XInclude processing, disable network access, and allow parsing of large documents.
     *
     * @see https://xmlsoft.org/html/libxml-parser.html#xmlParserOption
     */
    static constexpr int parser_options_ =
        XML_PARSE_XINCLUDE |
        XML_PARSE_NOBASEFIX |
        XML_PARSE_NONET |
        XML_PARSE_NOXINCNODE |
        XML_PARSE_COMPACT |
        XML_PARSE_HUGE;

private:
    /**
     * @brief Smart pointer managing the lifetime of the libxml2 document.
     *
     * Uses a custom deleter (`xmlFreeDoc`) to ensure proper cleanup.
     */
    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> doc_{nullptr, &xmlFreeDoc};
};

} // namespace canopy::io::xml