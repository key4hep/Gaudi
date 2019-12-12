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

// External include(s):
#include <boost/format.hpp>
#include <uuid/uuid.h>

// Local include(s):
#include "createGuidAsString.h"

std::string Gaudi::createGuidAsString() {
  uuid_t uuid;
  ::uuid_generate_time( uuid );
  struct Guid {
    unsigned int   Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
  }* g = (Guid*)&uuid;

  boost::format text( "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X" );
  text % g->Data1 % g->Data2 % g->Data3;
  for ( int i = 0; i < 8; ++i ) text % (unsigned short)g->Data4[i];
  return text.str();
}
