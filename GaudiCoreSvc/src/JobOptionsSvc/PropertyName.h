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
#ifndef JOBOPTIONSVC_PROPERTY_NAME_H_
#define JOBOPTIONSVC_PROPERTY_NAME_H_
// ============================================================================
// Includes:
// ============================================================================
// STD & STL:
// ============================================================================
#include "Position.h"
#include <string>
// ============================================================================
namespace Gaudi {
  namespace Parsers {
    // ============================================================================
    class Position;
    // ============================================================================
    class PropertyName final {
    public:
      // ----------------------------------------------------------------------------
      explicit PropertyName( std::string property ) : property_( std::move( property ) ) {}
      PropertyName( std::string property, const Position& pos )
          : property_( std::move( property ) ), position_( pos ) {}
      PropertyName( std::string client, std::string property )
          : client_( std::move( client ) ), property_( std::move( property ) ) {}
      PropertyName( std::string client, std::string property, Position pos )
          : client_( std::move( client ) ), property_( std::move( property ) ), position_( std::move( pos ) ) {}
      const std::string& client() const { return client_; }
      const std::string& property() const { return property_; }
      const Position&    position() const { return position_; }
      std::string        FullName() const;
      std::string        ToString() const;
      bool               HasClient() const { return !client_.empty(); }
      bool               HasPosition() const { return position_.Exists(); }

    private:
      std::string client_;
      std::string property_;
      Position    position_;
    };
    // ============================================================================
  } // namespace Parsers
} // namespace Gaudi
// ============================================================================
#endif // JOBOPTIONSVC_PROPERTY_NAME_H_
