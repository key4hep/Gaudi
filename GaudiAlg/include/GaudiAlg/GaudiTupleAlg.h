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
#ifndef GAUDIALG_GAUDITUPLEALG_H
#define GAUDIALG_GAUDITUPLEALG_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiHistoAlg.h"
#include "GaudiAlg/GaudiTuples.h"
// ============================================================================

// ============================================================================
/* @file GaudiTupleAlg.h
 *
 * Header file for class : GaudiTupleAlg
 *
 * @date 2004-01-23
 * @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 * @author Chris Jones   Christopher.Rob.Jones@cern.ch
 */
// ============================================================================

// ============================================================================
/** @class GaudiTupleAlg GaudiTupleAlg.h GaudiAlg/GaudiTupleAlg.h
 *
 *  Simple class to extend the functionality of class GaudiHistoAlg.
 *
 *  Class is instrumented with few methods
 *  for dealing with N-tuples and Event Tag collections
 *
 *  @attention
 *  See the class GaudiTuples, which implements the common functionality
 *  between GaudiTupleTool and GaudiTupleAlg
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   2004-01-23
 */
// ============================================================================

struct GAUDI_API GaudiTupleAlg : GaudiTuples<GaudiHistoAlg> {
  using GaudiTuples<GaudiHistoAlg>::GaudiTuples;
};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDITUPLEALG_H
