#pragma once

/**
 * @file error.h
 * @brief Defines error handling types and classes for XML processing in the canopy::io::xml namespace.
 *
 * This file provides a unified error type and exception class for XML parsing, validation, and I/O errors,
 * as well as utilities for error code string conversion.
 */

#include <exception>
#include <libxml/xmlerror.h>
#include <string>
#include <string_view>

namespace canopy::io::xml {

/**
 * @enum error_type
 * @brief Enumerates error codes for XML processing.
 *
 * This enum classifies the types of errors that can occur during XML operations,
 * such as parsing, logic, I/O, XInclude, and validity errors.
 *
 * @code
 * try {
 *     // ... XML processing ...
 * } catch (const canopy::io::xml::error& e) {
 *     if (e.code() == canopy::io::xml::error_type::parse) {
 *         std::cerr << "Parse error: " << e.what() << std::endl;
 *     }
 * }
 * @endcode
 */
enum class error_type {
    unknown = -1,  ///< Unknown error type.
    parse = 1,     ///< XML parsing error.
    logic = 2,     ///< Logic error (e.g., misuse of API).
    io = 3,        ///< Input/output error.
    x_include = 4, ///< XInclude processing error.
    validity = 5,  ///< XML validity or schema error.
};

/**
 * @brief Converts an error_type to a human-readable string keyword.
 *
 * This function maps each error_type to a descriptive string, useful for logging or diagnostics.
 *
 * @param code The error_type to convert.
 * @return A string view representing the error type.
 *
 * @code
 * std::string_view type_str = canopy::io::xml::to_string(canopy::io::xml::error_type::io);
 * // type_str == "IOError"
 * @endcode
 */
std::string_view to_string(error_type code) noexcept;

/**
 * @class error
 * @brief Unified XML error exception for all XML-related failures.
 *
 * This exception class encapsulates error codes and messages for XML processing,
 * including details from libxml2 errors when available. It is intended to be thrown
 * on any XML-related failure, providing both a code and a descriptive message.
 *
 * @code
 * try {
 *     throw canopy::io::xml::error(canopy::io::xml::error_type::parse, "Malformed tag");
 * } catch (const canopy::io::xml::error& e) {
 *     std::cerr << "XML error: " << e.what() << std::endl;
 * }
 * @endcode
 */
class error final : public std::exception {
  public:
    /**
     * @brief Constructs an error with a given error code and an empty message.
     *
     * @param code The error type.
     */
    explicit error(error_type code);

    /**
     * @brief Constructs an error with a given error code and message.
     *
     * @param code The error type.
     * @param msg  The error message.
     */
    error(error_type code, const std::string &msg);

    /**
     * @brief Constructs an error from a libxml2 error pointer.
     *
     * If the pointer is null, attempts to retrieve the last error from libxml2.
     * The message will include file and line information if available.
     *
     * @param code      The error type.
     * @param xml_error Pointer to a libxml2 error structure (may be null).
     */
    error(error_type code, xmlErrorPtr xml_error);

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
    error(error_type code, xmlErrorPtr xml_error, const std::string &msg);

    /**
     * @brief Returns the error code associated with this exception.
     *
     * @return The error_type value.
     */
    [[nodiscard]] error_type code() const noexcept { return code_; }

    /**
     * @brief Returns the error message as a C string.
     *
     * Overrides std::exception::what().
     *
     * @return The error message.
     */
    [[nodiscard]] const char *what() const noexcept override { return msg_.c_str(); }

  private:
    /**
     * @brief Formats the error message with code and message.
     *
     * @param code The error type.
     * @param msg  The error message.
     * @return A formatted string combining code and message.
     */
    static std::string format_message(error_type code, const std::string &msg);

    error_type code_; ///< The error code.
    std::string msg_; ///< The error message.
};

} // namespace canopy::io::xml
