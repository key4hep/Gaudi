/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_STATUSCODE_H
#define GAUDIKERNEL_STATUSCODE_H

#include "boost/preprocessor/facilities/overload.hpp"
#include <functional>
#include <ostream>
#include <type_traits>
#include <utility>

#include "GaudiKernel/Kernel.h"

template <typename T>
struct is_StatusCode_enum : std::false_type {};

/**
 * @class StatusCode
 *
 * This class is used for returning status codes from appropriate routines.
 * A StatusCode is comprised of an (integer) value and a category (similar to std::error_code).
 * By default StatusCodes are created within a default StatusCode::Category, which
 * defines the isSuccess(), isFailure() and isRecoverable() behaviour depending on
 * the value of the StatusCode. This behaviour can be modified by defining a custom
 * StatusCode::Category and overriding the respective methods.
 *
 * To define a new StatusCode::Category, do the following:
 * - Define an enum class with the values of the StatusCode
 * - Register the enum with `STATUSCODE_ENUM_DECL( MyEnum )`
 * - Derive your category from StatusCode::Category and implement/override the relevant methods
 * - Register the new category with `STATUSCODE_ENUM_IMPL( MyEnum, MyCategory )` in a .cpp file
 *
 * \par Combining StatusCodes
 *  - The bitwise logic operators (&, |, &=, |=) can be used to combine StatusCodes and follows
 *    three-valued (ternary) logic with RECOVERABLE being the third state. No short-circuiting is applied:
 *    \code
 *       StatusCode sc = sc1 & sc2;
 *       sc |= sc3;
 *    \endcode
 *  - The boolean logic operators (&&, ||) perform regular two-valued logic only considering
 *    success/failure. The usual short-circuiting applies:
 *    \code
 *       bool b = sc1 && sc2;
 *    \endcode
 *
 * \note
 * The StatusCode values 0 and 1 are considered FAILURE/SUCCESS for all categories, i.e.
 * - operator==() and operator!=() ignore the category for these two values
 * - isSuccess() and isFailure() cannot be overridden for these two values
 *
 * \remark See https://akrzemi1.wordpress.com/2017/07/12/your-own-error-code for details on the underlying design
 */
class
#if __cplusplus >= 201703L && !defined( __CLING__ )
    [[nodiscard]]
