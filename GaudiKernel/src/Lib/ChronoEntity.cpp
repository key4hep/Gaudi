#define GAUDIKERNEL_CHRONOENTITY_CPP 1
// ============================================================================
// include files
// ============================================================================
// STD & STL
// ============================================================================
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ChronoEntity.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/System.h"
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
  constexpr double microsecond = 1; // unit here is microsecond
  constexpr double millisecond = 1000 * microsecond;
  constexpr double second      = 1000 * millisecond;
  constexpr double minute      = 60 * second;
  constexpr double hour        = 60 * minute;
  constexpr double day         = 24 * hour;
  constexpr double week        = 7 * day;
  constexpr double month       = 30 * day;
  constexpr double year        = 365 * day;

  constexpr double nanosecond = 0.001 * microsecond;
}
// ============================================================================
// start the chrono
// ============================================================================
IChronoSvc::ChronoStatus ChronoEntity::start()
{
  if ( IChronoSvc::RUNNING == m_status ) {
    return m_status;
  }
  //
  // following lines could be platform dependent!
  //
  // Store in object the measured times
  m_start = System::getProcessTime();

  m_status = IChronoSvc::RUNNING;

  return m_status;
}
// ============================================================================
// stop the chrono
// ============================================================================
IChronoSvc::ChronoStatus ChronoEntity::stop()
{
  if ( IChronoSvc::RUNNING != m_status ) {
    return m_status;
  }

  // following lines could be platform dependent!
  m_delta = System::getProcessTime() - m_start;

  // update the counters:

  m_user += m_delta.userTime<TimeUnit>();
  m_kernel += m_delta.kernelTime<TimeUnit>();
  m_elapsed += m_delta.elapsedTime<TimeUnit>();

  // set new status

  m_status = IChronoSvc::STOPPED;

  return m_status;
}
// ============================================================================
// print user time
// ============================================================================
std::string ChronoEntity::outputUserTime() const
{
  return "Time User   : " +
         format( uTotalTime(), uMinimalTime(), uMeanTime(), uRMSTime(), uMaximalTime(), nOfMeasurements() );
}
// ============================================================================
// print system time
// ============================================================================
std::string ChronoEntity::outputSystemTime() const
{
  return "Time System : " +
         format( kTotalTime(), kMinimalTime(), kMeanTime(), kRMSTime(), kMaximalTime(), nOfMeasurements() );
}
// ============================================================================
// print time
std::string ChronoEntity::outputElapsedTime() const
{
  return "TimeElapsed: " +
         format( eTotalTime(), eMinimalTime(), eMeanTime(), eRMSTime(), eMaximalTime(), nOfMeasurements() );
}
// ============================================================================
// print the chrono
// ============================================================================
std::string ChronoEntity::format( const double total, const double minimal, const double mean, const double rms,
                                  const double maximal, const unsigned long number ) const
{

  /// @todo: cache the format
  boost::format fmt( "Tot=%2$5.3g%1$s %4$43s #=%3$3lu" );

  static const auto tbl = {std::make_tuple( 500, microsecond, " [us]" ), std::make_tuple( 500, millisecond, " [ms]" ),
                           std::make_tuple( 500, second, "  [s]" ),      std::make_tuple( 500, minute, "[min]" ),
                           std::make_tuple( 500, hour, "  [h]" ),        std::make_tuple( 10, day, "[day]" ),
                           std::make_tuple( 5, week, "  [w]" ),          std::make_tuple( 20, month, "[mon]" ),
                           std::make_tuple( -1, year, "  [y]" )};

  auto i = std::find_if(
      std::begin( tbl ), std::prev( std::end( tbl ) ),
      [&]( const std::tuple<int, double, const char*>& i ) { return total < std::get<0>( i ) * std::get<1>( i ); } );
  long double unit = std::get<1>( *i );
  fmt % std::get<2>( *i ) % (double)( total / unit ) % number;

  if ( number > 1 ) {
    /// @todo: cache the format
    boost::format fmt1( "Ave/Min/Max=%2$5.3g(+-%3$5.3g)/%4$5.3g/%5$5.3g%1$s" );
    auto          i = std::find_if(
        std::begin( tbl ), std::prev( std::end( tbl ) ),
        [&]( const std::tuple<int, double, const char*>& i ) { return total < std::get<0>( i ) * std::get<1>( i ); } );
    unit = std::get<1>( *i );
    fmt1 % std::get<2>( *i ) % (double)( mean / unit ) % (double)( rms / unit ) % (double)( minimal / unit ) %
        (double)( maximal / unit );
    fmt % fmt1.str();
  } else {
    fmt % "";
  }

  return fmt.str();
}
// ============================================================================
// comparison operator
// ============================================================================
bool ChronoEntity::operator<( const ChronoEntity& e ) const
{
  return ( &e == this )
             ? false
             : ( totalTime() < e.totalTime() )
                   ? true
                   : ( totalTime() > e.totalTime() )
                         ? false
                         : ( m_user < e.m_user )
                               ? true
                               : ( e.m_user < m_user ) ? false
                                                       : ( m_kernel < e.m_kernel )
                                                             ? true
                                                             : ( e.m_kernel < m_kernel )
                                                                   ? false
                                                                   : ( m_elapsed < e.m_elapsed )
                                                                         ? true
                                                                         : ( e.m_elapsed < m_elapsed ) ? false : false;
}
// ============================================================================
// compound assignment operator
// ============================================================================
ChronoEntity& ChronoEntity::operator+=( const ChronoEntity& e )
{
  // System::ProcessTime type
  m_delta += e.m_delta;

  // Summing, massaging here does not make too much sense.
  // This is done only for final reductions
  // Keep by convention the original one.
  //	m_start += e.m_start;

  // Tymevaluetypes type
  m_user += e.m_user;
  m_kernel += e.m_kernel;
  m_elapsed += e.m_elapsed;

  return *this;
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
std::string ChronoEntity::outputTime( IChronoSvc::ChronoType typ, const std::string& fmt, System::TimeType unit ) const
{
  boost::format _fmt( fmt );
  // allow various number of arguments
  using namespace boost::io;
  _fmt.exceptions( all_error_bits ^ ( too_many_args_bit | too_few_args_bit ) );
  //
  double _unit = microsecond;
  switch ( unit ) {
  case System::Year:
    _unit = year;
    break;
  case System::Month:
    _unit = month;
    break;
  case System::Day:
    _unit = day;
    break;
  case System::Hour:
    _unit = hour;
    break;
  case System::Min:
    _unit = minute;
    break;
  case System::Sec:
    _unit = second;
    break;
  case System::milliSec:
    _unit = millisecond;
    break;
  case System::microSec:
    _unit = microsecond;
    break;
  case System::nanoSec:
    _unit = nanosecond;
    break;
  default:
    _unit = microsecond;
    break;
  }
  //
  const StatEntity* stat = &m_user;
  switch ( typ ) {
  case IChronoSvc::USER:
    stat = &m_user;
    break;
  case IChronoSvc::KERNEL:
    stat = &m_kernel;
    break;
  case IChronoSvc::ELAPSED:
    stat = &m_elapsed;
    break;
  default:
    stat = &m_user;
    break;
  }
  //
  _fmt % ( stat->nEntries() )           // %1 : #entries
      % ( stat->flag() / _unit )        // %2 : total time
      % ( stat->flagMean() / _unit )    // %3 : mean time
      % ( stat->flagRMS() / _unit )     // %4 : RMS  time
      % ( stat->flagMeanErr() / _unit ) // %5 : error in mean time
      % ( stat->flagMin() / _unit )     // %6 : minimal time
      % ( stat->flagMax() / _unit );    // %7 : maximal time
  //
  return _fmt.str();
}
// ==========================================================================

// ============================================================================
// The END
// ============================================================================
