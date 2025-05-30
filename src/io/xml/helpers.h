#pragma once

/**
 * @file helpers.h
 * @brief Internal XML helper functions for parsing, conversion, and error handling in canopy::io::xml.
 */

#include "io/xml/error.h"
#include <cassert>
#include <libxml/xmlerror.h>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

namespace canopy::io::xml::helpers {

/**
 * @brief Converts a string view to a numeric or boolean value.
 *
 * Interprets the given string as an int, double, or bool, depending on the template parameter.
 * Throws a validity error if the conversion fails or the value is out of range.
 *
 * @tparam T Numeric type to convert to. Supported: int, double, bool.
 * @param[in] value The non-empty value string to interpret.
 * @return The interpreted value of type T.
 * @throws error If the value cannot be interpreted as the requested type.
 *
 * @code
 * int i = canopy::io::xml::helpers::to<int>("42");
 * double d = canopy::io::xml::helpers::to<double>("3.14");
 * bool b = canopy::io::xml::helpers::to<bool>("true");
 * @endcode
 */
template <typename T> std::enable_if_t<std::is_arithmetic_v<T>, T> to(const std::string_view &value) {
    if constexpr (std::is_same_v<T, int>) {
        char *end_char = nullptr;
        const std::int64_t ret = std::strtoll(value.data(), &end_char, 10);
        if (const auto len = end_char - value.data();
            len != value.size() || ret > std::numeric_limits<int>::max() || ret < std::numeric_limits<int>::min()) {
            throw error(error_type::validity, "Failed to interpret value as int: " + std::string(value));
        }
        return static_cast<int>(ret);
    } else if constexpr (std::is_same_v<T, double>) {
        char *end_char = nullptr;
        const double ret = std::strtod(value.data(), &end_char);
        if (const auto len = end_char - value.data(); len != value.size() || ret == HUGE_VAL || ret == -HUGE_VAL) {
            throw error(error_type::validity, "Failed to interpret value as double: " + std::string(value));
        }
        return ret;
    } else {
        static_assert(std::is_same_v<T, bool>, "Only default numeric types (int, double, bool) are supported.");
        if (value == "true" || value == "1")
            return true;
        if (value == "false" || value == "0")
            return false;
        throw error(error_type::validity, "Failed to interpret value as bool: " + std::string(value));
    }
}

/**
 * @brief Converts an XML library UTF-8 string to a C string.
 *
 * Reinterprets the pointer type for use in standard C/C++ string operations.
 *
 * @param[in] xml_string The string provided by the XML library (xmlChar*).
 * @return The same string as a const char*.
 * @pre xml_string must not be nullptr.
 *
 * @code
 * const char* cstr = canopy::io::xml::helpers::from_utf8(xmlNode->name);
 * @endcode
 */
inline const char *from_utf8(const xmlChar *xml_string) noexcept {
    assert(xml_string);
    return reinterpret_cast<const char *>(xml_string);
}

/**
 * @brief Converts a C string to an XML library UTF-8 string.
 *
 * Reinterprets the pointer type for use in libxml2 functions.
 *
 * @param[in] c_string The C string (UTF-8 encoded).
 * @return The same string as a const xmlChar*.
 * @pre c_string must not be nullptr and must be UTF-8 encoded.
 *
 * @code
 * xmlNodeSetContent(node, canopy::io::xml::helpers::to_utf8("value"));
 * @endcode
 */
inline const xmlChar *to_utf8(const char *c_string) noexcept {
    assert(c_string);
    return reinterpret_cast<const xmlChar *>(c_string);
}

/**
 * @brief Trims leading and trailing space characters from an XML value string.
 *
 * Returns a view to the substring with whitespace removed from both ends.
 * Assumes the string is normalized by the XML parser.
 *
 * @param[in] text The text from an XML attribute or text node.
 * @return A std::string_view to the trimmed substring.
 *
 * @code
 * std::string_view trimmed = canopy::io::xml::helpers::trim("   hello world   ");
 * // trimmed == "hello world"
 * @endcode
 */
inline std::string_view trim(const std::string_view &text) noexcept {
    const auto pos_first = text.find_first_not_of(' ');
    if (pos_first == std::string_view::npos)
        return {};
    const auto pos_last = text.find_last_not_of(' ');
    const auto len = pos_last - pos_first + 1;
    return {text.data() + pos_first, len};
}

/**
 * @brief Converts the last XML error (or a given error) into a typed exception.
 *
 * Retrieves the last error from the XML library (if none is provided) and converts it into an exception of type T.
 * Intended for use in error handling after libxml2 operations.
 *
 * @tparam T The exception type to construct (should be derived from std::exception).
 * @param[in] xml_error The error to translate, or nullptr to retrieve the latest error.
 * @return The exception object to be thrown.
 *
 * @code
 * try {
 *     // ... libxml2 operation ...
 * } catch (...) {
 *     throw canopy::io::xml::helpers::get_error<canopy::io::xml::error>();
 * }
 * @endcode
 */
template <typename T> T get_error(xmlErrorPtr xml_error = nullptr) {
    if (!xml_error)
        xml_error = xmlGetLastError();
    assert(xml_error && "No XML error is available.");
    std::string info;
    if (!xml_error) {
        T throw_error("No XML error is available.");
        return throw_error;
    }
    if (xml_error->file)
        info.append(xml_error->file);
    if (xml_error->line)
        info += std::to_string(xml_error->line);
    T throw_error(info);
    return throw_error;
}

} // namespace canopy::io::xml::helpers