/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/ObjectVector.h"
#include "GaudiKernel/SmartRef.h"

#include "GaudiExamples/Collision.h"
#include "GaudiExamples/Counter.h"
#include "GaudiExamples/Event.h"
#include "GaudiExamples/MyTrack.h"
#include "GaudiExamples/MyVertex.h"

// template ObjectVector<MyTrack>;
// template ObjectVector<MyVertex>;
// template std::vector<MyTrack*>;
// template std::vector<MyVertex*>;
// template std::vector<SmartRef<MyTrack> >;
// template std::vector<SmartRef<MyVertex> >;
// template KeyedContainer<MyTrack>;
// template KeyedContainer<MyVertex>;
// template KeyedObject<long>;
// template std::vector<KeyedObject<long int>* >;
struct POOLIOTestDict__Instantiations {
  ObjectVector<Gaudi::Examples::MyTrack>           i1;
  ObjectVector<Gaudi::Examples::MyVertex>          i2;
  std::vector<Gaudi::Examples::MyTrack*>           i3;
  std::vector<Gaudi::Examples::MyVertex*>          i4;
  std::vector<SmartRef<Gaudi::Examples::MyTrack>>  i5;
  std::vector<SmartRef<Gaudi::Examples::MyVertex>> i6;
  KeyedContainer<Gaudi::Examples::MyTrack>         i7;
  KeyedContainer<Gaudi::Examples::MyVertex>        i8;
  KeyedObject<int>                                 i9;
  std::vector<KeyedObject<int>*>                   i10;
  // KeyedObject<long> i11;
  // std::vector<KeyedObject<long int>* > i12;
};

#ifdef __ICC
// disable icc warning #191: type qualifier is meaningless on cast type
// ... a lot of noise produced by the dictionary
#  pragma warning( disable : 191 )
#endif
