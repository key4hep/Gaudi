
#include "GaudiKernel/DataHandleHolderVisitor.h"
#include "GaudiKernel/DataHandle.h"
#include <iostream>

DHHVisitor::DHHVisitor(DataObjIDColl& ido, DataObjIDColl& odo) :
  m_ido(ido), m_odo(odo) {
}


void 
DHHVisitor::visit(const IDataHandleHolder* idhh) {

  if (idhh == 0) return;
  for (auto h : idhh->inputHandles()) {
    if (! h->objKey().empty()) { 
      m_ido.insert(h->fullKey());
    } else {
      m_ign_i.insert(h->fullKey());
    }
  }
  for (auto h : idhh->outputHandles()) {
    if (! h->objKey().empty()) {
      m_odo.insert(h->fullKey()); 
    } else {
      m_ign_o.insert(h->fullKey());
    }
  }

  for (auto id : idhh->extraInputDeps()) {
    if (! id.key().empty()) { 
      m_ido.insert( id );
    } else {
      m_ign_i.insert(id);
    }
  }
  for (auto id : idhh->extraOutputDeps()) {
    if (! id.key().empty()) { 
      m_odo.insert( id );
    } else {
      m_ign_o.insert(id);
    }
  }

}

