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
#ifndef GAUDIALG_GAUDITUPLETOOL_H
#define GAUDIALG_GAUDITUPLETOOL_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiHistoTool.h"
#include "GaudiAlg/GaudiTuples.h"
// ============================================================================

// ============================================================================
/*  @file GaudiTupleTool.h
 *
 *  Header file for class : GaudiTupleTool
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   2004-06-28
 */
// ============================================================================

// ============================================================================
/** @class GaudiTupleTool GaudiTupleTool.h GaudiAlg/GaudiTupleTool.h
 *
 *  Simple class to extend the functionality of class GaudiHistoTool.
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
 *  @date   2004-06-28
 */
// ============================================================================

struct GAUDI_API GaudiTupleTool : GaudiTuples<GaudiHistoTool> {

  using GaudiTuples<GaudiHistoTool>::GaudiTuples;

  GaudiTupleTool()                        = delete;
  GaudiTupleTool( const GaudiTupleTool& ) = delete;
  GaudiTupleTool& operator=( const GaudiTupleTool& ) = delete;
};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDITUPLETOOL_H
