// $Id: Fill.cpp,v 1.1 2007/10/02 14:53:48 marcocle Exp $
// ============================================================================
// Include files 
// ============================================================================
// STD& STL 
// ============================================================================
#include <string>
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/Fill.h"
// ============================================================================
// AIDA
// ============================================================================
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"
#include "AIDA/IHistogram3D.h"
#include "AIDA/IProfile1D.h"
#include "AIDA/IProfile2D.h"
// ============================================================================
/** @file
 *  Implementation file for function form namespace Gaudi::Utils::Histos
 *  @date 2007-10-02 
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 */
// ============================================================================
/*  simple function to fill AIDA::IHistogram1D objects 
 *  @see AIDA::IHistogram1D 
 *  @param histo pointer to the histogram 
 *  @param value value to be added to the histogram
 *  @param weight the "weight" assciated with this entry
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-10-02
 */
// ============================================================================
void Gaudi::Utils::Histos::fill 
( AIDA::IHistogram1D* histo  , 
  const double        value  ,
  const double        weight ) 
{
  if ( 0 != histo ) { histo->fill ( value , weight ) ; }
}
// ============================================================================
/* simple function to fill AIDA::IHistogram2D objects 
 *  @see AIDA::IHistogram2D 
 *  @param histo pointer to the histogram 
 *  @param valueX value to be added to the histogram
 *  @param valueY value to be added to the histogram
 *  @param weight the "weight" assciated with this entry
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-10-02
 */
// ============================================================================
void Gaudi::Utils::Histos::fill 
( AIDA::IHistogram2D* histo  , 
  const double        valueX ,
  const double        valueY ,
  const double        weight ) 
{
  if ( 0 != histo ) { histo -> fill ( valueX , valueY , weight ) ; }
}
// ============================================================================
/*  simple function to fill AIDA::IHistogram3D objects 
 *  @see AIDA::IHistogram3D 
 *  @param histo pointer to the histogram 
 *  @param valueX value to be added to the histogram
 *  @param valueY value to be added to the histogram
 *  @param valueZ value to be added to the histogram
 *  @param weight the "weight" assciated with this entry
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-10-02
 */
// ============================================================================
void Gaudi::Utils::Histos::fill 
( AIDA::IHistogram3D* histo  , 
  const double        valueX ,
  const double        valueY ,
  const double        valueZ ,
  const double        weight ) 
{
  if ( 0 != histo ) { histo -> fill ( valueX , valueY , valueZ , weight ) ; }
}
// ============================================================================
/*  simple function to fill AIDA::IProfile1D objects 
 *  @see AIDA::IProfile1D 
 *  @param histo pointer to the histogram 
 *  @param valueX value to be added to the histogram
 *  @param valueY value to be added to the histogram
 *  @param weight the "weight" assciated with this entry
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-10-02
 */
// ============================================================================
void Gaudi::Utils::Histos::fill 
( AIDA::IProfile1D*   histo  , 
  const double        valueX ,
  const double        valueY ,
  const double        weight ) 
{
  if ( 0 != histo ) { histo -> fill ( valueX , valueY , weight ) ; }
}
// ============================================================================
/*  simple function to fill AIDA::IProfile2D objects 
 *  @see AIDA::IProfile2D 
 *  @param histo pointer to the histogram 
 *  @param valueX value to be added to the histogram
 *  @param valueY value to be added to the histogram
 *  @param valueZ value to be added to the histogram
 *  @param weight the "weight" assciated with this entry
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-10-02
 */
// ============================================================================
void Gaudi::Utils::Histos::fill 
( AIDA::IProfile2D*   histo  , 
  const double        valueX ,
  const double        valueY ,
  const double        valueZ ,
  const double        weight ) 
{
  if ( 0 != histo ) { histo -> fill ( valueX , valueY , valueZ , weight ) ; }
}
// ============================================================================
namespace 
{
  inline 
  std::string htitle_ 
  ( const AIDA::IBaseHistogram* histo , 
    const std::string&          title ) 
  {
    return 
      ( 0 != histo && title.empty() ) ? histo->title() : title ;
  }
}
// ======================================================================
// get the title
// ======================================================================
std::string Gaudi::Utils::Histos::htitle 
( const AIDA::IBaseHistogram* histo , 
  const std::string&          title ) 
{ return htitle_ ( histo , title ) ; }
// ======================================================================
// get the title
// ======================================================================
std::string Gaudi::Utils::Histos::htitle 
( const AIDA::IHistogram*     histo      , 
  const std::string&          title ) 
{ return htitle_ ( histo , title ) ; }
// ======================================================================
// get the title
// ======================================================================
std::string Gaudi::Utils::Histos::htitle 
( const AIDA::IHistogram1D*   histo      , 
  const std::string&          title ) 
{ return htitle_ ( histo , title ) ; }
// ======================================================================
// get the title
// ======================================================================
std::string Gaudi::Utils::Histos::htitle 
( const AIDA::IHistogram2D*   histo      , 
  const std::string&          title ) 
{ return htitle_ ( histo , title ) ; }
// ======================================================================
// get the title
// ======================================================================
std::string Gaudi::Utils::Histos::htitle 
( const AIDA::IHistogram3D*   histo      , 
  const std::string&          title ) 
{ return htitle_ ( histo , title ) ; }
// ======================================================================
// get the title
// ======================================================================
std::string Gaudi::Utils::Histos::htitle 
( const AIDA::IProfile*       histo      , 
  const std::string&          title ) 
{ return htitle_ ( histo , title ) ; }
// ======================================================================
// get the title
// ======================================================================
std::string Gaudi::Utils::Histos::htitle 
( const AIDA::IProfile1D*     histo      , 
  const std::string&          title ) 
{ return htitle_ ( histo , title ) ; }
// ======================================================================
// get the title
// ======================================================================
std::string Gaudi::Utils::Histos::htitle 
( const AIDA::IProfile2D*     histo      , 
  const std::string&          title ) 
{ return htitle_ ( histo , title ) ; }
// ======================================================================
AIDA::IBaseHistogram* 
Gaudi::Utils::Histos::toBase 
( AIDA::IHistogram1D* histo ) { return histo ; }
// ======================================================================
AIDA::IBaseHistogram* 
Gaudi::Utils::Histos::toBase 
( AIDA::IHistogram2D* histo ) { return histo ; }
// ======================================================================
AIDA::IBaseHistogram* 
Gaudi::Utils::Histos::toBase 
( AIDA::IHistogram3D* histo ) { return histo ; }
// ======================================================================
AIDA::IBaseHistogram* 
Gaudi::Utils::Histos::toBase 
( AIDA::IProfile1D*   histo ) { return histo ; }
// ======================================================================
AIDA::IBaseHistogram* 
Gaudi::Utils::Histos::toBase
( AIDA::IProfile2D*   histo ) { return histo ; }
// ======================================================================

// ============================================================================
// The END 
// ============================================================================
