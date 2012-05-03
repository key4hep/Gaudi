// $Id: HistoTableFormat.cpp,v 1.2 2008/04/03 22:12:27 marcocle Exp $
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
#include <cmath>
// ============================================================================
// AIDA
// ============================================================================
/// @FIXME: AIDA interfaces visibility
#include <vector>
#include "GaudiKernel/Kernel.h"
#define class class GAUDI_API
#include "AIDA/IHistogram1D.h"
#include "AIDA/IAxis.h"
#undef class
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IRegistry.h"
// ============================================================================
// Local
// ============================================================================
#include "GaudiUtils/HistoStats.h"
#include "GaudiUtils/HistoTableFormat.h"
// ============================================================================
// Boost
// ============================================================================
#include "boost/format.hpp"
// ============================================================================
/** @file
 *  Implementation file for class Gaudi::Utils::HistoTableFormat
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-08-07
 */
// ============================================================================
namespace
{
  // ==========================================================================
  // Examples of formats
  // ==========================================================================
  /// The base statisical format : title, #entries, mean, rms, skewness and kurtosis
  const std::string s_histoFormatStat       =
    "| %2$-45.45s | %3$=7d |%8$11.5g | %10$-11.5g|%12$11.5g |%14$11.5g |" ;
  // ==========================================================================
  /// The compact statisical format : #entries, mean, rms, skewness and kurtosis
  const std::string s_histoFormatStatOnly   =
    "| %3$=7d |%8$11.5g | %10$-11.5g|%12$11.5g |%14$11.5g |" ;
  // ==========================================================================
  /// The statisical format : #entries, mean+-, rms+-, skewness+- and kurtosis+-
  const std::string s_histoFormatStat1   =
    "| %3$=7d |%8$9.3g+-%9$-9.3g|%10$9.3g+-%11$-9.3g|%12$9.3g+-%13$-9.3g|%14$9.3g+-%15$-9.3g|" ;
  // ==========================================================================
  /** The compact statisical format for "shape-only" variables :
   *  mean, rms, skewness and kurtosis, underflow & overflow fractions
   */
  const std::string s_histoFormatShapeOnly   =
  "|%8$11.5g |%10$11.5g |%12$11.5g |%14$11.5g |%19$11.5g/%21$-11.5g|" ;
  // ==========================================================================
  /** The compact statisical format for "shape-only" variables :
   *  mean, rms, skewness and kurtosis, underflow & overflow fractions
   *  and their errors
   */
  const std::string s_histoFormatShape      =
  "|%8$9.3g+-%9$-9.3g|%10$9.3g+-%11$-9.3g|%12$9.3g+-%13$-9.3g|%14$9.3g+-%15$-9.3g|%19$9.3g+-%20$-9.3g|%21$9.3g+-%22$-9.3g|" ;
  // ==========================================================================
  /// The line printout (similar to Matt's requestt): title, mean, rms, skewnes and kurtosis
  const std::string s_histoFormatLineTitle  =
    " %2$-45.45s mean/sigma/skew/kurtosis = %8$10.4g/%10$10.4g/%12$10.4g/%14$10.4g" ;
  // ==========================================================================
  /// The line printout (similar to Matt's requiest): mean, rms, skewnes and kurtosis
  const std::string s_histoFormatLineOnly   =
    " mean/sigma/skew/kurtosis = %8$10.4g/%10$10.4g/%12$10.4g/%14$10.4g" ;
  // ==========================================================================
  /// "Old"-format: title, int, ent , mean, rms
  const std::string s_histoFormatOld        =
    " %2$-45.45s    Ents/All=%7$5s/%3$5s<X>/sX=%8$.5g/%10$-.5g" ;
  // ==========================================================================
  /// Path-title printout : path and title
  const std::string s_histoFormatPathTitle = "| %1$-45.45s | %2$-45.45s | " ;
  // ==========================================================================
  /// FULL format, with printout of all (15) predefined fields
  const std::string s_histoFormatFull       =
    "| %1$-45.45s | %2$-45.45s | %3$=7d |%4$5d/%5$-5d|%6$9.3g | %7$-9.3g|%8$9.3g+-%9$-9.3g|%10$9.3g+-%11$-9.3g|%12$9.3g+-%13$-9.3g|%14$9.3g+-%15$-9.3g|" ;
 /// FULL stat
  const std::string s_histoFormatFullStat   =
    "| %3$=7d |%4$5d/%5$-5d|%6$9.3g | %7$-9.3g|%8$9.3g+-%9$-9.3g|%10$9.3g+-%11$-9.3g|%12$9.3g+-%13$-9.3g|%14$9.3g+-%15$-9.3g|" ;
  // ==========================================================================
  // headers:
  // ==========================================================================
  /// The table header, suitable for usage with "Stat"-format
  const std::string s_histoFormatStatHeader =
    "|   Title                                       |    #    |     Mean   |    RMS     |  Skewness  |  Kurtosis  |" ;
  // ==========================================================================
  /// The table header, suitable for usage with "StatOnly"-format
  const std::string s_histoFormatStatOnlyHeader =
    "|    #    |     Mean   |    RMS     |  Skewness  |  Kurtosis  |" ;
  // ==========================================================================
 /// The table header, suitable for usage with "Stat1"-format
  const std::string s_histoFormatStat1Header    =
    "|    #    |     Mean+-Error    |      RMS+-Error    | Skewness+-Error    | Kurtosis+-Error    |" ;
  // ==========================================================================
  /// The table header, suitable for usage with "ShapeOnly"-format
  const std::string s_histoFormatShapeOnlyHeader =
  "|     Mean   |    RMS     |  Skewness  |  Kurtosis  | Underflow%/Overflow%  |" ;
  // ==========================================================================
  /// The table header, suitable for usage with "Shape"-format
  const std::string s_histoFormatShapeHeader =
  "|        Mean        |         RMS        |      Skewness      |     Kurtosis       |    Underflow [%]   |    Overflow [%]    |" ;
  // ==========================================================================
  /// The table header, suitable for usage with "LineTitle"-format
  const std::string s_histoFormatLineTitleHeader = "  Title " ;
  // ==========================================================================
  /// The table header, suitable for usage with "LineOnly"-format
  const std::string s_histoFormatLineOnlyHeader  = "" ;
  // ==========================================================================
  /// The table header, suitable for usage with "Old"-format
  const std::string s_histoFormatOldHeader       = "" ;
  // ==========================================================================
  /// The table header, suitable for usage with "PathTitle"-format
  const std::string s_histoFormatPathTitleHeader =
    "| Path in Histogram Data Store   |  Title                         | ";
  // ==========================================================================
  /// The table header, suitable for usage with "Full"-format
  const std::string s_histoFormatFullHeader =
    "| Path in Histogram Data Store   |  Title                         |    #    |Udflw/Ovflw|    nEff  | Sum      |     Mean+-Error    |      RMS+-Error    | Skewness+-Error    | Kurtosis+-Error    |" ;
  // ==========================================================================
  /// The table header, suitable for usage with "FullStat"-format
  const std::string s_histoFormatFullStatHeader =
    "|    #    |Udflw/Ovflw|    nEff  | Sum      |     Mean+-Error    |      RMS+-Error    | Skewness+-Error    | Kurtosis+-Error    |" ;
  // ==========================================================================
}
// ============================================================================
// get the format by enum
// ============================================================================
std::string Gaudi::Utils::Histos::Formats::format ( const int ID )
{
  switch ( ID )
  {
  case Old       : return s_histoFormatOld       ;
  case Full      : return s_histoFormatFull      ;
  case FullStat  : return s_histoFormatFullStat  ;
  case Stat      : return s_histoFormatStat      ;
  case StatOnly  : return s_histoFormatStatOnly  ;
  case Stat1     : return s_histoFormatStat1     ;
  case ShapeOnly : return s_histoFormatShapeOnly ;
  case Shape     : return s_histoFormatShape     ;
  case LineTitle : return s_histoFormatLineTitle ;
  case LineOnly  : return s_histoFormatLineOnly  ;
  case PathTitle : return s_histoFormatPathTitle ;
  default        : break ;
  }
  /// default:
  return s_histoFormatStat ;
}
// ============================================================================
// get the recommended header by enum
// ============================================================================
std::string Gaudi::Utils::Histos::Formats::header ( const int ID  )
{
  switch ( ID )
  {
  case Old       : return s_histoFormatOldHeader       ;
  case Full      : return s_histoFormatFullHeader      ;
  case FullStat  : return s_histoFormatFullStatHeader  ;
  case Stat      : return s_histoFormatStatHeader      ;
  case Stat1     : return s_histoFormatStat1Header     ;
  case StatOnly  : return s_histoFormatStatOnlyHeader  ;
  case ShapeOnly : return s_histoFormatShapeOnlyHeader ;
  case Shape     : return s_histoFormatShapeHeader     ;
  case LineTitle : return s_histoFormatLineTitleHeader ;
  case LineOnly  : return s_histoFormatLineOnlyHeader  ;
  case PathTitle : return s_histoFormatPathTitleHeader ;
  default        : ;
  }
  /// default:
  return s_histoFormatStatHeader ;
}
// ============================================================================
// get the path in TES for AIDA histogram
// ============================================================================
std::string Gaudi::Utils::Histos::path ( const AIDA::IBaseHistogram* aida )
{
  if ( 0 == aida     ) { return "" ; }                          // RETURN
  const DataObject* object   = dynamic_cast<const DataObject*>( aida ) ;
  if ( 0 == object   ) { return "" ; }                          // RETURN
  IRegistry*        registry = object->registry() ;
  if ( 0 == registry ) { return "" ; }                          // RETURN
  std::string _path = registry->identifier() ;
  std::string::size_type n = _path.find("/stat/") ;
  if ( 0 == n ) { return std::string(_path,6) ; }               // RETURN
  return _path ;                                                // RETURN
}
// ============================================================================
/*  Make the string representation of the historgam
 *  according to the specified format.
 */
