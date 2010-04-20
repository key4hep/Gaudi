// $Id: StatEntity.cpp,v 1.8 2008/07/15 10:53:26 marcocle Exp $ 
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.8 $
// ============================================================================
#define GAUDIKERNEL_STATENTITY_CPP 1 
// ============================================================================
// include files 
// ============================================================================
// STD & STL 
// ============================================================================
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <limits>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/StatEntity.h"
// ============================================================================
// Boost
// ============================================================================
#include "boost/format.hpp"
#include "boost/static_assert.hpp"
#include "boost/algorithm/string/case_conv.hpp"
// ============================================================================

#ifdef __ICC
// disable icc remark #1572: floating-point equality and inequality comparisons are unreliable
//  A lot of comparisons are done and "meant".
#pragma warning(disable:1572)
// disable icc remark #2259: non-pointer conversion from "long double" to "double" may lose significant bits
//  Internal operations are done with "long double", but the interface exposes only
//  "double", so the conversions are required.
#pragma warning(disable:2259)
#endif


/** @file 
 *  Implementation file for class StatEntity
 *  @date 26/06/2001 
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 */
// ============================================================================
// The full contructor from all important values
// ============================================================================
StatEntity::StatEntity
( const unsigned long entries , 
  const double        flag    ,
  const double        flag2   ,
  const double        minFlag , 
  const double        maxFlag ) 
  : m_se_nEntries            ( entries )
  , m_se_accumulatedFlag     ( flag    ) 
  , m_se_accumulatedFlag2    ( flag2   ) 
  , m_se_minimalFlag         ( minFlag ) 
  , m_se_maximalFlag         ( maxFlag ) 
  , m_se_nEntriesBeforeReset ( -1      )
{}
// ============================================================================
// the internal format description 
// ============================================================================
const std::string& StatEntity::format() 
{
  // check for "X" or "L"
  BOOST_STATIC_ASSERT(((sizeof(unsigned long)==4)||(sizeof(unsigned long)==8)));
  // check for "D"
  BOOST_STATIC_ASSERT((sizeof(double)==8))        ; ///< check for "D"
  //
  static const std::string s_fmt = 
    ( ( 8 == sizeof(unsigned long) ) ?"X:1;" : "L:1;" ) + std::string("D:4") ;
  return s_fmt ;
} 
// ============================================================================
// the actual size of published data 
// ============================================================================
int StatEntity::size  () 
{ 
  static const int s_size = sizeof(unsigned long) + 4 * sizeof(double) ; 
  return s_size ;
}   
// ============================================================================
// mean value of flag 
// ============================================================================
double StatEntity::mean   () const 
{ 
  if ( 0 >= nEntries() ) { return 0 ;}
  const long double f1 = m_se_accumulatedFlag ;
  const long double f2 = m_se_nEntries ;
  return f1 / f2 ;
}
// ============================================================================
// r.m.s of flag
// ============================================================================
double StatEntity::rms    () const 
{
  if ( 0 >= nEntries() ) { return 0 ; }
  const long double f1  = m_se_accumulatedFlag  ;
  const long double f2  = f1 / nEntries () ;
  const long double f3  = m_se_accumulatedFlag2 ;
  const long double f4  = f3 / nEntries () ;
  const long double result = f4 - f2 * f2  ;
  return ( 0 > result ) ? 0 : ::sqrtl ( result ) ; 
} 
// ============================================================================
// error in mean value of flag 
// ============================================================================
double StatEntity::meanErr() const 
{
  if ( 0 >= nEntries () ) { return 0 ; }
  const long double f1  = m_se_accumulatedFlag  ;
  const long double f2  = f1 / nEntries () ;
  const long double f3  = m_se_accumulatedFlag2 ;
  const long double f4  = f3 / nEntries () ;
  const long double result = f4 - f2 * f2  ;
  if ( 0 > result      ) { return 0 ; }
  //
  return ::sqrtl ( result / nEntries () ) ;
}
// ============================================================================
// interprete the content as efficiency
// ============================================================================
double StatEntity::efficiency    () const 
{
  if ( 1 > nEntries () || 0 > sum() || sum() > nEntries() ) { return -1 ; }
  const long double fMin = min () ;
  if ( 0 != fMin && 1 != fMin ) { return -1 ; }
  const long double fMax = max () ;
  if ( 0 != fMax && 1 != fMax ) { return -1 ; }
  return mean() ;
}
// ============================================================================
// evaluate the binomial error in efficiency
// ============================================================================
double StatEntity::efficiencyErr () const 
{
  if ( 0 > efficiency() ) { return -1 ; }
  //
  long double n1 = sum () ;
  // treat properly the bins with eff=0 
  if ( 0 == n1 ) { n1 = 1 ; } ///< @attention treat properly the bins with eff=0 
  const long double n3 = nEntries  () ;
  long double       n2 = n3 - flag () ;
  // treat properly the bins with eff=100% 
  if ( 1 > fabsl( n2 ) ) { n2 = 1 ; } ///< @attention treat properly the bins with eff=100% 
  //
  return ::sqrtl( n1 * n2 / n3 ) / n3 ;
}
// ============================================================================
// increment with other entity 
// ============================================================================
StatEntity& StatEntity::operator+=( const StatEntity& other ) 
{
  m_se_nEntries         += other.m_se_nEntries ;
  m_se_accumulatedFlag  += other.m_se_accumulatedFlag  ;
  m_se_accumulatedFlag2 += other.m_se_accumulatedFlag2 ;
  m_se_minimalFlag = std::min ( m_se_minimalFlag , other.m_se_minimalFlag ) ;
  m_se_maximalFlag = std::max ( m_se_maximalFlag , other.m_se_maximalFlag ) ;
  //
  return *this ;
}
// ============================================================================
/// comparison 
// ============================================================================
bool StatEntity::operator< ( const StatEntity& se ) const 
{  
  if      ( &se == this                     ) { return false ; }
  else if ( nEntries () <   se.nEntries ()  ) { return true  ; }
  else if ( nEntries () ==  se.nEntries () && 
            sum      () <   se.sum      ()  ) { return true  ; } 
  else if ( nEntries () ==  se.nEntries () && 
            sum      () ==  se.sum      () &&  
            min      () <   se.min      ()  ) { return true  ; } 
  else if ( nEntries () ==  se.nEntries () && 
            sum      () ==  se.sum      () &&  
            min      () ==  se.min      () &&
            max      () <   se.max      ()  ) { return true  ; } 
  else if ( nEntries () ==  se.nEntries () && 
            sum      () ==  se.flag     () &&  
            min      () ==  se.min      () &&
            max      () ==  se.max      () && 
            sum2     () <   se.sum2     ()  ) { return true  ; }
  ///
  return false; 
}
// ============================================================================
// increment a flag  
// ============================================================================
unsigned long StatEntity::add ( const double Flag ) 
{  
  //
  if      ( 0 <  m_se_nEntriesBeforeReset ) { --m_se_nEntriesBeforeReset; }
  else if ( 0 == m_se_nEntriesBeforeReset ) { reset(); } ///< reset everything
  /// accumulate the flag
  m_se_accumulatedFlag   += Flag        ; // accumulate the flag
  /// evaluate min/max 
  m_se_minimalFlag = std::min ( m_se_minimalFlag , Flag ) ; // evaluate min/max 
  m_se_maximalFlag = std::max ( m_se_maximalFlag , Flag ) ; // evaluate min/max 
  // accumulate statistics, but avoid FPE for small flags... 
  static const double s_min1 = 2 * ::sqrt ( std::numeric_limits<double>::min() ) ;
  if ( s_min1 < Flag || -s_min1 > Flag )  
  { m_se_accumulatedFlag2  += Flag * Flag ; }// accumulate statistics:
  //
  return ++m_se_nEntries ;  
}
// ============================================================================
// reset all quantities
// ============================================================================
void StatEntity::reset()
{
  //
  m_se_nEntries            =        0 ;
  m_se_accumulatedFlag     =        0 ; 
  m_se_minimalFlag         =       std::numeric_limits<double>::max() ;
  m_se_maximalFlag         =  -1 * std::numeric_limits<double>::max() ;
  m_se_accumulatedFlag2    =        0 ;
  m_se_nEntriesBeforeReset =       -1 ; // ? 
}
// ============================================================================
/// DR specify number of entry before reset
// ============================================================================
void StatEntity::setnEntriesBeforeReset(unsigned long nEntriesBeforeReset )
{ m_se_nEntriesBeforeReset = nEntriesBeforeReset; }
// ============================================================================
// representation as string
// ============================================================================
std::string StatEntity::toString () const
{
  std::ostringstream ost ;
  print ( ost )  ;
  return ost.str () ;
} 
// ============================================================================
// printout to std::ostream
// ============================================================================
std::ostream& StatEntity::print ( std::ostream& o ) const
{
  boost::format fmt1 ("#=%|-7lu| Sum=%|-11.5g|" ) ;
  o << fmt1 % nEntries() % sum() ;
  boost::format fmt2 ( " Mean=%|#10.4g| +- %|-#10.5g| Min/Max=%|#10.4g|/%|-#10.4g|" ) ;
  o << fmt2 % mean() % rms() % min() % max() ;
  return o ;
}
// ============================================================================
// external operator for addition of StatEntity and a number
// ============================================================================
StatEntity operator+( const StatEntity& entity , const double      value  ) 
{ StatEntity aux ( entity ) ; return aux+=value ; }
// ============================================================================
// external operator for addition of StatEntity and a number
// ============================================================================
StatEntity operator+( const double      value  , const StatEntity& entity ) 
{ return entity + value ; }
// ============================================================================
// external operator for addition of StatEntity and a number
// ============================================================================
StatEntity operator-( const StatEntity& entity , const double      value  ) 
{ StatEntity aux ( entity ) ; return aux-=value ; }
// ============================================================================
// external operator for addition of StatEntity and a number
// ============================================================================
StatEntity operator+( const StatEntity& entity , const StatEntity& value  ) 
{ StatEntity aux ( entity ) ; return aux+=value ; }
// ============================================================================
// external printout operator to std::ostream
// ============================================================================
std::ostream& operator<<( std::ostream& stream , const StatEntity& entity ) 
{ return entity.print ( stream ) ; }
// ============================================================================
namespace 
{
  /** simple function which checks the presence of "efficicncy"/"accepted"
   *  in the given string:
   *  @param  name input string
   *  @return true if the name "compatible" with efficiciency or 
   *  acceptance
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date 2007-09-11
   */
  inline bool effCounter ( const std::string& name ) 
  {
    const std::string lower  =  boost::algorithm::to_lower_copy( name ) ;
    return  
      std::string::npos != lower.find ( "eff"  ) ||
      std::string::npos != lower.find ( "acc"  ) ||
      std::string::npos != lower.find ( "filt" ) ||
      std::string::npos != lower.find ( "fltr" ) ||
      std::string::npos != lower.find ( "pass" )  ;
  }
}
// ============================================================================
/*  Format the counter in a form of the table row
 *  @param name    the name associated with the counter
 *  @param counter counter to be printed 
 *  @param flag    use the special format for "efficiency" rows
 *  @param format1 row format for the regular rows 
 *  @param format2 special row format for the "efficiency" rows 
 *  @return formatted row in the table 
 */
