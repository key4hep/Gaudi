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
 * @author Charles Leggett
 *
 */

#include <iostream>
#include <stdint.h>
#include <tuple>
#include <iomanip>

/**
 * @class EventIDBase
 *
 * @brief This class provides a unique identification for each event,
 * in terms of run/event number and/or a time stamp.
 *
 */

class EventIDBase
{
public:
  typedef unsigned int number_type;
  typedef uint64_t event_number_t;

  static const number_type UNDEFNUM;
  static const event_number_t UNDEFEVT;

public:
  /// \name structors
  //@{
  EventIDBase(){};
  EventIDBase( number_type run_number, event_number_t event_number, number_type time_stamp = UNDEFNUM,
               number_type time_stamp_ns_offset = 0, number_type lumi_block = UNDEFNUM,
               number_type bunch_crossing_id = 0 );
  // Use default copy constructor.
  virtual ~EventIDBase();
  //@}

  /// run number - 32 bit unsigned
  number_type run_number() const { return m_run_number; }

  /// event number - 64 bit unsigned
  event_number_t event_number() const { return m_event_number; }

  /// time stamp - posix time in seconds from 1970, 32 bit unsigned
  number_type time_stamp() const { return m_time_stamp; }

  /// time stamp ns - ns time offset for time_stamp, 32 bit unsigned
  number_type time_stamp_ns_offset() const { return m_time_stamp_ns_offset; }

  /// luminosity block identifier, 32 bit unsigned
  number_type lumi_block() const { return m_lumi_block; }

  /// bunch crossing ID,  32 bit unsigned
  number_type bunch_crossing_id() const { return m_bunch_crossing_id; }

  /// set run number
  void set_run_number( number_type runNumber )
  {
    m_run_number = runNumber;
    if ( m_event_number != UNDEFEVT ) setRE();
    if ( m_lumi_block != UNDEFNUM ) setRL();
  }

  /// set event number
  void set_event_number( event_number_t eventNumber )
  {
    m_event_number = eventNumber;
    if ( m_run_number != UNDEFNUM ) setRE();
    if ( m_lumi_block != UNDEFNUM ) setLE();
  }

  /// set time stamp
  void set_time_stamp( number_type timeStamp )
  {
    m_time_stamp = timeStamp;
    setTS();
  }

  /// set time stamp in ns
  void set_time_stamp_ns_offset( number_type timeStampNs ) { m_time_stamp_ns_offset = timeStampNs; }

  /// set luminosity block identifier
  void set_lumi_block( number_type lumiBlock )
  {
    m_lumi_block = lumiBlock;
    if ( m_run_number != UNDEFNUM ) setRL();
    if ( m_event_number != UNDEFEVT ) setLE();
  }

  /// set bunch crossing ID
  void set_bunch_crossing_id( number_type bcid ) { m_bunch_crossing_id = bcid; }

  /// Comparison operators
  friend bool operator<( const EventIDBase& lhs, const EventIDBase& rhs );
  friend bool operator>( const EventIDBase& lhs, const EventIDBase& rhs );
  friend bool operator==( const EventIDBase& lhs, const EventIDBase& rhs );
  friend bool operator!=( const EventIDBase& lhs, const EventIDBase& rhs );
  friend bool operator<=( const EventIDBase& lhs, const EventIDBase& rhs );
  friend bool operator>=( const EventIDBase& lhs, const EventIDBase& rhs );

  bool isRunEvent() const;
  bool isTimeStamp() const;
  bool isLumiEvent() const;
  bool isRunLumi() const;
  bool isValid() const;

  /// Extraction operators
  friend std::ostream& operator<<( std::ostream& os, const EventIDBase& rhs );

  class SortByTimeStamp
  {
  public:
    bool operator()( const EventIDBase& t1, const EventIDBase& t2 ) const
    {
      if ( t1.time_stamp() == t2.time_stamp() ) {
        return t1.time_stamp_ns_offset() > t2.time_stamp_ns_offset();
      } else {
        return t1.time_stamp() > t2.time_stamp();
      }
    }
    bool operator()( const EventIDBase* t1, const EventIDBase* t2 ) const { return ( SortByTimeStamp()( *t1, *t2 ) ); }
  };

  class SortByRunEvent
  {
  public:
    bool operator()( const EventIDBase& t1, const EventIDBase& t2 ) const
    {
      if ( t1.run_number() == t2.run_number() ) {
        return t1.event_number() > t2.event_number();
      } else {
        return t1.run_number() > t2.run_number();
      }
    }
    bool operator()( const EventIDBase* t1, const EventIDBase* t2 ) const { return ( SortByRunEvent()( *t1, *t2 ) ); }
  };

