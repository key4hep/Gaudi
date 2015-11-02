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
  typedef size_t ID_type;

  EventContext():
    m_evt_num(-1),
    m_evt_slot(0),    
    m_valid(false),
    m_evt_failed(false){};
  EventContext(const long int& e, const ID_type& s=0):m_evt_num(e), 
						     m_evt_slot(s),
						     m_evt_failed(false){
    m_valid = (e<0) ? false: true;
  }

  long int evt() const { return m_evt_num; }
  ID_type slot() const { return m_evt_slot; }
  bool valid() const {return m_valid;}
  bool evtFail() const { return m_evt_failed; }

  void set(const long int& e=0, const ID_type& s=0, const bool f=false) {
    m_valid = (e<0) ? false : true;
    m_evt_num = e;
    m_evt_slot = s;
    m_evt_failed = f;
  }

  void setEvt(const long int& e) {
    m_valid = (e<0) ? false : true;
    m_evt_num = e;
  }

  void setFail(const bool& b=true) {
    m_evt_failed = b;
  }

  void setValid(const bool& b=true) {
    m_valid = b;
  }

  EventContext& operator=(const EventContext& c) {
    m_evt_num = c.m_evt_num;
    m_evt_slot = c.m_evt_slot;
    m_valid = c.m_valid;
    m_evt_failed = c.m_evt_failed;
    return *this;
  }

private:
  long int m_evt_num;
  ID_type   m_evt_slot;
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
