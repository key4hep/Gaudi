// $Id: PoolDbBaseCnv.h,v 1.6 2006/09/28 16:57:56 hmd Exp $
//====================================================================
//	PoolDbBaseCnv definition
//
//	Author     : M.Frank
//====================================================================
#ifndef POOLDB_POOLDBBASECNV_H
#define POOLDB_POOLDBBASECNV_H
// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiPoolDb/GaudiPoolDb/PoolDbBaseCnv.h,v 1.6 2006/09/28 16:57:56 hmd Exp $"

// Framework include files
#include "GaudiKernel/Converter.h"
#include "Reflex/Reflex.h"
#include "POOLCore/Guid.h"
#include <set>

// Forward declarations
class LinkManager;
class IPoolDbMgr;
class IDataManagerSvc;
class ContainedObject;

class PoolDbAddress;
class PoolDbLinkManager;
class PoolDbDataObjectHandler;

namespace pool {
  class DataCallBack;
}

/** @class PoolDbBaseCnv PoolDbBaseCnv.h PoolDb/PoolDbBaseCnv.h
  *
  * Description:
  * Definition of the generic Db data converter.
  * The generic data converter provides the infrastructure
  * of all data converters. The templated class takes two arguments:
  * The Transient and the Persistent object type.
  *
  * For a detailed description of the overridden function see the the
  * base class.
  *
  * @author  M.Frank
  * @version 1.0
  */
class GAUDI_API PoolDbBaseCnv: public Converter {
protected:
  /// Services needed for proper operation: POOL database manager
  IPoolDbMgr*               m_dbMgr;
  /// Services needed for proper operation: Gaudi datastore manager
  IDataManagerSvc*          m_dataMgr;
  /// Object Guid
  pool::Guid                m_objGuid;
  /// Reference to seal reflection class
  ROOT::Reflex::Type        m_class;
  /// Definition callback
  pool::DataCallBack*       m_call;
  /// Set with bad files/tables
  std::set<std::string>     m_badFiles;
protected:

  /** Standard way to print errors. after the printout an exception is thrown.
    * @param      msg      [IN]     Message string to be printed.
    * @param      rethrow  [IN]     Flag to indicate if an exception must be thrown
    *
    * @return     Status code returning failure.
    */
  StatusCode makeError(const std::string& msg, bool rethrow=true);

  /** Retrieve the name of the container a given object is placed into
    * @param      pReg     [IN]    Pointer to registry entry.
    *
    * @return     Name of the container the object should be put to.
    */
  virtual const std::string containerName(IRegistry* pReg)  const;

public:

  /** Initializing Constructor
    * @param      typ      [IN]     Concrete storage type of the converter
    * @param      clid     [IN]     Class identifier of the object
    * @param      svc      [IN]     Pointer to service locator object
    *
    * @return Reference to PoolDbBaseCnv object
    */
  PoolDbBaseCnv(long typ, const CLID& clid, ISvcLocator* svc);

  /// Standard Destructor
  virtual ~PoolDbBaseCnv();

  /// Converter overrides: Initialize the Db converter
  virtual StatusCode initialize();

  /// Converter overrides: Finalize the Db data converter
  virtual StatusCode finalize();

  /// Retrieve the class type of the data store the converter uses.
  virtual long repSvcType() const  {
    return i_repSvcType();
  }

  /** Converter overrides: Create transient object from persistent data
    *
    * @param    pAddr       [IN]   Pointer to object address.
    * @param    refpObj     [OUT]  Location to pointer to store data object
    *
    * @return Status code indicating success or failure.
    */
  virtual StatusCode createObj(     IOpaqueAddress* pAddr,
                                    DataObject*& refpObj);

  /** Converter overrides: Resolve the references
    * of the created transient object.
    *
    * @param    pAddr    [IN]   Pointer to object address.
    * @param    pObj     [IN]   Pointer to data object
    *
    * @return Status code indicating success or failure.
    */
  virtual StatusCode fillObjRefs(   IOpaqueAddress* pAddr,
                                    DataObject* pObj);

  /** Converter overrides: Update transient object from persistent data
    *
    * @param    pAddr    [IN]   Pointer to object address.
    * @param    pObj     [IN]   Pointer to data object
    *
    * @return Status code indicating success or failure.
    */
  virtual StatusCode updateObj(     IOpaqueAddress* pAddr,
                                    DataObject* pObj);

  /** Converter overrides: Update the references of
    * an updated transient object.
    *
    * @param    pAddr    [IN]   Pointer to object address.
    * @param    pObj     [IN]   Pointer to data object
    *
    * @return Status code indicating success or failure.
    */
  virtual StatusCode updateObjRefs( IOpaqueAddress* pAddr,
                                    DataObject* pObj);

  /** Converter overrides: Convert the transient object to the
    * requested representation.
    *
    * @param    pObj     [IN]   Pointer to data object
    * @param    refpAddr [OUT]  Location to store pointer to object address.
    *
    * @return Status code indicating success or failure.
    */
  virtual StatusCode createRep(     DataObject* pObj,
                                    IOpaqueAddress*& refpAddr);

  /** Converter overrides: Resolve the references of the converted object.
    *
    * @param    pAddr    [IN]   Pointer to object address.
    * @param    pObj     [IN]   Pointer to data object
    *
    * @return Status code indicating success or failure.
    */
  virtual StatusCode fillRepRefs(   IOpaqueAddress* pAddr,
                                    DataObject* pObj);

  /** Converter overrides: Update the converted representation
    * of a transient object.
    *
    * @param    pAddr    [IN]   Pointer to object address.
    * @param    pObj     [IN]   Pointer to data object
    *
    * @return Status code indicating success or failure.
    */
  virtual StatusCode updateRep(     IOpaqueAddress* pAddr,
                                    DataObject* pObj);

  /** Converter overrides: Update the references of an
    * already converted object.
    *
    * @param    pAddr    [IN]   Pointer to object address.
    * @param    pObj     [IN]   Pointer to data object
    *
    * @return Status code indicating success or failure.
    */
  virtual StatusCode updateRepRefs( IOpaqueAddress* pAddr,
                                    DataObject* pObj);

  virtual PoolDbLinkManager* createReferences(DataObject* pObj);

  /** Set the references of a DataObject when reading.
    * @param    pMgr     [IN]   Pointer to POOL Link manager object
    * @param    pLinks   [IN]   Pointer to intermediate link manager
    * @param    pObj     [IN]   Pointer to data object
    * @param    pReg     [IN]   Pointer to registry entry
    *
    * @return Status code indicating success or failure.
    */
  virtual StatusCode setReferences(PoolDbLinkManager* pMgr,
                                   LinkManager* pLinks,
                                   DataObject* pObj,
                                   IRegistry* pReg);
  /** Dump the references of a DataObject after writing.
    * @param    pMgr     [IN]   Pointer to POOL Link manager object
    * @param    pLinks   [IN]   Pointer to intermediate link manager
    * @param    pObj     [IN]   Pointer to data object
    * @param    pReg     [IN]   Pointer to registry entry
    *
    * @return Status code indicating success or failure.
    */
  virtual StatusCode dumpReferences(PoolDbLinkManager* pMgr,
                                    LinkManager* pLinks,
                                    DataObject* pObj,
                                    IRegistry* pReg);
};

#endif    // POOLDB_PoolDbBaseCnv_H
