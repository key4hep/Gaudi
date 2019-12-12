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
#include "TBBMessageSvc.h"

// ----------------------------------------------------------------------------
// Implementation file for class: TBBMessageSvc
//
// 22/06/2012: Marco Clemencic
// ----------------------------------------------------------------------------
DECLARE_COMPONENT( TBBMessageSvc )

void TBBMessageSvc::reportMessage( const Message& msg, int outputLevel ) {
  m_messageQueue.add( new MessageWithLevel( *this, msg, outputLevel ) );
}

void TBBMessageSvc::reportMessage( const Message& msg ) { m_messageQueue.add( new MessageWithoutLevel( *this, msg ) ); }

void TBBMessageSvc::reportMessage( const StatusCode& code, const std::string& source ) {
  m_messageQueue.add( new StatusCodeMessage( *this, code, source ) );
}

// ============================================================================
