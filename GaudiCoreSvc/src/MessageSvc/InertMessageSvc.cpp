/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "MessageSvc.h"
#include <functional>
#include <memory>
#include <tbb/concurrent_queue.h>
#include <thread>

/** @class InertMessageSvc InertMessageSvc.h MessageSvc/InertMessageSvc.h
 *
 * Thread safe extension to the standard MessageSvc. The InertMessageSvc runs
 * in a separate thread sleeping until the arrival of a message to print out
 * therefore using a very limited amount of resources.
 * The most important part of its implementation is the presence of a
 * TBB concurrent bounded queue which not only allows to serialise the
 * received messages in order to print them but also to put the thread
 * where the service is located to sleep when the queue is empty.
 *
 * @author Danilo Piparo
 * @date 12/02/2013
 */
class InertMessageSvc : public MessageSvc {
public:
  using MessageSvc::MessageSvc;

  /// Initialization of the service.
  StatusCode initialize() override;

  /// Finalization of the service.
  StatusCode finalize() override;

  using MessageSvc::reportMessage;

  /// Implementation of IMessageSvc::reportMessage()
  void reportMessage( const Message& msg ) override;

  /// Implementation of IMessageSvc::reportMessage()
  void reportMessage( const Message& msg, int outputLevel ) override;

  /// Implementation of IMessageSvc::reportMessage()
  void reportMessage( const StatusCode& code, std::string_view source = "" ) override;

private:
  void              m_activate();
  void              m_deactivate();
  std::atomic<bool> m_isActive = false;

  void i_onSuppressChanged( bool v ) override {
    m_messageActionsQueue.emplace( [this, v] { m_suppress = v; } );
  }
  void i_onDefaultFormatChanged( const std::string& v ) override {
    m_messageActionsQueue.emplace( [this, v] { m_defaultFormat = v; } );
  }
  tbb::concurrent_bounded_queue<std::function<void()>> m_messageActionsQueue;

  std::thread m_thread;
};

DECLARE_COMPONENT( InertMessageSvc )

StatusCode InertMessageSvc::initialize() {
  StatusCode sc = MessageSvc::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;          // error printed already by MessageSvc

  info() << "Activating in a separate thread" << endmsg;
  m_thread = std::thread( &InertMessageSvc::m_activate, this );

  return StatusCode::SUCCESS;
}

StatusCode InertMessageSvc::InertMessageSvc::finalize() {
  m_messageActionsQueue.emplace( [this] { m_suppress = false; } );
  m_deactivate();
  m_thread.join();
  return MessageSvc::finalize(); // must be called after all other actions
}

void InertMessageSvc::m_activate() {
  m_isActive = true;
  std::function<void()> thisMessageAction;
  while ( m_isActive || !m_messageActionsQueue.empty() ) {
    m_messageActionsQueue.pop( thisMessageAction );
    if ( thisMessageAction ) thisMessageAction();
  }
}

void InertMessageSvc::m_deactivate() {
  if ( m_isActive ) {
    // This would be the last action
    m_messageActionsQueue.emplace( [this]() { m_isActive = false; } );
  }
}

/**
 * The message action is created and pushed to the message queue.
 * The message is captured by value since the one referenced by msg can
 * (and basically always will given the asynchronous nature of the printing)
 * go out of scope before the differed print.
 */
void InertMessageSvc::reportMessage( const Message& msg, int outputLevel ) {
  // msg has to be copied as the reference may become invalid by the time it is used
  m_messageActionsQueue.emplace(
      [this, m = Message( msg ), outputLevel]() { this->i_reportMessage( m, outputLevel ); } );
}

void InertMessageSvc::reportMessage( const Message& msg ) {
  // msg has to be copied as the reference may become invalid by the time it's used
  m_messageActionsQueue.emplace(
      [this, m = Message( msg )]() { this->i_reportMessage( m, this->outputLevel( m.getSource() ) ); } );
}

void InertMessageSvc::reportMessage( const StatusCode& code, std::string_view source ) {
  // msg has to be copied as the source may become invalid by the time it's used
  m_messageActionsQueue.emplace( [this, code, s = std::string{ source }]() { this->i_reportMessage( code, s ); } );
}
