// $Id: NTupleSvc.h,v 1.9 2006/11/30 14:57:04 mato Exp $
#ifndef GAUDI_NTUPLESVC_H
#define GAUDI_NTUPLESVC_H 1

// Framework include files
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/IDataSourceMgr.h"
#include "GaudiKernel/DataSvc.h"

// STL include files
#include <map>


/** @class NTupleSvc NTupleSvc.h
 *
 *  NTuple service. See the Interface definition files for more detailed
 *  documentation of the implementing class.
 *  Base class:                        Gaudi/DataSvc/DataSvc.h
 *  Generic N tuple service interface: Gaudi/Interfaces/INTupleSvc.h
 *  Generic IAddressCreator interface: Gaudi/Interfaces/IAddressCreator.h
 *  Generic IConversionSvc interface:  Gaudi/Interfaces/IConversionSvc.h
 *
 *  @author M.Frank
 */
class NTupleSvc : public extends2<DataSvc, INTupleSvc, IDataSourceMgr>
{
private:
protected:
  struct Connection  {
    IConversionSvc* service;
    Connection(IConversionSvc* s) : service(s) {}
    Connection(const Connection& c) : service(c.service) {}
  };
  typedef std::vector< std::string >              DBaseEntries;
  typedef std::map<std::string, Connection>       Connections;
  typedef std::pair<std::string,std::string>      Prop;

private:
public:
  /// DataSvc overrides: Initialize the service.
  virtual StatusCode initialize();
  /// DataSvc overrides: reinitialize service.
  virtual StatusCode reinitialize();
  /// DataSvc overrides: stop the service.
  virtual StatusCode finalize();
  /// DataSvc overrides: Retrieve data loader
  virtual IConversionSvc* getDataLoader(IRegistry* pReg);

  /// Create Ntuple directory and register it with the data store.
  virtual NTuple::Directory* createDirectory (DataObject* pParent, const std::string& title);
  /// Create Ntuple directory and register it with the data store.
  virtual NTuple::Directory* createDirectory (DataObject* pParent, long id);
  /// Create Ntuple directory and register it with the data store.
  virtual NTuple::Directory* createDirectory (const std::string& dirPath, long id);
  /// Create Ntuple directory and register it with the data store.
  virtual NTuple::Directory* createDirectory (const std::string& dirPath, const std::string& title);
  /// Create Ntuple directory and register it with the data store.
  virtual NTuple::Directory* createDirectory (const std::string& fullPath);

  /// Create requested N tuple (Hide constructor)
  virtual StatusCode create(const CLID& typ, const std::string& title, NTuple::Tuple*& refpTuple);
  /// Book Ntuple and register it with the data store.
  virtual NTuple::Tuple* book (const std::string& fullPath, const CLID& type, const std::string& title);
  /// Book Ntuple and register it with the data store.
  virtual NTuple::Tuple* book (const std::string& dirPath, const std::string& relPath, const CLID& type, const std::string& title);
  /// Book Ntuple and register it with the data store.
  virtual NTuple::Tuple* book (const std::string& dirPath, long id, const CLID& type, const std::string& title);
  /// Book Ntuple and register it with the data store.
  virtual NTuple::Tuple* book (DataObject* pParent, const std::string& relPath, const CLID& type, const std::string& title);
  /// Book Ntuple and register it with the data store.
  virtual NTuple::Tuple* book (DataObject* pParent, long id, const CLID& type, const std::string& title);
  /// Access N tuple on disk.
  virtual NTuple::Tuple* access(const std::string& fullPath, const std::string& filename);
  /// Save N tuple to disk. Must be called in order to close the ntuple file properly
  virtual StatusCode save(const std::string& fullPath);
  /// Save N tuple to disk. Must be called in order to close the ntuple file properly
  virtual StatusCode save(NTuple::Tuple* tuple);
  /// Save N tuple to disk. Must be called in order to close the ntuple file properly
  virtual StatusCode save(DataObject* pParent, const std::string& relPath);
  /// Write single record to N tuple.
  virtual StatusCode writeRecord( NTuple::Tuple* tuple );
  /// Write single record to N tuple.
  virtual StatusCode writeRecord( const std::string& fullPath );
  /// Write single record to N tuple.
  virtual StatusCode writeRecord( DataObject* pParent, const std::string& relPath );
  /// Read single record from N tuple.
  virtual StatusCode readRecord( NTuple::Tuple* tuple );
  /// Read single record from N tuple.
  virtual StatusCode readRecord( const std::string& fullPath );
  /// Read single record from N tuple.
  virtual StatusCode readRecord( DataObject* pParent, const std::string& relPath);

  /// Check if a datasource is connected
  virtual bool isConnected(const std::string& identifier)   const;
  /// Add file to list I/O list
  virtual StatusCode connect(const std::string& ident);
  /// Add file to list I/O list
  virtual StatusCode connect(const std::string& ident, std::string& logname);
  /// Close open connection
  virtual StatusCode disconnect(const std::string& nam);
  /// Close all open connections
  virtual StatusCode disconnectAll();

  /// Standard Constructor
  NTupleSvc(const std::string& name, ISvcLocator* svc);
  /// Standard Destructor
  virtual ~NTupleSvc();
protected:

  /// Create conversion service
  StatusCode createService( const std::string& nam,
                            const std::string& typ,
                            const std::vector<Prop>& props,
                            IConversionSvc*& pSvc);
  /// Finalize single service
  void releaseConnection(Connection& c);
  /// Attach output/input file
  StatusCode attachTuple(const std::string& filename, const std::string& logname, const char typ, const long t);
  /// Update directory data
  StatusCode updateDirectories();

  /// Output streams
  DBaseEntries                 m_output;
  /// Input streams
  DBaseEntries                 m_input;
  /// Container of connection points
  Connections                  m_connections;
};

#endif // GAUDI_NTUPLESVC_H
