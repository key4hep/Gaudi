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
//	============================================================
//
//	EventIterator.h
//	------------------------------------------------------------
//
//	Package   : EventSelector
//
//	Author    : Markus Frank
//
//	===========================================================
#ifndef GAUDISVC_EVENTSELECTOR_EVENTITERATOR_H
#define GAUDISVC_EVENTSELECTOR_EVENTITERATOR_H 1

// Include files
#include "GaudiKernel/IDataStreamTool.h"
#include "GaudiKernel/IEvtSelector.h"

/** Definition of class EventIterator

    Generic iterator to access event on the persistent store.
    The iterator interacts with the event selection service.

    <B>History:</B>
    +---------+----------------------------------------------+---------+
    |    Date |                 Comment                      | Who     |
    +---------+----------------------------------------------+---------+
    | 3/10/00 | Initial version                              | M.Frank |
    +---------+----------------------------------------------+---------+
   @author Markus Frank
   @version 1.0
*/
class EvtSelectorContext : public IEvtSelector::Context {
  // Friend declaration
  friend class EventSelector;
  friend class EventCollectionSelector;

private:
  /// Copy constructor
  EvtSelectorContext( const EvtSelectorContext& ) = default;

protected:
  /// Stream identifier
  IDataStreamTool::size_type m_streamID = -1;
  /// Event counter
  long m_count = -1;
  /// Event counter within stream
  long m_strCount = -1;
  /// Pointer to event selector
  const IEvtSelector* m_pSelector = nullptr;
  /// Pointer to "real" iterator
  IEvtSelector::Context* m_context = nullptr;
  /// Pointer to opaque address
  IOpaqueAddress* m_pAddress = nullptr;
  /// Set the address of the iterator
  void set( const IEvtSelector* sel, IDataStreamTool::size_type id, IEvtSelector::Context* it, IOpaqueAddress* pA ) {
    m_pSelector = sel;
    m_context   = it;
    m_streamID  = id;
    m_pAddress  = pA;
    m_strCount  = -1;
  }
  /// Set the address of the iterator
  void set( IEvtSelector::Context* it, IOpaqueAddress* pA ) {
    m_context  = it;
    m_pAddress = pA;
  }
  /// Set the address of the iterator
  void set( IOpaqueAddress* pA ) { m_pAddress = pA; }
  /// Access "real" iterator
  IEvtSelector::Context* context() const { return m_context; }
  /// Increase counters
  IDataStreamTool::size_type increaseCounters( bool reset = false ) {
    m_count++;
    m_strCount = ( reset ) ? 0 : m_strCount + 1;
    return m_count;
  }
  /// Decrease counters
  IDataStreamTool::size_type decreaseCounters( bool reset = false ) {
    m_count++;
    m_strCount = ( reset ) ? 0 : m_strCount - 1;
    return m_count;
  }

public:
  /// Standard constructor
  EvtSelectorContext( const IEvtSelector* selector ) : m_pSelector( selector ) {}

  /// Stream identifier
  virtual IDataStreamTool::size_type ID() const { return m_streamID; }
  /// Access counter
  long numEvent() const { return m_count; }
  /// Access counter within stream
  long  numStreamEvent() const { return m_strCount; }
  void* identifier() const override { return (void*)m_pSelector; }
};
#endif // GAUDISVC_EVENTSELECTOR_EVENTITERATOR_H
