// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/LinkManager.cpp,v 1.3 2004/01/23 11:22:04 mato Exp $

// Experiment specific include files
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IRegistry.h"

static LinkManager* (*s_newInstance)() = 0;

/// Access to the object's address from the link
IOpaqueAddress* LinkManager::Link::address() {
  if ( 0 != m_pObject )   {
    IRegistry* pReg = m_pObject->registry();
    if ( 0 != pReg )    {
      return pReg->address();
    }
  }
  return 0;
}

/// Assign new instantiator
void LinkManager::setInstantiator( LinkManager* (*newInstance)() ) {
  s_newInstance = newInstance;
}

/// Static instantiation
LinkManager* LinkManager::newInstance()  {
  return s_newInstance ? (*s_newInstance)() : new LinkManager();
}

/// Standard Constructor
LinkManager::LinkManager()  {
}

/// Standard Constructor
LinkManager::~LinkManager()    {
  clearLinks();
}

/// Retrieve symbolic link identified by ID
LinkManager::Link* LinkManager::link(long id)    {
  return (0<=id && (unsigned)id < m_linkVector.size()) ? m_linkVector[id] : 0;
}

/// Retrieve symbolic link identified by Object pointer
LinkManager::Link* LinkManager::link(const DataObject* pObject)  {
  if ( 0 != pObject )   {
    for ( LinkVector::iterator i = m_linkVector.begin(); i != m_linkVector.end(); i++ )   {
      if ( (*i)->object() == pObject )    {
        return (*i);
      }
    }
  }
  return 0;
}

/// Retrieve symbolic link identified by Object path
LinkManager::Link* LinkManager::link(const std::string& path)  {
  if ( 0 != path.length() )   {
    for ( LinkVector::iterator i = m_linkVector.begin(); i != m_linkVector.end(); i++ )   {
      if ( (*i)->path() == path )    {
        return (*i);
      }
    }
  }
  return 0;
}

/// Add link by object reference and path string
long LinkManager::addLink(const std::string& path, const DataObject* pObject)   const   {
  long n = 0;
  for ( LinkVector::const_iterator i = m_linkVector.begin(); i != m_linkVector.end(); i++ )   {
    Link* lnk = *i;
    const DataObject* pO = lnk->object();
    if ( 0 != pO && pO == pObject )   {
      return n;
    }
    bool same_path = lnk->path() == path;
    if ( same_path ) {
      if ( 0 != pObject && pObject != pO )  {
        lnk->setObject(pObject);
      }
      return n;
    }
    n++;
  }
  // Link is completely unknown
  Link* link = new Link(m_linkVector.size(), path, pObject);
  m_linkVector.push_back( link );
  return link->ID();
}

// Remove a link by object reference
long LinkManager::removeLink(const DataObject* pObject)  const  {
  long n = 0;
  for ( LinkVector::iterator i = m_linkVector.begin(); i != m_linkVector.end(); i++ )   {
    if ( (*i)->object() == pObject )    {
      delete (*i);
      m_linkVector.erase(i);
      return n;
    }
    n++;
  }
  return INVALID;
}

// Remove a link by object reference
long LinkManager::removeLink(const std::string& path)  const  {
  long n = 0;
  for ( LinkVector::iterator i = m_linkVector.begin(); i != m_linkVector.end(); i++ )   {
    if ( (*i)->path() == path )    {
      delete (*i);
      m_linkVector.erase(i);
      return n;
    }
    n++;
  }
  return INVALID;
}

// Remove a link by object reference
long LinkManager::removeLink(long id)  const  {
  LinkVector::iterator i = m_linkVector.begin();
  i += id;
  delete (*i);
  m_linkVector.erase(i);
  return id;
  return INVALID;
}

/// Remove all possibly existing symbolic links
void LinkManager::clearLinks()  {
  for ( LinkVector::iterator i = m_linkVector.begin(); i != m_linkVector.end(); i++ )   {
    delete (*i);
  }
  m_linkVector.erase(m_linkVector.begin(), m_linkVector.end());
}
