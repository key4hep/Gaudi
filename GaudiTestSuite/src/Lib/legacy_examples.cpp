/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiExamples/Counter.h>
#include <GaudiExamples/Event.h>
#include <GaudiExamples/MyTrack.h>
#include <GaudiExamples/MyVertex.h>

using namespace Gaudi::Examples;

Counter::Counter() : DataObject(), m_counter( 0 ) {}

Event::Event() : DataObject() {
  // m_collisions(this);
}

namespace {
  struct TrkInstanceCount {
    long count;
    TrkInstanceCount() : count( 0 ) {}
    ~TrkInstanceCount() {
      if ( count ) std::cout << "Number of MyTrack instances:" << count << std::endl;
    }
  };
} // namespace
static TrkInstanceCount s_trk_instances;

/// Standard constructor
MyTrack::MyTrack() : m_px( 0.0 ), m_py( 0.0 ), m_pz( 0.0 ) {
  // m_event(this);
  // m_originVertex(this);
  // m_decayVertices(this);
  s_trk_instances.count++;
}

/// Standard constructor
MyTrack::MyTrack( float x, float y, float z ) : m_px( x ), m_py( y ), m_pz( z ) {
  // m_event(this);
  // m_originVertex(this);
  // m_decayVertices(this);
  s_trk_instances.count++;
}

MyTrack::MyTrack( const MyTrack& t ) : KeyedObject<int>( t.key() ), m_px( t.m_px ), m_py( t.m_py ), m_pz( t.m_pz ) {
  s_trk_instances.count++;
}

/// Standard Destructor
MyTrack::~MyTrack() { s_trk_instances.count--; }

namespace {
  struct VtxInstanceCount {
    long count;
    VtxInstanceCount() : count( 0 ) {}
    ~VtxInstanceCount() {
      if ( count ) std::cout << "Number of MyVertex instances:" << count << std::endl;
    }
  };
} // namespace
static VtxInstanceCount s_vtx_instances;

/// Standard constructor
MyVertex::MyVertex() : m_x( 0.0 ), m_y( 0.0 ), m_z( 0.0 ) { s_vtx_instances.count++; }

/// Standard constructor
MyVertex::MyVertex( float x, float y, float z ) : m_x( x ), m_y( y ), m_z( z ) { s_vtx_instances.count++; }

/// Standard Destructor
MyVertex::~MyVertex() { s_vtx_instances.count--; }

/// Serialize the object for writing
inline StreamBuffer& MyVertex::serialize( StreamBuffer& s ) const {
  KeyedObject<int>::serialize( s );
  s << m_motherParticle( this );
  s << m_decayParticles( this );
  s << m_event( this );
  s << m_collisions( this );
  s << m_x << m_y << m_z;
  return s;
}

/// Serialize the object for reading
inline StreamBuffer& MyVertex::serialize( StreamBuffer& s ) {
  KeyedObject<int>::serialize( s );
  s >> m_motherParticle( this );
  s >> m_decayParticles( this );
  s >> m_event( this );
  s >> m_collisions( this );
  s >> m_x >> m_y >> m_z;
  return s;
}
