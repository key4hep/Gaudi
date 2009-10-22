// $Id: GaudiHistoTool.cpp,v 1.8 2007/11/20 13:01:06 marcocle Exp $
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiHistoTool.h"
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IHistogramSvc.h"
// ============================================================================

// ============================================================================
/** @file GaudiHistoTool.cpp
 *
 *  Implementation file for class : GaudiHistoTool
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date 2004-06-28
 */
// ============================================================================

// ============================================================================
// Force creation of templated class
#include "GaudiAlg/GaudiHistos.icpp"
template class GaudiHistos<GaudiTool> ;
// ============================================================================

// ============================================================================
// Forward declaration of the actual constructors, to tell the compiler that
// they do exist and will be found at link time (warning C4661 in VC7.1)
// ============================================================================
template <>
GaudiHistos<GaudiTool>::GaudiHistos( const std::string & /* name */,
                                     ISvcLocator * /* pSvcLocator */ );
template <>
GaudiHistos<GaudiTool>::GaudiHistos( const std::string& type   ,
                                     const std::string& name   ,
                                     const IInterface*  parent );
// ============================================================================

// ============================================================================
// Standard constructor
// ============================================================================
GaudiHistoTool::GaudiHistoTool ( const std::string& type   ,
                                 const std::string& name   ,
                                 const IInterface*  parent )
  : GaudiHistos<GaudiTool> ( type , name , parent ) { }
// ============================================================================

// ============================================================================
// destructor
// ============================================================================
GaudiHistoTool::~GaudiHistoTool() { }
// ============================================================================

// ============================================================================
// standard initialization method
// ============================================================================
StatusCode GaudiHistoTool::initialize()
{
  // initialize the base class and return
  return GaudiHistos<GaudiTool>::initialize();
}
// ============================================================================

// ============================================================================
// standard finalization method
// ============================================================================
StatusCode GaudiHistoTool::finalize()
{
  // finalize the base class and return
  return GaudiHistos<GaudiTool>::finalize();
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================
