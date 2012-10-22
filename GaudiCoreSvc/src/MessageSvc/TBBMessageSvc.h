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
  virtual void reportMessage(const Message& msg);

  /// Implementation of IMessageSvc::reportMessage()
  virtual void reportMessage(const Message& msg, int outputLevel);

  /// Implementation of IMessageSvc::reportMessage()
  virtual void reportMessage(const StatusCode& code, const std::string& source = "");

protected:
private:

  // ============================================================================
  // Helper tasks for message reporting.
  // ============================================================================
  /// Common base class for the different reportMessage cases.
  class MessageTaskCommon: public Gaudi::SerialTaskQueue::WorkItem {
  public:
    MessageTaskCommon(TBBMessageSvc& svc):
      m_svc(svc),
      m_sender(std::this_thread::get_id()) {}
  protected:
    TBBMessageSvc& m_svc;
    std::thread::id m_sender;
  };

  /// Specialized class to report a message with explicit output level.
  class MessageWithLevel: public MessageTaskCommon {
  public:
    MessageWithLevel(TBBMessageSvc& svc, Message msg, int level):
      MessageTaskCommon(svc),
      m_msg(msg), m_level(level) {}
    virtual void run() {
      m_svc.i_reportMessage(m_msg, m_level);
    }
  private:
    Message m_msg;
    int m_level;
  };

  /// Specialized class to report a message with implicit output level.
  class MessageWithoutLevel: public MessageTaskCommon {
  public:
    MessageWithoutLevel(TBBMessageSvc& svc, Message msg):
      MessageTaskCommon(svc),
      m_msg(msg) {}
    virtual void run() {
      const int level = m_svc.outputLevel(m_msg.getSource());
      m_svc.i_reportMessage(m_msg, level);
    }
  private:
    Message m_msg;
  };

  /// Specialized class to report a StatusCode message.
  class StatusCodeMessage: public MessageTaskCommon {
  public:
    StatusCodeMessage(TBBMessageSvc& svc, const StatusCode &sc, const std::string& source):
      MessageTaskCommon(svc),
      m_sc(sc),
      m_source(source) {}
    virtual void run() {
      m_svc.i_reportMessage(m_sc, m_source);
    }
  private:
    StatusCode m_sc;
    std::string m_source;
  };

  Gaudi::SerialTaskQueue m_messageQueue;
};

#endif // MESSAGESVC_TBBMESSAGESVC_H
