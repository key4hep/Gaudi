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
  void m_activate();
  void m_deactivate();
  bool m_isActive = false;

  tbb::concurrent_bounded_queue<std::function<void()>> m_messageActionsQueue;

  std::thread m_thread;
};
