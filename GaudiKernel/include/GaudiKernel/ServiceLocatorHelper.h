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

#include <GaudiKernel/IService.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/SmartIF.h>
#include <GaudiKernel/StatusCode.h>
#include <string>

class InterfaceID;
class MsgStream;

/** @class ServiceLocatorHelper
 *  @brief an helper to share the implementation of service() among the
 *         various kernel base classes
 */
class GAUDI_API ServiceLocatorHelper {
public:
  ServiceLocatorHelper( ISvcLocator& svcLoc, const INamedInterface& requester )
      : m_svcLoc( svcLoc )
      , m_msgLog( SmartIF<IMessageSvc>( &svcLoc ), requester.name() )
      , // use requester msg level
      m_requesterName( requester.name() ) {}
  ServiceLocatorHelper( ISvcLocator& svcLoc, std::string loggedName, std::string requesterName )
      : m_svcLoc( svcLoc )
      , m_msgLog( SmartIF<IMessageSvc>( &svcLoc ), std::move( loggedName ) )
      , // use requester msg level
      m_requesterName( std::move( requesterName ) ) {}
  ServiceLocatorHelper( ISvcLocator& svcLoc, std::string requesterName )
      : m_svcLoc( svcLoc )
      , m_msgLog( SmartIF<IMessageSvc>( &svcLoc ), requesterName )
      , // use requester msg level
      m_requesterName( std::move( requesterName ) ) {}

  StatusCode getService( std::string_view name, bool createIf, const InterfaceID& iid, void** ppSvc ) const {
    return createIf ? createService( name, iid, ppSvc ) : locateService( name, iid, ppSvc, true );
  }

  StatusCode locateService( std::string_view name, const InterfaceID& iid, void** ppSvc, bool quiet = false ) const;

  StatusCode createService( std::string_view name, const InterfaceID& iid, void** ppSvc ) const;

  StatusCode createService( std::string_view type, std::string_view name, const InterfaceID& iid, void** ppSvc ) const;

  SmartIF<IService> service( std::string_view name, const bool quiet = false, const bool createIf = true ) const;

  template <typename T>
  SmartIF<T> service( std::string_view name, const bool quiet = false, const bool createIf = true ) const {
    return service( name, quiet, createIf ).as<T>();
  }

private:
  ISvcLocator*       serviceLocator() const { return &m_svcLoc; }
  MsgStream&         log() const { return m_msgLog; }
  const std::string& requesterName() const { return m_requesterName; }
  ISvcLocator&       m_svcLoc;
  mutable MsgStream  m_msgLog;
  std::string        m_requesterName;
};
