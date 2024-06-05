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
//	============================================================
//
//	PersistencySvc.h
//	------------------------------------------------------------
//
//	Package   : PersistencySvc
//
//	Author    : Markus Frank
//
//	===========================================================
#pragma once

#include <GaudiKernel/IAddressCreator.h>
#include <GaudiKernel/IConversionSvc.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/IPersistencySvc.h>
#include <GaudiKernel/Service.h>
#include <algorithm>
#include <functional>
#include <map>

/** PersistencySvc class implementation definition.

    <P> System:  The LHCb Offline System
    <P> Package: PersistencySvc

     Dependencies:
    <UL>
    <LI> STL:                         <vector>, <algorithm>, <functional>
    <LI> Generic service definition:  "Kernel/Service.h"
    <LI> IConversionSvc definition:  "Kernel/IConversionSvc.h"
    <LI> PersistencySvc definition:  "Kernel/Interfaces/IPersistencySvc.h"
    </UL>

    History:
    <PRE>
    +---------+----------------------------------------------+---------+
    |    Date |                 Comment                      | Who     |
    +---------+----------------------------------------------+---------+
    | 3/11/98 | Initial version                              | M.Frank |
    +---------+----------------------------------------------+---------+
    </PRE>
   @author Markus Frank
   @version 1.0
*/
class PersistencySvc : public extends<Service, IConversionSvc, IPersistencySvc, IAddressCreator> {
protected:
  class ServiceEntry final {
    long                             m_serviceType;
    mutable SmartIF<IService>        m_service;
    mutable SmartIF<IConversionSvc>  m_cnvService;
    mutable SmartIF<IAddressCreator> m_addrCreator;

  public:
    ServiceEntry( long type, SmartIF<IService> svc, SmartIF<IConversionSvc> cnv, SmartIF<IAddressCreator> cr )
        : m_serviceType( type )
        , m_service( std::move( svc ) )
        , m_cnvService( std::move( cnv ) )
        , m_addrCreator( std::move( cr ) ) {}
    ServiceEntry( long type, IService* svc, IConversionSvc* cnv, IAddressCreator* cr ) {
      m_serviceType = type;
      m_addrCreator = cr;
      m_cnvService  = cnv;
      m_service     = svc;
    }
    ServiceEntry( const ServiceEntry& ) = default;

    SmartIF<IService>&        service() const { return m_service; }
    SmartIF<IConversionSvc>&  conversionSvc() const { return m_cnvService; }
    SmartIF<IAddressCreator>& addrCreator() const { return m_addrCreator; }
    long                      svcType() const { return m_serviceType; }
  };
  typedef std::map<long, ServiceEntry> Services;

public:
  /**@name IPersistencySvc Interface implementation */
  //@{
  /// Return default service type
  long repSvcType() const override;

  /// Retrieve the class type of objects the converter produces. (DUMMY)
  const CLID& objType() const override;

  /// Implementation of IConverter: Create the transient representation of an object.
  StatusCode createObj( IOpaqueAddress* pAddress, DataObject*& refpObject ) override;

