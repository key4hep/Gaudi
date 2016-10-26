#include "GaudiKernel/DataHandleHolderVisitor.h"
#include "GaudiKernel/IDataHandleHolder.h"
#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/DataObjID.h"
#include <tuple>
#include <typeinfo>

namespace {
   using std::make_tuple;
}

DHHVisitor::DHHVisitor(DataObjIDColl& ido, DataObjIDColl& odo)
   : m_ido(ido), m_odo(odo)
{
}

void DHHVisitor::visit(const IDataHandleHolder* v) {
   if (v == 0) {
      return;
   }

   for (auto& hs : {make_tuple(v->inputHandles(), std::ref(m_ido), std::ref(m_ign_i)),
                    make_tuple(v->outputHandles(), std::ref(m_odo), std::ref(m_ign_o))}) {
      for (const auto& h : std::get<0>(hs)) {
         if (! h->objKey().empty()) { 
            std::get<1>(hs).emplace(h->fullKey());
         } else {
            std::get<2>(hs).emplace(h->fullKey());
         }
      }
   }

   for (auto& hs : {make_tuple(v->extraInputDeps(), std::ref(m_ido), std::ref(m_ign_i)),
                    make_tuple(v->extraOutputDeps(), std::ref(m_odo), std::ref(m_ign_o)),
                    make_tuple(v->inputDataObjs(), std::ref(m_ido), std::ref(m_ign_i)),
                    make_tuple(v->outputDataObjs(), std::ref(m_odo), std::ref(m_ign_o))}) {            
      for (const auto& h : std::get<0>(hs)) {
         if (! h.key().empty()) { 
            std::get<1>(hs).emplace(h.fullKey());
         } else {
            std::get<2>(hs).emplace(h.fullKey());
         }
      }
   }
}
