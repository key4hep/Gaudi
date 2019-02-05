#define _inc_types_impl_
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/ThreadLocalContext.h"

Incident::Incident( const std::string& source, const std::string& type ) : m_source( source ), m_type( type ) {
  m_ctx = Gaudi::Hive::currentContext();
}
