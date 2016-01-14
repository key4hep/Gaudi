#ifndef GAUDIKERNEL_EVENTCONTEXT_H
#define GAUDIKERNEL_EVENTCONTEXT_H

#include <iostream>
#include <unistd.h>

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
  typedef long int ContextEvt_t;

  static const ContextID_t  INVALID_CONTEXT_ID  = 99999;
  static const ContextEvt_t INVALID_CONTEXT_EVT = -1;


  EventContext():
    m_evt_num(INVALID_CONTEXT_EVT),
    m_evt_slot(INVALID_CONTEXT_ID),    
    m_valid(false),
    m_evt_failed(false){};
  EventContext(const ContextEvt_t& e, const ContextID_t& s=INVALID_CONTEXT_ID)
    :m_evt_num(e), m_evt_slot(s), m_evt_failed(false){
    m_valid = (e<0 || s == INVALID_CONTEXT_ID) ? false: true;
  }

  ContextEvt_t evt() const { return m_evt_num; }
  ContextID_t slot() const { return m_evt_slot; }
  bool valid() const {return m_valid;}
  bool evtFail() const { return m_evt_failed; }

  void set(const ContextEvt_t& e=0, const ContextID_t& s=INVALID_CONTEXT_ID, 
           const bool f=false) {
    m_valid = (e<0 || s == INVALID_CONTEXT_ID) ? false : true;
    m_evt_num = e;
    m_evt_slot = s;
    m_evt_failed = f;
  }

  void setEvt(const ContextEvt_t& e) {
    if (e < 0) setValid(false); 
    m_evt_num = e;
  }

  void setSlot(const ContextID_t& s) {
    if ( s == INVALID_CONTEXT_ID ) setValid(false); 
    m_evt_slot = s;
  }

  void setFail(const bool& b=true) {
    m_evt_failed = b;
  }

  void setValid(const bool& b=true) {
    m_valid = b;
    if (!m_valid) {
      m_evt_slot = INVALID_CONTEXT_ID;
      m_evt_num =  INVALID_CONTEXT_EVT;
    }
  }

  EventContext& operator=(const EventContext& c) {
    m_evt_num = c.m_evt_num;
    m_evt_slot = c.m_evt_slot;
    m_valid = c.m_valid;
    m_evt_failed = c.m_evt_failed;
    return *this;
  }

private:
  ContextEvt_t m_evt_num;
  ContextID_t   m_evt_slot;
  bool m_valid;
  bool m_evt_failed;
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
