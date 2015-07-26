// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/LinkManager.cpp,v 1.3 2004/01/23 11:22:04 mato Exp $

// Experiment specific include files
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IRegistry.h"

static LinkManager* (*s_newInstance)() = 0;

/// Access to the object's address from the link
IOpaqueAddress* LinkManager::Link::address() {
  if ( m_pObject ) {
    IRegistry* pReg = m_pObject->registry();
    if ( pReg )  return pReg->address();
  }
  return nullptr;
}

/// Assign new instantiator
void LinkManager::setInstantiator( LinkManager* (*newInstance)() ) {
  s_newInstance = newInstance;
}

/// Static instantiation
LinkManager* LinkManager::newInstance()  {
  return s_newInstance ? (*s_newInstance)() : new LinkManager();
}

/// Retrieve symbolic link identified by ID
LinkManager::Link* LinkManager::link(long id)    {
  return (0<=id && (unsigned)id < m_linkVector.size()) ? m_linkVector[id].get() : nullptr;
}

/// Retrieve symbolic link identified by Object pointer
LinkManager::Link* LinkManager::link(const DataObject* pObject)  {
  if ( pObject )   {
    for ( auto& i : m_linkVector ) {
      if ( i->object() == pObject ) return i.get();
    }
  }
  return nullptr;
}

/// Retrieve symbolic link identified by Object path
LinkManager::Link* LinkManager::link(const std::string& path)  {
  if ( 0 != path.length() )   {
    for ( auto & i : m_linkVector ) {
      if ( i->path() == path )    return i.get();
    }
  }
  return nullptr;
}

/// Add link by object reference and path string
long LinkManager::addLink(const std::string& path, const DataObject* pObject)   const   {
  long n = 0;
  for ( auto& lnk : m_linkVector ) {
    const DataObject* pO = lnk->object();
    if ( pO && pO == pObject )  return n;
    bool same_path = lnk->path() == path;
    if ( same_path ) {
      if ( pObject && pObject != pO )  {
        lnk->setObject(pObject);
      }
      return n;
    }
    ++n;
  }
  // Link is completely unknown
  m_linkVector.emplace_back( new Link(m_linkVector.size(), path, pObject) );
  return m_linkVector.back()->ID();
}

// Remove a link by object reference
long LinkManager::removeLink(const DataObject* pObject)  const  {
  long n = 0;
  for ( auto i = m_linkVector.begin(); i != m_linkVector.end(); i++ )   {
    if ( (*i)->object() == pObject )    {
      m_linkVector.erase(i);
      return n;
    }
    ++n;
  }
  return INVALID;
}

// Remove a link by object reference
long LinkManager::removeLink(const std::string& path)  const  {
  long n = 0;
  for ( auto i = m_linkVector.begin(); i != m_linkVector.end(); i++ )   {
    if ( (*i)->path() == path )    {
      m_linkVector.erase(i);
      return n;
    }
    n++;
  }
  return INVALID;
}

// Remove a link by object reference
long LinkManager::removeLink(long id)  const  {
  auto i = std::next(m_linkVector.begin(), id );
  m_linkVector.erase(i);
  return id;
}

/// Remove all possibly existing symbolic links
void LinkManager::clearLinks()  {
  m_linkVector.clear();
}
