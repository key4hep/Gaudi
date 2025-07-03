/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/IInterface.h>

/** @class IDataSourceMgr IDataSourceMgr.h GaudiKernel/IDataSourceMgr.h

    IDataSourceMgr interface definition.

    @author Markus Frank
    @version 1.0
*/
class GAUDI_API IDataSourceMgr : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IDataSourceMgr, 2, 0 );

  /** Check if a datasource is connected
      @param    identifier  Connection string
      @return               Status code indicating success or failure.
  */
  virtual bool isConnected( const std::string& identifier ) const = 0;

  /** Connect data source
      @param    mode        Connection mode (READ,CREATE,UPDATE)
      @param    logon       Connection string
      @param    identifier  Logical name (OUTPUT)
      @return               Status code indicating success or failure.
  */
  virtual StatusCode connect( const std::string& logon, std::string& identifier ) = 0;

  /** Connect data source
      @param    mode        Connection mode (READ,CREATE,UPDATE)
      @param    logon       Connection string
      @return               Status code indicating success or failure.
  */
  virtual StatusCode connect( const std::string& logon ) = 0;

  /** Disconnect data source
      @param    identifier  Connection string
      @return               Status code indicating success or failure.
  */
  virtual StatusCode disconnect( const std::string& identifier ) = 0;

  /// Close all open connections
  virtual StatusCode disconnectAll() = 0;
};