// ============================================================================
std::string Gaudi::Utils::formatAsTableRow 
( const StatEntity&  counter , 
  const bool         flag    , 
  const std::string& format1 ,
  const std::string& format2 )
{
  using namespace boost::io ;
  if ( flag && 0 <= counter.eff() && 0 <= counter.effErr() ) 
  {
    boost::format fmt( format2 ) ;
    fmt.exceptions ( all_error_bits ^ ( too_many_args_bit | too_few_args_bit ) ) ;
    fmt 
      %   counter.nEntries ()       
      %   counter.sum      () 
      % ( counter.eff      () * 100 ) 
      % ( counter.effErr   () * 100 ) ;
    return fmt.str() ;
  }
  boost::format fmt  ( format1 ) ;
  fmt.exceptions ( all_error_bits ^ ( too_many_args_bit | too_few_args_bit ) ) ;
  fmt 
    %   counter.nEntries () 
    %   counter.sum      () 
    %   counter.mean     () 
    %   counter.rms      ()  
    %   counter.min      () 
    %   counter.max      () ;
  return fmt.str() ;
}
// ============================================================================
/* Format the counter in a form of the table row
 * @param name the name associated with the counter
 * @param counter counter to be printed 
 * @param flag use the special format for efficiency rows
 * @param format1 row format for the regular rows 
 * @param format2 the special row format for the "efficiency" rows 
 * @return formatted row in the table 
 */
