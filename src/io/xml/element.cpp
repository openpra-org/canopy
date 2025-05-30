/**
 * @file element.cpp
 * @brief Implementation of the canopy::io::xml::element class.
 *
 * This file provides the implementation for the element class, which adapts libxml2 xmlElement pointers
 * for safe and convenient access to XML element data.
 */

#include "io/xml/element.h"

namespace io::xml {

/**
 * @brief Constructs an element adaptor from a non-null xmlElement pointer.
 *
 * @param elem Pointer to a libxml2 xmlElement. Must not be null.
 * @throws Assertion failure if the pointer is null.
 *
 * @see element::element(const xmlElement*)
 */
element::element(const xmlElement *elem) : element_(elem) { assert(element_ && "element pointer must not be null"); }

/**
 * @brief Returns the URI of the file from which this element originated.
 *
 * This is typically the document's URI as provided by libxml2.
 *
 * @return C-string containing the document URI, or nullptr if not available.
 */
const char *element::filename() const {
    // Provided by libxml, URL is the document's URI.
    return helpers::from_utf8(element_->doc->URL);
}

/**
 * @brief Returns the line number where this element appears in the source file.
 *
 * @return Line number (1-based) or 0 if unavailable.
 */
int element::line() const { return XML_GET_LINE(to_node()); }

/**
 * @brief Returns the name of this XML element (UTF-8).
 *
 * @return Name of the element as a string_view.
 */
std::string_view element::name() const { return helpers::from_utf8(element_->name); }

/**
 * @brief Checks whether an attribute with the given name exists.
 *
 * @param name Name of the attribute (UTF-8).
 * @return True if the attribute exists, false otherwise.
 */
bool element::has_attribute(const char *name) const {
    return xmlHasProp(to_node(), helpers::to_utf8(name)) != nullptr;
}

/**
 * @brief Retrieves the value of the given attribute, or an empty string_view if absent.
 *
 * @param name Name of the attribute (UTF-8).
 * @return Value of the attribute as a string_view, or empty if not present.
 *
 * @warning If the attribute exists but is not a text node, this will assert.
 */
std::string_view element::attribute(const char *name) const {
    const xmlAttr *property = xmlHasProp(to_node(), helpers::to_utf8(name));
    if (!property) {
        return {};
    }
    const xmlNode *text_node = property->children;
    assert(text_node && text_node->type == XML_TEXT_NODE);
    assert(text_node->content);
    return helpers::trim(helpers::from_utf8(text_node->content));
}

/**
 * @brief Returns the trimmed textual content of this element.
 *
 * @return Text content as a string_view.
 * @throws Assertion failure if the element does not have text content.
 *
 * @warning Only the first text node child is returned; mixed content is not supported.
 */
std::string_view element::text() const {
    const xmlNode *text_node = element_->children;
    while (text_node && text_node->type != XML_TEXT_NODE) {
        text_node = text_node->next;
    }
    assert(text_node && "element does not have text.");
    assert(text_node->content && "Missing text in element.");
    return helpers::trim(helpers::from_utf8(text_node->content));
}

/**
 * @brief Finds the first child element, optionally filtered by name.
 *
 * @param name Optional name to filter children (empty string means any child).
 * @return Optional containing the first matching child element, or std::nullopt if none found.
 */
std::optional<element> element::child(std::string_view name) const {
    for (auto e : children()) {
        if (name.empty() || name == e.name()) {
            return e;
        }
    }
    return {};
}

/**
 * @brief Returns a range over all child elements.
 *
 * @return A range object for iterating over child elements.
 */
range element::children() const { return range(element_->children); }

/**
 * @brief Converts the internal xmlElement pointer to an xmlNode pointer for libxml2 calls.
 *
 * @return Pointer to the underlying xmlNode.
 *
 * @warning This is a reinterpret_cast; use only for libxml2 API compatibility.
 */
xmlNode *element::to_node() const {
    // Downcast xmlElement* -> xmlNode*
    return reinterpret_cast<xmlNode *>(const_cast<xmlElement *>(element_));
}

} // namespace canopy::io::xml