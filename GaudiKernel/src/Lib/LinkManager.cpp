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
// Experiment specific include files
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IRegistry.h"
#include <algorithm>

static LinkManager* ( *s_newInstance )() = nullptr;

/// destructor
LinkManager::~LinkManager() {
  for ( auto& i : m_linkVector ) delete i;
}

/// Access to the object's address from the link
IOpaqueAddress* LinkManager::Link::address() {
  if ( m_pObject ) {
    IRegistry* pReg = m_pObject->registry();
    if ( pReg ) return pReg->address();
  }
  return nullptr;
}

/// Assign new instantiator
void LinkManager::setInstantiator( LinkManager* ( *newInstance )() ) { s_newInstance = newInstance; }

/// Static instantiation
LinkManager* LinkManager::newInstance() { return s_newInstance ? ( *s_newInstance )() : new LinkManager(); }

/// Retrieve symbolic link identified by ID
LinkManager::Link* LinkManager::link( long id ) {
  return ( 0 <= id && (unsigned)id < m_linkVector.size() ) ? m_linkVector[id] : nullptr;
}

/// Retrieve symbolic link identified by Object pointer
LinkManager::Link* LinkManager::link( const DataObject* pObject ) {
  if ( !pObject ) return nullptr;
  auto i = std::find_if( m_linkVector.begin(), m_linkVector.end(), [=]( auto* j ) { return j->object() == pObject; } );
  return i != m_linkVector.end() ? *i : nullptr;
}

/// Retrieve symbolic link identified by Object path
LinkManager::Link* LinkManager::link( std::string_view path ) {
  if ( path.empty() ) return nullptr;
  auto i = std::find_if( m_linkVector.begin(), m_linkVector.end(), [=]( auto* j ) { return j->path() == path; } );
  return i != m_linkVector.end() ? *i : nullptr;
}

/// Add link by object reference and path string
long LinkManager::addLink( const std::string& path, const DataObject* pObject ) {
  long n = 0;
  for ( auto& lnk : m_linkVector ) {
    const DataObject* pO = lnk->object();
    if ( pO && pO == pObject ) return n;
    if ( lnk->path() == path ) {
      if ( pObject && pObject != pO ) { lnk->setObject( const_cast<DataObject*>( pObject ) ); }
      return n;
    }
    ++n;
  }
  // Link is completely unknown
  return m_linkVector.emplace_back( new Link( m_linkVector.size(), path, const_cast<DataObject*>( pObject ) ) )->ID();
}
