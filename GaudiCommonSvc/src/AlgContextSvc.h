// ============================================================================
#ifndef GAUDISVC_ALGCONTEXTSVC_H
#define GAUDISVC_ALGCONTEXTSVC_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/Service.h"
// ============================================================================
// Forward declarations
// ============================================================================
class IIncidentSvc ;
// ============================================================================
/** @class AlgContexSvc
 *  Simple implementation of interface IAlgContextSvc
 *  for Algorithm Context Service
 *  @author ATLAS Collaboration
 *  @author modified by Vanya BELYAEV ibelyaev@physics.sye.edu
 *  @date 2007-03-07 (modified)
 */
class AlgContextSvc: public extends2<Service, IAlgContextSvc, IIncidentListener>
{
public:
  /// set     the currently executing algorithm  ("push_back") @see IAlgContextSvc
  StatusCode     setCurrentAlg  ( IAlgorithm* a )  override ;
  /// remove the algorithm                       ("pop_back") @see IAlgContextSvc
  StatusCode   unSetCurrentAlg  ( IAlgorithm* a )  override ;
  /// accessor to current algorithm: @see IAlgContextSvc
  IAlgorithm*       currentAlg  () const  override ;
  /// get the stack of executed algorithms @see IAlgContextSvc
  const IAlgContextSvc::Algorithms& algorithms  () const override
  { return m_algorithms ; }
public:
  /// handle incident @see IIncidentListener
  void handle ( const Incident& ) override;
public:
  /// standard initialization of the service @see IService
  StatusCode initialize () override;
  /// standard finalization  of the service  @see IService
  StatusCode finalize   () override;
public:
  /// Standard Constructor @see Service
  AlgContextSvc
  ( const std::string& name ,
    ISvcLocator*       svc  ) ;
  /// Standard Destructor
  ~AlgContextSvc() override = default;
private:
  // default/copy constructor & asignment are deleted
  AlgContextSvc () = delete  ;
  AlgContextSvc ( const AlgContextSvc& ) = delete ;
  AlgContextSvc& operator=( const AlgContextSvc& ) = delete;
private:
  // the stack of current algorithms
  IAlgContextSvc::Algorithms m_algorithms ; ///< the stack of current algorithms
  // pointer to Incident Service
  SmartIF<IIncidentSvc>     m_inc      = nullptr  ; ///< pointer to Incident Service
  // flag to perform more checking
  bool                       m_check    = true   ;
} ;

// ============================================================================
// The END
// ============================================================================
#endif // GAUDISVC_ALGCONTEXTSVC_H
// ============================================================================

