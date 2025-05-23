#pragma once

#include <cassert>

#include "io/xml/error.h"

namespace canopy::io::xml::helpers {  // Internal XML helper functions.

/// Gets a number from an XML value.
///
/// @tparam T  Numeric type.
///
/// @param[in] value  The non-empty value string.
///
/// @returns The interpreted value.
///
/// @throws ValidityError  The interpretation is unsuccessful.
template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, T> to(const std::string_view& value) {
   if constexpr (std::is_same_v<T, int>) {
       char* end_char = nullptr;
       std::int64_t ret = std::strtoll(value.data(), &end_char, 10);
       auto len = end_char - value.data();
       if (len != value.size() || ret > std::numeric_limits<int>::max() ||
           ret < std::numeric_limits<int>::min()) {
           throw error(error_type::validity, "Failed to interpret value as int"+std::string(value));
       }
       return ret;
   } else if constexpr (std::is_same_v<T, double>) {  // NOLINT
       char* end_char = nullptr;
       double ret = std::strtod(value.data(), &end_char);
       if (const auto len = end_char - value.data(); len != value.size() || ret == HUGE_VAL || ret == -HUGE_VAL) {
           throw error(error_type::validity, "Failed to interpret value as double"+std::string(value));
       }
       return ret;
   } else {
       static_assert(std::is_same_v<T, bool>, "Only default numeric types.");
       if (value == "true" || value == "1")
           return true;
       if (value == "false" || value == "0")
           return false;
       throw error(error_type::validity, "Failed to interpret value as bool"+std::string(value));
   }
}

/// Reinterprets the XML library UTF-8 string into C string.
///
/// @param[in] xml_string  The string provided by the XML library.
///
/// @returns The same string adapted for use as C string.
inline const char* from_utf8(const xmlChar* xml_string) noexcept {
   assert(xml_string);
   return reinterpret_cast<const char*>(xml_string);
}

/// Reinterprets C string as XML library UTF-8 string.
///
/// @param[in] c_string  The C byte-array encoding the XML string.
///
/// @returns The same string adapted for use in XML library functions.
///
/// @pre The C string has UTF-8 encoding.
inline const xmlChar* to_utf8(const char* c_string) noexcept {
   assert(c_string);
   return reinterpret_cast<const xmlChar*>(c_string);
}

/// Removes leading and trailing space characters from XML value string.
///
/// @param[in] text  The text in XML attribute or text nodes.
///
/// @returns View to the trimmed substring.
///
/// @pre The string is normalized by the XML parser.
inline std::string_view trim(const std::string_view& text) noexcept {
   auto pos_first = text.find_first_not_of(' ');
   if (pos_first == std::string_view::npos)
       return {};

   auto pos_last = text.find_last_not_of(' ');
   auto len = pos_last - pos_first + 1;

   return {text.data() + pos_first, len};
}

/// Gets the last XML error converted from the library error codes.
///
/// @tparam T  The SCRAM error type to convert XML error into.
///
/// @param[in] xml_error  The error to translate.
///                       nullptr to retrieve the latest error in the library.
///
/// @returns The exception object to be thrown.
template <typename T>
T get_error(xmlErrorPtr xml_error = nullptr) {
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

}  // namespace canopy::io::xml::helpers