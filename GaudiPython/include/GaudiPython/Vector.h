/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIPYTHON_VECTOR_H
#define GAUDIPYTHON_VECTOR_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <functional>
#include <vector>
// ============================================================================
// GaudiPython
// ============================================================================
#include <GaudiPython/GaudiPython.h>
// ============================================================================

namespace GaudiPython {
  /// useful type definition for  implicit loos
  typedef std::vector<double> Vector;
  typedef Vector              Row;
  typedef std::vector<Row>    Matrix;

  /** @struct _identity
   *  It is here due to 'missing'(?) std::identity
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date 2005-08-02
   */
  struct _identity {
    template <typename T>
    T operator()( const T& value ) const {
      return value;
    }
  };

} // end of namespace GaudiPython
#endif // GAUDIPYTHON_VECTOR_H
