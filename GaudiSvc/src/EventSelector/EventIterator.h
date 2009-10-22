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
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IDataStreamTool.h"

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
class EvtSelectorContext  : public IEvtSelector::Context {
  // Friend declaration
  friend class EventSelector;
  friend class EventCollectionSelector;
private:
  /// Copy constructor
  EvtSelectorContext ( const EvtSelectorContext& copy)
  : IEvtSelector::Context(copy),
    m_count(copy.m_count),
    m_strCount(copy.m_strCount),
    m_pSelector(copy.m_pSelector), 
    m_context(copy.m_context),
    m_pAddress(copy.m_pAddress)
  {
  }
protected:
  /// Stream identifier
  IDataStreamTool::size_type m_streamID;
  /// Event counter
  long                       m_count;
  /// Event counter within stream
  long                       m_strCount;
  /// Pointer to event selector
  const IEvtSelector*     m_pSelector;
  /// Pointer to "real" iterator
  IEvtSelector::Context*  m_context;
  /// Pointer to opaque address
  IOpaqueAddress*         m_pAddress;
  /// Set the address of the iterator
  void set(const IEvtSelector* sel, IDataStreamTool::size_type id, IEvtSelector::Context* it, IOpaqueAddress* pA)    {
    m_pSelector = sel;
    m_context   = it;
    m_streamID  = id;
    m_pAddress  = pA;
    m_strCount  = -1;
  }
  /// Set the address of the iterator
  void set(IEvtSelector::Context* it, IOpaqueAddress* pA)    {
    m_context = it;
    m_pAddress = pA;
  }
  /// Set the address of the iterator
  void set(IOpaqueAddress* pA)    {
    m_pAddress = pA;
  }
  /// Access "real" iterator
  IEvtSelector::Context* context()  const  {
    return m_context;
  }
  /// Increase counters
  IDataStreamTool::size_type increaseCounters(bool reset=false)   {
    m_count++;
    m_strCount = (reset) ? 0 : m_strCount+1;
    return m_count;
  }
  /// Decrease counters
  IDataStreamTool::size_type decreaseCounters(bool reset=false)   {
    m_count++;
    m_strCount = (reset) ? 0 : m_strCount-1;
    return m_count;
  }
public:
  /// Standard constructor
  EvtSelectorContext( const IEvtSelector* selector )
  : m_streamID(-1),
    m_count(-1),
    m_strCount(-1),
    m_pSelector(selector),
    m_context(0),
    m_pAddress(0)
  {
  }
  /// Standard Destructor
  virtual ~EvtSelectorContext() { 
  }
  /// Copy constructor
  virtual EvtSelectorContext& operator=(const EvtSelectorContext& copy)   {
    m_streamID  = copy.m_streamID;
    m_count     = copy.m_count;
    m_strCount  = copy.m_strCount;
    m_pSelector = copy.m_pSelector;
    m_context   = copy.m_context;
    m_pAddress  = copy.m_pAddress;
    return *this;
  }
  /// Stream identifier
  virtual IDataStreamTool::size_type ID()   const   {
    return m_streamID;
  }
  /// Access counter
  long numEvent()  const    {
    return m_count;
  }
  /// Access counter within stream
  long numStreamEvent()  const    {
    return m_strCount;
  }
  void* identifier() const  {
    return (void*)m_pSelector;
  }
};
#endif // GAUDISVC_EVENTSELECTOR_EVENTITERATOR_H
