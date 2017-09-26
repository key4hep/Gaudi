/*
 * SerialTaskQueue.h
 *
 * @date   2012-10-21
 * @author Marco Clemencic
 */

#ifndef SERIALTASKQUEUE_H_
#define SERIALTASKQUEUE_H_

#include <atomic>
#include <memory>

#include <tbb/concurrent_queue.h>
#include <tbb/task.h>

namespace Gaudi
{

  /** Class for a generic serial queue of tasks (modeled on the Intel Threading
   *  Building Blocks Design Pattern "Local Serializer").
   *
   *  Users of SerialTaskQueue must define the tasks to be executed specializing
   *  the class SerialTaskQueue::WorkItem, implementing the method WorkItem::run().
   *
   *  Tasks are enqueued via the method SerialTaskQueue::add(WorkItem*).
   *
   *  When the instance goes out of scope (or is destructed), the thread blocks
   *  until all the enqueued tasks are completed.
   *
   *  @author Marco Clemencic
   */
  class SerialTaskQueue
  {
  public:
    /// Base class for the task to be executed by the serial queue.
    /// Actual tasks must specialize this class implementing the method run().
    class WorkItem
    {
    public:
      virtual ~WorkItem();
      /// Method to be implemented by the actual task classes.
      virtual void run() = 0;
    };

    /// Default constructor.
    SerialTaskQueue();

    /// Block until all the enqueued tasks are completed.
    virtual ~SerialTaskQueue();

    /// Enqueue a WorkItem for execution.
    void add( WorkItem* item );

    /// Method used by the tasks to trigger the execution of the next task in
    /// the queue.
    void noteCompletion();

    /// Block until all the currently enqueued tasks are completed.
    /// @note while a thread is waiting for the tasks to be completes, another
    ///       may still enqueue other tasks.
    void wait() const;

  private:
    /// Wrapper for the WorkItem class for internal concurrency bookkeeping.
    class SerialWorkItem
    {
    public:
      /// Initialize the instance from the WorkiItem and the SerialTaskQueue
      /// (for synchronization).
      /// @note the object takes ownership of the WorkItem pointer.
      SerialWorkItem( WorkItem* item, SerialTaskQueue* serializer ) : m_item( item ), m_serializer( serializer ) {}
      /// Execute the WorkItem and notify the SerialTaskQueue of the completion.
      void run();

    private:
      /// Pointer to the WorkItem to run.
      std::unique_ptr<WorkItem> m_item;
      /// Pointer to the SerialTaskQueue used for the synchronization.
      SerialTaskQueue* m_serializer;
    };

    /// Helper class to wrap a SerialWorkItem in a tbb::task.
    class SerialWorkItemRunner : public tbb::task
    {
    public:
      /// Initialize the instance.
      SerialWorkItemRunner( SerialWorkItem* item ) : m_item( item ) {}
      /// Call the run method of the work item.
      tbb::task* execute() override
      {
        m_item->run();
        return NULL;
      }

    private:
      /// Pointer to the work item to be executed.
      /// @note we do not own the work item (it deletes itself)
      SerialWorkItem* m_item;
    };

    void i_startNextItem();

    /// Counter of the currently running tasks.
    std::atomic<int> m_count;
    /// Queue of the tasks to be executed.
    tbb::concurrent_queue<SerialWorkItem*> m_queue;
  };

  inline void SerialTaskQueue::SerialWorkItem::run()
  {
    // run the wrapped task
    m_item->run();

    // We need to keep the pointer on the stack because we are going to delete
    // ourselves.
    SerialTaskQueue* serializer = m_serializer;

    // We call the delete before returning the control to the serialized so that
    // possible complex code in the task destructor is executed serially.
    delete this;

    // Notify the queue of the completion, so that it can schedule the next task.
    serializer->noteCompletion();
  }

} /* namespace Gaudi */
#endif /* SERIALTASKQUEUE_H_ */
