// Include files

// From Gaudi
#include "GaudiKernel/SvcFactory.h"
// local
#include "InertMessageSvc.h"

// ----------------------------------------------------------------------------
// Implementation file for class: InertMessageSvc
//
// 12/02/2013: Danilo Piparo
// ----------------------------------------------------------------------------
DECLARE_SERVICE_FACTORY( InertMessageSvc )

//---------------------------------------------------------------------------

StatusCode InertMessageSvc::initialize()
{
  StatusCode sc = MessageSvc::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;          // error printed already by MessageSvc

  info() << "Activating in a separate thread" << endmsg;
  m_thread = std::thread( &InertMessageSvc::m_activate, this );

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode InertMessageSvc::InertMessageSvc::finalize()
{
  m_deactivate();
  m_thread.join();
  return MessageSvc::finalize(); // must be called after all other actions
}

//---------------------------------------------------------------------------

void InertMessageSvc::m_activate()
{
  m_isActive = true;
  std::function<void()> thisMessageAction;
  while ( m_isActive || !m_messageActionsQueue.empty() ) {
    m_messageActionsQueue.pop( thisMessageAction );
    if ( thisMessageAction ) thisMessageAction();
  }
}

//---------------------------------------------------------------------------

void InertMessageSvc::m_deactivate()
{
  if ( m_isActive ) {
// This would be the last action
#if defined( __clang__ ) || defined( __CLING__ )
    m_messageActionsQueue.push( [this]() { m_isActive = false; } );
#else
    m_messageActionsQueue.emplace( [this]() { m_isActive = false; } );
#endif
  }
}

//---------------------------------------------------------------------------
/**
 * The message action is created and pushed to the message queue.
 * The message is captured by value since the one referenced by msg can
 * (and basically always will given the asynchronous nature of the printing)
 * go out of scope before the differed print.
 */
void InertMessageSvc::reportMessage( const Message& msg, int outputLevel )
{
// msg has to be copied as the reference may become invalid by the time it is used
#if defined( __clang__ ) || defined( __CLING__ )
  m_messageActionsQueue.push(
      [ this, m = Message( msg ), outputLevel ]() { this->i_reportMessage( m, outputLevel ); } );
#else
  m_messageActionsQueue.emplace(
      [ this, m = Message( msg ), outputLevel ]() { this->i_reportMessage( m, outputLevel ); } );
#endif
}

//---------------------------------------------------------------------------

void InertMessageSvc::reportMessage( const Message& msg )
{
// msg has to be copied as the reference may become invalid by the time it's used
#if defined( __clang__ ) || defined( __CLING__ )
  m_messageActionsQueue.push(
      [ this, m = Message( msg ) ]() { this->i_reportMessage( m, this->outputLevel( m.getSource() ) ); } );
#else
  m_messageActionsQueue.emplace(
      [ this, m = Message( msg ) ]() { this->i_reportMessage( m, this->outputLevel( m.getSource() ) ); } );
#endif
}

//---------------------------------------------------------------------------

void InertMessageSvc::reportMessage( const StatusCode& code, const std::string& source )
{
// msg has to be copied as the source may become invalid by the time it's used
#if defined( __clang__ ) || defined( __CLING__ )
  m_messageActionsQueue.push( [ this, code, s = std::string( source ) ]() { this->i_reportMessage( code, s ); } );
#else
  m_messageActionsQueue.emplace( [ this, code, s = std::string( source ) ]() { this->i_reportMessage( code, s ); } );
#endif
}

//---------------------------------------------------------------------------
