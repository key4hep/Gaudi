// $Id: PoolDbTupleCallback.cpp,v 1.5 2008/10/27 16:41:33 marcocle Exp $
//------------------------------------------------------------------------------
//
// Implementation of class :  PoolDbTupleCallback
//
// Author :                   Markus Frank
//
//------------------------------------------------------------------------------

// FIXME: missing in CORAL
#include <algorithm>

#include "GaudiPoolDb/PoolDbTupleCallback.h"
#include "GaudiKernel/INTuple.h"
#include "GaudiKernel/ISelectStatement.h"

#include "StorageSvc/DbType.h"
#include "StorageSvc/DbSelect.h"
#include "StorageSvc/DbColumn.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbInstanceCount.h"

static pool::DbInstanceCount::Counter* s_count = 
  pool::DbInstanceCount::getCounter(typeid(PoolDbTupleCallback));

/// Initializing constructor for N-tuple callback object
PoolDbTupleCallback::PoolDbTupleCallback(DataObject* pObj)
: pool::DbDefaultDataHandler(),
  pool::DbObjectCallBack(0), 
  m_iter(0), m_cntH(pool::POOL_StorageType)
{
  m_refCount = 1;
  setObject(pObj);
  s_count->increment();
}

/// Standard Destructor
PoolDbTupleCallback::~PoolDbTupleCallback()  
{
  pool::deletePtr(m_iter);
  s_count->decrement();
  const pool::DbTypeInfo* typ = (const pool::DbTypeInfo*)shape();
  if ( typ ) typ->deleteRef();
}

/// Virtual copy
pool::DataCallBack* PoolDbTupleCallback::clone() const     
{
  PoolDbTupleCallback* cb = const_cast<PoolDbTupleCallback*>(this);
  cb->m_refCount++;
  return cb;
}

/// Virtual delete
void PoolDbTupleCallback::release()
{
  if ( --m_refCount <= 0 ) {
    delete this;
  }
}

/// Configure N-tuple callback and store properties for re-use
void PoolDbTupleCallback::configure(const INTuple* pTuple, 
                                    const pool::DbTypeInfo* typ,
                                    const pool::DbContainer& cntH)  
{
  pool::DbTypeInfo::Columns::const_iterator ic;
  const INTuple::ItemContainer& items = pTuple->items();
  const pool::DbTypeInfo::Columns& cols = typ->columns();
  size_t colsize = cols.size();
  m_map.resize(items.size()+1,-1);
  for(size_t item_no = 0; item_no < items.size(); ++item_no ) {
    int icc = 0;
    m_map[item_no] = -1;
    const std::string& itm_nam = items[item_no]->name();
    for (ic=cols.begin(); ic != cols.end(); ++ic, ++icc )  {
      if ( (*ic)->name() == itm_nam )  {
        m_map[item_no] = icc;
        break;
      }
    }
  }
  m_addr.resize(colsize);
  m_addr[colsize-1] = &m_stream;
  m_links.resize(m_map.size());
  m_plinks.resize(m_map.size());
  for(size_t i = 0; i < m_map.size(); ++i )  {
    m_plinks[i] = &m_links[i];
  }
  m_cntH = cntH;
  typ->addRef();
  setShape(typ);
}

pool::DbSelect* 
PoolDbTupleCallback::select(ISelectStatement* sel)  {
  pool::deletePtr(m_iter);
  if ( m_cntH.isValid() )  {
    std::string criteria = (sel && (sel->type() & ISelectStatement::STRING))
      ? sel->criteria() : std::string("");
    m_iter = new pool::DbSelect( criteria );
    pool::DbDatabase dbH = m_cntH.containedIn();
    m_iter->start(dbH, m_cntH.name());
  }
  return m_iter;
}

/// Callback when a read sequence should be started
pool::DbStatus 
PoolDbTupleCallback::start (CallType /* action_type */, 
                            void*    /* pObj        */, 
                            void**   /* context     */)
{
  return pool::Success;
}

/// Callback when a read sequence should be started
pool::DbStatus PoolDbTupleCallback::end(CallType /* action_type */, 
                                        void*    /* context     */) 
{
  return pool::Success;
}

/// Callback to retrieve the absolute address of a column
pool::DbStatus 
PoolDbTupleCallback::bind(CallType                 action_type,
                          const pool::DbColumn* /* col_ident */,
                          int                      col_number,
                          void*                 /* context   */,
                          void**                   data_pointer)
{
  switch(action_type)  {
  case GET:
    *data_pointer = m_addr[col_number];
    break;
  case PUT:
    *data_pointer = m_addr[col_number];
    break;
  }
  return pool::Success;
}

