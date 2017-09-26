#ifndef MESSAGESVC_INERTMESSAGESVC_H
#define MESSAGESVC_INERTMESSAGESVC_H 1
// Include files
// FW
#include "MessageSvc.h"
// C++
#include <functional>
#include <memory>
#include <thread>
// External libs
#include "tbb/concurrent_queue.h"

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
class InertMessageSvc : public MessageSvc
{
public:
  /// Standard constructor
  InertMessageSvc( const std::string& name, ISvcLocator* pSvcLocator );

  ~InertMessageSvc() override; ///< Destructor

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
  void reportMessage( const StatusCode& code, const std::string& source = "" ) override;

private:
  void m_activate();
  void m_deactivate();
  bool m_isActive;

  typedef std::function<void()> messageAction;
  /// This is done since the copy of the lambda storage is too expensive
  typedef std::shared_ptr<messageAction> messageActionPtr;
  tbb::concurrent_bounded_queue<messageActionPtr> m_messageActionsQueue;

  std::thread m_thread;
};

#endif // MESSAGESVC_INERTMESSAGESVC_H
