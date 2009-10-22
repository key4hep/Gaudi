// $Id: PoolDbCnvSvc.h,v 1.14 2008/01/23 14:31:50 hmd Exp $
//====================================================================
//	PoolDbCnvSvc definition
//--------------------------------------------------------------------
//
//	Author     : M.Frank
//====================================================================
#ifndef POOLDB_POOLDBCNVSVC_H
#define POOLDB_POOLDBCNVSVC_H
// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiPoolDb/GaudiPoolDb/PoolDbCnvSvc.h,v 1.14 2008/01/23 14:31:50 hmd Exp $

// Framework include files
#include "GaudiPoolDb/IPoolDbMgr.h"
#include "GaudiKernel/ConversionSvc.h"
#include "GaudiKernel/DataObject.h"
#include "StorageSvc/DbSession.h"
#include "StorageSvc/DbDomain.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbOptionCallback.h"
#include "StorageSvc/DbDefaultDataHandler.h"


#include <map>

// Forward declarations
class IPoolCacheSvc;
class IDataManagerSvc;
namespace Gaudi {
  class IIncidentSvc;
  class IFileCatalog;
  class IIODataManager;
  class IDataConnection;
}
class PoolDbDataConnection;

/** @class PoolDbCnvSvc PoolDbCnvSvc.h PoolDb/PoolDbCnvSvc.h
  *
  * Description:
  *
  * PoolDbCnvSvc class implementation definition.
  *
  * @author Markus Frank
  * @version 1.0
  */
