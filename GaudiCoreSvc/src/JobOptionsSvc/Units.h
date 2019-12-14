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
#ifndef JOBOPTIONSVC_UNITS_H_
#define JOBOPTIONSVC_UNITS_H_
// ============================================================================
// Include files
// ============================================================================
// STD & STL:
// ============================================================================
#include <map>
#include <string>
// ============================================================================
#include "Position.h"
// ============================================================================

// ============================================================================
namespace Gaudi {
  namespace Parsers {
    // ============================================================================
    class Units final {
    public:
      typedef std::map<std::string, std::pair<double, Position>> Container;
      typedef Container::mapped_type                             ValueWithPosition;

      bool Add( std::string name, double value );
      bool Add( std::string name, double value, const Position& pos );

      bool        Find( const std::string& name, ValueWithPosition& result ) const;
      bool        Find( const std::string& name, double& result ) const;
      std::string ToString() const;

    private:
      Container units_;
    };

    // ============================================================================
  } // namespace Parsers
} // namespace Gaudi
// ============================================================================

#endif /* UNITS_H_ */
