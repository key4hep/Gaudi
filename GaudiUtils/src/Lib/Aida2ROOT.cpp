// $Id: Aida2ROOT.cpp,v 1.3 2007/08/06 14:49:36 hmd Exp $
#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, since it comes from ROOT TMathBase.h
#pragma warning(disable:2259)
#endif
#ifdef WIN32
// Disable warning
//   warning C4996: 'sprintf': This function or variable may be unsafe.
// coming from TString.h
#pragma warning(disable:4996)
#endif
// ============================================================================
// Include files
// ============================================================================
// AIDA
// ============================================================================
#include "GaudiKernel/Kernel.h"
//-- to avoid that the class macro trick affects them
#include <string>
#include <vector>
/// @FIXME: AIDA interfaces visibility
#define class class GAUDI_API
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"
#include "AIDA/IHistogram3D.h"
#include "AIDA/IProfile1D.h"
#include "AIDA/IProfile2D.h"
#undef class
// ============================================================================
// ROOT
// ============================================================================
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TProfile.h"
#include "TProfile2D.h"
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/HistogramBase.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiUtils/Aida2ROOT.h"
// ============================================================================
/** @file
 *  Implementation file for class Gaudi::Utils::Aida2ROOT
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date  2007-01-23
 */
// ============================================================================
// get the underlying pointer for 1D-histogram
// ============================================================================
TH1D* Gaudi::Utils::Aida2ROOT::aida2root ( AIDA::IHistogram1D* aida )
{
  if ( 0 == aida ) { return 0 ; }
  Gaudi::HistogramBase* base = dynamic_cast<Gaudi::HistogramBase*> ( aida ) ;
  if ( 0 == base ) { return 0 ; }
  return dynamic_cast<TH1D*>( base->representation() ) ;
}
// ============================================================================
// get the underlying pointer for 2D-histogram
// ============================================================================
TH2D* Gaudi::Utils::Aida2ROOT::aida2root ( AIDA::IHistogram2D* aida )
{
  if ( 0 == aida ) { return 0 ; }
  Gaudi::HistogramBase* base = dynamic_cast<Gaudi::HistogramBase*> ( aida ) ;
  if ( 0 == base ) { return 0 ; }
  return dynamic_cast<TH2D*>( base->representation() ) ;
}
// ============================================================================
// get the underlying pointer for 3D-histogram
// ============================================================================
TH3D* Gaudi::Utils::Aida2ROOT::aida2root ( AIDA::IHistogram3D* aida )
{
  if ( 0 == aida ) { return 0 ; }
  Gaudi::HistogramBase* base = dynamic_cast<Gaudi::HistogramBase*> ( aida ) ;
  if ( 0 == base ) { return 0 ; }
  return dynamic_cast<TH3D*>( base->representation() ) ;
}
// ============================================================================
// get the underlying pointer for 1D-profile
// ============================================================================
TProfile* Gaudi::Utils::Aida2ROOT::aida2root ( AIDA::IProfile1D*   aida )
{
  if ( 0 == aida ) { return 0 ; }
  Gaudi::HistogramBase* base = dynamic_cast<Gaudi::HistogramBase*> ( aida ) ;
  if ( 0 == base ) { return 0 ; }
  return dynamic_cast<TProfile*>( base->representation() ) ;
}
// ============================================================================
// get the underlying pointer for 2D-profile
// ============================================================================
TProfile2D* Gaudi::Utils::Aida2ROOT::aida2root ( AIDA::IProfile2D*   aida )
{
  if ( 0 == aida ) { return 0 ; }
  Gaudi::HistogramBase* base = dynamic_cast<Gaudi::HistogramBase*> ( aida ) ;
  if ( 0 == base ) { return 0 ; }
  return dynamic_cast<TProfile2D*>( base->representation() ) ;
}
// ============================================================================
// get root representation for other cases
// ============================================================================
TObject* Gaudi::Utils::Aida2ROOT::aida2root ( AIDA::IHistogram*   aida )
{
  if ( 0 == aida ) { return 0 ; }
  Gaudi::HistogramBase* base = dynamic_cast<Gaudi::HistogramBase*> ( aida ) ;
  if ( 0 == base ) { return 0 ; }
  return base->representation() ;
}
// ============================================================================
// The END
// ============================================================================
