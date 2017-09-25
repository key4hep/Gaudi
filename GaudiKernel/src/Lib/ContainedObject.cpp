#include "GaudiKernel/ContainedObject.h"

ContainedObject::~ContainedObject()
{
  // If the object is contained in a container, remove it from there
  if ( m_parent ) m_parent->remove( this );
}
