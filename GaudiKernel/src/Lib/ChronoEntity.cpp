#define GAUDIKERNEL_CHRONOENTITY_CPP 1
// ============================================================================
// include files
// ============================================================================
// STD & STL
// ============================================================================
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <cstdio>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/ChronoEntity.h"
// ============================================================================
// Boost
// ============================================================================
#include "boost/format.hpp"
// ============================================================================
/** @file
 *  implementation file for class ChronoEntity
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date:   December 1, 1999
 */
// ============================================================================
namespace
{
  /// the unit used by ChronoEntity is microsecond
  const double microsecond  =    1                 ; // unit here is microsecond
  const double millisecond  = 1000  * microsecond ;
  const double second       = 1000  * millisecond ;
  const double minute       =   60  *      second ;
  const double hour         =   60  *      minute ;
  const double day          =   24  *        hour ;
  const double week         =    7  *         day ;
  const double month        =   30  *         day ;
  const double year         =  365  *         day ;
  const double nanosecond   = 0.001 * microsecond ;
}
// ============================================================================
// Constructor
// ============================================================================
ChronoEntity::ChronoEntity()
  // current status of this chrono object;
  : m_status( IChronoSvc::UNKNOWN )  // current status
{}
// ============================================================================
// start the chrono
// ============================================================================
IChronoSvc::ChronoStatus  ChronoEntity::start()
{
  if ( IChronoSvc::RUNNING == m_status ) { return m_status ; }
  //
  // following lines could be platform dependent!
  //
  // Store in object the measured times
  m_start = System::getProcessTime();

  m_status = IChronoSvc::RUNNING;

  return m_status ;
}
// ============================================================================
// stop the chrono
// ============================================================================
IChronoSvc::ChronoStatus  ChronoEntity::stop()
{
  if ( IChronoSvc::RUNNING != m_status ) { return m_status ; }

  // following lines could be platform dependent!
  m_delta = System::getProcessTime() - m_start;

  // update the counters:

  m_user     += m_delta.userTime<TimeUnit>();
  m_kernel   += m_delta.kernelTime<TimeUnit>();
  m_elapsed  += m_delta.elapsedTime<TimeUnit>();

  // set new status

  m_status = IChronoSvc::STOPPED;

  return m_status ;
}
// ============================================================================
// print user time
// ============================================================================
std::string ChronoEntity::outputUserTime      () const
{
  std::string res ("Time User   : ") ;
  return res += format
    ( uTotalTime     () ,
      uMinimalTime   () ,
      uMeanTime      () ,
      uRMSTime       () ,
      uMaximalTime   () ,
      nOfMeasurements() );
}
// ============================================================================
// print system time
// ============================================================================
std::string ChronoEntity::outputSystemTime      () const
{
  std::string res ("Time System : ") ;
  return res += format
    ( kTotalTime     () ,
      kMinimalTime   () ,
      kMeanTime      () ,
      kRMSTime       () ,
      kMaximalTime   () ,
      nOfMeasurements() );
}
// ============================================================================
// print time
std::string ChronoEntity::outputElapsedTime      () const
{
  std::string res ("TimeElapsed: ") ;
  return res += format
    ( eTotalTime     () ,
      eMinimalTime   () ,
      eMeanTime      () ,
      eRMSTime       () ,
      eMaximalTime   () ,
      nOfMeasurements() );
}
// ============================================================================
// print the chrono
// ============================================================================
std::string ChronoEntity::format
( const double        total     ,
  const double        minimal   ,
  const double        mean      ,
  const double        rms       ,
  const double        maximal   ,
  const unsigned long number    ) const
{

  /// @todo: cache the format
  boost::format fmt("Tot=%2$5.3g%1$s %4$43s #=%3$3lu");

  long double  unit = 1.0 ;

  if     ( total / microsecond  <  500 )
  { unit = microsecond ; fmt % " [us]" ; }
  else if( total / millisecond  <  500 )
  { unit = millisecond ; fmt % " [ms]" ; }
  else if( total /      second  <  500 )
  { unit =      second ; fmt % "  [s]" ; }
  else if( total /      minute  <  500 )
  { unit =      minute ; fmt % "[min]" ; }
  else if( total /        hour  <  500 )
  { unit =        hour ; fmt % "  [h]" ; }
  else if( total /         day  <   10 )
  { unit =         day ; fmt % "[day]" ; }
  else if( total /        week  <    5 )
  { unit =        week ; fmt % "  [w]" ; }
  else if( total /       month  <   20 )
  { unit =       month ; fmt % "[mon]" ; }
  else
  { unit =        year ; fmt % "  [y]" ; }

  fmt % (double) (total / unit) % number;

  if( 1 < number )
  {
    /// @todo: cache the format
    boost::format fmt1("Ave/Min/Max=%2$5.3g(+-%3$5.3g)/%4$5.3g/%5$5.3g%1$s");
    if     ( mean / microsecond  <  500 )
    { unit = microsecond ; fmt1 % " [us]" ; }
    else if( mean / millisecond  <  500 )
    { unit = millisecond ; fmt1 % " [ms]" ; }
    else if( mean /      second  <  500 )
    { unit =      second ; fmt1 % "  [s]" ; }
    else if( mean /      minute  <  500 )
    { unit =      minute ; fmt1 % "[min]" ; }
    else if( mean /        hour  <  500 )
    { unit =        hour ; fmt1 % "  [h]" ; }
    else if( mean /         day  <   10 )
    { unit =         day ; fmt1 % "[day]" ; }
    else if( mean /        week  <    5 )
    { unit =        week ; fmt1 % "  [w]" ; }
    else if( mean /       month  <   20 )
    { unit =       month ; fmt1 % "[mon]" ; }
    else
    { unit =        year ; fmt1 % "  [y]" ; }

    fmt1 % (double) ( mean / unit ) % (double) ( rms  / unit )
         % (double) ( minimal  / unit ) % (double) ( maximal  / unit );
    fmt % fmt1.str();
  }
  else {
    fmt % "";
  }

  return fmt.str();
}
// ============================================================================
// comparison operator
// ============================================================================
bool ChronoEntity::operator<( const ChronoEntity& e ) const
{
  return
    ( &e           == this          ) ? false :
    ( totalTime () < e.totalTime () ) ? true  :
    ( totalTime () > e.totalTime () ) ? false :
    ( m_user       < e.m_user       ) ? true  :
    ( e.m_user     <   m_user       ) ? false :
    ( m_kernel     < e.m_kernel     ) ? true  :
    ( e.m_kernel   <   m_kernel     ) ? false :
    ( m_elapsed    < e.m_elapsed    ) ? true  :
    ( e.m_elapsed  <   m_elapsed    ) ? false : false ;
}
// ============================================================================
/*  print the chrono according the format and units
 *  @param typ  the chrono type
 *  @param fmt  the format string
 *  @param unit the unit
 *  @return the string representations
 *  @see boost::format
 */
