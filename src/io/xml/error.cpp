/**
 * @file error.cpp
 * @brief Implements error handling for XML processing in the canopy::io::xml namespace.
 *
 * This file provides the implementation for the error_type string conversion and the error exception class,
 * including integration with libxml2 error reporting.
 */

#include "io/xml/error.h"
#include <format>

namespace canopy::io::xml {

/**
 * @brief Converts an error_type to a human-readable string keyword.
 *
 * This function is used for diagnostics, logging, and error reporting.
 *
 * @param code The error_type to convert.
 * @return A string view representing the error type.
 *
 * @see error_type
 */
std::string_view to_string(const error_type code) noexcept {
    using enum error_type;
    switch (code) {
    case parse:
        return "ParseError";
    case logic:
        return "LogicError";
    case io:
        return "IOError";
    case x_include:
        return "XIncludeError";
    case validity:
        return "ValidityError";
    default:
        return "UnknownError";
    }
}

/**
 * @brief Constructs an error with a given error code and an empty message.
 *
 * @param code The error type.
 */
error::error(const error_type code) : code_{code} { msg_ = ""; }

/**
 * @brief Constructs an error with a given error code and message.
 *
 * @param code The error type.
 * @param msg  The error message.
 */
error::error(const error_type code, const std::string &msg) : code_{code}, msg_{format_message(code, msg)} {}

/**
 * @brief Constructs an error from a libxml2 error pointer.
 *
 * If the pointer is null, attempts to retrieve the last error from libxml2.
 * The message will include file and line information if available.
 *
 * @param code      The error type.
 * @param xml_error Pointer to a libxml2 error structure (may be null).
 */
error::error(const error_type code, xmlErrorPtr xml_error) : code_{code} {
    msg_ = "";
    if (!xml_error)
        xml_error = xmlGetLastError();
    if (!xml_error) {
        msg_ += " (No XML error is available.)";
    } else {
        if (xml_error->file)
            msg_ += " " + std::string(xml_error->file);
        if (xml_error->line)
            msg_ += " line " + std::to_string(xml_error->line);
    }
    msg_ = format_message(code, msg_);
}

/**
 * @brief Constructs an error from a libxml2 error pointer and a custom message.
 *
 * If the pointer is null, attempts to retrieve the last error from libxml2.
 * The message will include file and line information if available, appended to the custom message.
 *
 * @param code      The error type.
 * @param xml_error Pointer to a libxml2 error structure (may be null).
 * @param msg       Additional error message.
 */
error::error(const error_type code, xmlErrorPtr xml_error, const std::string &msg) : code_{code} {
    msg_ = msg;
    if (!xml_error)
        xml_error = xmlGetLastError();
    if (!xml_error) {
        msg_ += " (No XML error is available.)";
    } else {
        if (xml_error->file)
            msg_ += " " + std::string(xml_error->file);
        if (xml_error->line)
            msg_ += " line " + std::to_string(xml_error->line);
    }
    msg_ = format_message(code, msg_);
}

/**
 * @brief Formats the error message with code and message.
 *
 * This static helper combines the error type string and the message for consistent reporting.
 *
 * @param code The error type.
 * @param msg  The error message.
 * @return A formatted string combining code and message.
 */
std::string error::format_message(const error_type code, const std::string &msg) {
    return std::format("{}: {}", to_string(code), msg);
}

} // namespace canopy::io::xml