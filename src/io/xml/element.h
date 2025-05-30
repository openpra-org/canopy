#pragma once

/**
 * @file element.h
 * @brief Defines the canopy::io::xml::element class, an adaptor for libxml2 XML elements.
 *
 * This file provides the interface for the element class, which wraps a libxml2 xmlElement pointer,
 * offering convenient and type-safe access to element names, attributes, text, and children.
 */

#include "io/xml/error.h"
#include "io/xml/helpers.h"
#include "io/xml/range.h"
#include <libxml/tree.h>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>

namespace canopy::io::xml {

/**
 * @class element
 * @brief Adaptor for libxml2 XML elements, providing safe and convenient access to element data.
 *
 * The element class wraps a non-null xmlElement pointer, exposing methods to query element name, attributes,
 * text content, and children. It also provides type-safe conversions for attribute and text values.
 *
 * Example usage:
 * @code
 * canopy::io::xml::document doc("file.xml");
 * auto root = doc.root();
 * std::cout << root.name() << std::endl;
 * if (root.has_attribute("id")) {
 *     int id = root.attribute<int>("id").value_or(-1);
 * }
 * for (auto child : root.children()) {
 *     std::cout << child.name() << ": " << child.text() << std::endl;
 * }
 * @endcode
 */
class element {
  public:
    /**
     * @brief Constructs an element adaptor from a non-null xmlElement pointer.
     *
     * @param elem Pointer to a libxml2 xmlElement. Must not be null.
     * @throws std::assertion if the pointer is null.
     *
     * @code
     * // Typically constructed via document::root() or element::child()
     * @endcode
     */
    explicit element(const xmlElement *elem);

    /**
     * @brief Returns the URI of the file from which this element originated.
     *
     * @return C-string containing the document URI (can be a nullptr if not available).
     *
     * @code
     * std::cout << elem.filename() << std::endl;
     * @endcode
     */
    [[nodiscard]] const char *filename() const;

    /**
     * @brief Returns the line number where this element appears in the source file.
     *
     * @return Line number (1-based) or 0 if unavailable.
     *
     * @code
     * std::cout << "Line: " << elem.line() << std::endl;
     * @endcode
     */
    [[nodiscard]] int line() const;

    /**
     * @brief Returns the name of this XML element (UTF-8).
     *
     * @return Name of the element as a string_view.
     *
     * @code
     * std::string_view tag = elem.name();
     * @endcode
     */
    [[nodiscard]] std::string_view name() const;

    /**
     * @brief Checks whether an attribute with the given name exists.
     *
     * @param name Name of the attribute (UTF-8).
     * @return True if the attribute exists, false otherwise.
     *
     * @code
     * if (elem.has_attribute("id")) { ... }
     * @endcode
     */
    [[maybe_unused]] bool has_attribute(const char *name) const;

    /**
     * @brief Retrieves the value of the given attribute, or an empty string_view if absent.
     *
     * @param name Name of the attribute (UTF-8).
     * @return Value of the attribute as a string_view, or empty if not present.
     *
     * @code
     * std::string_view val = elem.attribute("type");
     * @endcode
     */
    [[nodiscard]] std::string_view attribute(const char *name) const;

    /**
     * @brief Retrieves the value of the given attribute as a numeric type (e.g., int, double).
     *
     * If the attribute is absent, returns std::nullopt. If present but not convertible, throws error.
     *
     * @tparam T Numeric type (int, double, etc.).
     * @param name Name of the attribute (UTF-8).
     * @return Optional containing the converted value, or std::nullopt if attribute is missing.
     * @throws error if conversion fails, with context (element name, line, filename).
     *
     * @code
     * auto id = elem.attribute<int>("id");
     * if (id) { ... }
     * @endcode
     */
    template <typename T>
    std::enable_if_t<std::is_arithmetic_v<T>, std::optional<T>> attribute(const char *name) const {
        std::string_view value = attribute(name);
        if (value.empty()) {
            return {};
        }
        try {
            return helpers::to<T>(value);
        } catch (error &) {
            // Add element name/line/filename to our error message
            const std::string msg = std::string(this->name()) + ", " + std::to_string(line()) + "," + filename();
            throw error(error_type::validity, msg);
        }
    }

    /**
     * @brief Returns the trimmed textual content of this element.
     *
     * @return Text content as a string_view.
     * @throws std::assertion if the element does not have text content.
     *
     * @code
     * std::string_view txt = elem.text();
     * @endcode
     */
    [[nodiscard]] std::string_view text() const;

    /**
     * @brief Retrieves the text content as a numeric type (e.g., int, double).
     *
     * @tparam T Numeric type (int, double, etc.).
     * @return The converted value.
     * @throws error if conversion fails, with context (element name, line, filename).
     *
     * @code
     * int value = elem.text<int>();
     * @endcode
     */
    template <typename T> std::enable_if_t<std::is_arithmetic_v<T>, T> text() const {
        try {
            return helpers::to<T>(text());
        } catch (error &) {
            // Add element name/line/filename to our error message
            const std::string msg = std::string(this->name()) + ", " + std::to_string(line()) + "," + filename();
            throw error(error_type::validity, msg);
        }
    }

    /**
     * @brief Finds the first child element, optionally filtered by name.
     *
     * @param name Optional name to filter children (empty string means any child).
     * @return Optional containing the first matching child element, or std::nullopt if none found.
     *
     * @code
     * auto child = elem.child("item");
     * if (child) { ... }
     * @endcode
     */
    [[nodiscard]] std::optional<element> child(std::string_view name = "") const;

    /**
     * @brief Returns a range over all child elements.
     *
     * @return A range object for iterating over child elements.
     *
     * @code
     * for (auto c : elem.children()) { ... }
     * @endcode
     */
    [[nodiscard]] range children() const;

    /**
     * @brief Returns a filtered range of child elements with a given name.
     *
     * @param name Name to filter children by (string_view must remain valid while using the range).
     * @return A filtered range of child elements.
     *
     * @code
     * for (auto c : elem.children("item")) { ... }
     * @endcode
     */
    [[nodiscard]] auto children(std::string_view name) const {
        using namespace std::ranges;
        return children() | views::filter([name](const element &e) { return e.name() == name; });
    }

  private:
    /**
     * @brief Converts the internal xmlElement pointer to an xmlNode pointer for libxml2 calls.
     *
     * @return Pointer to the underlying xmlNode.
     */
    [[nodiscard]] xmlNode *to_node() const;

    /**
     * @brief Pointer to the underlying libxml2 xmlElement (non-owning, non-null).
     */
    const xmlElement *element_{nullptr};
};

} // namespace canopy::io::xml