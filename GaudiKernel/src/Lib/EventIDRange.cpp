/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/EventIDRange.h>

/*****************************************************************************
 *
 *  EventIDRange.cpp
 *
 *  Author: Charles Leggett
 *
 *  Validity Range object. Holds two EventIDBases (start and stop)
 *
 *****************************************************************************/

EventIDRange::EventIDRange()
    : // defauld constructor: Range covering everything
    m_start( 0, 0, 0, 0, 0, 0 )
    , m_stop( EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM,
              EventIDBase::UNDEFNUM, 0 ) {}

EventIDRange::EventIDRange( const EventIDBase& start, const EventIDBase& stop ) : m_start( start ), m_stop( stop ) {}

EventIDRange& EventIDRange::operator=( const EventIDRange& r ) {
  if ( this != &r ) {
    m_start = r.m_start;
    m_stop  = r.m_stop;
  }
  return *this;
}

std::ostream& operator<<( std::ostream& os, const EventIDRange& rhs ) {
  os << (std::string)rhs;
  return os;
}
