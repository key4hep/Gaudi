// Include files

// From Gaudi
#include "GaudiKernel/SvcFactory.h"
// local
#include "TBBMessageSvc.h"

// ----------------------------------------------------------------------------
// Implementation file for class: TBBMessageSvc
//
// 22/06/2012: Marco Clemencic
// ----------------------------------------------------------------------------
DECLARE_SERVICE_FACTORY( TBBMessageSvc )

void TBBMessageSvc::reportMessage( const Message& msg, int outputLevel )
{
  m_messageQueue.add( new MessageWithLevel( *this, msg, outputLevel ) );
}

void TBBMessageSvc::reportMessage( const Message& msg ) { m_messageQueue.add( new MessageWithoutLevel( *this, msg ) ); }

void TBBMessageSvc::reportMessage( const StatusCode& code, const std::string& source )
{
  m_messageQueue.add( new StatusCodeMessage( *this, code, source ) );
}

// ============================================================================
