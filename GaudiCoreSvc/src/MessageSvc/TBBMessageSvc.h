#ifndef MESSAGESVC_TBBMESSAGESVC_H
#define MESSAGESVC_TBBMESSAGESVC_H 1
// Include files
#include "MessageSvc.h"

#include "GaudiKernel/SerialTaskQueue.h"

// just needed for the thread id
#include <thread>

/** @class TBMessageSvc TBMessageSvc.h MessageSvc/TBMessageSvc.h
  *
  * Extension to the standard MessageSvc that
  *
  * @author Marco Clemencic
  * @date 22/06/2012
  */
class TBBMessageSvc: public MessageSvc {
public:
  /// Standard constructor
  TBBMessageSvc(const std::string& name, ISvcLocator* pSvcLocator);

  virtual ~TBBMessageSvc(); ///< Destructor

  /// Initialization of the service.
  virtual StatusCode initialize();

  /// Finalization of the service.
  virtual StatusCode finalize();

  using MessageSvc::reportMessage;
  /// Implementation of IMessageSvc::reportMessage()
  virtual void reportMessage(const Message& msg, int outputLevel);

protected:
private:

  /// Simple class to wrap the data needed to report a message.
  class MessageTask: public Gaudi::SerialTaskQueue::WorkItem {
  public:
    MessageTask(TBBMessageSvc& svc, Message msg, int level):
      m_svc(svc),
      m_msg(msg), m_level(level),
      m_sender(std::this_thread::get_id()) {}

    virtual void run() {
      m_svc.MessageSvc::reportMessage(m_msg, m_level);
    }
  private:
    TBBMessageSvc& m_svc;
    Message m_msg;
    int m_level;
    std::thread::id m_sender;
  };

  Gaudi::SerialTaskQueue m_messageQueue;
};

#endif // MESSAGESVC_TBBMESSAGESVC_H
