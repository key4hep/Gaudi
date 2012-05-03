// $Id: MyTrack.h,v 1.4 2006/01/26 18:11:06 mato Exp $
#ifndef GAUDIEXAMPLES_MYTRACK_H
#define GAUDIEXAMPLES_MYTRACK_H

// Include files
#include "GaudiKernel/SmartRef.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/KeyedContainer.h"

#include "Event.h"

class MyVertex;


// External declaration
static const CLID& CLID_MyTrack = 9999;


/** @class MyTack

    Simple class that represents a track for testing purposes

    @author Markus Frank
    @author Pere Mato
*/
class MyTrack : public KeyedObject<int>   {
private:
  /// The track momentum
  float                    m_px, m_py, m_pz;
  /// Link to Top level event
  SmartRef<Event>          m_event;
  SmartRef<MyVertex>       m_decay;
public:
  /// Standard constructor
  MyTrack()   : m_px(0.0), m_py(0.0), m_pz(0.0)  {
  }
  /// Standard constructor
  MyTrack(float x, float y, float z)   : m_px(x), m_py(y), m_pz(z)  {
  }
  /// Standard Destructor
  virtual ~MyTrack()  {
  }
	/// Retrieve pointer to class definition structure
	virtual const CLID& clID() const    { return classID(); }
	static const CLID& classID()        { return CLID_MyTrack; }
  /// Accessors: Retrieve x-component of the track momentum
  float px()  const         { return m_px;  }
  /// Accessors: Retrieve y-component of the track momentum
  float py()  const         { return m_py;  }
  /// Accessors: Retrieve z-component of the track momentum
  float pz()  const         { return m_pz;  }
  /// Accessors: Update x-component of the track momentum
  void setPx(float px)      { m_px = px;    }
  /// Accessors: Update y-component of the track momentum
  void setPy(float py)      { m_py = py;    }
  /// Accessors: Update z-component of the track momentum
  void setPz(float pz)      { m_pz = pz;    }

  /// Access to the source track object (constant case)
  const Event* event()  const  {
    return m_event;
  }
  /// Access to event object
  void setEvent(Event* evt)    {
    m_event = evt;
  }

  /// Access to the source track object (constant case)
  const MyVertex* decayVertex()  const;

  /// Access to event object
  void setDecayVertex(MyVertex* decay);

  std::ostream& fillStream( std::ostream& s ) const {
    s << "px: " << px() << "py: " << py() << "px: " << pz();
    return s;
  }


};

#include "MyVertex.h"
/// Access to the source track object (constant case)
inline const MyVertex* MyTrack::decayVertex()  const  {
  return m_decay;
}

/// Access to event object
inline void MyTrack::setDecayVertex(MyVertex* decay)    {
  m_decay = decay;
}

// Definition of all container types of MCParticle
typedef KeyedContainer<MyTrack> MyTrackVector;

#endif // RIO_EXAMPLE1_MYTRACK_H
