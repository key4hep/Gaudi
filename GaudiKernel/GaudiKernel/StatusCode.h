#ifndef GAUDIKERNEL_STATUSCODE_H
#define GAUDIKERNEL_STATUSCODE_H

#include <ostream>

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/IssueSeverity.h"

#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#include <memory>
#else
#include "boost/shared_ptr.hpp"
#endif

/**
 * @class StatusCode StatusCode.h GaudiKernel/StatusCode.h
 *
 * This class is used for returning status codes from appropriate routines.
 *
 * @author Iain Last
 * @author Pere Mato
 * @author Sebastien Ponce
 */

class IMessageSvc;
class IStatusCodeSvc;

class IgnoreError {};

class StatusCode {
public:
  enum {
    FAILURE = 0,
    SUCCESS = 1,
    RECOVERABLE = 2
  };

  /// Constructor.
  StatusCode():
    d_code(SUCCESS), m_checked(false), m_severity() {}
  StatusCode( unsigned long code, const IssueSeverity& sev ):
    d_code(code),m_checked(false), m_severity() {
    try { // ensure that we do not throw even if we cannot copy the severity
#if (defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L) && ! defined(__GCCXML__)
      m_severity = std::make_shared<IssueSeverity>(sev);
#else
      m_severity = SeverityPtr(new IssueSeverity(sev));
#endif
    }
    catch (...) {}
  }
  StatusCode( unsigned long code, bool checked = false ):
    d_code(code),m_checked(checked), m_severity() {}

  StatusCode( const StatusCode& rhs ):
    d_code(rhs.d_code), m_checked(rhs.m_checked),
    m_severity(rhs.m_severity)
    { rhs.m_checked = true; }

#ifndef __GCCXML__
  /// Move constructor.
  StatusCode( StatusCode&& rhs ):
    d_code(rhs.d_code), m_checked(rhs.m_checked),
    m_severity( std::move(rhs.m_severity) )
  { rhs.m_checked = true; }
#endif

  /// Destructor.
  ~StatusCode()
  { if (UNLIKELY(s_checking)) check(); }

  /** Test for a status code of SUCCESS.
   * N.B. This is the only case where a function has succeeded.
   */
  bool isSuccess() const {
    m_checked = true;
    return (d_code == SUCCESS);
  }

  /** Test for a status code of FAILURE.
   * N.B. This is a specific type of failure where there aren't any more
   * appropriate status codes. To test for any failure use :
   * if ( !StatusCode.isSuccess() ) ...
   */
  bool isFailure() const { return !isSuccess(); }
  bool isRecoverable() const {
    m_checked = true;
    return (d_code == RECOVERABLE);
  }

  /// Get the status code by value.
  unsigned long getCode() const{
    m_checked = true;
    return d_code;
  }

  /// Set the status code by value.
  void setCode(unsigned long value)  {
    m_checked = false;
    d_code = value;
  }

  /// Ignore the checking code;
  void setChecked() const{
    m_checked = true;
  }
  void ignore() const { setChecked(); }

  /// Cast operator.
  operator unsigned long() const { return getCode(); }

  /// Severity
  GAUDI_API const IssueSeverity& severity() const;

  /// Assignment operator.
  StatusCode& operator=(unsigned long value) {
    setCode(value);
    return *this;
  }
  StatusCode& operator=(const StatusCode& rhs){
    if (this == &rhs) return *this; // Protection against self-assignment
    d_code = rhs.d_code;
    m_checked = rhs.m_checked;
    rhs.m_checked = true;
    m_severity = rhs.m_severity;
    return *this;
  }

  /// Comparison operator.
  friend bool operator< ( const StatusCode& a, const StatusCode& b ) {
    return a.d_code < b.d_code;
  }

  /// Comparison operator.
  friend bool operator> ( const StatusCode& a, const StatusCode& b ) {
    return a.d_code > b.d_code;
  }

#ifndef _WIN32
  operator IgnoreError() const {
    m_checked = true;
    return IgnoreError();
  }
#endif

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
    ScopedDisableChecking(): m_enabled(StatusCode::checkingEnabled()) {
      if (m_enabled) StatusCode::disableChecking();
    }
    ~ScopedDisableChecking() {
      if (m_enabled) StatusCode::enableChecking();
    }
  };

protected:
  /// The status code.
  unsigned long   d_code;      ///< The status code
  mutable bool    m_checked;   ///< If the Status code has been checked
#if defined(__GCCXML__)
  // This is because GCCXML needs to see something that is not too in conflict with
  // boost or std
  typedef IssueSeverity* SeverityPtr;
#elif defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
  typedef std::shared_ptr<IssueSeverity> SeverityPtr;
#else
  typedef boost::shared_ptr<IssueSeverity> SeverityPtr;
#endif
  SeverityPtr     m_severity;  ///< Pointer to a IssueSeverity

  static bool     s_checking;  ///< Global flag to control if StatusCode need to be checked

private:
   void check();
};

inline std::ostream& operator<< ( std::ostream& s , const StatusCode& sc )
{
  if ( sc.isSuccess() ) { return s << "SUCCESS" ; }
  else if ( sc.isRecoverable() ) { return s << "RECOVERABLE" ; }
  s << "FAILURE" ;
  if ( StatusCode::FAILURE != sc.getCode() )
    { s << "(" << sc.getCode() << ")" ;}
  return s ;
}

#endif  // GAUDIKERNEL_STATUSCODES_H



