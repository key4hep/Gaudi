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
#include "TupleTool.h"
// ============================================================================

/** @file
 * Implementation file for class : TupleTool
 *
 * @date 2004-06-28
 * @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 */

// ============================================================================
// Declaration of the Tool Factory
// ============================================================================
DECLARE_COMPONENT( TupleTool )
// ============================================================================

// ============================================================================
/** Standard constructor
 *  @see GaudiTupleTool
 *  @see GaudiHistoTool
 *  @see      GaudiTool
 *  @see        AlgTool
 *  @param type   tool type (?)
 *  @param name   tool name
 *  @param parent pointer to parent component
 */
// ============================================================================
TupleTool::TupleTool( const std::string& type, const std::string& name, const IInterface* parent )
    : GaudiTupleTool( type, name, parent ) {
  declareInterface<ITupleTool>( this );
}
