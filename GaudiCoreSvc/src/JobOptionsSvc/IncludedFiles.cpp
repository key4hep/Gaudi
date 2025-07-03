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
#include "IncludedFiles.h"
#include "Position.h"
#include <assert.h>

namespace gp = Gaudi::Parsers;

bool gp::IncludedFiles::IsIncluded( std::string_view filename ) const {
  return container_.find( filename ) != container_.end();
}
bool gp::IncludedFiles::AddFile( std::string filename, Position from ) {
  return container_.emplace( std::move( filename ), std::move( from ) ).second;
}
bool gp::IncludedFiles::GetPosition( std::string_view filename, const Position** pos ) const {
  auto iter = container_.find( filename );
  if ( iter == container_.end() ) return false;
  *pos = &iter->second;
  assert( pos );
  return true;
}
