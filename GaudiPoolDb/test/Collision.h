// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiPoolDb/test/Collision.h,v 1.7 2006/03/13 16:51:07 hmd Exp $
#ifndef GAUDIEXAMPLES_COLLISION_H
#define GAUDIEXAMPLES_COLLISION_H 1

// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/DataObject.h"

// CLID definition
static const CLID& CLID_Collision = 111;

/** @class Collision
*/

class Collision : public DataObject {
public:
  /// Constructors
  Collision() : DataObject(), m_collision(0) { }
  Collision(long num) : DataObject(), m_collision(num) { }
  /// Destructor
  virtual ~Collision() { }

  /// Retrieve reference to class definition structure
  virtual const CLID& clID() const  { return Collision::classID(); }
  static const CLID& classID() { return CLID_Collision; }

  void setCollision(long num) {
    m_collision = num;
  }
  long collision() const {
    return m_collision;
  }
  /// Serialize the object for writing
  virtual StreamBuffer& serialize( StreamBuffer& s ) const;
  /// Serialize the object for reading
  virtual StreamBuffer& serialize( StreamBuffer& s );

  std::vector<std::pair<int,int> > m_mcTags;

private:
  /// Collision number
  long                m_collision;
};

/// Serialize the object for writing
inline StreamBuffer& Collision::serialize( StreamBuffer& s ) const {
  DataObject::serialize(s);
  return s << m_collision;
}


/// Serialize the object for reading
inline StreamBuffer& Collision::serialize( StreamBuffer& s ) {
  DataObject::serialize(s);
  return s >> m_collision;
}

#endif    // GAUDIEXAMPLES_COLLISION_H
