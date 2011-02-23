// $Id: IPoolDbMgr.h,v 1.7 2008/02/21 18:01:54 hmd Exp $
//====================================================================
//	IPoolDbMgr.h
//--------------------------------------------------------------------
//
//	Author     : M.Frank
//====================================================================
// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiPoolDb/GaudiPoolDb/IPoolDbMgr.h,v 1.7 2008/02/21 18:01:54 hmd Exp $
#ifndef POOLDB_IPOOLDBMGR_H
#define POOLDB_IPOOLDBMGR_H

// C/C++ include files
#include <string>

// Framework include files
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/ClassID.h"
#include "GaudiUtils/IIODataManager.h"
#include "StorageSvc/DbContainer.h"

// Forward declarations
class DataObject;
class PoolDbAddress;
class IOpaqueAddress;
class ContainedObject;
class IPoolCacheSvc;

namespace pool {
  class DataCallBack;
  class DbSelect;
  class DbType;
  class Token;
  class Guid;
}
namespace seal {
  namespace reflex {
    class Type;
  }
}

/** @class IPoolDbMgr IPoolDbMgr.h PoolDb/IPoolDbMgr.h
  *
  * Description:
  *
  * The IPoolDbMgr service interface allows to store and
  * retrieve data stored with the POOL persistency mechanism.
  *
  * @author  Markus Frank
  * @version 1.0
  */
class GAUDI_API IPoolDbMgr: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IPoolDbMgr,2,0);

  enum { UNKNOWN = Gaudi::IIODataManager::UNKNOWN,
	 PFN = Gaudi::IIODataManager::PFN,
	 LFN = Gaudi::IIODataManager::LFN,
	 FID = Gaudi::IIODataManager::FID
  };
  enum { BAD_DATA_CONNECTION=Gaudi::IDataConnection::BAD_DATA_CONNECTION };

  /// Access to cache service
  virtual IPoolCacheSvc* cacheSvc() const = 0;

  /** Create access to the POOL persistency mechanism using
    * the Gaudi String identifier. Also all appropriate DLLS
    * will be loaded on the fly.
    *
    * @param type_string      Gaudi storage string identifier
    * @param gaudi_type       Gaudi storage identifier
    *
    * @return Status code indicating failure or success.
    */
  virtual StatusCode accessStorageType(const std::string& type_string,
                                       long& gaudi_type) = 0;

  /** Create a Generic address using explicit arguments to
    * identify a single object.
    * @param      svc_type    Technology identifier encapsulated
    *                         in this address.
    * @param      clid        Class identifier of the DataObject
    *                         represented by the opaque address
    * @param      par         Array of strings needed to
    *                         construct the opaque address.
    * @param      ipar        Array of integers needed to
    *                         construct the opaque address.
    * @param      refpAddress Reference to pointer to the address where the created
    *                         Address should be stored.
    *
    * @return     Status code indicating success or failure.
    */
  virtual StatusCode createAddress(long                 svc_type,
                                   const CLID&          clid,
                                   const std::string*   par,
                                   const unsigned long* ipar,
                                   IOpaqueAddress*&     refpAddress) = 0;

  /** Create a Generic address using explicit arguments to
    * identify a single object.
    * @param      pToken      Pointer to valid POOL token object
    * @param      ppAddress   Reference to pointer to the address where the created
    *                         Address should be stored.
    *
    * @return     Status code indicating success or failure.
    */
  virtual StatusCode createAddress(pool::Token*       pToken,
                                   PoolDbAddress**    ppAddress) = 0;
  /** Connect the output file to the service with open mode.
    * @param      outputFile  String containig output file
    * @param      openMode    String containig opening mode of the output file
    *
    * @return     Status code indicating success or failure.
    */
  virtual StatusCode connectOutput(const std::string& outputFile,
                                   const std::string& openMode) = 0;

  /** Commit pending output.
    * @param      outputFile  String containig output file
    * @param      do_commit   if true commit the output and flush
    *                         eventually pending items to the database
    *                         if false, discard pending buffers.
    *                         Note: The possibility to commit or rollback
    *                         depends on the database technology used!
    * @return     Status code indicating success or failure.
    */
  virtual StatusCode commitOutput( const std::string& outputFile,
                                   bool               do_commit) = 0;

  /** Connect to a database container in read mode.
    * @param      dbName      String containing name of the database
    * @param      cntName     String containing the name of the
    *                         container object
    * @param      cntH        Reference to the POOL container object
    *
    * @return     Status code indicating success or failure.
    */
  virtual StatusCode connect(const std::string& dbName,
                             const std::string& cntName,
                             pool::DbContainer& cntH) = 0;

  /** Connect to a database container
    * @param      typ         Database technology type
    * @param      dbName      String containing name of the database
    * @param      cntName     String containing the name of the
    *                         container object
    * @param      openMode    Open mode (READ,WRITE,UPDATE,...)
    * @param      shape       Default shape of objects in the container
    * @param      cntH        Reference to the POOL container object
    *
    * @return     Status code indicating success or failure.
    */
  virtual StatusCode connectContainer(int typ,
                                      const std::string& dbName,
                                      const std::string& cntName,
                                      pool::DbAccessMode openMode,
                                      const pool::DbTypeInfo* shape,
                                      pool::DbContainer& cntH
                                      )=0;

  /** Request an iterator over a container from the service
    * @param      criteria    Selection criteria to restrict iteration
    * @param      dbName      String containing name of the database
    * @param      cntName     String containing the name of the
    *                         container object
    * @return     Reference to the iterator object
    */
  virtual pool::DbSelect* createSelect(const std::string& criteria,
                                       const std::string& dbName,
                                       const std::string& cntName) = 0;

  /** Disconnect from an existing data stream.
    * @param      dbName      String containing name of the database
    *
    * @return     Status code indicating success or failure.
    */
  virtual StatusCode disconnect(   const std::string&  dbName) = 0;

  /** Marks a reference to be written.
    * @param      call        Pointer to callback object
    * @param      cntName     Container name the object should be written to.
    * @param      refpAddr    Persistent object location to be used for
    *                         link setup.
    *
    * @return     Status code indicating success or failure.
    */
  virtual StatusCode markWrite(     pool::DataCallBack* call,
                                    const std::string& cntName,
                                    PoolDbAddress**    ppAddr=0) = 0;

  /** Mark an object for update.
    * @param      call        Pointer to callback object
    * @param      pAddr       Persistent object location to be used for
    *                         link setup.
    * @return     Status code indicating success or failure.
    */
  virtual StatusCode markUpdate(    pool::DataCallBack* call,
                                    PoolDbAddress*     pAddr) = 0;

  /** Read existing object. Open transaction in read mode if not active.
    * @param      call        Pointer to callback object
    * @param      pAddr       Pointer to valid object address.
    *
    * @return     Status code indicating success or failure.
    */
  virtual StatusCode read(          pool::DataCallBack* call,
                                    PoolDbAddress*     pAddr) = 0;

  /// Read existing object. Open transaction in read mode if not active
  /** @param      call        Pointer to callback object
    * @param      tok         Reference to pool token describing the object
    *
    * @return     Status code indicating success or failure.
    */
  virtual StatusCode read(          pool::DataCallBack* call,
                                    pool::Token& tok) = 0;
};

#endif  // POOLDB_IPOOLDBMGR_H
