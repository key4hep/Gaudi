#ifndef GAUDIKERNEL_EVENTIDBASE_H
#define GAUDIKERNEL_EVENTIDBASE_H 1

/**
 * @file EventIDBase.h
 *
 * @brief This class provides a unique identification for each event,
 * in terms of run/event number and/or a time stamp.
 *
 * @author RD Schaffer <R.D.Schaffer@cern.ch>
 * @author Paolo Calafiura <pcalafiura@lbl.gov>
 *
 */

//<<<<<< INCLUDES                                                       >>>>>>

#include <iostream>
#include <stdint.h>

/**
 * @class EventIDBase
 *
 * @brief This class provides a unique identification for each event,
 * in terms of run/event number and/or a time stamp.
 *
 */

class EventIDBase {
public:
  
  typedef unsigned int       number_type;
  
  /// \name structors
  //@{
  EventIDBase() {};
  EventIDBase(number_type run_number, 
	      uint64_t event_number,
	      number_type time_stamp=0,
	      number_type time_stamp_ns_offset=0,
	      number_type lumi_block=0,
	      number_type bunch_crossing_id=0);
  // Use default copy constructor.
  virtual ~EventIDBase();
  //@}
  
  /// run number - 32 bit unsigned
  number_type   run_number () const { return m_run_number; }
  
  /// event number - 64 bit unsigned
  uint64_t      event_number () const { return m_event_number; }
  
  /// time stamp - posix time in seconds from 1970, 32 bit unsigned
  number_type   time_stamp   () const { return m_time_stamp; }
  
  /// time stamp ns - ns time offset for time_stamp, 32 bit unsigned
  number_type   time_stamp_ns_offset () const { return m_time_stamp_ns_offset; }
  
  /// luminosity block identifier, 32 bit unsigned
  number_type   lumi_block           () const { return m_lumiBlock; }
  
  /// bunch crossing ID,  32 bit unsigned
  number_type   bunch_crossing_id    () const { return m_bunch_crossing_id; }
  
  /// set run number
  void set_run_number  (number_type runNumber) { m_run_number = runNumber; }
  
  /// set event number
  void set_event_number (uint64_t eventNumber) { m_event_number = eventNumber; }
  
  /// set time stamp
  void set_time_stamp   (number_type timeStamp) { m_time_stamp = timeStamp; }
  
  /// set time stamp in ns
  void set_time_stamp_ns_offset (number_type timeStampNs) {
    m_time_stamp_ns_offset = timeStampNs; 
  }
  
  /// set luminosity block identifier
  void set_lumi_block (number_type lumiBlock) { m_lumiBlock = lumiBlock; }
  
  /// set bunch crossing ID
  void set_bunch_crossing_id (number_type bcid) { m_bunch_crossing_id = bcid; }
  
  /// Comparison operators
  friend bool operator<(const EventIDBase& lhs, const EventIDBase& rhs);
  friend bool operator>(const EventIDBase& lhs, const EventIDBase& rhs);
  friend bool operator==(const EventIDBase& lhs, const EventIDBase& rhs);
  friend bool operator!=(const EventIDBase& lhs, const EventIDBase& rhs);
  friend bool operator<=(const EventIDBase& lhs, const EventIDBase& rhs);
  friend bool operator>=(const EventIDBase& lhs, const EventIDBase& rhs);
  
  /// Extraction operators
  friend std::ostream& operator<<(std::ostream& os, const EventIDBase& rhs);
  
private:
  
  /// run number
  number_type   m_run_number {0};
  
  /// event number
  uint64_t      m_event_number {0};
  
  /// posix time in seconds since 1970/01/01
  number_type   m_time_stamp {0};
  
  /// time stamp ns - ns time offset for time_stamp, 32 bit unsigned
  number_type   m_time_stamp_ns_offset {0}; 
  
  /// luminosity block number: 
  /// the number which uniquely tags a luminosity block within a run
  number_type   m_lumiBlock {0};
  
  /// bunch crossing ID,  32 bit unsigned
  number_type   m_bunch_crossing_id {0};
  
};


inline bool operator<(const EventIDBase& lhs, const EventIDBase& rhs) {
  // We are assuming that ALL events will have run and event numbers,
  // and never just a time stamp.
  // FIXME: any use for also ordering by lumi-block ?
  return lhs.m_run_number<rhs.m_run_number ||
         ( lhs.m_run_number==rhs.m_run_number && 
           lhs.m_event_number<rhs.m_event_number) ;
}

inline bool operator==(const EventIDBase& lhs, const EventIDBase& rhs) {
    // We assume that equality via run/event numbers is sufficient
  return lhs.m_run_number   == rhs.m_run_number && 
         lhs.m_event_number == rhs.m_event_number;
}
inline bool operator>(const EventIDBase& lhs, const EventIDBase& rhs) {
  return !( (lhs < rhs) || (lhs == rhs));
}
inline bool operator!=(const EventIDBase& lhs, const EventIDBase& rhs) {
  return !(lhs == rhs);
}
inline bool operator<=(const EventIDBase& lhs, const EventIDBase& rhs) {
  return !(lhs > rhs);
}
inline bool operator>=(const EventIDBase& lhs, const EventIDBase& rhs) {
  return !(lhs < rhs);
}

inline std::ostream& operator<<(std::ostream& os, const EventIDBase& rhs) {
  os << "[" 
     << rhs.m_run_number 
     << "," << rhs.m_event_number;
  
  if ( rhs.m_time_stamp != 0 ) {
    os << "," << rhs.m_time_stamp << ":" << rhs.m_time_stamp_ns_offset;
  }

  if ( rhs.m_lumiBlock != 0) {
    os << ",l:" << rhs.m_lumiBlock;
  }

  if ( rhs.m_bunch_crossing_id != 0) {
    os << ",b:" << rhs.m_bunch_crossing_id;
  }
  os << "]";
  return os;
}


//<<<<<< INLINE MEMBER FUNCTIONS                                        >>>>>>

#endif // EVENTINFO_EVENTID_H






