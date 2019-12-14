/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
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
