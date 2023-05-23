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

namespace {
  template <typename Container, typename Pred>
  auto findLink( Container& c, Pred pred ) {
    auto i = std::find_if( c.begin(), c.end(), pred );
    return i != c.end() ? *i : nullptr;
  }

} // namespace

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

/// Retrieve symbolic link identified by ID
const LinkManager::Link* LinkManager::link( long id ) const {
  return ( 0 <= id && (unsigned)id < m_linkVector.size() ) ? m_linkVector[id] : nullptr;
}

LinkManager::Link* LinkManager::link( long id ) {
  return ( 0 <= id && (unsigned)id < m_linkVector.size() ) ? m_linkVector[id] : nullptr;
}

/// Retrieve symbolic link identified by Object pointer
const LinkManager::Link* LinkManager::link( const DataObject* pObject ) const {
  return pObject ? findLink( m_linkVector, [=]( auto* j ) { return j->object() == pObject; } ) : nullptr;
}

LinkManager::Link* LinkManager::link( const DataObject* pObject ) {
  return pObject ? findLink( m_linkVector, [=]( auto* j ) { return j->object() == pObject; } ) : nullptr;
}

/// Retrieve symbolic link identified by Object path
const LinkManager::Link* LinkManager::link( std::string_view path ) const {
  return !path.empty() ? findLink( m_linkVector, [=]( auto* j ) { return j->path() == path; } ) : nullptr;
}
LinkManager::Link* LinkManager::link( std::string_view path ) {
  return !path.empty() ? findLink( m_linkVector, [=]( auto* j ) { return j->path() == path; } ) : nullptr;
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
