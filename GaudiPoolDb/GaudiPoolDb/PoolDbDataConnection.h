// $Id: PoolDbDataConnection.h,v 1.2 2008/01/30 13:49:09 marcocle Exp $
//====================================================================
//	PoolDbDataConnection.h
//--------------------------------------------------------------------
//
//	Author     : M.Frank
//====================================================================
// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiPoolDb/GaudiPoolDb/PoolDbDataConnection.h,v 1.2 2008/01/30 13:49:09 marcocle Exp $
#ifndef GAUDIPOOLDB_POOLDBDATACONNECTION_H
#define GAUDIPOOLDB_POOLDBDATACONNECTION_H

// Framework include files
#include "GaudiUtils/IIODataManager.h"
#include "StorageSvc/DbDomain.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbTransaction.h"

/** @class PoolDbDataConnection
  *
  *  The DataConnection incanation for the usage in GaudiPoolDb.
  *  This entity interacts with the IODataManager from the GaudiUtils
  *  package and allows to connect refereneces between objects stored
  *  in POOL files.
  *
  *  @authot  M.Frank
  *  @version 1.0
  *  @date    20/10/2007
  */
class GAUDI_API PoolDbDataConnection: virtual public Gaudi::IDataConnection  {
protected:
  int                 m_type;
  /// Datasource access mode
  int                 m_mode;
  /// Transaction handle for this data source
  pool::DbTransaction m_transaction;
  /// Database handle for this data source
  pool::DbDatabase    m_dbH;
  /// Domain handle specifying the storage domain
  pool::DbDomain      m_domH;

  /// Internal connect call
  virtual StatusCode i_connect();
public:
  /// Standard constructor
  PoolDbDataConnection(IInterface* owner, const std::string& nam, int typ, int mode, pool::DbDomain& domH);
  /// Standard destructor
  virtual ~PoolDbDataConnection();
  /// Access to POOL database handle
  pool::DbDatabase& database()            {     return m_dbH;                }
  /// Access to POOL transaction handle
  pool::DbTransaction& transaction()      {     return m_transaction;        }
  /// Check if connected to data source
  virtual bool isConnected() const        {     return m_dbH.ptr() != 0;     }
  /// Open data stream in read mode
  virtual StatusCode connectRead()        {     return i_connect();          }
  /// Open data stream in write mode
  virtual StatusCode connectWrite(IoType /* typ */)  {return i_connect();    }
  /// Release data stream
  virtual StatusCode disconnect();
  /// Read raw byte buffer from input stream
  virtual StatusCode read(void* const /* data */, size_t /* len */)
  {  return StatusCode::FAILURE;                                             }
  /// Write raw byte buffer to output stream
  virtual StatusCode write(const void* /* data */, int /* len */ )
  {  return StatusCode::FAILURE;                                             }
  /// Seek on the file described by ioDesc. Arguments as in ::seek()
  virtual long long int seek(long long int /* where */, int /* origin */)
  {  return -1;                                                              }
};
#endif    // GAUDIPOOLDB_POOLDBDATACONNECTION_H
