#ifdef __ICC
// disable icc warning #191: type qualifier is meaningless on cast type
#pragma warning(disable:191)
#endif

#include "Event.h"
#include "MyTrack.h"
#include "MyVertex.h"

struct __Instantiations 
{
  KeyedContainer<MyTrack>  i0;
  KeyedContainer<MyVertex> i1;
};
