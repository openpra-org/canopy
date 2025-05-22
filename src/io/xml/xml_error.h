#pragma once

#include <exception>
#include <string>

namespace io::xml {

///
/// @note The copy constructor is not noexcept as required by std::exception.
///       However, this class may only throw std::bad_alloc upon copy,
///       which may be produced anyway
///       even if the copy constructor were noexcept.
class Error : public std::exception {
  public:
    /// Constructs a new error with a provided message.
    ///
    /// @param[in] msg  The message to be passed with this error.
    explicit Error(std::string msg) : msg_(std::move(msg)) {}

    /// @returns The formatted error message to be printed.
    [[nodiscard]] const char* what() const noexcept override { return msg_.c_str(); }

  private:
    std::string msg_;  ///< The error message.
};

/// XML parsing errors.
struct ParseError : public Error {
    using Error::Error;
};

struct LogicError : public Error {
    using Error::Error;
};

struct IOError : public Error {
    IOError(const std::string& msg, const std::string& file, int errnum, const std::string& mode)
        : Error(FormatMessage(msg, file, errnum, mode)),
          file_(file),
          errnum_(errnum),
          mode_(mode)
    {}

    [[nodiscard]] const std::string& file() const noexcept { return file_; }
    [[nodiscard]] int errnum() const noexcept { return errnum_; }
    [[nodiscard]] const std::string& mode() const noexcept { return mode_; }

  private:
    std::string file_;
    int errnum_;
    std::string mode_;

    static std::string FormatMessage(const std::string& msg,
                                     const std::string& file,
                                     int errnum,
                                     const std::string& mode)
    {
        return "IOError: " + msg +
               " [file: " + file +
               ", errno: " + std::to_string(errnum) +
               ", mode: " + mode + "]";
    }
};

/// XInclude resolution issues.
struct XIncludeError : public Error {
    using Error::Error;
};

/// XML document validity errors.
struct ValidityError : public Error {
    using Error::Error;
};

/// NOTE: In the Boost version, you could attach error info such as
/// attribute name or element name to exceptions.
/// In STL, you must encode such info in the message or add members.

}  // namespace canopy::io::xml