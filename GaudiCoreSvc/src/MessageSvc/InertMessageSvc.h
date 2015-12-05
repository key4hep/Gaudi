#ifndef MESSAGESVC_INERTMESSAGESVC_H
#define MESSAGESVC_INERTMESSAGESVC_H 1
// Include files
// FW
#include "MessageSvc.h"
// C++
#include <thread>
#include <functional>
#include <memory>
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
class InertMessageSvc: public MessageSvc {
public:
  /// Standard constructor
  InertMessageSvc(const std::string& name, ISvcLocator* pSvcLocator);

  virtual ~InertMessageSvc(); ///< Destructor

  /// Initialization of the service.
  virtual StatusCode initialize();

  /// Finalization of the service.
  virtual StatusCode finalize();

  using MessageSvc::reportMessage;

  /// Implementation of IMessageSvc::reportMessage()
  virtual void reportMessage(const Message& msg);

  /// Implementation of IMessageSvc::reportMessage()
  virtual void reportMessage(const Message& msg, int outputLevel);

  /// Implementation of IMessageSvc::reportMessage()
  virtual void reportMessage(const StatusCode& code, const std::string& source = "");

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
