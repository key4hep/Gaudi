#include "GaudiKernel/SmartRef.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/ObjectVector.h"

#include "Event.h"
#include "Collision.h"
#include "MyTrack.h"
#include "MyVertex.h"

//template ObjectVector<MyTrack>;
//template ObjectVector<MyVertex>;

//template std::vector<KeyedObject<long int>* >;

struct __Instantiations  {
  ObjectVector<MyTrack> o1;
  ObjectVector<MyVertex> o2;
  std::vector<KeyedObject<long int>* > o3;
  std::vector<MyTrack*> o4;
  std::vector<MyVertex*> o5;
  std::vector<SmartRef<MyTrack> > o6;
  std::vector<SmartRef<MyVertex> > o7;
  KeyedContainer<MyTrack> o8;
  KeyedContainer<MyVertex> o9;
  KeyedObject<long> o10;
  std::pair<int,int> o11;
};