// ============================================================================
std::string Gaudi::Utils::formatAsTableRow 
( const std::string& name    ,
  const StatEntity&  counter , 
  const bool         flag    , 
  const std::string& format1 , 
  const std::string& format2 )
{
  using namespace boost::io ;
  if ( flag && effCounter ( name ) && 0 <= counter.eff() && 0 <= counter.effErr() ) 
  {
    boost::format fmt( format2 ) ;
    fmt.exceptions ( all_error_bits ^ ( too_many_args_bit | too_few_args_bit ) ) ;
    fmt 
      % ( "\"" + name + "\"" ) 
      %   counter.nEntries ()       
      %   counter.sum      () 
      % ( counter.eff      () * 100 ) 
      % ( counter.effErr   () * 100 ) ;
    return fmt.str() ;
  }
  boost::format fmt  ( format1 ) ;
  fmt.exceptions ( all_error_bits ^ ( too_many_args_bit | too_few_args_bit ) ) ;
  fmt 
    % ( "\"" + name + "\"" ) 
    %   counter.nEntries () 
    %   counter.sum      () 
    %   counter.mean     () 
    %   counter.rms      ()  
    %   counter.min      () 
    %   counter.max      () ;
  return fmt.str() ;
}
// ============================================================================
/* Format the counter in a form of the table row
 * @param name the name associated with the counter
 * @param group the group associated with the counter
 * @param counter counter to be printed 
 * @param flag use the special format for efficiency rows
 * @param format1 row format for the regular rows 
 * @param format2 the special row format for the "efficiency" rows 
 * @return formatted row in the table 
 */
