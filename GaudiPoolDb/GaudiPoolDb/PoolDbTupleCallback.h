// $Id: PoolDbTupleCallback.h,v 1.3 2006/01/27 08:27:02 hmd Exp $
//====================================================================
// NTuple callback class definition
//--------------------------------------------------------------------
//
//  Package    : PoolDb ( The LHCb Offline System)
//  Author     : M.Frank
//
//====================================================================
#ifndef POOL_POOLDBTUPLECALLBACK_H
#define POOL_POOLDBTUPLECALLBACK_H

// Include files
#include "StorageSvc/DbBlob.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbObjectCallBack.h"
#include "StorageSvc/DbDefaultDataHandler.h"
#include "GaudiPoolDb/PoolDbNTupleDescriptor.h"
#include "GaudiKernel/Kernel.h"

// Forward declarations
class INTuple;
class DbSelect;
class DataObject;
class ISelectStatement;
namespace pool {
  class DbTypeInfo;
  class IContainer;
  class ITokenIterator;
}

/** @class PoolDbTupleCallback PoolDbTupleCallback.h
  *
  * Description:
  * Specialized callback to handle N-tuple interactions
  * while saving and lodaing POOL data.
  *
  * @author  M.Frank
  * @version 1.0
  */
class PoolDbTupleCallback
  : public pool::DbDefaultDataHandler,
    public pool::DbObjectCallBack
{
  ///Reflection information
  typedef const ROOT::Reflex::Type& TypeH;
  /// Vector with all addresses mapped
  std::vector< void* >          m_addr;
  /// Vector to store pointers link information for address columns
  std::vector<PoolDbTokenWrap>  m_links;
  /// Vector to store pointers link information for address columns
  std::vector<PoolDbTokenWrap*> m_plinks;
  /// Redirection map between N-tuple columns and DbTypeInfo columns
  std::vector<int>              m_map;
  /// Pointer to hold the select statement
  pool::DbSelect*               m_iter;
  /// Reference to streambuffer to hold blobified data.
  pool::DbBlob                  m_stream;
  /// Pointer to pool container handle
  pool::DbContainer             m_cntH;
  /// Reference counter
  long                          m_refCount;

public:
  /// Initializing constructor for N-tuple callback object
  /**
    * @param svc         [IN]  Manager interface of conversion service
    * @param pObj        [IN]  Pointer to N-tuple object
    * @param reading     [IN]  Read/Write flag (Needed by base class
    *
    * @return Reference to object
    */
  PoolDbTupleCallback(DataObject* pObj);
  /// Standard Destructor
  virtual ~PoolDbTupleCallback();
  /// Virtual copy
  virtual pool::DataCallBack* clone() const;
  /// Virtual delete
  virtual void release();
  /// Access to the bound addresses
  std::vector<void*>& addresses()               {  return m_addr;     }
  /// Access re-direction map
  const std::vector<int>& mapping()             {  return m_map;      }
  /// Access to pointers of the links
  std::vector<PoolDbTokenWrap*>& links()        {  return m_plinks; }
  /// Access to streambuffer object, for items which must be blobified.
  pool::DbBlob& stream()                        {  return m_stream;   }
  /// Access to select statement
  pool::DbSelect* iterator() const              {  return m_iter;     }
  /// Select new tokens according to new selection criteria
  pool::DbSelect* select(ISelectStatement* sel);
  /// Configure N-tuple callback and store properties for re-use
  void configure(const INTuple* pTuple,
                 const pool::DbTypeInfo* typ,
                 const pool::DbContainer& cntH);

  /// Callback when a read sequence should be started
  virtual pool::DbStatus start (CallType  action_type,
                                void*     pObj,
                                void**    context);

  /// Callback when a read sequence should be started
  virtual pool::DbStatus end(CallType action_type, void* context);

  /// Callback to retrieve the absolute address of a column
  virtual pool::DbStatus bind(CallType              action_type,
                              const pool::DbColumn* col_ident,
                              int                   col_number,
                              void*                 context,
                              void**                data_pointer);
};
#endif // POOL_POOLDBTUPLECALLBACK_H
