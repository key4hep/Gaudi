// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiExamples/src/Lib/Event.h,v 1.4 2007/09/28 11:48:17 marcocle Exp $
#ifndef GAUDIEXAMPLES_EVENT_H
#define GAUDIEXAMPLES_EVENT_H 1

// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/Time.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/SmartRefVector.h"
#include <iostream>



namespace Gaudi {
  namespace Examples {

    class Collision;

    // CLID definition
    static const CLID& CLID_Event = 110;

    /** @class Event

    Essential information of the event used in examples
    It can be identified by "/Event"


    @author Pavel Binko
    */

    class GAUDI_API Event : public DataObject {

    public:
      /// Constructors
      Event();
      /// Destructor
      virtual ~Event() { }

      /// Retrieve reference to class definition structure
      virtual const CLID& clID() const  { return classID(); }
      static const CLID& classID() { return CLID_Event; }

      /// Retrieve event number
      int event () const { return m_event; }
      /// Update event number
      void setEvent (int value) { m_event = value; }

      /// Retrieve run number
      int run () const { return m_run; }
      /// Update run number
      void setRun (int value) { m_run = value; }

      /// Retrieve reference to event time stamp
      const Gaudi::Time& time () const { return m_time; }
      /// Update reference to event time stamp
      void setTime (const Gaudi::Time& value) { m_time = value; }

      /// Access to collisions
      const SmartRefVector<Collision>& collisions() const;

      /// Add collision
      void addCollision(Collision* vtx);

      /// Remove collision
      void removeCollision(Collision* vtx);


      /// Serialize the object for writing
      virtual StreamBuffer& serialize( StreamBuffer& s ) const;
      /// Serialize the object for reading
      virtual StreamBuffer& serialize( StreamBuffer& s );

      /// Fill the output stream (ASCII)
      virtual std::ostream& fillStream( std::ostream& s ) const;

    private:
      /// Event number
      int                 m_event;
      /// Run number
      int                 m_run;
      /// Time stamp
      Gaudi::Time         m_time;

      /// Vector of collisions this object belongs to
      SmartRefVector<Collision> m_collisions;
    };

  }
}

#include "Collision.h"


namespace Gaudi {
  namespace Examples {

    //
    // Inline code must be outside the class definition
    //

    /// Serialize the object for writing
    inline StreamBuffer& Event::serialize( StreamBuffer& s ) const {
      return s << m_event << m_run << m_time << m_collisions(this);
    }


    /// Serialize the object for reading
    inline StreamBuffer& Event::serialize( StreamBuffer& s ) {
      return s >> m_event >> m_run >> m_time >> m_collisions(this);
    }


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

    /// Access to decay vertices
    inline const SmartRefVector<Collision>& Event::collisions() const
    {
      return m_collisions;
    }

    /// Add decay vertex
    inline void Event::addCollision(Collision* c)
    {
      m_collisions.push_back(SmartRef<Collision>(c));
    }

    /// Remove decay vertex
    inline void Event::removeCollision(Collision* c)
    {
      SmartRefVector<Collision>::iterator i;
      for(i=m_collisions.begin(); i != m_collisions.end(); ++i) {
	if ( i->target() == c ) {
	  m_collisions.erase(i);
	  return;
	}
      }
    }

    /// Output operator (ASCII)
    inline std::ostream& operator<< ( std::ostream& s, const Event& obj ) {
      return obj.fillStream(s);
    }


  }
}
#endif    // GAUDIEXAMPLES_EVENT_H