#endif
    StatusCode final {
public:
  typedef unsigned long code_t; ///< type of StatusCode value

  enum class ErrorCode : code_t { FAILURE = 0, SUCCESS = 1, RECOVERABLE = 2 };

  /**
   * @class StatusCode::Category
   *
   * The category assigned to a StatusCode. Derive from this class to implement your own category.
   * The mapping of StatusCode values to success and recoverable conditions can be defined by
   * overriding the appropriate methods.
   */
  struct Category {
    constexpr Category() noexcept = default;
    virtual ~Category() {}

    /// Name of the category
    virtual const char* name() const = 0;

    /// Description for code within this category
    virtual std::string message( code_t code ) const { return "UNKNOWN(" + std::to_string( code ) + ")"; }

    /// Is code considered success ?
    /// \note isFailure() cannot be overridden as it is defined as `!`isSuccess()
    virtual bool isSuccess( code_t code ) const { return code == static_cast<code_t>( ErrorCode::SUCCESS ); }

    /// Is code considered recoverable ?
    virtual bool isRecoverable( code_t code ) const { return code == static_cast<code_t>( ErrorCode::RECOVERABLE ); }
  };

  /// Default Gaudi StatusCode category
  static const Category& default_category() noexcept;

  // Provide shorthands for default code values
  constexpr const static auto SUCCESS     = ErrorCode::SUCCESS;
  constexpr const static auto FAILURE     = ErrorCode::FAILURE;
  constexpr const static auto RECOVERABLE = ErrorCode::RECOVERABLE;

  /// Default constructor
  StatusCode() = default;

  /// Constructor from enum type (allowing implicit conversion)
  template <typename T, typename = std::enable_if_t<is_StatusCode_enum<T>::value>>
  StatusCode( T sc, bool checked = false ) noexcept {
    *this     = StatusCode( static_cast<StatusCode::code_t>( sc ), is_StatusCode_enum<T>::instance );
    m_checked = checked;
  }

  /// Constructor from code_t in the default category (explicit conversion only)
  explicit StatusCode( code_t code, const StatusCode::Category& cat = default_category(),
                       bool checked = false ) noexcept
      : m_cat( &cat ), m_code( code ), m_checked( checked ) {}

  /// Constructor from code_t and category (explicit conversion only)
  explicit StatusCode( code_t code, bool checked ) noexcept : StatusCode( code, default_category(), checked ) {}

  /// Copy constructor
  StatusCode( const StatusCode& rhs ) noexcept : m_cat( rhs.m_cat ), m_code( rhs.m_code ), m_checked( rhs.m_checked ) {
    rhs.m_checked = true;
  }

  /// Move constructor
  StatusCode( StatusCode && rhs ) noexcept : m_cat( rhs.m_cat ), m_code( rhs.m_code ), m_checked( rhs.m_checked ) {
    rhs.m_checked = true;
  }

  /// Destructor.
  ~StatusCode() {
    if ( UNLIKELY( s_checking ) ) check();
  }

  StatusCode& operator=( const StatusCode& rhs ) noexcept {
    m_cat     = rhs.m_cat;
    m_code    = rhs.m_code;
    m_checked = std::exchange( rhs.m_checked, true );
    return *this;
  }

  bool isSuccess() const;
  bool isFailure() const { return !isSuccess(); }
  bool isRecoverable() const;

  /// Shorthand for isSuccess()
  explicit operator bool() const { return isSuccess(); }

  /// Retrieve value ("checks" the StatusCode)
  code_t getCode() const {
    m_checked = true;
    return m_code;
  }

  /// Check/uncheck StatusCode
  const StatusCode& setChecked( bool checked = true ) const {
    m_checked = checked;
    return *this;
  }
  StatusCode& setChecked( bool checked = true ) {
    m_checked = checked;
    return *this;
  }

  /// Ignore/check StatusCode
  const StatusCode& ignore() const {
    setChecked( true );
    return *this;
  }
  StatusCode& ignore() {
    setChecked( true );
    return *this;
  }

  /// Chain code blocks making the execution conditional a success result.
  ///
  /// The chained execution stops on the first non-success StatusCode in the chain and
  /// returns it, or continues until the end and returns the last produced StatusCode.
  ///
  /// For example:
  /// \code
  /// StatusCode myFunction() {
  ///   return subFunction()
  ///       .andThen([]() {
  ///         do_something();
  ///       })
  ///       .andThen(anotherFunction)
  ///       .andThen([]() {
  ///         if (is_special_case())
  ///           return do_something_else();
  ///         return StatusCode::SUCCESS;
  ///       });
  /// }
  /// \endcode
  template <typename F, typename... ARGS>
  StatusCode andThen( F && f, ARGS && ... args ) const {
    if ( isFailure() ) return *this;
    return i_invoke( std::forward<F>( f ), std::forward<ARGS>( args )... );
  }

  /// Chain code blocks making the execution conditional a failure result.
  ///
  /// Inverse of StatusCode::andThen(), the passed function gets invoked only if
  /// if the StatusCode is a failure, in which case it either pass it on or overrides it,
  /// If the StatusCode is a success, it is passed on.
  ///
  /// For example:
  /// \code
  /// StatusCode myFunction() {
  ///   return subFunction()
  ///       .andThen([]() {
  ///         do_something();
  ///       })
  ///       .orElse(reportProblem);
  /// }
  /// \endcode
  template <typename F, typename... ARGS>
  StatusCode orElse( F && f, ARGS && ... args ) const {
    if ( isSuccess() ) return *this;
    return i_invoke( std::forward<F>( f ), std::forward<ARGS>( args )... );
  }

  /// Throw a GaudiException in case of failures.
  ///
  /// It can be chained with StatusCode::then, behaving as a pass-through for a success StatusCode,
  /// while for non-success a GaudiException is thrown, propagating the failure into the exception
  /// (using the StatusCode field of the exception).
  ///
  /// For example:
  /// \code
  /// void myFunction() {
  ///   doSomething()
  ///       .andThen( doSomethingElse )
  ///       .orThrow( "some error", "myFunction" )
  ///       .andThen( moreActions )
  ///       .orThrow( "too bad, we were nearly there", "myFunction" );
  /// }
  /// \endcode
  const StatusCode& orThrow( std::string message, std::string tag ) const {
    if ( isFailure() ) i_doThrow( std::move( message ), std::move( tag ) );
    return *this;
  }

  /// Has the StatusCode been checked?
  bool checked() const { return m_checked; }

  /// Retrieve category (does not "check" the StatusCode)
  const StatusCode::Category& getCategory() const { return *m_cat; }

  /// Description (or name) of StatusCode value
  std::string message() const { return getCategory().message( m_code ); }

  friend std::ostream& operator<<( std::ostream& s, const StatusCode& sc ) {
    s << sc.message();
    return s;
  }

  /// Check if StatusCode value and category are the same
  /// \note For code values `0(FAILURE)` and `1(SUCCESS)` the category is ignored
  /// \note e.g. `sc==StatusCode::SUCCESS` is equivalent to `sc.isSuccess()` for all categories
  friend bool operator==( const StatusCode& lhs, const StatusCode& rhs );
  friend bool operator!=( const StatusCode& lhs, const StatusCode& rhs ) { return !( lhs == rhs ); }

  /// Comparison (values are grouped by category first)
  friend bool operator<( const StatusCode& lhs, const StatusCode& rhs ) {
    lhs.m_checked = true;
    rhs.m_checked = true;
    return ( lhs.m_cat < rhs.m_cat || ( lhs.m_cat == rhs.m_cat && lhs.m_code < rhs.m_code ) );
  }

  /// Ternary logic operator with RECOVERABLE being the "third" state
  StatusCode& operator&=( const StatusCode& rhs );
  StatusCode& operator|=( const StatusCode& rhs ); ///< @copydoc operator&=

  static GAUDI_API void enableChecking();
  static GAUDI_API void disableChecking();
  static GAUDI_API bool checkingEnabled();

  /**
   * Simple RAII class to ignore unchecked StatusCode instances in a scope.
   *
   * Example:
   * @code{.cpp}
   * void myFunction() {
   *   StatusCode sc1 = aFunction(); // must be checked
   *   {
   *     StatusCode::ScopedDisableChecking _sc_ignore;
   *     StatusCode sc2 = anotherFunction(); // automatically ignored
   *   }
   * }
   * @endcode
   */
  class ScopedDisableChecking {
    bool m_enabled;

  public:
    ScopedDisableChecking() : m_enabled( StatusCode::checkingEnabled() ) {
      if ( m_enabled ) StatusCode::disableChecking();
    }
    ~ScopedDisableChecking() {
      if ( m_enabled ) StatusCode::enableChecking();
    }
  };

private:
  const Category* m_cat{&default_category()};                        ///< The status code category
  code_t          m_code{static_cast<code_t>( ErrorCode::SUCCESS )}; ///< The status code value
  mutable bool    m_checked{false};                                  ///< If the StatusCode has been checked
  static bool     s_checking; ///< Global flag to control if StatusCode need to be checked

  ErrorCode default_value() const; ///< Project onto the default StatusCode values
  void      check();               ///< Do StatusCode check

  /// Helper function to avoid circular dependency between GaudiException.h and StatusCode.h
  void i_doThrow( std::string message, std::string tag ) const;

  /// Helper to invoke a callable and return the resulting StatusCode or this, if the callable returns void.
  template <typename F, typename... ARGS, typename = std::enable_if_t<std::is_invocable_v<F, ARGS...>>>
  StatusCode i_invoke( F && f, ARGS && ... args ) const {
    if constexpr ( std::is_invocable_r_v<StatusCode, F, ARGS...> ) {
      return std::invoke( std::forward<F>( f ), std::forward<ARGS>( args )... );
    } else {
      // static_assert( std::is_same_v<void,std::invoke_result_t<F,ARGS...>>); // how paranoid should this be?
      std::invoke( std::forward<F>( f ), std::forward<ARGS>( args )... );
      return *this;
    }
  }
};