  /// Implementation of IConverter: Resolve the references of the created transient object.
  StatusCode fillObjRefs( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  /// Implementation of IConverter: Update the transient object from the other representation.
  StatusCode updateObj( IOpaqueAddress* pAddress, DataObject* refpObject ) override;

  /// Implementation of IConverter: Update the references of an updated transient object.
  StatusCode updateObjRefs( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  /// Implementation of IConverter: Convert the transient object to the requested representation.
  StatusCode createRep( DataObject* pObject, IOpaqueAddress*& refpAddress ) override;

  /// Implementation of IConverter: Resolve the references of the converted object.
  StatusCode fillRepRefs( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  /// Implementation of IConverter: Update the converted representation of a transient object.
  StatusCode updateRep( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  /// Implementation of IConverter: Update the references of an already converted object.
  StatusCode updateRepRefs( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  /// Define transient datastore.
  StatusCode setDataProvider( IDataProviderSvc* pStore ) override;

  /// Access reference to transient datastore
  SmartIF<IDataProviderSvc>& dataProvider() const override;

  /// Set conversion service the converter is connected to
  StatusCode setConversionSvc( IConversionSvc* svc ) override;

  /// Get conversion service the converter is connected to
  SmartIF<IConversionSvc>& conversionSvc() const override;

  /// Set address creator facility
  StatusCode setAddressCreator( IAddressCreator* creator ) override;

  /// Retrieve address creator facility
  SmartIF<IAddressCreator>& addressCreator() const override;

  /// Add converter object to conversion service.
  StatusCode addConverter( IConverter* pConverter ) override;

  /// Add converter object to conversion service.
  StatusCode addConverter( const CLID& clid ) override;

  /// Retrieve converter from list
  IConverter* converter( const CLID& clid ) override;

  /// Remove converter object from conversion service (if present).
  StatusCode removeConverter( const CLID& clid ) override;

  /// Connect the output file to the service with open mode.
  StatusCode connectOutput( const std::string& outputFile, const std::string& openMode ) override;
  /// Connect the output file to the service.
  StatusCode connectOutput( const std::string& outputFile ) override;

  /// Commit pending output.
  StatusCode commitOutput( const std::string& output, bool do_commit ) override;

  /// Create a Generic address using explicit arguments to identify a single object.
  StatusCode createAddress( long svc_type, const CLID& clid, const std::string* pars, const unsigned long* ipars,
                            IOpaqueAddress*& refpAddress ) override;

  /// Convert an address to string form
  StatusCode convertAddress( const IOpaqueAddress* pAddress, std::string& refAddress ) override;

  /// Convert an address in string form to object form
  StatusCode createAddress( long svc_type, const CLID& clid, const std::string& refAddress,
                            IOpaqueAddress*& refpAddress ) override;
  //@}

  /**@name: IPersistencySvc implementation  */
  //@{
  /// Add a new Service
  StatusCode addCnvService( IConversionSvc* service ) override;

  /// Remove a Service
  StatusCode removeCnvService( long type ) override;

  /// Set default service type
  StatusCode setDefaultCnvService( long type ) override;

  /// Retrieve conversion service identified by technology
  StatusCode getService( long service_type, IConversionSvc*& refpSvc ) override;

  /// Retrieve conversion service identified by technology
  StatusCode getService( const std::string& service_type, IConversionSvc*& refpSvc ) override;
  //@}

  /**@name: IService implementation */
  //@{
  /// Initialize the service.
  StatusCode initialize() override;

  /// stop the service.
  StatusCode finalize() override;
  //@}

  /**@name: Object implementation  */
  //@{
  /// Standard Constructor
  using extends::extends;

protected:
  /// Retrieve conversion service by name
  SmartIF<IConversionSvc>& service( const std::string& nam );

  /// Retrieve conversion service from list
  SmartIF<IConversionSvc>& service( long service_type );

  /// Retrieve address creator from list
  SmartIF<IAddressCreator>& addressCreator( long service_type );

  /// Retrieve string from storage type and clid
  void encodeAddrHdr( long service_type, const CLID& clid, std::string& address ) const;

  /// Retrieve storage type and clid from address header of string
  void decodeAddrHdr( const std::string& address, long& service_type, CLID& clid, std::string& address_trailer ) const;

  /// Implementation helper
  StatusCode makeCall( int typ, IOpaqueAddress*& pAddress, DataObject*& pObject );

  /// Set enabled flag
  bool enable( bool value );
  //@}

  /// Default service type
  long m_cnvDefType = TEST_StorageType;
  /// Pointer to data provider service
  mutable SmartIF<IDataProviderSvc> m_dataSvc;
  /// List of conversion workers
  Services m_cnvServices;
  /// Default output service
  mutable SmartIF<IConversionSvc> m_cnvDefault;

  Gaudi::Property<std::vector<std::string>> m_svcNames{
      this,
      "CnvServices",
      {},
      [this]( auto& p ) {
        if ( this->msgLevel( MSG::DEBUG ) ) this->debug() << p << endmsg;
      },
      "Names of services to be requested from the service locator and added by default",
      "OrderedSet<std::string>" };

  /// Flag to indicate that the service is enabled
  bool m_enable = true;

  /// Pointer to the IAddressCreator interface of this, for addressCreator().
  mutable SmartIF<IAddressCreator> m_addrCreator;
};
