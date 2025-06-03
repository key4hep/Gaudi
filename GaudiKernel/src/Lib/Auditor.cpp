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
#include <Gaudi/Auditor.h>

#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/MsgStream.h>

StatusCode Gaudi::Auditor::sysInitialize() {
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
    try {
      // Invoke the initialize() method of the derived class
      return initialize().andThen( [&] { m_isInitialized = true; } );
    } catch ( const GaudiException& Exception ) { /// catch Gaudi Exception
      /// (1) perform the printout of message
      MsgStream log( msgSvc(), name() + ".sysInitialize()" );
      log << MSG::FATAL << " Exception with tag=" << Exception.tag() << " is caught " << endmsg;
      /// (2) print  the exception itself (NB!  - GaudiException is a linked list of all "previous exceptions")
      MsgStream logEx( msgSvc(), Exception.tag() );
      logEx << MSG::ERROR << Exception << endmsg;
    } catch ( const std::exception& Exception ) { /// catch std::exception
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
  return StatusCode::FAILURE;
}

StatusCode Gaudi::Auditor::sysFinalize() {
  try {
    // Invoke the finalize() method of the derived class if
    // it has been initialized.
    if ( m_isInitialized && !m_isFinalized ) {
      m_isFinalized = true;
      return finalize();
    }
    return StatusCode::SUCCESS;
  } catch ( const GaudiException& Exception ) { /// catch GaudiExeption
    /// (1) perform the printout of message
    MsgStream log( msgSvc(), name() + ".sysFinalize()" );
    log << MSG::FATAL << " Exception with tag=" << Exception.tag() << " is catched " << endmsg;
    /// (2) print  the exception itself
    ///       (NB!  - GaudiException is a linked list of all "previous exceptions")
    MsgStream logEx( msgSvc(), Exception.tag() );
    logEx << MSG::ERROR << Exception << endmsg;
  } catch ( const std::exception& Exception ) { /// catch std::exception
    /// (1) perform the printout of message
    MsgStream log( msgSvc(), name() + ".sysFinalize()" );
    log << MSG::FATAL << " Standard std::exception is caught " << endmsg;
    /// (2) print  the exception itself
    MsgStream logEx( msgSvc(), name() + "*std::exception*" );
    logEx << MSG::ERROR << Exception.what() << endmsg;
  } catch ( ... ) { /// catch unknown exception
    /// (1) perform the printout
    MsgStream log( msgSvc(), name() + ".sysFinalize()" );
    log << MSG::FATAL << " UNKNOWN Exception is caught " << endmsg;
  }
  return StatusCode::FAILURE;
}
