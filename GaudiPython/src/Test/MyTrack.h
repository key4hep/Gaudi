#ifndef GAUDIEXAMPLES_MYTRACK_H
#define GAUDIEXAMPLES_MYTRACK_H

// Include files
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/KeyedContainer.h"
#include "GaudiKernel/SmartRef.h"
#include "GaudiKernel/StreamBuffer.h"

#include "Event.h"

// External declaration
static const CLID& CLID_GPyTest_MyTrack = 9999;

namespace GPyTest
{

  class MyVertex;

  /** Simple class that represents a track for testing purposes

      @author Markus Frank
      @author Pere Mato
  */
  class MyTrack : public KeyedObject<int>
  {
  private:
    /// The track momentum
    float m_px, m_py, m_pz;
    /// Link to Top level event
    SmartRef<Event> m_event;
    SmartRef<MyVertex> m_decay;

  public:
    /// Standard constructor
    MyTrack() : m_px( 0.0 ), m_py( 0.0 ), m_pz( 0.0 ) {}
    /// Standard constructor
    MyTrack( float x, float y, float z ) : m_px( x ), m_py( y ), m_pz( z ) {}
    /// Standard Destructor
    virtual ~MyTrack() {}
    /// Retrieve pointer to class definition structure
    const CLID& clID() const override { return classID(); }
    static const CLID& classID() { return CLID_GPyTest_MyTrack; }
    /// Accessors: Retrieve x-component of the track momentum
    float px() const { return m_px; }
    /// Accessors: Retrieve y-component of the track momentum
    float py() const { return m_py; }
    /// Accessors: Retrieve z-component of the track momentum
    float pz() const { return m_pz; }
    /// Accessors: Update x-component of the track momentum
    void setPx( float px ) { m_px = px; }
    /// Accessors: Update y-component of the track momentum
    void setPy( float py ) { m_py = py; }
    /// Accessors: Update z-component of the track momentum
    void setPz( float pz ) { m_pz = pz; }

    /// Access to the source track object (constant case)
    const Event* event() const { return m_event; }
    /// Access to event object
    void setEvent( Event* evt ) { m_event = evt; }

    /// Access to the source track object (constant case)
    const MyVertex* decayVertex() const { return m_decay; }

    /// Access to event object
    void setDecayVertex( MyVertex* decay ) { m_decay = decay; }

    std::ostream& fillStream( std::ostream& s ) const override
    {
      s << "px: " << px() << "py: " << py() << "px: " << pz();
      return s;
    }
  };
} // namespace GPyTest

#include "MyVertex.h"

// Definition of all container types of MCParticle
typedef KeyedContainer<GPyTest::MyTrack> MyTrackVector;

#endif // RIO_EXAMPLE1_MYTRACK_H
