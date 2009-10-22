// $Id: HistoDef.cpp,v 1.1 2007/09/26 16:13:42 marcocle Exp $
// ============================================================================
// Include files 
// ============================================================================
// ST D& STL 
// ============================================================================
#include <ostream>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/HistoDef.h"
#include "GaudiKernel/ToStream.h"
// ============================================================================
/** @file
 *  The implementation file for class Gaudi::Histo1DDef and related functions 
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2007-09-17
 */
// ============================================================================
/*  full constructor from edges, #bins and the title 
 *  @param low  the low  edge of the histogram 
 *  @param high the high edge of the histogram 
 *  @param bins number of bins 
 *  @param title the historgam title 
 */
// ============================================================================
Gaudi::Histo1DDef::Histo1DDef
( const double       low   ,
  const double       high  ,
  const int          bins  , 
  const std::string& title )
  : m_title ( title ) 
  , m_low   ( low   ) 
  , m_high  ( high  ) 
  , m_bins  ( bins  )
{}
// ============================================================================
/*  full constructor from edges, #bins and the title 
 *  @param title the historgam title 
 *  @param low  the low  edge of the histogram 
 *  @param high the high edge of the histogram 
 *  @param bins number of bins 
 */
// ============================================================================
Gaudi::Histo1DDef::Histo1DDef
( const std::string& title , 
  const double       low   ,
  const double       high  ,      
  const int          bins  ) 
  : m_title ( title ) 
  , m_low   ( low   ) 
  , m_high  ( high  ) 
  , m_bins  ( bins  )
{}
// ============================================================================
// destructor 
// ============================================================================
Gaudi::Histo1DDef::~Histo1DDef(){}
// ============================================================================
// printout of the histogram definition 
// ============================================================================
std::ostream& Gaudi::Histo1DDef::fillStream( std::ostream& o ) const 
{
  return o << "(" << Gaudi::Utils::toString ( title () )  
           << "," << lowEdge  () 
           << "," << highEdge () 
           << "," << bins     ()  << ")" ;
}
// ============================================================================
// ordering operator (to please BoundedVerifier) 
// ============================================================================
bool Gaudi::Histo1DDef::operator< ( const Gaudi::Histo1DDef& right ) const 
{
  return 
    this      ==  &right             ? false : 
    title    () <  right.title    () ? true  :
    title    () >  right.title    () ? false :
    lowEdge  () <  right.lowEdge  () ? true  :
    lowEdge  () >  right.lowEdge  () ? false :
    highEdge () <  right.highEdge () ? true  :
    highEdge () >  right.highEdge () ? false : bins () < right.bins () ;
}
// ============================================================================
// equality operator 
// ============================================================================
bool Gaudi::Histo1DDef::operator==( const Gaudi::Histo1DDef& right ) const 
{
  return ( this == &right ) || 
    ( title    () == right.title    () && 
      lowEdge  () == right.lowEdge  () && 
      highEdge () == right.highEdge () && 
      bins     () == right.bins     () );
}
// ============================================================================
// non-equality 
// ============================================================================
bool Gaudi::Histo1DDef::operator!=( const Gaudi::Histo1DDef& right ) const 
{ return  !( *this == right ) ; }
// ============================================================================
// the streamer operator for class Gaudi::Histo1DDef 
// ============================================================================
std::ostream& operator<<( std::ostream& o , const Gaudi::Histo1DDef& histo ) 
{ return histo.fillStream ( o ) ; }
// ============================================================================





// ============================================================================
/*  helper function to book 1D-histogram
 *  @param svc pointer to Histogram Service 
 *  @param path full path in Histogram Data Store 
 *  @param hist histogram desctriprion
 */ 
// ============================================================================
AIDA::IHistogram1D* 
Gaudi::Histos::book
( IHistogramSvc*           svc  , 
  const std::string&       path ,
  const Gaudi::Histo1DDef& hist ) 
{
  if ( 0 == svc ) { return 0 ; }
  return svc -> book 
    ( path , 
      hist.title() , hist.bins() , hist.lowEdge() , hist.lowEdge() ) ;
}
// ============================================================================
/*  helper function to book 1D-histogram
 *  @param svc pointer to Histogram Service 
 *  @param dir  directory path in Histogram Data Store 
 *  @param id  historgam identifier 
 *  @param hist histogram desctriprion
 */ 
// ============================================================================
AIDA::IHistogram1D* 
Gaudi::Histos::book
( IHistogramSvc*           svc  , 
  const std::string&       dir  ,
  const std::string&       id   ,
  const Gaudi::Histo1DDef& hist ) 
{
  if ( 0 == svc ) { return 0 ; }
  return svc -> book 
    ( dir , id  , 
      hist.title() , hist.bins() , hist.lowEdge() , hist.lowEdge() ) ;  
}
// ============================================================================
/*  helper function to book 1D-histogram
 *  @param svc pointer to Histogram Service 
 *  @param dir  directory path in Histogram Data Store 
 *  @param id  historgam identifier 
 *  @param hist histogram desctriprion
 */ 
// ============================================================================
AIDA::IHistogram1D* 
Gaudi::Histos::book
( IHistogramSvc*           svc  , 
  const std::string&       dir  ,
  const int                id   ,
  const Gaudi::Histo1DDef& hist ) 
{
  if ( 0 == svc ) { return 0 ; }
  return svc -> book 
    ( dir , id  , 
      hist.title() , hist.bins() , hist.lowEdge() , hist.lowEdge() ) ;  
}
// ============================================================================




// ============================================================================
// The END 
// ============================================================================
