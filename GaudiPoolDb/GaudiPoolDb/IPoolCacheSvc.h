// $Id: IPoolCacheSvc.h,v 1.8 2008/01/17 13:20:51 marcocle Exp $
//====================================================================
//	IPoolCacheSvc.h
//--------------------------------------------------------------------
//
//	Author     : M.Frank
//====================================================================
// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiPoolDb/GaudiPoolDb/IPoolCacheSvc.h,v 1.8 2008/01/17 13:20:51 marcocle Exp $
#ifndef POOLDB_IPOOLCACHESVC_H
#define POOLDB_IPOOLCACHESVC_H

// Framework include files
#include "GaudiKernel/IInterface.h"
#include "StorageSvc/DbSession.h"

// C++ header files
#include <string>

// Forward declarations
namespace pool {
  class DbType;
  class DbSession;
  class DbOptionCallback;
}

/** @class IPoolCacheSvc IPoolCacheSvc.h PoolDb/IPoolCacheSvc.h
  *
  * Description:
  *
  * The IPoolCacheSvc service interface allows to access the
  * functionality provided by the POOL services. The interface
  * allows to access the
  *
  * - POOL persistency interface.
  * - POOL file catalog interface.
  * - POOL data service interface.
  *
  * @author  Markus Frank
  * @version 1.0
  */
class GAUDI_API IPoolCacheSvc: virtual public IInterface
{
public:
  /// InterfaceID
  DeclareInterfaceID(IPoolCacheSvc,2,0);

  /// Access the (single) POOL database session
  virtual pool::DbSession& session() = 0;

  /// Load dictionary library
  /** @param      name   [IN]   dictionary name/file path
    *
    * @return     Status code indicating success or failure
    */
  virtual StatusCode loadDictionary(const std::string& name) = 0;

  /// Access to POOL callback handler
  /** @return     Reference to POOL callback handler
    */
  virtual pool::DbOptionCallback* callbackHandler() const = 0;
};

#endif  // POOLDB_IPOOLCACHESVC_H
