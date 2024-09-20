/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDI_NTUPLESVC_H
#define GAUDI_NTUPLESVC_H 1

// Framework include files
#include <GaudiKernel/DataSvc.h>
#include <GaudiKernel/IDataSourceMgr.h>
#include <GaudiKernel/INTupleSvc.h>

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
class NTupleSvc : public extends<DataSvc, INTupleSvc, IDataSourceMgr> {
protected:
  struct Connection final {
    IConversionSvc* service;
    Connection( IConversionSvc* s ) : service( s ) {}
    Connection( const Connection& c ) : service( c.service ) {}
  };
  typedef std::vector<std::string>            DBaseEntries;
  typedef std::pair<std::string, std::string> Prop;

public:
  /// DataSvc overrides: Initialize the service.
  StatusCode initialize() override;
  /// DataSvc overrides: reinitialize service.
  StatusCode reinitialize() override;
  /// DataSvc overrides: stop the service.
  StatusCode finalize() override;
  /// DataSvc overrides: Retrieve data loader
  IConversionSvc* getDataLoader( IRegistry* pReg ) override;

  /// Create Ntuple directory and register it with the data store.
  NTuple::Directory* createDirectory( DataObject* pParent, const std::string& title ) override;
  /// Create Ntuple directory and register it with the data store.
  NTuple::Directory* createDirectory( DataObject* pParent, long id ) override;
  /// Create Ntuple directory and register it with the data store.
  NTuple::Directory* createDirectory( const std::string& dirPath, long id ) override;
  /// Create Ntuple directory and register it with the data store.
  NTuple::Directory* createDirectory( const std::string& dirPath, const std::string& title ) override;
  /// Create Ntuple directory and register it with the data store.
  NTuple::Directory* createDirectory( const std::string& fullPath ) override;

  /// Create requested N tuple (Hide constructor)
  StatusCode create( const CLID& typ, const std::string& title, NTuple::Tuple*& refpTuple ) override;
  /// Book Ntuple and register it with the data store.
  NTuple::Tuple* book( const std::string& fullPath, const CLID& type, const std::string& title ) override;
  /// Book Ntuple and register it with the data store.
  NTuple::Tuple* book( const std::string& dirPath, const std::string& relPath, const CLID& type,
                       const std::string& title ) override;
  /// Book Ntuple and register it with the data store.
  NTuple::Tuple* book( const std::string& dirPath, long id, const CLID& type, const std::string& title ) override;
  /// Book Ntuple and register it with the data store.
  NTuple::Tuple* book( DataObject* pParent, const std::string& relPath, const CLID& type,
                       const std::string& title ) override;
  /// Book Ntuple and register it with the data store.
  NTuple::Tuple* book( DataObject* pParent, long id, const CLID& type, const std::string& title ) override;
  /// Access N tuple on disk.
  NTuple::Tuple* access( const std::string& fullPath, const std::string& filename ) override;
  /// Save N tuple to disk. Must be called in order to close the ntuple file properly
  StatusCode save( const std::string& fullPath ) override;
  /// Save N tuple to disk. Must be called in order to close the ntuple file properly
  StatusCode save( NTuple::Tuple* tuple ) override;
  /// Save N tuple to disk. Must be called in order to close the ntuple file properly
  StatusCode save( DataObject* pParent, const std::string& relPath ) override;
  /// Write single record to N tuple.
  StatusCode writeRecord( NTuple::Tuple* tuple ) override;
  /// Write single record to N tuple.
  StatusCode writeRecord( const std::string& fullPath ) override;
  /// Write single record to N tuple.
  StatusCode writeRecord( DataObject* pParent, const std::string& relPath ) override;
  /// Read single record from N tuple.
  StatusCode readRecord( NTuple::Tuple* tuple ) override;
  /// Read single record from N tuple.
  StatusCode readRecord( const std::string& fullPath ) override;
  /// Read single record from N tuple.
  StatusCode readRecord( DataObject* pParent, const std::string& relPath ) override;

  /// Check if a datasource is connected
  bool isConnected( const std::string& identifier ) const override;
  /// Add file to list I/O list
  StatusCode connect( const std::string& ident ) override;
  /// Add file to list I/O list
  StatusCode connect( const std::string& ident, std::string& logname ) override;
  /// Close open connection
  StatusCode disconnect( const std::string& nam ) override;
  /// Close all open connections
  StatusCode disconnectAll() override;

  /// Standard Constructor
  NTupleSvc( const std::string& name, ISvcLocator* svc );

protected:
  /// Create conversion service
  StatusCode createService( const std::string& nam, const std::string& typ, const std::vector<Prop>& props,
                            IConversionSvc*& pSvc );
  /// Finalize single service
  void releaseConnection( Connection& c );
  /// Attach output/input file
  StatusCode attachTuple( const std::string& filename, const std::string& logname, const char typ, const long t );
  /// Update directory data
  StatusCode updateDirectories();

  Gaudi::Property<DBaseEntries> m_input{ this, "Input", {}, "input streams" };
  Gaudi::Property<DBaseEntries> m_output{ this, "Output", {}, "output streams" };

  /// Container of connection points
  std::map<std::string, Connection> m_connections;
};

#endif // GAUDI_NTUPLESVC_H
