// $Id: PoolDbLinkManager.cpp,v 1.4 2008/01/17 13:20:51 marcocle Exp $
//====================================================================
//	PoolDbLinkManager implementation
//--------------------------------------------------------------------
//
//	Package    : System ( The LHCb Offline System)
//
//  Description: Link Manager for POOL objects
//
//	Author     : M.Frank
//====================================================================

// Framework include files
#include "GaudiPoolDb/PoolDbLinkManager.h"
#include "StorageSvc/DbInstanceCount.h"
#include "POOLCore/Token.h"

static pool::DbInstanceCount::Counter* counter() {
  static pool::DbInstanceCount::Counter* s_count = 0;
  if ( 0 == s_count )  {
    s_count = pool::DbInstanceCount::getCounter(typeid(PoolDbLinkManager));
  }
  return s_count;
}

// Standard constructor
PoolDbLinkManager::PoolDbLinkManager() 
{
  static pool::DbInstanceCount::Counter* c = counter();
  c->increment();
}

// Standard destructor
PoolDbLinkManager::~PoolDbLinkManager() 
{
  static pool::DbInstanceCount::Counter* c = counter();
  c->decrement();
  for (std::vector<pool::Token*>::iterator i = m_refs.begin(); i != m_refs.end(); ++i )  {
    if ( *i )  {
      (*i)->release();
    }
  }
  m_links.clear();
  m_refs.clear();
}
