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
#pragma once
#include <GaudiKernel/Bootstrap.h>
#include <GaudiKernel/IAppMgrUI.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/ISvcManager.h>
#include <GaudiKernel/IToolSvc.h>
#include <GaudiKernel/SmartIF.h>
#include <sstream>
#include <stdexcept>
namespace GaudiTesting {
  void throwConditionFailed( bool condition, const std::string file_name, int line_no,
                             const std::string condition_string );
}

#define Ensures( a ) GaudiTesting::throwConditionFailed( a, __FILE__, __LINE__, #a )
#ifndef NDEBUG
#  define DEBUG_TRACE( a )                                                                                             \
    do { a; } while ( false )
#else
#  define DEBUG_TRACE( a )                                                                                             \
    do {                                                                                                               \
    } while ( false )
#endif

namespace GaudiTesting {
  class GaudiEnv {
  public:
    GaudiEnv()
        : m_iface{ Gaudi::createApplicationMgr() }
        , m_svcMgr( m_iface )
        , m_appMgr( m_iface )
        , m_propMgr( m_iface )
        , m_svcLoc( m_iface ) {
      Ensures( m_iface != nullptr );
      Ensures( m_svcMgr.isValid() );
      Ensures( m_appMgr.isValid() );
      Ensures( m_propMgr.isValid() );
      Ensures( m_svcLoc.isValid() );
      m_toolSvc = m_svcLoc->service( "ToolSvc" );
      Ensures( m_toolSvc.isValid() );
      //      pSvcLoc =
      (void)m_svcLoc.pRef();
      m_propMgr->setProperty( "EvtSel", "NONE" ).ignore();
      m_propMgr->setProperty( "JobOptionsType", "NONE" ).ignore();
      Ensures( ( m_appMgr->configure().isSuccess() ) );
      Ensures( ( m_appMgr->initialize().isSuccess() ) );
    }
    IInterface*          m_iface;
    SmartIF<ISvcManager> m_svcMgr;
    SmartIF<IAppMgrUI>   m_appMgr;
    SmartIF<IProperty>   m_propMgr;
    SmartIF<ISvcLocator> m_svcLoc;
    SmartIF<IToolSvc>    m_toolSvc;
  };
} // namespace GaudiTesting
