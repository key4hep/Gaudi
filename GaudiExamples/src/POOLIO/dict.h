#include "GaudiKernel/SmartRef.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/ObjectVector.h"

#include "Event.h"
#include "Collision.h"
#include "MyTrack.h"
#include "MyVertex.h"
#include "Counter.h"

//template ObjectVector<MyTrack>;
//template ObjectVector<MyVertex>;
//template std::vector<MyTrack*>;
//template std::vector<MyVertex*>;
//template std::vector<SmartRef<MyTrack> >;
//template std::vector<SmartRef<MyVertex> >;
//template KeyedContainer<MyTrack>;
//template KeyedContainer<MyVertex>;
//template KeyedObject<long>;
//template std::vector<KeyedObject<long int>* >;
struct __Instantiations 
{
  ObjectVector<Gaudi::Examples::MyTrack> i1;
  ObjectVector<Gaudi::Examples::MyVertex> i2;
  std::vector<Gaudi::Examples::MyTrack*> i3;
  std::vector<Gaudi::Examples::MyVertex*> i4;
  std::vector<SmartRef<Gaudi::Examples::MyTrack> > i5;
  std::vector<SmartRef<Gaudi::Examples::MyVertex> > i6;
  KeyedContainer<Gaudi::Examples::MyTrack> i7;
  KeyedContainer<Gaudi::Examples::MyVertex> i8;
  KeyedObject<int> i9;
  std::vector<KeyedObject< int>* > i10;
  //KeyedObject<long> i11;
  //std::vector<KeyedObject<long int>* > i12;
};
