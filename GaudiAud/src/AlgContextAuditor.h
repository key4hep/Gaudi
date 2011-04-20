#ifndef GAUDIAUD_ALGCONTEXTAUDITOR_H
#define GAUDIAUD_ALGCONTEXTAUDITOR_H
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Auditor.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/SmartIF.h"
// ============================================================================
// Forward declarations
// ============================================================================
class IAlgContextSvc  ;
// ============================================================================
/** @class AlgContextAuditor
 *  Description:  Register/Unregister the AlgContext of each
 *  algorithm before entering the algorithm and after leaving it
 *  @author M. Shapiro, LBNL
 *  @author modified by Vanya BELYAEV ibelyaev@physics.syr.edu
 */
class AlgContextAuditor
  : public Auditor
{
public:
  // IAuditor implementation
  virtual void beforeInitialize ( INamedInterface*  a ) ;
  virtual void afterInitialize  ( INamedInterface*  a ) ;
  //
  virtual void beforeExecute    ( INamedInterface*  a ) ;
  virtual void afterExecute     ( INamedInterface*  a ,
                                  const StatusCode& s ) ;
  //
  virtual void beforeFinalize   ( INamedInterface*  a ) ;
  virtual void afterFinalize    ( INamedInterface*  a ) ;
public:
  /// standard constructor @see Auditor
  AlgContextAuditor
  ( const std::string& name ,
    ISvcLocator*       pSvc ) ;
  /// virtual destructor
  virtual ~AlgContextAuditor    () ;
  /// standard initialization, @see IAuditor
  virtual StatusCode initialize () ;
  /// standard finalization, @see IAuditor
  virtual StatusCode finalize   () ;
private:
  /// the default constructor is disabled
  AlgContextAuditor () ;
  /// copy constructor is disabled
  AlgContextAuditor ( const AlgContextAuditor& ) ;
  /// assignment operator is disabled
  AlgContextAuditor& operator=( const AlgContextAuditor& ) ;
private:
  /// the pointer to Algorithm Context Service
  IAlgContextSvc* m_svc ;
} ;

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIAUD_ALGCONTEXTAUDITOR_H
// ============================================================================
