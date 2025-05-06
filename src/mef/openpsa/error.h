/// @file
/// Exceptions for SCRAM (STL-only version).
/// This version uses only the C++ Standard Library.
/// Exception classes act as tags and do not carry extra data members,
/// except for the error message string.
/// If you need to carry extra data, you must add data members to derived classes.
/// All error information must be encoded in the message or in custom members.

#pragma once

#include <exception>
#include <string>

namespace mef::openpsa {

/// The Error class is the base class
/// for all exceptions specific to the SCRAM code.
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

/// For input/output related errors.
struct IOError : public Error {
    using Error::Error;
    // If you want to carry e.g. filename, add a member here.
};

/// Dynamic library errors.
struct DLError : public Error {
    using Error::Error;
};

/// Signals internal logic errors,
/// for example, pre-condition failure
/// or use of functionality in ways not designed to.
struct [[maybe_unused]] LogicError : public Error {
    using Error::Error;
};

/// This error can be used to indicate
/// that call for a function or operation is not legal.
struct [[maybe_unused]] IllegalOperation : public Error {
    using Error::Error;
};

/// The error in analysis settings.
struct [[maybe_unused]] SettingsError : public Error {
    using Error::Error;
};

/// The minimum required version is not satisfied.
struct VersionError : public Error {
    using Error::Error;
};

/// Model validity errors.
struct ValidityError : public Error {
    using Error::Error;
};

/// This error indicates that elements must be unique.
struct DuplicateElementError : public ValidityError {
    explicit DuplicateElementError() : ValidityError("Duplicate Element Error") {}
    explicit DuplicateElementError(const std::string& msg) : ValidityError("Duplicate Element Error: "+msg) {}
};

/// The error for undefined elements in a model.
struct UndefinedElement : public ValidityError {
    UndefinedElement() : ValidityError("Undefined Element Error") {}
    UndefinedElement(const std::string& msg) : ValidityError("Undefined Element Error: "+msg) {}
};

/// Unacceptable cycles in model structures.
struct CycleError : public ValidityError {
    CycleError() : ValidityError("Cycle Error") {}
};

/// Invalid domain for values or arguments.
struct DomainError : public ValidityError {
    using ValidityError::ValidityError;
};

/// NOTE: In the Boost version, you could attach error info such as
/// container id, element id, attribute, etc. to exceptions.
/// In STL, you must encode such info in the message or add members.
/// For example, you could do:
/// struct DomainError : public ValidityError {
///     DomainError(std::string msg, std::string element_id)
///         : ValidityError(std::move(msg)), element_id_(std::move(element_id)) {}
///     std::string element_id_;
/// };
/// But this violates the "no new data members" rule.
}  // namespace scram