/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/ChronoEntity.h>
#include <GaudiKernel/Kernel.h>
#include <GaudiKernel/System.h>
#include <algorithm>
#include <boost/format.hpp>
#include <format>

namespace {
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
} // namespace
// ============================================================================
// start the chrono
// ============================================================================
IChronoSvc::ChronoStatus ChronoEntity::start() {
  if ( IChronoSvc::RUNNING == m_status ) { return m_status; }
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
IChronoSvc::ChronoStatus ChronoEntity::stop() {
  if ( IChronoSvc::RUNNING != m_status ) { return m_status; }

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
std::string ChronoEntity::outputUserTime() const {
  return "Time User   : " +
         format( uTotalTime(), uMinimalTime(), uMeanTime(), uRMSTime(), uMaximalTime(), nOfMeasurements() );
}
std::string ChronoEntity::outputSystemTime() const {
  return "Time System : " +
         format( kTotalTime(), kMinimalTime(), kMeanTime(), kRMSTime(), kMaximalTime(), nOfMeasurements() );
}
std::string ChronoEntity::outputElapsedTime() const {
  return "TimeElapsed: " +
         format( eTotalTime(), eMinimalTime(), eMeanTime(), eRMSTime(), eMaximalTime(), nOfMeasurements() );
}
std::string ChronoEntity::format( const double total, const double minimal, const double mean, const double rms,
                                  const double maximal, const unsigned long number ) const {

  /// @todo: cache the format
  constexpr auto fmt      = "Tot={1:5.3g}{0:5} {3} #={2:3}";
  constexpr auto stat_fmt = "Ave/Min/Max={1:8.3g}(+-{2:8.3g})/{3:8.3g}/{4:8.3g}{0:5}";

  static const std::array<std::tuple<int, double, std::string_view>, 9> tbl{ { { 500, microsecond, " [us]" },
                                                                               { 500, millisecond, " [ms]" },
                                                                               { 500, second, "  [s]" },
                                                                               { 500, minute, "[min]" },
                                                                               { 500, hour, "  [h]" },
                                                                               { 10, day, "[day]" },
                                                                               { 5, week, "  [w]" },
                                                                               { 20, month, "[mon]" },
                                                                               { -1, year, "  [y]" } } };

  auto             i         = find_if( begin( tbl ), prev( end( tbl ) ),
                                        [&]( const auto& i ) { return total < std::get<0>( i ) * std::get<1>( i ); } );
  long double      unit      = std::get<1>( *i );
  std::string_view unit_name = std::get<2>( *i );

  auto stats = [&]() -> std::string {
    if ( number > 1 ) {
      auto             i         = find_if( begin( tbl ), prev( end( tbl ) ),
                                            [&]( const auto& i ) { return total < std::get<0>( i ) * std::get<1>( i ); } );
      auto             unit      = std::get<1>( *i );
      std::string_view unit_name = std::get<2>( *i );
      return std::format( stat_fmt, unit_name, (double)( mean / unit ), (double)( rms / unit ),
                          (double)( minimal / unit ), (double)( maximal / unit ) );
    } else {
      return {};
    }
  };

  return std::format( fmt, unit_name, (double)( total / unit ), number, stats() );
}
ChronoEntity& ChronoEntity::operator+=( const ChronoEntity& e ) {
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

std::string ChronoEntity::outputTime( IChronoSvc::ChronoType typ, std::string_view fmt, System::TimeType unit ) const {
  boost::format _fmt( std::string{ fmt } );
  // allow various number of arguments
  using namespace boost::io;
  _fmt.exceptions( all_error_bits ^ ( too_many_args_bit | too_few_args_bit ) );
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
  _fmt % ( stat->nEntries() )           // %1 : #entries
      % ( stat->flag() / _unit )        // %2 : total time
      % ( stat->flagMean() / _unit )    // %3 : mean time
      % ( stat->flagRMS() / _unit )     // %4 : RMS  time
      % ( stat->flagMeanErr() / _unit ) // %5 : error in mean time
      % ( stat->flagMin() / _unit )     // %6 : minimal time
      % ( stat->flagMax() / _unit );    // %7 : maximal time
  return _fmt.str();
}
