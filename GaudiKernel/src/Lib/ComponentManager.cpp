/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
/*
 * ComponentManager.cpp
 *
 *  Created on: Feb 20, 2009
 *      Author: Marco Clemencic
 */

#include <GaudiKernel/ComponentManager.h>

//------------------------------------------------------------------------------
// Constructor
ComponentManager::ComponentManager( IInterface* application, const InterfaceID& baseIID )
    : m_application( application ), m_stateful( application ), m_basicInterfaceId( baseIID ) {}

//------------------------------------------------------------------------------
// Basic interface id of the managed components.
const InterfaceID& ComponentManager::componentBaseInterface() const { return m_basicInterfaceId; }

void const* ComponentManager::i_cast( const InterfaceID& iid ) const {
  // try local interfaces
  if ( auto output = base_class::i_cast( iid ); output ) { return output; }
  // fall back on the owner, if local interface didn't match...
  return m_application->i_cast( iid );
}
