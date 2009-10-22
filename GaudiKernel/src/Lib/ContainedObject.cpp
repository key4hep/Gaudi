#include "GaudiKernel/ContainedObject.h"

ContainedObject::~ContainedObject() {
  // If the object is contained in a container, remove it from there
  if( 0 != m_parent ) {
    m_parent->remove(this);
  }
}
