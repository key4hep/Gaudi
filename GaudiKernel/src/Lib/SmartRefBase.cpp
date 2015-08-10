//	====================================================================
//	SmartRefBase.cpp
//	--------------------------------------------------------------------
//
//	Package   : Root conversion example
//
//	Author    : Markus Frank
//
//	====================================================================
#define KERNEL_SMARTREFBASE_CPP 1

// Framework include files
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/SmartRefBase.h"
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/ObjectContainerBase.h"

typedef ObjectContainerBase _Container;

/// Setup smart reference
void  SmartRefBase::set(DataObject* pObj, long hint_id, long link_id) {
  m_data = pObj;
  m_hintID = hint_id;
  m_linkID = link_id;
}

/// Load on demand: ContainedObject type references
const ContainedObject* SmartRefBase::accessData(const ContainedObject*)   const   {
  if ( m_hintID != StreamBuffer::INVALID && m_linkID != StreamBuffer::INVALID )   {
    const _Container* cnt = dynamic_cast<const _Container*>(accessData(m_data));
    if ( 0 != cnt )  {
      return cnt->containedObject(m_linkID);
    }
  }
  return 0;
}

/// Load on demand: DataObject type references
const DataObject* SmartRefBase::accessData(const DataObject*)   const   {
  DataObject* target = 0;
  DataObject* source  =0;
  if ( 0 == m_data && 0 != m_contd )  {
    m_data = m_contd->parent();
  }
  source  = const_cast<DataObject*>(m_data);            
  if ( m_hintID != StreamBuffer::INVALID && source != 0 )   {
    LinkManager* mgr = source->linkMgr();
    if ( 0 != mgr )   {
      LinkManager::Link* link = mgr->link(m_hintID);
      if ( 0 != link )  {
        target = link->object();
        if ( 0 == target )    {
          IRegistry* reg = source->registry();
          if ( 0 != reg )   {
            IDataProviderSvc* datasvc = reg->dataSvc();
            if ( 0 != datasvc )   {
              if ( datasvc->retrieveObject(link->path(), target).isSuccess() )   {
                link->setObject(target);
              }
            }
          }
        }
      }
    }
  }
  return target;
}

// Extended equality check
bool SmartRefBase::isEqualEx(const DataObject* pObj, const SmartRefBase& c)  const    {
  if ( c.m_hintID != StreamBuffer::INVALID && 0 != pObj )   {
    DataObject* source  = const_cast<DataObject*>(c.m_data);
    if ( 0 != source )   {
      LinkManager* mgr = source->linkMgr();
      if ( 0 != mgr )   {
        LinkManager::Link* link = mgr->link(c.m_hintID);
        if ( 0 != link )  {
          IRegistry* pReg = pObj->registry();
          if ( pReg != 0 )   {
            return link->path() == pReg->identifier();
          }
        }
      }
    }
  }
  return false;
}

// Extended equality check
bool SmartRefBase::isEqualEx(const ContainedObject* pObj, const SmartRefBase& c)  const   {
  if ( isEqualEx(pObj->parent(), c) )   {
    return pObj->index() == c.m_linkID;
  }
  return false;
}

const std::string &SmartRefBase::path () const {
  static std::string s_empty_string = std::string();
  DataObject *source = 0;
  if ( 0 == m_data && 0 != m_contd )  {
    m_data = m_contd->parent();
  }
  source = const_cast<DataObject*>(m_data);   
  if ( m_hintID != StreamBuffer::INVALID && source != 0 ) {
    LinkManager* mgr = source->linkMgr();
    if ( 0 != mgr ) {
      LinkManager::Link* link = mgr->link(m_hintID);
      if ( 0 != link )
        return link->path();
    }
  }
  return s_empty_string;
}