// ============================================================================
std::string Gaudi::Utils::Histos::format
( const AIDA::IHistogram1D* histo  ,
  const std::string&        fmt    )
{
  if ( 0 == histo ) { return "<NULL>" ; }
  using namespace Gaudi::Utils ;
  using namespace boost::io ;
  boost::format _fmt ( fmt ) ;
  // allow various number of arguments
  _fmt.exceptions ( all_error_bits ^ ( too_many_args_bit | too_few_args_bit ) ) ;

  _fmt
    % ( "\"" + path ( histo )    + "\"" )   //  1) histogram path
    % ( "\"" + histo -> title () + "\"" )   //  2) title
    % histo -> allEntries            ()     //  3) # entries
    % histo -> binEntries ( AIDA::IAxis::UNDERFLOW_BIN ) // 4) # underflow
    % histo -> binEntries ( AIDA::IAxis::OVERFLOW_BIN  ) // 5) # overflow
    % histo -> equivalentBinEntries  ()     //  6) equivalent entries
    % histo -> sumBinHeights         ()     //  7) integral
    % histo -> mean                  ()     //  8) mean value
    % HistoStats:: meanErr    ( histo )     //  9) error in mean
    % histo -> rms                   ()     // 10) rms
    % HistoStats::rmsErr      ( histo )     // 11) error in rms
    % HistoStats::skewness    ( histo )     // 12) skewness
    % HistoStats::skewnessErr ( histo )     // 13) error in skewness
    % HistoStats::kurtosis    ( histo )     // 14) kurtosis
    % HistoStats::kurtosisErr ( histo )     // 15) error in kurtosis
    //
    % histo -> sumAllBinHeights      ()     // 16) full integral (in and out range)
    % HistoStats::sumAllBinHeightErr ( histo ) // 17) error on 16
    % HistoStats::sumBinHeightErr    ( histo ) // 18) error on  7
    //
    % ( 100 * HistoStats::underflowEntriesFrac     ( histo ) ) // 19) fraction of underflow entries
    % ( 100 * HistoStats::underflowEntriesFracErr  ( histo ) ) // 20) error on 19
    % ( 100 * HistoStats::overflowEntriesFrac      ( histo ) ) // 21) fraction of overflow entries
    % ( 100 * HistoStats::overflowEntriesFracErr   ( histo ) ) // 22) error on 21
    //
    % HistoStats::underflowIntegralFrac    ( histo ) // 23) fraction of underflow integral
    % HistoStats::underflowIntegralFracErr ( histo ) // 24) error on 23
    % HistoStats::overflowIntegralFrac     ( histo ) // 25) fraction of overflow intergal
    % HistoStats::overflowIntegralFracErr  ( histo ) ; // 26) error on 25
  //
  return _fmt.str() ;
}
// ============================================================================
/* format a full row in table, including ID, label, path or any other
 *  "extra" identifier in string form
 */
