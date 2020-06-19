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

#include "createGuidAsString.h"
#include <uuid/uuid.h>

std::string Gaudi::createGuidAsString() {
  uuid_t      uuid;
  std::string uuid_s( 36, '0' );
  ::uuid_generate_time( uuid );
  ::uuid_unparse_upper( uuid, uuid_s.data() );
  return uuid_s;
}
