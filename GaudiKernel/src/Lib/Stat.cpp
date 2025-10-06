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
#include <GaudiKernel/IStatSvc.h>
#include <GaudiKernel/Stat.h>
#include <GaudiKernel/StatEntity.h>
#include <format>
#include <iostream>

/** @file
 *  Implementation file for class Stat
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2005-08-02
 *  @date 2007-07-08
 */

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

Stat::Stat( IStatSvc* svc, const std::string& tag, const double flag ) : m_tag( tag ), m_group(), m_stat( svc ) {
  if ( m_stat ) {
    m_stat->stat( tag, flag );
    // get from the service
    m_entity = m_stat->stat( tag );
  }
}

std::string Stat::toString() const {
  std::ostringstream ost;
  print( ost );
  return ost.str();
}

std::ostream& Stat::print( std::ostream& o ) const {
  auto        entity = ( m_entity ) ? m_entity->toString() : "NULL";
  std::string tag;
  if ( !m_tag.empty() ) {
    if ( !m_group.empty() ) {
      tag = std::format( "\"{}::{}\"", m_group, m_tag );
    } else {
      tag = std::format( "\"{}\"", m_tag );
    }
  }
  return o << std::format( " {:^30} {}", tag, entity );
}

Stat operator+( const Stat& stat, const double value ) {
  Stat s( stat );
  s += value;
  return s;
}

Stat operator-( const Stat& stat, const double value ) {
  Stat s( stat );
  s -= value;
  return s;
}

Stat operator+( const double value, const Stat& stat ) {
  Stat s( stat );
  s += value;
  return s;
}

Stat operator+( const Stat& stat, const Stat& value ) {
  Stat s( stat );
  s += value;
  return s;
}

std::ostream& operator<<( std::ostream& stream, const Stat& stat ) { return stat.print( stream ); }
