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
#include <GaudiKernel/ContainedObject.h>
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/LinkManager.h>
#include <GaudiKernel/ObjectContainerBase.h>
#include <GaudiKernel/SmartRefBase.h>
#include <GaudiKernel/StreamBuffer.h>

typedef ObjectContainerBase _Container;

/// Setup smart reference
void SmartRefBase::set( DataObject* pObj, long hint_id, long link_id ) {
  m_data   = pObj;
  m_hintID = hint_id;
  m_linkID = link_id;
}

/// Load on demand: ContainedObject type references
const ContainedObject* SmartRefBase::accessData( const ContainedObject* ) const {
  if ( m_hintID != StreamBuffer::INVALID && m_linkID != StreamBuffer::INVALID ) {
    const _Container* cnt = dynamic_cast<const _Container*>( accessData( m_data ) );
    if ( cnt ) return cnt->containedObject( m_linkID );
  }
  return nullptr;
}

/// Load on demand: DataObject type references
const DataObject* SmartRefBase::accessData( const DataObject* ) const {
  if ( !m_data && m_contd ) m_data = m_contd->parent();
  DataObject* source = const_cast<DataObject*>( m_data );
  if ( m_hintID == StreamBuffer::INVALID || !source ) return nullptr;
  LinkManager* mgr = source->linkMgr();
  if ( !mgr ) return nullptr;
  LinkManager::Link* link = mgr->link( m_hintID );
  if ( !link ) return nullptr;
  DataObject* target = link->object();
  if ( !target ) {
    IRegistry* reg = source->registry();
    if ( !reg ) return nullptr;
    IDataProviderSvc* datasvc = reg->dataSvc();
    if ( datasvc && datasvc->retrieveObject( link->path(), target ).isSuccess() ) { link->setObject( target ); }
  }
  return target;
}

// Extended equality check
bool SmartRefBase::isEqualEx( const DataObject* pObj, const SmartRefBase& c ) const {
  if ( c.m_hintID != StreamBuffer::INVALID && pObj ) {
    DataObject* source = const_cast<DataObject*>( c.m_data );
    if ( source ) {
      LinkManager* mgr = source->linkMgr();
      if ( mgr ) {
        const LinkManager::Link* link = mgr->link( c.m_hintID );
        if ( link ) {
          IRegistry* pReg = pObj->registry();
          return pReg && link->path() == pReg->identifier();
        }
      }
    }
  }
  return false;
}

// Extended equality check
bool SmartRefBase::isEqualEx( const ContainedObject* pObj, const SmartRefBase& c ) const {
  return isEqualEx( pObj->parent(), c ) && pObj->index() == c.m_linkID;
}

const std::string& SmartRefBase::path() const {
  static std::string s_empty_string{};
  DataObject*        source = nullptr;
  if ( !m_data && m_contd ) m_data = m_contd->parent();
  source = const_cast<DataObject*>( m_data );
  if ( m_hintID != StreamBuffer::INVALID && source ) {
    LinkManager* mgr = source->linkMgr();
    if ( mgr ) {
      const LinkManager::Link* link = mgr->link( m_hintID );
      if ( link ) return link->path();
    }
  }
  return s_empty_string;
}
