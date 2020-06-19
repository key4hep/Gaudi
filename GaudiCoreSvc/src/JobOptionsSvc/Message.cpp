/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "Messages.h"

#include <fmt/format.h>

namespace gp = Gaudi::Parsers;

void gp::Messages::AddMessage( MSG::Level level, const std::string& message ) { stream_ << level << message << endmsg; }

void gp::Messages::AddMessage( MSG::Level level, const Position& pos, const std::string& message ) {
  if ( pos.filename() != m_currentFilename ) {
    stream_ << level << "# =======> " << pos.filename() << endmsg;
    m_currentFilename = pos.filename();
  }
  stream_ << level << fmt::format( "# ({},{}): {}", pos.line(), pos.column(), message ) << endmsg;
}
