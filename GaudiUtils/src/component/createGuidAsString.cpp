
// External include(s):
#include <boost/format.hpp>
#include <uuid/uuid.h>

// Local include(s):
#include "createGuidAsString.h"

std::string Gaudi::createGuidAsString()
{
  uuid_t uuid;
  ::uuid_generate_time( uuid );
  struct Guid {
    unsigned int Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
  }* g = (Guid*)&uuid;

  boost::format text( "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X" );
  text % g->Data1 % g->Data2 % g->Data3;
  for ( int i = 0; i < 8; ++i ) text % (unsigned short)g->Data4[i];
  return text.str();
}
