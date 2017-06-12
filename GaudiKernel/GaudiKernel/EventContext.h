#ifndef GAUDIKERNEL_EVENTCONTEXT_H
#define GAUDIKERNEL_EVENTCONTEXT_H 1

#include <iostream>
#include <unistd.h>
#include <limits>
#include "GaudiKernel/EventIDBase.h"
#include <boost/any.hpp>

/** @class EventContext EventContext.h GaudiKernel/EventContext.h
 *
 * This class represents an entry point to all the event specific data.
 * It is needed to make the algorithm "aware" of the event it is operating on.
 * This was not needed in the serial version of Gaudi where the assumption
 * of 1-event-at-the-time processing was implicit.
 *
 * This class has nothing to do with the AlgContextSvc or the Context of
 * the EvtSelector!
 *
 * @author Danilo Piparo, Charles Leggett
 * @date 2012
 **/

class EventContext{
public:
  typedef size_t ContextID_t;
  typedef size_t ContextEvt_t;

  static const ContextID_t  INVALID_CONTEXT_ID  = std::numeric_limits<ContextID_t>::max();
  static const ContextEvt_t INVALID_CONTEXT_EVT = std::numeric_limits<ContextEvt_t>::max();


  EventContext() {};
  EventContext(const ContextEvt_t& e, const ContextID_t& s=INVALID_CONTEXT_ID)
    :m_evt_num(e), m_evt_slot(s) {
    m_valid = (e == INVALID_CONTEXT_EVT || s == INVALID_CONTEXT_ID) ? false: true;
  }

  ContextEvt_t evt() const { return m_evt_num; }
  ContextID_t slot() const { return m_evt_slot; }
  bool valid() const {return m_valid;}
  const EventIDBase& eventID() const { return m_eid; }

  void set(const ContextEvt_t& e=0, const ContextID_t& s=INVALID_CONTEXT_ID) {
    m_valid = (e == INVALID_CONTEXT_EVT || s == INVALID_CONTEXT_ID) ? false : true;
    m_evt_num = e;
    m_evt_slot = s;
  }

  void setEvt(const ContextEvt_t& e) {
    if (e == INVALID_CONTEXT_EVT) setValid(false); 
    m_evt_num = e;
  }

  void setSlot(const ContextID_t& s) {
    if ( s == INVALID_CONTEXT_ID ) setValid(false); 
    m_evt_slot = s;
  }

  void setValid(const bool& b=true) {
    m_valid = b;
    if (!m_valid) {
      m_evt_slot = INVALID_CONTEXT_ID;
      m_evt_num =  INVALID_CONTEXT_EVT;
    }
  }

  void setEventID(const EventIDBase& e) {
    m_eid = e;
  }

  template <typename T>
  void setExtension(const T& t) {
    m_extension = t;
  }

  template <typename T>
  T* getExtension() {
    return boost::any_cast<T>(&m_extension);
  }

  template <typename T>
  const T* getExtension() const {
    return boost::any_cast<T>(&m_extension);
  }

  const std::type_info& getExtensionType() const {
    return m_extension.type();
  }


private:
  ContextEvt_t m_evt_num  {INVALID_CONTEXT_EVT};
  ContextID_t  m_evt_slot {INVALID_CONTEXT_ID};
  bool m_valid {false};

  boost::any m_extension;

  EventIDBase m_eid {};
};

inline std::ostream& operator<<( std::ostream& os, const EventContext& ctx ) {
  if (ctx.valid()) {
    return os << "s: " << ctx.slot() << "  e: " << ctx.evt();
  } else {
    return os << "INVALID";
  }
}

inline std::ostream& operator<< (std::ostream& os, const EventContext* c) {
  if (c != 0) {
    return os << *c;
  } else {
    return os << "INVALID";
  }
}



#endif //GAUDIKERNEL_EVENTCONTEXT_H
