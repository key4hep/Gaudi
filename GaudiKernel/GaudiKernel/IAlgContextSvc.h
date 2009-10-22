// $Id: IAlgContextSvc.h,v 1.2 2007/05/24 13:48:50 hmd Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.2 $
// ============================================================================
#ifndef GAUDIKERNEL_IALGCONTEXTSVC_H
#define GAUDIKERNEL_IALGCONTEXTSVC_H
// ============================================================================
// Include files:
// ============================================================================
// STD & STL
// ============================================================================
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IInterface.h"
// ============================================================================
// Forward declarations
// ============================================================================
class IAlgorithm ;
// ============================================================================
/** @class IAlgContextSvc
 *  An abstract interface for Algorithm Context Service
 *  @author ATLAS Collaboration
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-03-07 (modified)
 */
class GAUDI_API IAlgContextSvc: virtual public IInterface
{
public:
  /// InterfaceID
  DeclareInterfaceID(IAlgContextSvc,3,0);
  /// the actual type of algorithm' stack
  typedef std::vector<IAlgorithm*>  Algorithms ;
public:
  /// set     the currently executing algorithm  ("push_back")
  virtual StatusCode     setCurrentAlg  ( IAlgorithm* a ) = 0 ;
  /// remove the algorithm                       ("pop_back")
  virtual StatusCode   unSetCurrentAlg  ( IAlgorithm* a ) = 0 ;
  /// accessor to current algorithm:
  virtual IAlgorithm*       currentAlg  () const = 0 ;
  /// get the stack of executed algorithms
  virtual const Algorithms& algorithms  () const = 0 ;
protected:
  /// virtual and protected desctructor
  virtual ~IAlgContextSvc() ;
} ;
// ============================================================================
namespace Gaudi
{
  namespace Utils
  {
    /** @class AlgContext
     *  Helper "sentry" class to automatize the safe register/unregister
     *  the algorithm's context
     *
     *  Typical explicit usage:
     *
     *  @code
     *
     *   StatusCode MyAlg::execute()
     *    {
     *       IAlgContextSvc* acs = ... ;
     *       // define the context
     *       Gaudi::Utils::AlgContext sentry ( this , acs ) ;
     *
     *       ...
     *
     *       return StatusCode::SUCCESS ;
     *    }
     *
     *  @endcode
     *
     *  Note: for the regular job the context is properly
     *  defined with the help of corresponding auditor
     *  AlgContextAuditor. This helper class is needed only
     *  if one needs to ensure that the algorithm must register
     *  its context independently on job/auditor configuration
     *
     *  @see AlgContextAuditor
     *  @author Vanya BELYAEV ibelyaev@phys.syr.edu
     *  @date   2007-03-07
     */
    class GAUDI_API AlgContext
    {
    public:
      /** constructor from the service and the algorithm
       *  Internally invokes IAlgContextSvc::setCurrentAlg
       *  @see IAlgorithm
       *  @see IAlgContextSvc
       *  @param svc pointer to algorithm context service
       *  @param alg pointer to the current algorithm
       */
      AlgContext
      ( IAlgContextSvc* svc ,
        IAlgorithm*     alg ) ;
      /** constructor from the algorithm and the service
       *  Internally invokes IAlgContextSvc::setCurrentAlg
       *  @see IAlgorithm
       *  @see IAlgContextSvc
       *  @param alg pointer to the current algorithm
       *  @param svc pointer to algorithm context service
       */
      AlgContext
      ( IAlgorithm*     alg ,
        IAlgContextSvc* svc ) ;
      /** destructor
       *  Internally invokes IAlgContextSvc::unSetCurrentAlg
       *  @see IAlgorithm
       *  @see IAlgContextSvc
       */
      ~AlgContext() ;
    private:
      // default constructor is disabled
      AlgContext()                                     ; ///< no default constructor!
      // copy constructor is disabled
      AlgContext           ( const AlgContext& right ) ; ///< no copy!
      // assignement operator is disabled
      AlgContext& operator=( const AlgContext& right ) ; ///< no assignement!
    private:
      IAlgContextSvc* m_svc ;
      IAlgorithm*     m_alg ;
    };
  }  // end of namespace Gaudi::Utils
} // end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_IALGCONTEXTSVC_H
// ============================================================================


