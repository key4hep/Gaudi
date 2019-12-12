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
#ifndef JOBOPTIONSVC_INCLUDED_FILES_H_
#define JOBOPTIONSVC_INCLUDED_FILES_H_
// ============================================================================
// Includes:
// ============================================================================
// STD & STL:
// ============================================================================
#include <map>
#include <string>
// ============================================================================
namespace Gaudi {
  namespace Parsers {
    // ============================================================================
    class Position;
    // ============================================================================
    class IncludedFiles {
    public:
      typedef std::map<std::string, Position> Container;

    public:
      bool AddFile( std::string filename, Position from );
      bool IsIncluded( const std::string& filename ) const;
      bool GetPosition( const std::string& filename, const Position** pos ) const;

    private:
      Container container_;
    };
    // ============================================================================
  } // namespace Parsers
} // namespace Gaudi
// ============================================================================
#endif //  JOBOPTIONSVC_INCLUDED_FILES_H_
