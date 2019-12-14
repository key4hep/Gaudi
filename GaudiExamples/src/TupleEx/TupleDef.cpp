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
// ============================================================================
// Include files
// ============================================================================
// local
// ============================================================================
#include "TupleDef.h"
// ============================================================================

/** @file
 *  Implementation file for helper function TupleExample::fillNTuple
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2005-11-29
 */

namespace TupleExample {

  /** helper function useful for the partial
   *  specialization of N-Tuple fills
   *  @param t tuple
   *  @param n base column name
   *  @param p data dublet
   *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   *  @date   2005-11-29
   */
  Tuples::Tuple& fillNTuple( Tuples::Tuple& t, const std::string& n, const Dublet& p ) {
    if ( !t.valid() ) { return t; }
    t->column( n + "F", p.first );
    t->column( n + "S", p.second );
    return t;
  }

  /** helper function useful for the partial
   *  specialization of N-Tuple fills
   *  @param t tuple
   *  @param n base column name
   *  @param p data triplet
   *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   *  @date   2005-11-29
   */
  Tuples::Tuple& fillNTuple( Tuples::Tuple& t, const std::string& n, const Triplet& p ) {
    if ( !t.valid() ) { return t; }
    t->column( n + "F", p.first.first );
    t->column( n + "S", p.first.second );
    t->column( n + "T", p.second );
    return t;
  }
} // namespace TupleExample

// ============================================================================
// The END
// ============================================================================
