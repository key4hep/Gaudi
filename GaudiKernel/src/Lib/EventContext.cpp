/***********************************************************************************\
* (c) Copyright 2024-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/EventContext.h>

std::ostream& operator<<( std::ostream& os, const EventContext& ctx ) { return os << std::format( "{}", ctx ); }

std::ostream& operator<<( std::ostream& os, const EventContext* c ) {
  if ( c ) { return os << std::format( "{}", *c ); }
  return os << "INVALID";
}