/*
 * Macros to declare/implement StatusCode enums/categories
 */

/// Declare an enum to be used as StatusCode value
/// @param ENUM enum class
#define STATUSCODE_ENUM_DECL( ENUM )                                                                                   \
  template <>                                                                                                          \
  struct is_StatusCode_enum<ENUM> : std::true_type {                                                                   \
    static const StatusCode::Category& instance;                                                                       \
  };

/// Assign a category to the StatusCode enum declared with STATUSCODE_ENUM_DECL( ENUM )
/// @param ENUM     enum class
/// @param CATEGORY (optional) category, otherwise use default StatusCode category
#define STATUSCODE_ENUM_IMPL( ... ) BOOST_PP_OVERLOAD( STATUSCODE_ENUM_IMPL_, __VA_ARGS__ )( __VA_ARGS__ )

#define STATUSCODE_ENUM_IMPL_1( ENUM )                                                                                 \
  const StatusCode::Category& is_StatusCode_enum<ENUM>::instance = StatusCode::default_category();

#define STATUSCODE_ENUM_IMPL_2( ENUM, CATEGORY )                                                                       \
  const StatusCode::Category& is_StatusCode_enum<ENUM>::instance = CATEGORY{};

// Declare the default StatusCode enum
STATUSCODE_ENUM_DECL( StatusCode::ErrorCode )

