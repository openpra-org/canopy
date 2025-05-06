/// @file
/// The MEF facilities to call external functions in expressions.

#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/dll/shared_library.hpp>
#include <boost/exception/errinfo_nested_exception.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/system/system_error.hpp>

#include "mef/openpsa/element.h"
#include "mef/openpsa/error.h"
#include "mef/openpsa/expression.h"

namespace fs = boost::filesystem;

namespace mef::openpsa {

/// The MEF construct to extend expressions with external libraries.
/// This class dynamically loads and manages libraries.
/// It supports only very basic interface for C function lookup with its symbol.
class ExternLibrary : public Element, public Usage {
 public:
   /// Type string for error messages.
   static constexpr const char* kTypeString = "extern library";

   /// @copydoc Element::Element
   ///
   /// @param[in] lib_path  The library path with its name.
   /// @param[in] reference_dir  The reference directory for relative paths.
   /// @param[in] system  Search for the library in system paths.
   /// @param[in] decorate  Decorate the library name with prefix and suffix.
   ///
   /// @throws ValidityError  The library path is invalid.
   /// @throws DLError  The library cannot be found.
   ExternLibrary(std::string name, std::string lib_path,
                 const boost::filesystem::path& reference_dir, bool system,
                 bool decorate)   : Element(std::move(name)) {
       fs::path fs_path(lib_path);
       std::string filename = fs_path.filename().string();
       // clang-format off
  if (fs_path.empty() ||
      filename == "." ||
      filename == ".." ||
      lib_path.back() == ':' ||
      lib_path.back() == '/' ||
      lib_path.back() == '\\') {
    throw(ValidityError("Invalid library path format: "+lib_path+" "+Element::name()+ " "+std::string(kTypeString)));
  }
       // clang-format on

       boost::dll::load_mode::type load_type = boost::dll::load_mode::default_mode;
       if (decorate)
           load_type |= boost::dll::load_mode::append_decorations;
       if (system)
           load_type |= boost::dll::load_mode::search_system_folders;

       fs::path ref_path = lib_path;
       if (!system || ref_path.has_parent_path())
           ref_path = fs::absolute(ref_path, reference_dir);

       try {
           lib_handle_.load(ref_path, load_type);
       } catch (const boost::system::system_error& err) {
           throw(DLError(err.what() + to_string(boost::current_exception())+ " " +Element::name() + " "+std::string(kTypeString)));
       }
   }

   /// @tparam F  The C free function type.
   ///
   /// @param[in] symbol  The function symbol in the library.
   ///
   /// @returns The function pointer resolved from the symbol.
   ///
   /// @throws DLError  The symbol is not in the library.
   template <typename F>
   std::enable_if_t<std::is_function_v<F>, std::add_pointer_t<F>>
   get(const std::string& symbol) const {
       try {
           return lib_handle_.get<F>(symbol);
       } catch (const boost::system::system_error& err) {
           throw(DLError(err.what() + symbol + to_string(boost::current_exception())));
       }
   }

 private:
   boost::dll::shared_library lib_handle_;  ///< Shared Library abstraction.
};

template <typename R, typename... Args>
class ExternFunction;  // Forward declaration to specialize abstract base.

/// Abstract base class for ExternFunction concrete types.
/// This interface hides the return and argument types
/// of generic extern functions and expressions.
///
/// The base acts as a factory for generating expressions with given arguments.
template <>
class ExternFunction<void> : public Element, public Usage {
 public:
   /// Type string for error messages.
   static constexpr const char* kTypeString = "extern function";

   using Element::Element;

   virtual ~ExternFunction() = default;

   /// Applies the function to arguments.
   /// This interface hides the complexity of concrete types of the function.
   ///
   /// @param[in] args  The argument expressions.
   ///
   /// @returns Newly constructed expression as a result of function application.
   ///
   /// @throws ValidityError  The number of arguments is invalid.
   virtual std::unique_ptr<Expression>
   apply(std::vector<Expression*> args) const = 0;
};

using ExternFunctionPtr = std::unique_ptr<ExternFunction<void>>;  ///< Base ptr.
using ExternFunctionBase = ExternFunction<void>;  ///< To help Doxygen.

/// Extern function abstraction to be referenced by expressions.
///
/// @tparam R  Numeric return type.
/// @tparam Args  Numeric argument types.
///
/// @pre The source dynamic library is loaded as long as this function lives.
template <typename R, typename... Args>
class ExternFunction : public ExternFunctionBase {
   static_assert(std::is_arithmetic_v<R>, "Numeric type functions only.");

   using Pointer = R (*)(Args...);  ///< The function pointer type.

 public:
   /// Loads a function from a library for further usage.
   ///
   /// @copydoc Element::Element
   ///
   /// @param[in] symbol  The symbol name for the function in the library.
   /// @param[in] library  The dynamic library to lookup the function.
   ///
   /// @throws DLError  There is no such symbol in the library.
   ExternFunction(std::string name, const std::string& symbol,
                  const ExternLibrary& library)
       : ExternFunctionBase(std::move(name)),
         fptr_(library.get<R(Args...)>(symbol)) {}

   /// Calls the library function with the given numeric arguments.
   R operator()(Args... args) const noexcept { return fptr_(args...); }

   /// @copydoc ExternFunction<void>::apply
   std::unique_ptr<Expression>
   apply(std::vector<Expression*> args) const override;

 private:
   const Pointer fptr_;  ///< The pointer to the extern function in a library.
};

/// Expression evaluating an extern function with expression arguments.
///
/// @tparam R  Numeric return type.
/// @tparam Args  Numeric argument types.
template <typename R, typename... Args>
class ExternExpression
   : public ExpressionFormula<ExternExpression<R, Args...>> {
 public:
   /// @param[in] extern_function  The library function.
   /// @param[in] args  The argument expression for the function.
   ///
   /// @throws ValidityError  The number of arguments is invalid.
   explicit ExternExpression(const ExternFunction<R, Args...>* extern_function,
                             std::vector<Expression*> args)
       : ExpressionFormula<ExternExpression>(std::move(args)),
         extern_function_(*extern_function) {
       if (Expression::args().size() != sizeof...(Args))
           throw(
               ValidityError("The number of function arguments does not match."));
   }

   /// Computes the extern function with the given evaluator for arguments.
   template <typename F>
   double Compute(F&& eval) noexcept {
       return Marshal(std::forward<F>(eval),
                      std::make_index_sequence<sizeof...(Args)>());
   }

 private:
   /// Evaluates the argument expressions and marshals the result to function.
   /// Marshaller of expressions to extern function calls.
   ///
   /// @tparam F  The expression evaluator type.
   /// @tparam Is  The index sequence for the arguments vector.
   ///
   /// @param[in] eval  The evaluator of the expressions.
   ///
   /// @returns The result of the function call.
   ///
   /// @pre The number of arguments is exactly the same at runtime.
   template <typename F, std::size_t... Is>
   double Marshal(F&& eval, std::index_sequence<Is...>) noexcept {
       return extern_function_(eval(Expression::args()[Is])...);
   }

   const ExternFunction<R, Args...>& extern_function_;  ///< The source function.
};

template <typename R, typename... Args>
std::unique_ptr<Expression>
ExternFunction<R, Args...>::apply(std::vector<Expression*> args) const {
   return std::make_unique<ExternExpression<R, Args...>>(this, std::move(args));
}

}  // namespace scram::mef
