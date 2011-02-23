// $Id: GetAlgs.h,v 1.1 2007/09/25 16:12:41 marcocle Exp $
// ============================================================================
#ifndef GAUDIALG_GETALGS_H
#define GAUDIALG_GETALGS_H 1
// ============================================================================
// Include files
// ============================================================================
#include "GaudiKernel/Kernel.h"
// ============================================================================
// forward declarations
// ============================================================================
class IAlgContextSvc ;
class GaudiAlgorithm ;
class GaudiHistoAlg  ;
class GaudiTupleAlg  ;
class GaudiSequencer ;
class Sequencer      ;
// ============================================================================
namespace Gaudi
{
  namespace Utils
  {
    // ========================================================================
    /** simple function to extract the last active
     *   GaudiAlgorithm from the context
     *
     *  @code
     *
     *  // get the context service:
     *  const IAlgContextSvc* svc = ... ;
     *
     *  GaudiAlgorithm* ga = getGaudiAlg ( svc ) ;
     *
     *  @endcode
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-07
     */
    GAUDI_API GaudiAlgorithm* getGaudiAlg       ( const IAlgContextSvc* svc ) ;
    // ========================================================================
    /** simple function to extract the last active
     *   GaudiHistoAlg from the context
     *
     *  @code
     *
     *  // get the context service:
     *  const IAlgContextSvc* svc = ... ;
     *
     *  GaudiHistoAlg* ha = getHistoAlg ( svc ) ;
     *
     *  @endcode
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-07
     */
    GAUDI_API GaudiHistoAlg*  getHistoAlg       ( const IAlgContextSvc* svc ) ;
    // ========================================================================
    /** simple function to extract the last active
     *   GaudiTupleAlg from the context
     *
     *  @code
     *
     *  // get the context service:
     *  const IAlgContextSvc* svc = ... ;
     *
     *  GaudiTupleAlg* ta = getTupleAlg ( svc ) ;
     *
     *  @endcode
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-07
     */
    GAUDI_API GaudiTupleAlg*  getTupleAlg       ( const IAlgContextSvc* svc ) ;
    // ========================================================================
    /** simple function to extract the last active
     *   GaudiSequencer from the context
     *
     *  @code
     *
     *  // get the context service:
     *  const IAlgContextSvc* svc = ... ;
     *
     *  GaudiSequencer* sa = getGaudiSequencer ( svc ) ;
     *
     *  @endcode
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-07
     */
    GAUDI_API GaudiSequencer* getGaudiSequencer ( const IAlgContextSvc* svc ) ;
    // ========================================================================
    /** simple function to extract the last active
     *   Sequencer from the context
     *
     *  @code
     *
     *  // get the context service:
     *  const IAlgContextSvc* svc = ... ;
     *
     *  Sequencer* sa = getSequencerAlg ( svc ) ;
     *
     *  @endcode
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-07
     */
    GAUDI_API Sequencer*      getSequencerAlg   ( const IAlgContextSvc* svc ) ;
    // ========================================================================
    /** simple function to extract the last active
     *   "Sequencer" () GaudiSequencer or simple Sequencer)
     *   from the context
     *
     *  @code
     *
     *  // get the context service:
     *  const IAlgContextSvc* svc = ... ;
     *
     *  IAlgorithm* a = getSequencer ( svc ) ;
     *
     *  @endcode
     *
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-09-07
     */
    GAUDI_API IAlgorithm*     getSequencer      ( const IAlgContextSvc* svc ) ;
    // ========================================================================
  } // end of namespace Gaudi::Utils
} // end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GETALGS_H
// ============================================================================