  class SortByLumiEvent
  {
  public:
    bool operator()( const EventIDBase& t1, const EventIDBase& t2 ) const
    {
      if ( t1.lumi_block() == t2.lumi_block() ) {
        return t1.event_number() > t2.event_number();
      } else {
        return t1.lumi_block() > t2.lumi_block();
      }
    }
    bool operator()( const EventIDBase* t1, const EventIDBase* t2 ) const { return ( SortByLumiEvent()( *t1, *t2 ) ); }
  };

  class SortByRunLumi
  {
  public:
    bool operator()( const EventIDBase& t1, const EventIDBase& t2 ) const
    {
      if ( t1.run_number() == t2.run_number() ) {
        return t1.lumi_block() > t2.lumi_block();
      } else {
        return t1.run_number() > t2.run_number();
      }
    }
    bool operator()( const EventIDBase* t1, const EventIDBase* t2 ) const { return ( SortByRunLumi()( *t1, *t2 ) ); }
  };

private:
  enum Type { Invalid = 0, RunEvent = 1 << 1, TimeStamp = 1 << 2, LumiEvent = 1 << 3, RunLumi = 1 << 4 };

  unsigned m_type{Invalid};

  void setRE() { m_type = m_type | RunEvent; }
  void setTS() { m_type = m_type | TimeStamp; }
  void setLE() { m_type = m_type | LumiEvent; }
  void setRL() { m_type = m_type | RunLumi; }

  /// run number
  number_type m_run_number{UNDEFNUM};

  /// event number
  event_number_t m_event_number{UNDEFEVT};

  /// posix time in seconds since 1970/01/01
  number_type m_time_stamp{UNDEFNUM};

  /// time stamp ns - ns time offset for time_stamp, 32 bit unsigned
  number_type m_time_stamp_ns_offset{UNDEFNUM};

  /// luminosity block number:
  /// the number which uniquely tags a luminosity block within a run
  number_type m_lumi_block{UNDEFNUM};

  /// bunch crossing ID,  32 bit unsigned
  number_type m_bunch_crossing_id{UNDEFNUM};
};

inline bool operator<( const EventIDBase& lhs, const EventIDBase& rhs )
{
  // first try ordering by timestamp if both are non-zero
  // then try ordering by run/lumi/event
  // this assumes that both EventIDBase have the same set of values defined.

  if ( lhs.isTimeStamp() && rhs.isTimeStamp() ) {
    return ( lhs.m_time_stamp < rhs.m_time_stamp );
  } else {
    return ( std::tie( lhs.m_run_number, lhs.m_lumi_block, lhs.m_event_number ) <
             std::tie( rhs.m_run_number, rhs.m_lumi_block, rhs.m_event_number ) );
  }
}

inline bool operator>( const EventIDBase& lhs, const EventIDBase& rhs )
{
  // first try ordering by timestamp if both are non-zero
  // then try ordering by run/lumi/event
  // this assumes that both EventIDBase have the same set of values defined.

  if ( lhs.isTimeStamp() && rhs.isTimeStamp() ) {
    return ( lhs.m_time_stamp > rhs.m_time_stamp );
  } else {
    return ( std::tie( lhs.m_run_number, lhs.m_lumi_block, lhs.m_event_number ) >
             std::tie( rhs.m_run_number, rhs.m_lumi_block, rhs.m_event_number ) );
  }
}

inline bool operator==( const EventIDBase& lhs, const EventIDBase& rhs )
{
  // We assume that equality via run/event/lumi numbers is sufficient
  return ( lhs.m_run_number == rhs.m_run_number && lhs.m_event_number == rhs.m_event_number &&
           lhs.m_lumi_block == rhs.m_lumi_block );
}
inline bool operator!=( const EventIDBase& lhs, const EventIDBase& rhs ) { return !( lhs == rhs ); }
inline bool operator<=( const EventIDBase& lhs, const EventIDBase& rhs ) { return !( lhs > rhs ); }
inline bool operator>=( const EventIDBase& lhs, const EventIDBase& rhs ) { return !( lhs < rhs ); }

inline std::ostream& operator<<( std::ostream& os, const EventIDBase& rhs )
{
  if ( rhs.m_type == EventIDBase::Invalid ) {
    os << "[INVALID]";
    return os;
  }

  os << "[" << rhs.m_run_number;

  if ( rhs.m_event_number != EventIDBase::UNDEFEVT ) {
    os << "," << rhs.m_event_number;
  }
 
  if ( rhs.isTimeStamp() ) {
    os << ",t:" << rhs.m_time_stamp;
    if ( rhs.m_time_stamp_ns_offset != 0 ) {
      os << "." << std::setfill('0') << std::setw(9) 
         << rhs.m_time_stamp_ns_offset;
    }
  }

  if ( rhs.isLumiEvent() || rhs.isRunLumi() ) {
    os << ",l:" << rhs.m_lumi_block;
  }

  if ( rhs.m_bunch_crossing_id != 0 ) {
    os << ",b:" << rhs.m_bunch_crossing_id;
  }
  os << "]";
  return os;
}

#endif // EVENTINFO_EVENTID_H
