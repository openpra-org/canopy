#pragma once

#include <exception>
#include <string>
#include <string_view>
#include <libxml/xmlerror.h>

namespace canopy::io::xml {

    /// Error codes for XML processing.
    enum class error_type {
        unknown = -1,
        parse = 1,
        logic = 2,
        io = 3,
        x_include = 4,
        validity = 5,
    };

    /// Map error_type to a string keyword.
    inline std::string_view to_string(error_type code) noexcept {
        using enum error_type;
        switch (code) {
            case parse:     return "ParseError";
            case logic:     return "LogicError";
            case io:        return "IOError";
            case x_include: return "XIncludeError";
            case validity:  return "ValidityError";
            default:        return "UnknownError";
        }
    }

    /// Unified XML error exception.
    class error final : public std::exception {
    public:
        /// Construct a general error.
        explicit error(const error_type code);

        /// Construct a general error.
        error(const error_type code, const std::string &msg);

        error(const error_type code, xmlErrorPtr xml_error);

        error(const error_type code, xmlErrorPtr xml_error, const std::string &msg);

        /// Return the error code.
        [[nodiscard]] error_type code() const noexcept { return code_; }

        /// Return the error message.
        [[nodiscard]] const char* what() const noexcept override { return msg_.c_str(); }

    private:
        static std::string format_message(const error_type code, const std::string& msg);

        error_type code_;
        std::string msg_;
    };

} // namespace canopy::io::xml
