/*
 * SerialTaskQueue.cpp
 *
 * @date   2012-10-21
 * @author Marco Clemencic
 */

#include "GaudiKernel/SerialTaskQueue.h"

namespace Gaudi {

  SerialTaskQueue::SerialTaskQueue() : m_count( 0 ) {}

  SerialTaskQueue::~SerialTaskQueue() { wait(); }

  SerialTaskQueue::WorkItem::~WorkItem() {}

  void SerialTaskQueue::add( WorkItem* item ) {
    m_queue.push( new SerialWorkItem( item, this ) );
    if ( ++m_count == 1 ) i_startNextItem();
  }

  void SerialTaskQueue::noteCompletion() {
    if ( --m_count != 0 ) i_startNextItem();
  }

  void SerialTaskQueue::wait() const {
    // wait until the queue is empty and all the tasks are completed
    while ( ( !m_queue.empty() ) || m_count ) {}
  }

  void SerialTaskQueue::i_startNextItem() {
    SerialWorkItem* item = 0;
    m_queue.try_pop( item );
    tbb::task::enqueue( *new ( tbb::task::allocate_root() ) SerialWorkItemRunner( item ) );
  }

} /* namespace Gaudi */
