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
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiTupleAlg.h"
// ============================================================================

// ============================================================================
/*  @file GaudiTupleAlg.cpp
 *
 *  Implementation file for class : GaudiTupleAlg
 *
 *  @date 2004-01-23
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 */
// ============================================================================

// ============================================================================
// Force creation of templated class
#include "GaudiAlg/GaudiTuples.icpp"
template class GaudiTuples<GaudiHistoAlg>;
// ============================================================================

// ============================================================================
// The END
// ============================================================================