// ============================================================================
std::string Gaudi::Utils::Histos::format
( const AIDA::IHistogram1D* histo  ,
  const std::string&        ID     ,
  const std::string&        fmt1   ,
  const std::string&        fmt2   )
{
  using namespace boost::io ;
  boost::format _fmt ( fmt1 ) ;
  // allow various number of arguments
  _fmt.exceptions ( all_error_bits ^ ( too_many_args_bit | too_few_args_bit ) ) ;
  //
  _fmt
    % ID                         // format ID
    % format ( histo , fmt2 )  ; // format the histogram
  //
  return _fmt.str() ;
}
// ============================================================================
// helper method to merge the headers for short format table
// ============================================================================
std::string Gaudi::Utils::Histos::format
( const std::string& val1 ,
  const std::string& val2 ,
  const std::string& fmt  )
{
  using namespace boost::io ;
  boost::format _fmt ( fmt ) ;
  // allow various number of arguments
  _fmt.exceptions ( all_error_bits ^ ( too_many_args_bit | too_few_args_bit ) ) ;
  //
  _fmt
    % val1   // format ID
    % val2 ; // format the histogram
  //
  return _fmt.str() ;
}
// ============================================================================
// default constructor
// ============================================================================
Gaudi::Utils::Histos::Table::Table ( const int ID )
  : m_header ( Gaudi::Utils::Histos::Formats::header ( ID ) )
  , m_footer ()
  , m_format ( Gaudi::Utils::Histos::Formats::format ( ID ) )
{}
// ============================================================================
Gaudi::Utils::Histos::Table::Table
(  const std::string& format ,
   const std::string& header ,
   const std::string& footer )
  : m_header ( header )
  , m_footer ( footer )
  , m_format ( format )
{}
// ============================================================================
// format the table row using the default format
// ============================================================================
std::string Gaudi::Utils::Histos::Table::toString
( const AIDA::IHistogram1D* histo  ) const
{ return Gaudi::Utils::Histos::format ( histo , format() ) ; }
// ============================================================================
/** format a full row in table, including ID, label, path or any other
 *  "extra" identifier
 */
// ============================================================================
std::string Gaudi::Utils::Histos::Table::toString
( const AIDA::IHistogram1D* histo  ,
  const std::string&        ID     ,
  const std::string&        fmt    ) const
{ return Gaudi::Utils::Histos::format ( histo , ID , fmt , format() ) ; }
// ============================================================================

// ============================================================================
// The END
// ============================================================================
