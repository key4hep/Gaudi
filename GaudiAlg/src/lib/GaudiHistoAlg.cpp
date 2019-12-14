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
#include "GaudiAlg/GaudiHistoAlg.h"
// ============================================================================

// ============================================================================
/** @file GaudiHistoAlg.cpp
 *
 *  Implementation file for class : GaudiHistoAlg
 *
 *  @author Vanya  BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date 2003-12-11
 */
// ============================================================================

// ============================================================================
// Force creation of templated class
#include "GaudiAlg/GaudiHistos.icpp"
// ============================================================================
template class GaudiHistos<GaudiAlgorithm>;

// ============================================================================
// The END
// ============================================================================
