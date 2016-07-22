#ifndef GAUDIKERNEL_EVENTIDRANGE_H
#define GAUDIKERNEL_EVENTIDRANGE_H 1

/** **************************************************************************
 *
 *  @file EventIDRange.h
 *  @brief Event Range object. Holds two EventIDBase instances (start and stop)
 *
 *  @author Charles Leggett
 *
 *****************************************************************************/

#include "GaudiKernel/EventIDBase.h"
#include <iostream>
#include <string>
#include <sstream>

/**
 *  @class EventIDRange
 *  @brief Event ID Range object. Holds two EventIDBases (start and stop)
 */

class EventIDRange {
public:
  EventIDRange() {};
  EventIDRange( const EventIDBase& start, const EventIDBase& stop );
  EventIDRange( const EventIDRange& r ):m_start(r.m_start),m_stop(r.m_stop) {};
  EventIDRange& operator= (const EventIDRange& r);

  EventIDBase start() const { return m_start; }
  EventIDBase stop() const { return m_stop; }

  bool isInRange(const EventIDBase& t) const {
    return ( t>=m_start && t<m_stop );
  }

  static EventIDRange intersect(const EventIDRange& it) { return it; }
  template <typename ... T>
  static EventIDRange intersect(const EventIDRange& first, const T& ... rest) {
    EventIDRange r = intersect(rest...);
    
    EventIDBase i1 = std::max(first.start(), r.start());
    EventIDBase i2 = std::min(first.stop(), r.stop());
    
    return EventIDRange(i1,i2);
  }

  friend bool operator==(const EventIDRange& lhs, const EventIDRange& rhs);
  friend bool operator!=(const EventIDRange& lhs, const EventIDRange& rhs);

  friend std::ostream& operator<<(std::ostream& os, const EventIDRange& rhs);

  operator std::string() const;

private:

  EventIDBase m_start {};
  EventIDBase m_stop  {};

};

inline bool operator==(const EventIDRange& lhs, const EventIDRange& rhs) {
  return lhs.m_start==rhs.m_start && 
    lhs.m_stop==rhs.m_stop ;
}

inline bool operator!=(const EventIDRange& lhs, const EventIDRange& rhs) {
  return ! (lhs == rhs);
}

inline EventIDRange::operator std::string () const {
  std::ostringstream os;
  os << "{" << m_start << " - " << m_stop << "}";
  return os.str();
}

#endif
