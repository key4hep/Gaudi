// $Id: Event.h,v 1.5 2006/11/30 14:51:54 mato Exp $
#ifndef GAUDIEXAMPLES_EVENT_H
#define GAUDIEXAMPLES_EVENT_H 1

// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/Time.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/DataObject.h"
#include <iostream>
#include <iomanip>

/** @class Event

    Essential information of the event used in examples
    It can be identified by "/Event"


    @author Pavel Binko
*/

static const CLID CLID_Event = 110;

class Event : public DataObject {

public:
  /// Constructors
  Event() : DataObject() { }
  /// Destructor
  virtual ~Event() { }

  /// Retrieve reference to class definition structure
  virtual const CLID& clID() const  { return classID(); }
  static const CLID& classID() { return CLID_Event; }

  /// Retrieve event number
  long event () const { return m_event; }
  /// Update event number
  void setEvent (long value) { m_event = value; }

  /// Retrieve run number
  long run () const { return m_run; }
  /// Update run number
  void setRun (long value) { m_run = value; }

  /// Retrieve reference to event time stamp
  const Gaudi::Time& time () const { return m_time; }
  /// Update reference to event time stamp
  void setTime (const Gaudi::Time& value) { m_time = value; }

  /// Output operator (ASCII)
  friend std::ostream& operator<< ( std::ostream& s, const Event& obj ) {
    return obj.fillStream(s);
  }
  /// Fill the output stream (ASCII)
  virtual inline std::ostream& fillStream( std::ostream& s ) const;

private:
  /// Event number
  long                m_event;
  /// Run number
  long                m_run;
  /// Time stamp
  Gaudi::Time         m_time;
};

/// Fill the output stream (ASCII)
inline std::ostream& Event::fillStream( std::ostream& s ) const {
  return s
    << "class Event :"
    << "\n    Event number = "
    << std::setw(12)
    << m_event
    << "\n    Run number   = "
    << std::setw(12)
    << m_run
    << "\n    Time         = " << m_time;
}


#endif    // GAUDIEXAMPLES_EVENT_H
