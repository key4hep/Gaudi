#include "MyTrack.h"

namespace {
  struct InstanceCount {
    long count;
    InstanceCount() : count(0) {}
    ~InstanceCount() {
      std::cout << "Number of MyTrack instances:" << count << std::endl;
    }
  };
}
static InstanceCount s_instances;

/// Standard constructor
MyTrack::MyTrack()  {
  m_v.m_px = m_v.m_py = m_v.m_pz = 0.0;
  s_instances.count++;
  // std::cout << "  MTrack:" << (void*)this << " " << s_instances.count << std::endl;
}

/// Standard constructor
MyTrack::MyTrack(float x, float y, float z)  {
  m_v.m_px = x;
  m_v.m_py = y;
  m_v.m_pz = z;
  s_instances.count++;
//  std::cout << "  MTrack:" << (void*)this << " " << s_instances.count << std::endl;
}

/// Standard Destructor
MyTrack::~MyTrack()  {
  s_instances.count--;
//  std::cout << " ~MTrack:" << (void*)this << " " << s_instances.count << std::endl;
}

/// Serialize the object for writing
inline StreamBuffer& MyTrack::serialize( StreamBuffer& s ) const {
#ifdef __PLAIN_GAUDI
  ContainedObject::serialize(s);
#else
  KeyedObject<long>::serialize(s);
#endif
  return s << m_event(this) << m_originVertex(this) << m_decayVertices(this) << m_v.m_px << m_v.m_py << m_v.m_pz;
}


/// Serialize the object for reading
inline StreamBuffer& MyTrack::serialize( StreamBuffer& s ) {
#ifdef __PLAIN_GAUDI
  ContainedObject::serialize(s);
#else
  KeyedObject<long>::serialize(s);
#endif
  return s >> m_event(this) >> m_originVertex(this) >> m_decayVertices(this) >> m_v.m_px >> m_v.m_py >> m_v.m_pz;
}
