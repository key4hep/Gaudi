
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
    m_ido.insert(h->fullKey());
  }
  for (auto h : idhh->outputHandles()) {
    m_odo.insert(h->fullKey());
  }
  m_ido.insert(idhh->extraInputDeps().begin(), idhh->extraInputDeps().end());
  m_odo.insert(idhh->extraOutputDeps().begin(), idhh->extraOutputDeps().end());

}

