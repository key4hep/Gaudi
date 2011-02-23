// $Id: MyVertex.h,v 1.4 2006/01/26 18:11:06 mato Exp $
#ifndef GAUDIEXAMPLES_MYVERTEX_H
#define GAUDIEXAMPLES_MYVERTEX_H

// Include files
#include "GaudiKernel/SmartRef.h"
#include "GaudiKernel/SmartRefVector.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/KeyedContainer.h"

// Forward declaration
class MyTrack;

// External declaration
static const CLID& CLID_MyVertex = 9998;

/** @class MyVertex

    Simple class that represents a vertex for testing purposes

    @author Pere Mato
*/
class MyVertex : public KeyedObject<int>   {
private:
  /// The track momentum
  double    m_x, m_y, m_z;
  /// Link to Top level event
  SmartRef<MyTrack> m_mother;
  SmartRefVector<MyTrack> m_daughters;
public:
  /// Standard constructor
  MyVertex()   : m_x(0.0), m_y(0.0), m_z(0.0)  {
  }
  /// Standard constructor
  MyVertex(double x, double y, double z)   : m_x(x), m_y(y), m_z(z)  {
  }
  /// Standard Destructor
  virtual ~MyVertex()  {
  }
	/// Retrieve pointer to class definition structure
	virtual const CLID& clID() const    { return MyVertex::classID(); }
	static const CLID& classID()        { return CLID_MyVertex; }
  /// Accessors: Retrieve x-position
  double x()  const         { return m_x;  }
  /// Accessors: Retrieve y-position
  double y()  const         { return m_y;  }
  /// Accessors: Retrieve z-position
  double z()  const         { return m_z;  }
  /// Accessors: Update x-position
  void setX(double x)       { m_x = x;    }
  /// Accessors: Update y-position
  void setY(double y)       { m_y = y;    }
  /// Accessors: Update z-position
  void setZ(double z)       { m_z = z;    }

    /// Retrieve pointer to mother particle (const or non-const)
  const MyTrack* motherTrack() const { return m_mother; }
        MyTrack* motherTrack() { return m_mother; }
  /// Update pointer to mother particle (by a C++ pointer or a smart reference)
  void setMotherTrack( MyTrack* value ) { m_mother = value; }
  //void setMotherTrack( SmartRef<MyTrack> value );

  /// Retrieve pointer to vector of daughter particles (const or non-const)
  //const SmartRefVector<MyTrack>& daughterTracks() const;
  //      SmartRefVector<MyTrack>& daughterTracks();
  /// Update all daughter particles
  //void setDaughterTracks( const SmartRefVector<MyTrack>& value );
  /// Remove all daughter particles
  //void removeDaughterTracks();
  /// Add single daughter particle to vector of daughter particles
  ///   (by a C++ pointer or a smart reference)
  void addDaughterTrack( MyTrack* value ) { m_daughters.push_back(value); }
  //void addDaughterTrack( SmartRef<MyTrack> value );

  /// Input streamer
  virtual StreamBuffer& serialize(StreamBuffer& s);

  /// Output streamer
  virtual StreamBuffer& serialize(StreamBuffer& s)  const;
};

#include "MyTrack.h"

/// Input streamer
inline StreamBuffer& MyVertex::serialize(StreamBuffer& s)    {
  return s >> m_x >> m_y >> m_z >> m_mother(this) >> m_daughters(this);
}
/// Output streamer
inline StreamBuffer& MyVertex::serialize(StreamBuffer& s)  const   {
  return s << m_x << m_y << m_z << m_mother(this) << m_daughters(this);
}


// Definition of all container types of MyVertex
typedef KeyedContainer<MyVertex> MyVertexVector;

#endif // RIO_EXAMPLE1_MYTRACK_H
