/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Kernel.h"

#include "GaudiKernel/Auditor.h"

#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/MsgStream.h"

// Constructor
Auditor::Auditor( std::string name, ISvcLocator* pSvcLocator )
    : m_name( std::move( name ) ), m_pSvcLocator( pSvcLocator ) {}

// IAuditor implementation
StatusCode Auditor::sysInitialize() {
  StatusCode sc;

  // Bypass the initialization if the auditor is disabled or
  // has already been initialized.
  if ( isEnabled() && !m_isInitialized ) {

    // Setup the default service ... this should be upgraded so as to be configurable.
    if ( !m_pSvcLocator ) return StatusCode::FAILURE;

    // this initializes the messaging, in case property update handlers need to print
    // and update the property value bypassing the update handler
    m_outputLevel.value() = setUpMessaging();

    // Set the Algorithm's properties
    bindPropertiesTo( serviceLocator()->getOptsSvc() );

    if ( !sc.isSuccess() ) return StatusCode::FAILURE;

    {
      try {
        // Invoke the initialize() method of the derived class
        sc = initialize();
        if ( !sc.isSuccess() ) return StatusCode::FAILURE;
        m_isInitialized = true;

        return sc;                                /// RETURN !!!
      } catch ( const GaudiException& Exception ) /// catch Gaudi Exception
      {
        /// (1) perform the printout of message
        MsgStream log( msgSvc(), name() + ".sysInitialize()" );
        log << MSG::FATAL << " Exception with tag=" << Exception.tag() << " is caught " << endmsg;
        /// (2) print  the exception itself (NB!  - GaudiException is a linked list of all "previous exceptions")
        MsgStream logEx( msgSvc(), Exception.tag() );
        logEx << MSG::ERROR << Exception << endmsg;
      } catch ( const std::exception& Exception ) /// catch std::exception
      {
        /// (1) perform the printout of message
        MsgStream log( msgSvc(), name() + ".sysInitialize()" );
        log << MSG::FATAL << " Standard std::exception is caught " << endmsg;
        /// (2) print  the exception itself (NB!  - GaudiException is a linked list of all "previous exceptions")
        MsgStream logEx( msgSvc(), name() + "*std::exception*" );
        logEx << MSG::ERROR << Exception.what() << endmsg;
      } catch ( ... ) {
        /// (1) perform the printout
        MsgStream log( msgSvc(), name() + ".sysInitialize()" );
        log << MSG::FATAL << " UNKNOWN Exception is  caught " << endmsg;
      }
    }
  }
  ///
  return StatusCode::FAILURE;
}

StatusCode Auditor::initialize() { return StatusCode::SUCCESS; }

// Implemented for backward compatibility
void Auditor::before( StandardEventType evt, INamedInterface* obj ) {
  switch ( evt ) {
  case Initialize:
    beforeInitialize( obj );
    break;
  case ReInitialize:
    beforeReinitialize( obj );
    break;
  case Execute:
    beforeExecute( obj );
    break;
  case Finalize:
    beforeFinalize( obj );
    break;
  case Start:
    break;
  case Stop:
    break;
  case ReStart:
    break;
  default:
    break; // do nothing
  }
}
void Auditor::before( StandardEventType, const std::string& ) {}

void Auditor::before( CustomEventTypeRef, INamedInterface* ) {}
void Auditor::before( CustomEventTypeRef, const std::string& ) {}

// Implemented for backward compatibility
void Auditor::after( StandardEventType evt, INamedInterface* obj, const StatusCode& sc ) {
  switch ( evt ) {
  case Initialize:
    afterInitialize( obj );
    break;
  case ReInitialize:
    afterReinitialize( obj );
    break;
  case Execute:
    afterExecute( obj, sc );
    break;
  case Finalize:
    afterFinalize( obj );
    break;
  case Start:
    break;
  case Stop:
    break;
  case ReStart:
    break;
  default:
    break; // do nothing
  }
}
void Auditor::after( StandardEventType, const std::string&, const StatusCode& ) {}

void Auditor::after( CustomEventTypeRef, INamedInterface*, const StatusCode& ) {}
void Auditor::after( CustomEventTypeRef, const std::string&, const StatusCode& ) {}

void Auditor::beforeInitialize( INamedInterface* ) {}
void Auditor::afterInitialize( INamedInterface* ) {}
void Auditor::beforeReinitialize( INamedInterface* ) {}
void Auditor::afterReinitialize( INamedInterface* ) {}
void Auditor::beforeExecute( INamedInterface* ) {}
void Auditor::afterExecute( INamedInterface*, const StatusCode& ) {}
void Auditor::beforeFinalize( INamedInterface* ) {}
void Auditor::afterFinalize( INamedInterface* ) {}

StatusCode Auditor::sysFinalize() {
  StatusCode sc = StatusCode::SUCCESS;
  try {
    //
    // Invoke the finalize() method of the derived class if
    // it has been initialized.
    if ( m_isInitialized && !m_isFinalized ) {
      m_isFinalized = true;
      sc            = finalize();
      if ( !sc.isSuccess() ) return StatusCode::FAILURE;
    }
    return sc; /// RETURN !!!
    //
  } catch ( const GaudiException& Exception ) /// catch GaudiExeption
  {
    /// (1) perform the printout of message
    MsgStream log( msgSvc(), name() + ".sysFinalize()" );
    log << MSG::FATAL << " Exception with tag=" << Exception.tag() << " is catched " << endmsg;
    /// (2) print  the exception itself
    ///       (NB!  - GaudiException is a linked list of all "previous exceptions")
    MsgStream logEx( msgSvc(), Exception.tag() );
    logEx << MSG::ERROR << Exception << endmsg;
  } catch ( const std::exception& Exception ) /// catch std::exception
  {
    /// (1) perform the printout of message
    MsgStream log( msgSvc(), name() + ".sysFinalize()" );
    log << MSG::FATAL << " Standard std::exception is caught " << endmsg;
    /// (2) print  the exception itself
    MsgStream logEx( msgSvc(), name() + "*std::exception*" );
    logEx << MSG::ERROR << Exception.what() << endmsg;
  } catch ( ... ) /// catch unknown exception
  {
    /// (1) perform the printout
    MsgStream log( msgSvc(), name() + ".sysFinalize()" );
    log << MSG::FATAL << " UNKNOWN Exception is caught " << endmsg;
  }
  ///
  return StatusCode::FAILURE;
}

StatusCode Auditor::finalize() { return StatusCode::SUCCESS; }

const std::string& Auditor::name() const { return m_name; }

bool Auditor::isEnabled() const { return m_isEnabled; }

SmartIF<ISvcLocator>& Auditor::serviceLocator() const { return m_pSvcLocator; }
