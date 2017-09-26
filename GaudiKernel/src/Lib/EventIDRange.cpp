#include "GaudiKernel/EventIDRange.h"

/*****************************************************************************
 *
 *  EventIDRange.cpp
 *
 *  Author: Charles Leggett
 *
 *  Validity Range object. Holds two EventIDBases (start and stop)
 *
 *****************************************************************************/

EventIDRange::EventIDRange( const EventIDBase& start, const EventIDBase& stop ) : m_start( start ), m_stop( stop ) {}

EventIDRange& EventIDRange::operator=( const EventIDRange& r )
{
  if ( this != &r ) {
    m_start = r.m_start;
    m_stop  = r.m_stop;
  }
  return *this;
}

std::ostream& operator<<( std::ostream& os, const EventIDRange& rhs )
{
  os << (std::string)rhs;
  return os;
}
