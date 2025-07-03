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
#pragma once
// ============================================================================
// Includes:
// ============================================================================
// STD & STL:
// ============================================================================
#include <map>
#include <string>
#include <string_view>
// ============================================================================
namespace Gaudi {
  namespace Parsers {
    // ============================================================================
    class Position;
    // ============================================================================
    class IncludedFiles {
    public:
      typedef std::map<std::string, Position, std::less<>> Container;

    public:
      bool AddFile( std::string filename, Position from );
      bool IsIncluded( std::string_view filename ) const;
      bool GetPosition( std::string_view filename, const Position** pos ) const;

    private:
      Container container_;
    };
    // ============================================================================
  } // namespace Parsers
} // namespace Gaudi
// ============================================================================
