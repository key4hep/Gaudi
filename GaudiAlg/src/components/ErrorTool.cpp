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
#include "ErrorTool.h"
// ============================================================================

// ============================================================================
/** @file
 * Implementation file for class : ErrorTool
 * @date 2004-06-28
 * @author Vanay  BELYAEV Ivan.Belyaev@itep.ru
 */
// ============================================================================

// ============================================================================
// Declaration of the Tool Factory
// ============================================================================
DECLARE_COMPONENT( ErrorTool )
// ============================================================================

// ============================================================================
/** Standard constructor
 *  @see GaudiTool
 *  @see   AlgTool
 *  @param type   tool type (?)
 *  @param name   tool name
 *  @param parent pointer to parent component
 */
// ============================================================================
ErrorTool::ErrorTool( const std::string& type, const std::string& name, const IInterface* parent )
    : GaudiTool( type, name, parent ) {
  declareInterface<IErrorTool>( this );
}
// ============================================================================
