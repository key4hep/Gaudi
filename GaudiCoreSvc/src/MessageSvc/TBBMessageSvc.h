#ifndef MESSAGESVC_TBBMESSAGESVC_H
#define MESSAGESVC_TBBMESSAGESVC_H 1
// Include files
#include "MessageSvc.h"

// Include TBB headers
#include <tbb/concurrent_queue.h>
#include <tbb/task.h>
// just needed for the thread id
#include <tbb/compat/thread>

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
  struct MessageType {
    MessageType(Message _msg, int _level):
      msg(_msg), level(_level), sender(std::this_thread::get_id()) {}
    Message msg;
    int level;
    std::thread::id sender;
  };
  typedef tbb::concurrent_queue<MessageType*> QueueType;

  /// Helper class to report messages.
  class ReportTask: public tbb::task {
  public:
    /// Create an instance of the task, bound to the TBBMessageSvc, which is
    /// used to pop messages from the queue and actually print them.
    ReportTask(TBBMessageSvc& svc): m_svc(svc) {}
    tbb::task* execute() {
      MessageType* msg = NULL;
      if (m_svc.m_messageQueue.try_pop(msg)) {
        // the queue, was not empty -> process the message
        m_svc.MessageSvc::reportMessage(msg->msg, msg->level);
        delete msg; msg = NULL;
        // prepare continuation
        tbb::task* t = new(this->allocate_continuation()) ReportTask(m_svc);
        t->increment_ref_count();
        return t;
      }
      // if the queue was empty, we stop the continuation
      return NULL;
    }
  private:
    TBBMessageSvc& m_svc;
  };

  friend class ReportTask;

  inline void i_processMessages() {
    if (m_barrier->ref_count() == 1) { // no running children
      // start a new child
      m_barrier->increment_ref_count();
      tbb::task* t = new(m_barrier->allocate_child()) ReportTask(*this);
      tbb::task::spawn(*t);
    }
  }

  tbb::task* m_barrier;
  QueueType m_messageQueue;
};

#endif // MESSAGESVC_TBBMESSAGESVC_H
