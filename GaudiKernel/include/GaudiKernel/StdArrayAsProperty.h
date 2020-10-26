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
#ifndef GAUDIKERNEL_STDARRAYASPROPERTY_H
#define GAUDIKERNEL_STDARRAYASPROPERTY_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include "GaudiKernel/ToStream.h"
#include <algorithm>
#include <array>
// ============================================================================
/** @file
 *  Collection of utilities, which allows to use class std::array
 *  as property for Gaudi-components
 *
 *  @attention this file must be "included" before Gaudi/Property.h
 *
 *  @author Alexander Mazurov alexander.mazurov@cern.ch
 *  @date 2015-02-23
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-09-16
 */
// ============================================================================
#include <Gaudi/Parsers/CommonParsers.h>
// ============================================================================
namespace Gaudi {
  // ==========================================================================
  namespace Parsers {
    // ========================================================================
    /** parse class std::array from the string
     *  @param result (OUTPUT) the actual result
     *  @param input  (INPUT)  the input string
     *  @return status code
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2009-09-16
     */
    template <class TYPE, std::size_t N>
    StatusCode parse( std::array<TYPE, N>& result, const std::string& input ) {
      std::vector<TYPE> tmp;
      StatusCode        sc = parse( tmp, input );
      if ( sc.isFailure() ) return sc; // RETURN
      if ( N != tmp.size() ) return StatusCode( StatusCode::FAILURE, true );
      std::copy( tmp.begin(), tmp.end(), result.begin() );
      return StatusCode::SUCCESS;
    }
    // ========================================================================
  } // namespace Parsers
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_STDARRAYASPROPERTY_H