// ============================================================================
std::string Gaudi::Utils::formatAsTableRow 
( const std::string& name    ,
  const std::string& group   ,
  const StatEntity&  counter ,
  const bool         flag    , 
  const std::string& format1 , 
  const std::string& format2 )
{
  using namespace boost::io ;
  if ( flag && ( effCounter ( name ) || effCounter ( group ) ) 
       && 0 <= counter.eff() && 0 <= counter.effErr() ) 
  {
    boost::format fmt( format2 ) ;
    fmt.exceptions ( all_error_bits ^ ( too_many_args_bit | too_few_args_bit ) ) ;
    fmt 
      % ( "\"" + name  + ":"  ) 
      % ( ":"  + group + "\"" ) 
      %   counter.nEntries () 
      %   counter.sum      () 
      % ( counter.eff      () * 100 ) 
      % ( counter.effErr   () * 100 ) ;
    return fmt.str() ;
  }
  boost::format fmt  ( format1 ) ;
  fmt.exceptions ( all_error_bits ^ ( too_many_args_bit | too_few_args_bit ) ) ;
  fmt 
    % ( "\""  + name  + ":"  ) 
    % ( ":"   + group + "\"" ) 
    %   counter.nEntries () 
    %   counter.sum      () 
    %   counter.mean     () 
    %   counter.rms      ()  
    %   counter.min      () 
    %   counter.max      () ;
  return fmt.str() ;
}
// ============================================================================


// ============================================================================
// The END 
// ============================================================================
