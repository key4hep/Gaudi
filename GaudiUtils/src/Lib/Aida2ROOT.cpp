#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, since it comes from ROOT TMathBase.h
#pragma warning( disable : 2259 )
#endif
#ifdef WIN32
// Disable warning
//   warning C4996: 'sprintf': This function or variable may be unsafe.
// coming from TString.h
#pragma warning( disable : 4996 )
#endif
// ============================================================================
// Include files
// ============================================================================
#include <type_traits>
// ============================================================================
// AIDA
// ============================================================================
#include "GaudiKernel/Kernel.h"
//-- to avoid that the class macro trick affects them
/// @FIXME: AIDA interfaces visibility
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-macro"
#endif
#define class class GAUDI_API
#ifdef __clang__
#pragma clang diagnostic pop
#endif
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

namespace
{
  template <typename Out, typename In>
  Out* a2r_cast( In* aida )
  {
    using Base =
        typename std::conditional<std::is_const<Out>::value, const Gaudi::HistogramBase, Gaudi::HistogramBase>::type;
    auto base = dynamic_cast<Base*>( aida );
    return base ? dynamic_cast<Out*>( base->representation() ) : nullptr;
  }
}
// ============================================================================
// get the underlying pointer for 1D-histogram
// ============================================================================
TH1D* Gaudi::Utils::Aida2ROOT::aida2root( AIDA::IHistogram1D* aida ) { return a2r_cast<TH1D>( aida ); }
const TH1D* Gaudi::Utils::Aida2ROOT::aida2root( const AIDA::IHistogram1D* aida )
{
  return a2r_cast<const TH1D>( aida );
}
// ============================================================================
// get the underlying pointer for 2D-histogram
// ============================================================================
TH2D* Gaudi::Utils::Aida2ROOT::aida2root( AIDA::IHistogram2D* aida ) { return a2r_cast<TH2D>( aida ); }
const TH2D* Gaudi::Utils::Aida2ROOT::aida2root( const AIDA::IHistogram2D* aida )
{
  return a2r_cast<const TH2D>( aida );
}
// ============================================================================
// get the underlying pointer for 3D-histogram
// ============================================================================
TH3D* Gaudi::Utils::Aida2ROOT::aida2root( AIDA::IHistogram3D* aida ) { return a2r_cast<TH3D>( aida ); }
const TH3D* Gaudi::Utils::Aida2ROOT::aida2root( const AIDA::IHistogram3D* aida )
{
  return a2r_cast<const TH3D>( aida );
}
// ============================================================================
// get the underlying pointer for 1D-profile
// ============================================================================
TProfile* Gaudi::Utils::Aida2ROOT::aida2root( AIDA::IProfile1D* aida ) { return a2r_cast<TProfile>( aida ); }
const TProfile* Gaudi::Utils::Aida2ROOT::aida2root( const AIDA::IProfile1D* aida )
{
  return a2r_cast<const TProfile>( aida );
}
// ============================================================================
// get the underlying pointer for 2D-profile
// ============================================================================
TProfile2D* Gaudi::Utils::Aida2ROOT::aida2root( AIDA::IProfile2D* aida ) { return a2r_cast<TProfile2D>( aida ); }
const TProfile2D* Gaudi::Utils::Aida2ROOT::aida2root( const AIDA::IProfile2D* aida )
{
  return a2r_cast<const TProfile2D>( aida );
}
// ============================================================================
// get root representation for other cases
// ============================================================================
TObject* Gaudi::Utils::Aida2ROOT::aida2root( AIDA::IHistogram* aida ) { return a2r_cast<TObject>( aida ); }
const TObject* Gaudi::Utils::Aida2ROOT::aida2root( const AIDA::IHistogram* aida )
{
  return a2r_cast<const TObject>( aida );
}
// ============================================================================
// The END
// ============================================================================
