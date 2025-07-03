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
// STD & STL
// ============================================================================
#include <string>
#include <string_view>
// ============================================================================
// Boost
// ============================================================================

// ============================================================================
// Gaudi:
// ============================================================================
#include "Position.h"
#include <GaudiKernel/MsgStream.h>
// ============================================================================
namespace Gaudi {
  namespace Parsers {
    class Messages final {
    public:
      Messages( MsgStream& stream ) : stream_( stream ) {}
      // Messages(const MsgStream& stream):stream_(stream){}
      void AddInfo( std::string_view info ) { AddMessage( MSG::INFO, info ); }

      void AddWarning( std::string_view warning ) { AddMessage( MSG::WARNING, warning ); }

      void AddError( std::string_view error ) { AddMessage( MSG::ERROR, error ); }

      void AddInfo( const Position& pos, std::string_view info ) { AddMessage( MSG::INFO, pos, info ); }

      void AddWarning( const Position& pos, std::string_view warning ) { AddMessage( MSG::WARNING, pos, warning ); }

      void AddError( const Position& pos, std::string_view error ) { AddMessage( MSG::ERROR, pos, error ); }

    private:
      void AddMessage( MSG::Level level, std::string_view message );

      void AddMessage( MSG::Level level, const Position& pos, std::string_view message );

    private:
      MsgStream& stream_;
      /// Name of last printed filename.
      std::string m_currentFilename;
    };

    // ============================================================================

    // ============================================================================
  } // namespace Parsers
} // namespace Gaudi
// ============================================================================