/*
 * Inline methods
 */

inline const StatusCode::Category& StatusCode::default_category() noexcept {
  return is_StatusCode_enum<StatusCode::ErrorCode>::instance;
}

inline bool StatusCode::isSuccess() const {
  m_checked = true;
  return ( m_code == static_cast<code_t>( ErrorCode::SUCCESS ) || m_cat->isSuccess( m_code ) );
}

inline bool StatusCode::isRecoverable() const {
  m_checked = true;
  return m_cat->isRecoverable( m_code );
}

inline StatusCode::ErrorCode StatusCode::default_value() const {
  bool save_checked = m_checked; // Preserve checked status
  auto r    = isSuccess() ? ErrorCode::SUCCESS : ( isRecoverable() ? ErrorCode::RECOVERABLE : ErrorCode::FAILURE );
  m_checked = save_checked;
  return r;
}

inline bool operator==( const StatusCode& lhs, const StatusCode& rhs ) {
  lhs.m_checked = true;
  rhs.m_checked = true;
  return ( lhs.m_code == rhs.m_code ) &&
         ( lhs.m_code == static_cast<StatusCode::code_t>( StatusCode::ErrorCode::SUCCESS ) ||
           lhs.m_code == static_cast<StatusCode::code_t>( StatusCode::ErrorCode::FAILURE ) ||
           ( lhs.m_cat == rhs.m_cat ) );
}

inline StatusCode& StatusCode::operator&=( const StatusCode& rhs ) {
  // Ternary AND lookup matrix
  static constexpr StatusCode::code_t AND[3][3] = {{0, 0, 0}, {0, 1, 2}, {0, 2, 2}};

  StatusCode::code_t l = static_cast<StatusCode::code_t>( default_value() );
  StatusCode::code_t r = static_cast<StatusCode::code_t>( rhs.default_value() );
  m_code               = AND[l][r];
  rhs.m_checked        = true;
  return *this;
}

inline StatusCode& StatusCode::operator|=( const StatusCode& rhs ) {
  // Ternary OR lookup matrix
  static constexpr StatusCode::code_t OR[3][3] = {{0, 1, 2}, {1, 1, 1}, {2, 1, 2}};

  StatusCode::code_t l = static_cast<StatusCode::code_t>( default_value() );
  StatusCode::code_t r = static_cast<StatusCode::code_t>( rhs.default_value() );
  m_code               = OR[l][r];
  rhs.m_checked        = true;
  return *this;
}

/// Ternary AND operator
inline StatusCode operator&( StatusCode lhs, const StatusCode& rhs ) { return lhs &= rhs; }

/// Ternary OR operator
inline StatusCode operator|( StatusCode lhs, const StatusCode& rhs ) { return lhs |= rhs; }

/// Boolean AND assignment operator
inline bool& operator&=( bool& lhs, const StatusCode& sc ) { return lhs &= sc.isSuccess(); }

/// Boolean OR assignment operator
inline bool& operator|=( bool& lhs, const StatusCode& sc ) { return lhs |= sc.isSuccess(); }

#endif // GAUDIKERNEL_STATUSCODE_H
