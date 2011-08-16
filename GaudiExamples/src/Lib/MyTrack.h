// $Id: MyTrack.h,v 1.5 2008/10/10 08:06:32 marcocle Exp $
// ============================================================================
#ifndef GAUDIPOOLDB_TEST_MYTRACK_H
#define GAUDIPOOLDB_TEST_MYTRACK_H
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/SmartRefVector.h"
#include "GaudiKernel/KeyedContainer.h"
#include "GaudiKernel/ObjectVector.h"
#include "GaudiKernel/SharedObjectsContainer.h"
// =============================================================================
#include "Event.h"
// ============================================================================

namespace Gaudi
{
  namespace Examples
  {

    // Forward declarations
    class MyVertex ;
    template <class T> class GaudiObjectHandler;

    // CLID definition
    static const CLID& CLID_MyTrack = 355;

    /** @class MyTack

    Simple class that represents a track for testing purposes

    @author Markus Frank
    @author Pere Mato
    */
    //class MyTrack : public ContainedObject {
    class GAUDI_API MyTrack
#ifdef __PLAIN_GAUDI
      : public ContainedObject
#else
        : public KeyedObject<int>
#endif
    {
      friend class GaudiObjectHandler<MyTrack>;
    public:
      // ======================================================================
      /// the type of plain vector
      typedef std::vector<MyTrack*>                           Vector ;
      /// the type of vector of const-pointers
      typedef std::vector<const MyTrack*>                ConstVector ;
      /// the type of selection
      typedef SharedObjectsContainer<MyTrack>              Selection ;
#ifdef __PLAIN_GAUDI
      /// the actual type of container in TES
      typedef ObjectVector<MyTrack>                        Container ;
#else
      /// the actual type of container in TES
      typedef KeyedContainer<MyTrack, Containers::HashMap> Container ;
#endif
      // ======================================================================
    protected:
      /// The track momentum
      float                    m_px, m_py, m_pz;
      /// Link to Top level event
      SmartRef<Event>          m_event;
      /// Link to origin vertex
      SmartRef<MyVertex>       m_originVertex;
      /// Links to all decay vertices
      SmartRefVector<MyVertex> m_decayVertices;

    public:
      /// Standard constructor
      MyTrack();
      /// Standard constructor
      MyTrack(float x, float y, float z);
      /// Standard Destructor
      virtual ~MyTrack();
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
      /// Origin vertex
      const MyVertex* originVertex()  const;

      /// Set origin vertex
      void setOriginVertex(MyVertex* origin);

      /// Access to decay vertices
      const SmartRefVector<MyVertex>& decayVertices() const;

      /// Add decay vertex
      void addDecayVertex(MyVertex* vtx);

      /// Remove decay vertex
      void removeDecayVertex(MyVertex* vtx);

      /// Serialize the object for writing
      virtual StreamBuffer& serialize( StreamBuffer& s ) const;
      /// Serialize the object for reading
      virtual StreamBuffer& serialize( StreamBuffer& s );
    };

    // Definition of all container types of MCParticle
#ifdef __PLAIN_GAUDI
    typedef ObjectVector<MyTrack> MyTrackVector;
#else
    typedef KeyedContainer<MyTrack> MyTrackVector;
#endif

  }
}

#include "MyVertex.h"

namespace Gaudi {
  namespace Examples {

    /// Origin vertex
    inline const MyVertex* MyTrack::originVertex()  const
    {
      return m_originVertex;
    }

    /// Set origin vertex
    inline void MyTrack::setOriginVertex(MyVertex* origin)
    {
      m_originVertex = origin;
    }

    /// Access to decay vertices
    inline const SmartRefVector<MyVertex>& MyTrack::decayVertices() const
    {
      return m_decayVertices;
    }

    /// Add decay vertex
    inline void MyTrack::addDecayVertex(MyVertex* vtx)
    {
      m_decayVertices.push_back(SmartRef<MyVertex>(vtx));
    }

    /// Remove decay vertex
    inline void MyTrack::removeDecayVertex(MyVertex* vtx)
    {
      SmartRefVector<MyVertex>::iterator i;
      for(i=m_decayVertices.begin(); i != m_decayVertices.end(); ++i) {
	if ( i->target() == vtx ) {
	  m_decayVertices.erase(i);
	  return;
	}
      }
    }

    /// Serialize the object for writing
    inline StreamBuffer& MyTrack::serialize( StreamBuffer& s ) const {
#ifdef __PLAIN_GAUDI
      ContainedObject::serialize(s);
#else
      KeyedObject<int>::serialize(s);
#endif
      return s << m_event(this) << m_originVertex(this) << m_decayVertices(this) << m_px << m_py << m_pz;
    }


    /// Serialize the object for reading
    inline StreamBuffer& MyTrack::serialize( StreamBuffer& s ) {
#ifdef __PLAIN_GAUDI
      ContainedObject::serialize(s);
#else
      KeyedObject<int>::serialize(s);
#endif
      return s >> m_event(this) >> m_originVertex(this) >> m_decayVertices(this) >> m_px >> m_py >> m_pz;
    }

  }
}

#endif // GAUDIPOOLDB_TEST_MYTRACK_H