// ============================================================================
std::string ChronoEntity::outputTime
( IChronoSvc::ChronoType typ  ,
  const std::string&     fmt  ,
  System::TimeType       unit ) const
{
  boost::format _fmt ( fmt ) ;
  // allow various number of arguments
  using namespace boost::io ;
  _fmt.exceptions ( all_error_bits ^ ( too_many_args_bit | too_few_args_bit ) ) ;
  //
  double _unit = microsecond ;
  switch ( unit )
  {
  case System::Year     : _unit = year        ; break ;
  case System::Month    : _unit = month       ; break ;
  case System::Day      : _unit = day         ; break ;
  case System::Hour     : _unit = hour        ; break ;
  case System::Min      : _unit = minute      ; break ;
  case System::Sec      : _unit = second      ; break ;
  case System::milliSec : _unit = millisecond ; break ;
  case System::microSec : _unit = microsecond ; break ;
  case System::nanoSec  : _unit = nanosecond  ; break ;
  default               : _unit = microsecond ; break ;
  }
  //
  const StatEntity* stat = &m_user;
  switch ( typ )
  {
  case IChronoSvc::USER    : stat = &m_user    ; break ;
  case IChronoSvc::KERNEL  : stat = &m_kernel  ; break ;
  case IChronoSvc::ELAPSED : stat = &m_elapsed ; break ;
  default                  : stat = &m_user    ; break ;
  }
  //
  _fmt
    %   ( stat -> nEntries    ()         )   // %1 : #entries
    %   ( stat -> flag        () / _unit )   // %2 : total time
    %   ( stat -> flagMean    () / _unit )   // %3 : mean time
    %   ( stat -> flagRMS     () / _unit )   // %4 : RMS  time
    %   ( stat -> flagMeanErr () / _unit )   // %5 : error in mean time
    %   ( stat -> flagMin     () / _unit )   // %6 : minimal time
    %   ( stat -> flagMax     () / _unit ) ; // %7 : maximal time
  //
  return _fmt.str() ;
}
// ==========================================================================

// ============================================================================
// The END
// ============================================================================