class GAUDI_API PoolDbCnvSvc: public extends1<ConversionSvc, IPoolDbMgr>,
                              public pool::DbOptionCallback
{
public:
  struct DbH  {
    pool::DbDatabase     first;
    pool::DbTransaction* second;
    int                  count;

    DbH(const pool::DbDatabase& db, pool::DbTransaction* tr, int cr)
    : first(db), second(tr), count(cr) {}
    DbH(const DbH& c)
    : first(c.first), second(c.second), count(c.count) {}
  };
  typedef std::pair< void*, const ROOT::Reflex::Type >      ObjH;
  typedef std::map<std::string, DbH>                        DbMap;
  typedef std::map<std::string, std::string>                DbFidMap;

protected:
  /// Name of the POOL cache service instance to connect to
  std::string              m_cacheSvcName;

  /// Name of the OODataBase implementation
  std::string              m_implementation;

  /// Connection string to server
  std::string              m_serverConnect;

  /// ROOT name of optional records triggering incident
  std::string              m_recordName;

  /// Flag to indicate safe transactions if requested
  bool                     m_safeTransactions;

  /// Buffer for domain options
  std::vector<std::string> m_domainOpts;

  /// Buffer for database options
  std::vector<std::string> m_databaseOpts;

  /// Buffer for container options
  std::vector<std::string> m_containerOpts;

  /// Services needed for proper operation: Data Manager
  IDataManagerSvc*         m_dataMgr;

  /// Reference to POOL cache service
  IPoolCacheSvc*           m_cacheSvc;

  /// On writing: reference to active output stream
  PoolDbDataConnection*    m_current;

  /// POOL Datahandler
  pool::DbDefaultDataHandler m_handler;

  /// POOL technology domain handle
  pool::DbDomain           m_domH;

  /// Share files ? If set to YES, files will not be closed on finalize
  std::string              m_shareFiles;

  /// Flag to enforce FID checking and verification
  bool                     m_checkFIDs;

  /// Reference to the file catalog
  Gaudi::IFileCatalog*     m_catalog;

  /// Reference to the I/O data manager
  Gaudi::IIODataManager*   m_ioMgr;

  /// Reference to incident service
  IIncidentSvc*            m_incidentSvc;

  /// Flag to enable incidents on FILE_OPEN
  bool                     m_incidentEnabled;
public:
  /// Standard constructor
  PoolDbCnvSvc(const std::string& name, ISvcLocator* svc);

  /// Standard destructor
  virtual ~PoolDbCnvSvc();

  /// Update state of the service
  virtual StatusCode updateServiceState(IOpaqueAddress* pAddress);

  /** Standard way to print errors. after the printout an exception is thrown.
    * @param      msg      [IN]     Message string to be printed.
    * @param      rethrow  [IN]     Flag to indicate if an exception should be thrown
    *
    * @return     Status code returning failure.
    */
  StatusCode error(const std::string& msg, bool rethrow=true);

  /** Connect to data provider service. Re-connects to data manager service.
   *  @param      pDataSvc    New data provider object.
   *  @return     Status code indicating success or failure.
   */
  StatusCode setDataProvider(IDataProviderSvc* pDataSvc);

public:

  /// ConversionSvc overload: initialize Db service
  virtual StatusCode initialize();

  /// ConversionSvc overload: Finalize Db service
  virtual StatusCode finalize();

  /// ConversionSvc overload: Create new Converter using factory
  virtual IConverter* createConverter(long typ,
                                      const CLID& wanted,
                                      const ICnvFactory* fac);

  /// ConversionSvc overload: Load the class (dictionary) for the converter
  virtual void loadConverter(DataObject*);

  /// Access to cache service
  virtual IPoolCacheSvc* cacheSvc() const {  return m_cacheSvc; }

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
                                       long& gaudi_type);

  /** Connect the output file to the service with open mode.
   *  @param      outputFile  String containing output file
   *  @param      openMode    String containing opening mode of the output file
   *  @return     Status code indicating success or failure.
   */
  virtual StatusCode connectOutput(const std::string& outputFile,
                                   const std::string& openMode);

  /// Connect the output file to the service with open mode.
  /** @param      outputFile  String containing output file
   *  @param      openMode    Opening mode of the output file
   *  @param      con         Data connection reference
   *
   *  @return     Status code indicating success or failure.
   */
  StatusCode  connectDatabase(int typ,
                              const std::string& outputFile,
                              pool::DbAccessMode openMode,
                              PoolDbDataConnection** con);

  /** Connect to a database container
    * @param      dbH         Handle to hosting database
    * @param      cntName     String containing the name of the
    *                         container object
    * @param      openMode    Open mode (READ,WRITE,UPDATE,...)
    * @param      shape       Default shape of objects in the container
    * @param      cntH        Reference to the POOL container object
    *
    * @return     Status code indicating success or failure.
    */
  StatusCode connectContainer(        pool::DbDatabase& dbH,
                                      const std::string& cntName,
                                      pool::DbAccessMode openMode,
                                      const pool::DbTypeInfo* shape,
                                      pool::DbContainer& cntH);

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
                                      );

  /** Connect the output file to the service.
   *  @param      outputFile  String containing output file
   *  @return     Status code indicating success or failure.
   */
  virtual StatusCode connectOutput(const std::string& outputFile);

  /** Commit pending output.
   *  @param      outputFile  String containing output file
   *  @param      do_commit   if true commit the output and flush
   *                          eventually pending items to the database
   *                          if false, discard pending buffers.
   *                          Note: The possibility to commit or rollback
   *                          depends on the database technology used!
   *  @return     Status code indicating success or failure.
   */
  virtual StatusCode commitOutput(const std::string& outputFile,
                                  bool               do_commit);

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
                             pool::DbContainer& cntH);

  /** Disconnect from an existing data stream.
    * @param      dbName      String containing name of the database
    *
    * @return     Status code indicating success or failure.
    */
  virtual StatusCode disconnect(const std::string& dbName);

  /** IAddressCreator implementation: Address creation.
    * Create an address using the link information together with
    * the triple (database name/container name/object name).
    *
    * @param refLink        Reference to abstract link information
    * @param dbName         Database name
    * @param containerName  Object container name
    * @param refpAddress    Opaque address information to retrieve object
    * @return               StatusCode indicating SUCCESS or failure
    */
  virtual StatusCode createAddress( long                 svc_type,
                                    const CLID&          clid,
                                    const std::string*   par,
                                    const unsigned long* ip,
                                    IOpaqueAddress*&     refpAddress);

  /** IPoolDbMgr implementation: Address creation.
    * Create an address using an existing POOL token object.
    *
    * @param pToken         Reference to valid POOL token object.
    * @param ppAddress      Opaque address information to retrieve object
    * @return               StatusCode indicating SUCCESS or failure
    */
  virtual StatusCode createAddress( pool::Token*         pToken,
                                    PoolDbAddress**      ppAddress);

  /** Request an iterator over a container from the service
    * @param      criteria    Selection criteria to restrict iteration
    * @param      dbName      String containing name of the database
    * @param      cntName     String containing the name of the
    *                         container object
    * @return     Reference to the selection object
    */
  virtual pool::DbSelect* createSelect(const std::string& criteria,
                                       const std::string& dbName,
                                       const std::string& cntName);

  /** Request an iterator over a container from the service
    * @param      criteria    Selection criteria to restrict iteration
    * @param      dbH         Handle to POOL database
    * @param      cntName     String containing the name of the
    *                         container object
    * @return     Reference to the selection object
    */
  virtual pool::DbSelect* createSelect(const std::string& criteria,
                                       pool::DbDatabase&  dbH,
                                       const std::string& cntName);

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
                                    PoolDbAddress**    ppAddr=0);

  /** Mark an object for update.
    * @param      call        Pointer to callback object
    * @param      pAddr       Persistent object location to be used for
    *                         link setup.
    * @return     Status code indicating success or failure.
    */
  virtual StatusCode markUpdate(    pool::DataCallBack* call,
                                    PoolDbAddress*     pAddr);

  /** Read existing object. Open transaction in read mode if not active.
    * @param      call        Pointer to callback object
    * @param      pAddr       Pointer to valid object address.
    *
    * @return     Status code indicating success or failure.
    */
  virtual StatusCode read(          pool::DataCallBack* call,
                                    PoolDbAddress*     pAddr);

  /// Read existing object. Open transaction in read mode if not active
  /** @param      call        Pointer to callback object
    * @param      tok         Reference to pool token describing the object
    *
    * @return     Status code indicating success or failure.
    */
  virtual StatusCode read(          pool::DataCallBack* call,
                                    pool::Token& tok);
};
#endif  // POOLDB_POOLDBCNVSVC_H
