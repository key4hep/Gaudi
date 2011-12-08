#ifndef GAUDISVC_EXCEPTIONSVC_H
#define GAUDISVC_EXCEPTIONSVC_H
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IExceptionSvc.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/MsgStream.h"
// ============================================================================
/** @class ExceptionSvc
 *  Simple implementation of IExceptionSvc abstract interface
 *  @author (1) ATLAS collaboration
 *  @author (2) modified by Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-03-08
 */
// ============================================================================
class ExceptionSvc: public extends1<Service, IExceptionSvc> {
  friend class SvcFactory<ExceptionSvc> ;
public:
  /// Handle caught GaudiExceptions
  virtual StatusCode handle
  ( const INamedInterface& o ,
    const GaudiException&  e ) const     ; ///< Handle caught exceptions
  /// Handle caught std::exceptions
  virtual StatusCode handle
  ( const INamedInterface& o ,
    const std::exception & e ) const     ; ///< Handle caught exceptions
  /// Handle caught (unknown)exceptions
  virtual StatusCode handle
  ( const INamedInterface& o ) const     ; ///< Handle caught exceptions
  /// Handle errors
  virtual StatusCode handleErr
  ( const INamedInterface& o ,
    const StatusCode&      s ) const     ; ///< Handle errors
public:
  /// initialize the service
  virtual StatusCode initialize   () ;
  /// finalize the service
  virtual StatusCode finalize     () ;
public:
  /** standard constructor
   *  @param name service instance name
   *  @param pSvc pointer to Service Locator
   */
  ExceptionSvc
  ( const std::string& name ,
    ISvcLocator*       svc  ) ;
  /// Destructor.
  virtual ~ExceptionSvc();
private:
  // default constructor is disabled
  ExceptionSvc () ; ///< no default constructor
  // copy constructor is disabled
  ExceptionSvc ( const ExceptionSvc& ) ; ///< no copy constructor
  // assignment operator is disabled
  ExceptionSvc& operator=( const ExceptionSvc& ) ; ///< no assignement
  // process exceptions
  virtual StatusCode process  ( const INamedInterface& o ) const ;
private:

  enum Policy { ALL, NONE };
  enum ReturnState { SUCCESS, FAILURE, RECOVERABLE, RETHROW, DEFAULT };

  Policy m_mode_exc, m_mode_err;
  StringProperty m_mode_exc_s, m_mode_err_s;
  std::map<std::string,ReturnState> m_retCodesExc, m_retCodesErr;

  mutable MsgStream m_log;

};

// ============================================================================
#endif // GAUDISVC_EXCEPTIONSVC_H
// ============================================================================
// The END
// ============================================================================
