/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiKernel/Stat.h"
#include "GaudiKernel/IStatSvc.h"
#include "GaudiKernel/StatEntity.h"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <sstream>
// ============================================================================
/** @file
 *  Implementation file for class Stat
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2005-08-02
 *  @date 2007-07-08
 */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<StatEntity> : fmt::ostream_formatter {};
#endif

// ============================================================================
/*   Constructor from IStatSvc,tag and value
 *
 *   @code
 *
 *   IStatSvc* statSvc = ... ;
 *   double eTotal = .... ;
 *
 *   Stat eTot ( statSvc , "total energy" ) ;
 *   eTot += eTotal ;
 *
 *   @endcode
 *
 *   @see IStatSvc
 *   @param svc pointer to Chrono&Stat Service
 *   @paran tag unique tag for the entry
 */
// ============================================================================
Stat::Stat( IStatSvc* svc, const std::string& tag ) : m_tag( tag ), m_group(), m_stat( svc ) {
  if ( m_stat ) {
    // get from the service
    StatEntity* tmp = m_stat->stat( tag );
    if ( !tmp ) {
      // create if needed
      m_stat->stat( tag, 0 );
      tmp = m_stat->stat( tag );
      tmp->reset();
    }
    m_entity = tmp;
  }
}
// ============================================================================
/*   Constructor from IStatSvc,tag and value
 *
 *   @code
 *
 *   IStatSvc* statSvc = ... ;
 *   double eTotal = .... ;
 *
 *   Stat stat( statSvc , "total energy" , eTotal ) ;
 *
 *   @endcode
 *
 *   @see IStatSvc
 *   @param svc pointer to Chrono&Stat Service
 *   @paran tag unique tag for the entry
 *   @param flag    "flag"(additive quantity) to be used
 */
// ============================================================================
Stat::Stat( IStatSvc* svc, const std::string& tag, const double flag ) : m_tag( tag ), m_group(), m_stat( svc ) {
  if ( m_stat ) {
    m_stat->stat( tag, flag );
    // get from the service
    m_entity = m_stat->stat( tag );
  }
}

// ============================================================================
// representation as string
// ============================================================================
std::string Stat::toString() const {
  std::ostringstream ost;
  print( ost );
  return ost.str();
}
// ============================================================================
/*  printout to std::ostream
 *  @param o the reference to the output stream
 */
// ============================================================================
std::ostream& Stat::print( std::ostream& o ) const {
  auto        entity = ( m_entity ) ? fmt::format( "{}", *m_entity ) : "NULL";
  std::string tag;
  if ( !m_tag.empty() ) {
    if ( !m_group.empty() ) {
      tag = fmt::format( "\"{}::{}\"", m_group, m_tag );
    } else {
      tag = fmt::format( "\"{}\"", m_tag );
    }
  }
  return o << fmt::format( " {:^30} {}", tag, entity );
}
// ============================================================================
// external operator for addition of Stat and a number
// ============================================================================
Stat operator+( const Stat& stat, const double value ) {
  Stat s( stat );
  s += value;
  return s;
}
// ============================================================================
// external operator for subtraction of Stat and a number
// ============================================================================
Stat operator-( const Stat& stat, const double value ) {
  Stat s( stat );
  s -= value;
  return s;
}
// ============================================================================
// external operator for addition of Stat and a number
// ============================================================================
Stat operator+( const double value, const Stat& stat ) {
  Stat s( stat );
  s += value;
  return s;
}
// ============================================================================
// external operator for addition of Stat and Stat
Stat operator+( const Stat& stat, const Stat& value ) {
  Stat s( stat );
  s += value;
  return s;
}
// ============================================================================
// external printout operator to std::ostream
std::ostream& operator<<( std::ostream& stream, const Stat& stat ) { return stat.print( stream ); }
// ============================================================================

// ============================================================================
// The END
// ============================================================================
