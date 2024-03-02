/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiTestSuite/MyVertex.h"

using namespace Gaudi::TestSuite;

namespace {
  struct InstanceCount {
    long count;
    InstanceCount() : count( 0 ) {}
    ~InstanceCount() {
      if ( count ) std::cout << "Number of MyVertex instances:" << count << std::endl;
    }
  };
} // namespace
static InstanceCount s_instances;

/// Standard constructor
MyVertex::MyVertex() : m_x( 0.0 ), m_y( 0.0 ), m_z( 0.0 ) { s_instances.count++; }

/// Standard constructor
MyVertex::MyVertex( float x, float y, float z ) : m_x( x ), m_y( y ), m_z( z ) { s_instances.count++; }

/// Standard Destructor
MyVertex::~MyVertex() { s_instances.count--; }

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
