// $Id: PoolDbCacheSvc.h,v 1.10 2008/01/30 13:49:09 marcocle Exp $
//====================================================================
//	PoolCacheMgr.h
//--------------------------------------------------------------------
//
//	Author     : M.Frank
//====================================================================
// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiPoolDb/GaudiPoolDb/PoolDbCacheSvc.h,v 1.10 2008/01/30 13:49:09 marcocle Exp $
#ifndef POOLDB_POOLDBCACHESVC_H
#define POOLDB_POOLDBCACHESVC_H

// Framework include files
#include "StorageSvc/DbOptionCallback.h"
#include "StorageSvc/DbSession.h"
#include "GaudiPoolDb/IPoolCacheSvc.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/System.h"

// STL include files
#include <vector>

/** @class PoolDbCacheSvc PoolDbCacheSvc.h PoolDb/PoolDbCacheSvc.h
  *
  * Description:
  *
  * The PoolCacheMgr service interface allows to access special
  * functionality provided by the POOL persistency interface.
  *
  * @author  Markus Frank
  * @version 1.0
  */
class GAUDI_API PoolDbCacheSvc : public extends1<Service, IPoolCacheSvc>,
                                 public pool::DbOptionCallback
{

protected:
  /// The POOL database session
  pool::DbSession          m_session;

  /// Reference to POOL callback handler
  pool::DbOptionCallback*  m_callbackHandler;

  /// Driver DLL to be loaded at initialization
  std::vector<std::string> m_dlls;

  /// Buffer for domain options
  std::vector<std::string> m_domainOpts;

  /// Buffer for database options
  std::vector<std::string> m_databaseOpts;

  /// Buffer for database options to be set after "open"
  std::vector<std::string> m_databaseOpenOpts;

  /// Buffer for container options
  std::vector<std::string> m_containerOpts;

  /// Buffer for container options to be set after "open"
  std::vector<std::string> m_containerOpenOpts;

  /// Shared Gaudi libraries
  std::vector<System::ImageHandle>  m_sharedHdls;

protected:

  /// Load all required libraries
  StatusCode loadLibraries();

public:
  /// Standard constructor
  PoolDbCacheSvc(const std::string& name, ISvcLocator* svc);

  /// Standard destructor
  virtual ~PoolDbCacheSvc();

  /// Service overload: initialize service
  virtual StatusCode initialize();

  /// Service overload: Finalize service
  virtual StatusCode finalize();

  /** Access the (single) POOL database session
    * @return     Reference to POOL session handle
    */
  virtual pool::DbSession& session()    {
    return m_session;
  }

  /// Load dictionary library
  /** @param      name   [IN]   dictionary name/file path
    *
    * @return     Status code indicating success or failure
    */
  virtual StatusCode loadDictionary(const std::string& name);

  /** Access to POOL callback handler
    * @return     Reference to POOL callback handler
    */
  virtual pool::DbOptionCallback* callbackHandler() const  {
    return m_callbackHandler;
  }

  /// Default callback (does nothing)
  /** @param pObj      [IN]   Reference to object receiving options
    * @param typ       [IN]   Object type to receive options
    * @param name      [IN]   Object's name.
    *
    * @return Status code indicating success or failure.
    */
  virtual pool::DbStatus setMyOptions(pool::IDbOptionProxy* pObj,
                                      pool::DbOptionCallback::OptionType typ,
                                      const std::string& name);

  /// Set callback specific options
  /** @param pObj      [IN]   Reference to object receiving options
    * @param name      [IN]   Object's name.
    *
    * @return Status code indicating success or failure.
    */
  virtual pool::DbStatus setCallbackOptions(pool::IDbOptionProxy* pObj,
                                            const std::vector<std::string>& v,
                                            const std::string& name);
};

#endif  // POOLDB_POOLDBCACHESVC_H
