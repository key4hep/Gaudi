// $Id: GaudiHistoAlg.cpp,v 1.8 2008/01/17 15:06:06 marcocle Exp $
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiHistoAlg.h"
// ============================================================================

// ============================================================================
/** @file GaudiHistoAlg.cpp
 *
 *  Implementation file for class : GaudiHistoAlg
 *
 *  @author Vanya  BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date 2003-12-11
 */
// ============================================================================

// ============================================================================
// Force creation of templated class
#include "GaudiAlg/GaudiHistos.icpp"
template class GaudiHistos<GaudiAlgorithm> ;
// ============================================================================

// ============================================================================
// Forward declaration of the actual constructors, to tell the compiler that
// they do exist and will be found at link time (warning C4661 in VC7.1)
// ============================================================================
template <>
GaudiHistos<GaudiAlgorithm>::GaudiHistos( const std::string& /* type */  ,
                                          const std::string& /* name */  ,
                                          const IInterface*  /* parent */ );
template <>
GaudiHistos<GaudiAlgorithm>::GaudiHistos( const std::string & name,
                                          ISvcLocator * pSvcLocator );
// ============================================================================

// ============================================================================
// Standard constructor
// ============================================================================
GaudiHistoAlg::GaudiHistoAlg ( const std::string& name,
                               ISvcLocator*       pSvc )
  : GaudiHistos<GaudiAlgorithm> ( name , pSvc ) { }
// ============================================================================

// ============================================================================
// destructor
// ============================================================================
GaudiHistoAlg::~GaudiHistoAlg() { }
// ============================================================================

// ============================================================================
// standard initialization method
// ============================================================================
StatusCode GaudiHistoAlg::initialize()
{
  // initialize the base class and return
  return GaudiHistos<GaudiAlgorithm>::initialize();
}
// ============================================================================

// ============================================================================
// standard finalization method
// ============================================================================
StatusCode GaudiHistoAlg::finalize()
{ 
  // finalize the base class and return
  return GaudiHistos<GaudiAlgorithm>::finalize();
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================
