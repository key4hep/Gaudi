// $Id: AlgContextSvc.h,v 1.4 2007/05/24 13:49:47 hmd Exp $
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
#include "GaudiKernel/Service.h"
#include <boost/thread.hpp>
// ============================================================================
// Forward declarations
// ============================================================================
// ============================================================================
/** @class AlgContexSvc
 *  Simple implementation of interface IAlgContextSvc
 *  for Algorithm Context Service
 *  @author ATLAS Collaboration
 *  @author modified by Vanya BELYAEV ibelyaev@physics.sye.edu
 *  @author incident listening  removed by Benedikt Hegner
 *  @date 2007-03-07 (modified)
 */
class AlgContextSvc: public extends1<Service, IAlgContextSvc>
{
public:
  /// set     the currently executing algorithm  ("push_back") @see IAlgContextSvc
  virtual StatusCode     setCurrentAlg  ( IAlgorithm* a ) ;
  /// remove the algorithm                       ("pop_back") @see IAlgContextSvc
  virtual StatusCode   unSetCurrentAlg  ( IAlgorithm* a ) ;
  /// accessor to current algorithm: @see IAlgContextSvc
  virtual IAlgorithm*       currentAlg  () const ;
  /// get the stack of executed algorithms @see IAlgContextSvc
  virtual const IAlgContextSvc::Algorithms& algorithms  () const
  { return *m_algorithms ; }
public:
  /// standard initialization of the service @see IService
  virtual StatusCode initialize () ;
  /// standard finalization  of the service  @see IService
  virtual StatusCode finalize   () ;
public:
  /// Standard Constructor @see Service
  AlgContextSvc
  ( const std::string& name ,
    ISvcLocator*       svc  ) ;
  /// Standard Destructor
  virtual ~AlgContextSvc();
private:
  // default constructor is disabled
  AlgContextSvc () ; ///< no default constructor
  // copy constructor is disabled
  AlgContextSvc ( const AlgContextSvc& ); ///< no copy constructor
  // assignment operator is disabled
  AlgContextSvc& operator=( const AlgContextSvc& ); ///< no assignment
private:
  // the stack of current algorithms
  boost::thread_specific_ptr<IAlgContextSvc::Algorithms> m_algorithms; ///< the stack of current algorithms
} ;

// ============================================================================
// The END
// ============================================================================
#endif // GAUDISVC_ALGCONTEXTSVC_H
// ============================================================================

