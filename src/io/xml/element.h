#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <type_traits>
#include <stdexcept>
#include <libxml/tree.h>
#include <ranges>

#include "io/xml/range.h"
#include "io/xml/error.h"
#include "io/xml/helpers.h"

namespace canopy::io::xml {

/// XML element adaptor.
class element {
public:
/// Construct with a pointer to xmlElement.
/// The pointer must be non-null.
explicit element(const xmlElement* element);

/// @returns The URI of the file from which this element originated.
[[nodiscard]] const char* filename() const;

/// @returns The line number where this element appears.
[[nodiscard]] int line() const;

/// @returns The name of this XML element (UTF-8).
[[nodiscard]] std::string_view name() const;

/// Check whether an attribute with the given name exists.
[[maybe_unused]] bool has_attribute(const char* name) const;

/// Retrieve the given attribute’s value (or empty if absent).
[[nodiscard]] std::string_view attribute(const char* name) const;

/// Numeric attribute retrieval (e.g., int, double).
template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, std::optional<T>>
attribute(const char* name) const
{
    std::string_view value = attribute(name);
    if (value.empty()) {
        return {};
    }
    try {
        return helpers::to<T>(value);
    } catch (error&) {
        // Add element name/line/filename to our error message
        const std::string msg = std::string(this->name()) + ", " +
                          std::to_string(line()) + "," + filename();
        throw error(error_type::validity, msg);
    }
}

/// @returns The textual content of this element (trimmed).
[[nodiscard]] std::string_view text() const;

/// Numeric text retrieval (e.g., int, double).
template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, T>
text() const
{
    try {
        return helpers::to<T>(text());
    } catch (error&) {
        // Add element name/line/filename to our error message
        const std::string msg = std::string(this->name()) + ", " +
                          std::to_string(line()) + "," + filename();
        throw error(error_type::validity, msg);
    }
}

/// Find the first child element (with optional name filter).
[[nodiscard]] std::optional<element> child(std::string_view name = "") const;

/// @returns A range over all child elements.
[[nodiscard]] range children() const;

/// @returns A filtered range of the children with a given name.
/// @pre The (string_view) name must remain valid while using the range.
[[nodiscard]] auto children(std::string_view name) const
{
    using namespace std::ranges;
    return children() | views::filter([name](const element& e) {
        return e.name() == name;
    });
}
private:
/// Convert our element_ pointer to xmlNode* (for libxml calls).
[[nodiscard]] xmlNode* to_node() const;

const xmlElement* element_{nullptr};
};

} // namespace canopy::io::xml