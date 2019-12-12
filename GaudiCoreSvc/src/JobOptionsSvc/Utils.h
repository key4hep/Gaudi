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
// ===========================================================================
#ifndef JOBOPTIONSVC_UNITS_H_
#define JOBOPTIONSVC_UNITS_H_ 1
// ===========================================================================
// Include files
// ===========================================================================
#include <string>

namespace Gaudi {
  namespace Parsers {
    namespace Utils {
      // ===========================================================================
      std::string replaceEnvironments( const std::string& input );
      // ===========================================================================
    } // namespace Utils
  }   // namespace Parsers
} // namespace Gaudi
// ===========================================================================
#endif
