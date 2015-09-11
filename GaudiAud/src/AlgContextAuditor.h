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
  virtual ~AlgContextAuditor    () = default;
  /// standard initialization, @see IAuditor
  virtual StatusCode initialize () ;
  /// standard finalization, @see IAuditor
  virtual StatusCode finalize   () ;
private:
  /// delete the default/copy constructor and assignment
  AlgContextAuditor () = delete;
  AlgContextAuditor ( const AlgContextAuditor& ) = delete;
  AlgContextAuditor& operator=( const AlgContextAuditor& ) = delete;
private:
  /// the pointer to Algorithm Context Service
  IAlgContextSvc* m_svc = nullptr;
} ;

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIAUD_ALGCONTEXTAUDITOR_H
// ============================================================================
