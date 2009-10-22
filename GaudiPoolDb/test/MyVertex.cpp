#include "MyVertex.h"

namespace {
  struct InstanceCount {
    long count;
    InstanceCount() : count(0) {}
    ~InstanceCount() {
      std::cout << "Number of MyVertex instances:" << count << std::endl;
    }
  };
}
static InstanceCount s_instances;

/// Standard constructor
MyVertex::MyVertex()  {
  m_v.m_x = m_v.m_y = m_v.m_z = 0.0;
  s_instances.count++;
}

/// Standard constructor
MyVertex::MyVertex(float x, float y, float z) {
  m_v.m_x = x;
  m_v.m_y = y;
  m_v.m_z = z;
  s_instances.count++;
}

/// Standard Destructor
MyVertex::~MyVertex()  {
  s_instances.count--;
}

/// Serialize the object for writing
inline StreamBuffer& MyVertex::serialize( StreamBuffer& s ) const {
  KeyedObject<int>::serialize(s);
  s << m_motherParticle(this);
  s << m_decayParticles(this);
  s << m_event(this);
  s << m_collisions(this);
  s << m_v.m_x << m_v.m_y << m_v.m_z;
  return s;
}


/// Serialize the object for reading
inline StreamBuffer& MyVertex::serialize( StreamBuffer& s ) {
  KeyedObject<int>::serialize(s);
  s >> m_motherParticle(this);
  s >> m_decayParticles(this);
  s >> m_event(this);
  s >> m_collisions(this);
  s >> m_v.m_x >> m_v.m_y >> m_v.m_z;
  return s;
}
