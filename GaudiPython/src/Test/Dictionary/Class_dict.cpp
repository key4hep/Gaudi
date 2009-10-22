// $Id: Class_dict.cpp,v 1.8 2006/02/06 13:15:15 hmd Exp $
#include <iostream>
#include <string>

#define private public
#include "../Event.h"
#include "../MyTrack.h"
#include "../MyVertex.h"
#undef private

#include "GaudiKernel/DataObject.h"

class C_dict {
public: C_dict();
};

void* Event_run( void* o ) {
  static long r;
  r = ((Event*)o)->run();
  return &r;
}
void  Event_setRun( void* o, std::vector<void*> argv) {
  ((Event*)o)->setRun( *(long*)(argv[0]) );
  return;
}

void* MyTrack_event( void* o ) {
  return const_cast<Event*>(((MyTrack*)o)->event());
}
void  MyTrack_setEvent( void* o, std::vector<void*> argv) {
  ((MyTrack*)o)->setEvent( (Event*)(argv[0]) );
  return;
}

static C_dict instance;

C_dict::C_dict()
{
        MetaClass* meta1 = new MetaClass("Event", "Event header");
  meta1->addField("event", "long", "Event Number", OffsetOf(Event,m_event));
  meta1->addField("run",   "long", "Run Number",  OffsetOf(Event,m_run));
  meta1->addField("time",  "Time", "Event creation Time", OffsetOf(Event,m_time));
  meta1->addMethod("getRun",    "Get run number", "long", Event_run );
  std::vector<std::string> argnames1;
  argnames1.push_back("long");
  meta1->addMethod("setRun", "Set run number", argnames1, Event_setRun );

 	MetaClass* meta2 = new MetaClass("Time", "Absolute time (64-bit) in ns");
  meta2->addField("time", "longlong", "Time",OffsetOf(Gaudi::Time,m_time));

 	MetaClass* meta3 = new MetaClass("MyTrack", "Track test object");
  meta3->addField("px", "float", "Momentum in x direction", OffsetOf(MyTrack,m_px));
  meta3->addField("py", "float", "Momentum in y direction", OffsetOf(MyTrack,m_py));
  meta3->addField("pz", "float", "Momentum in z direction", OffsetOf(MyTrack,m_pz));
  meta3->addField("event", "SmartRef<Event>", "Link to event root",OffsetOf(MyTrack,m_event));
  meta3->addField("decay", "SmartRef<MyVertex>", "Link to decay Vertex", OffsetOf(MyTrack,m_decay));
  meta3->addMethod("getEvent", "Get Event pointer", "Event*", MyTrack_event );
  std::vector<std::string> argnames3;
  argnames3.push_back("Event*");
  meta3->addMethod("setEvent", "Set Event", argnames3, MyTrack_setEvent );

 	MetaClass* meta4 = new MetaClass("MyVertex", "Vertex test object");
  meta4->addField("x", "double", "Position in x", OffsetOf(MyVertex,m_x));
  meta4->addField("y", "double", "Position in y", OffsetOf(MyVertex,m_y));
  meta4->addField("z", "double", "Position in z", OffsetOf(MyVertex,m_z));
  meta4->addField("mother",    "SmartRef<MyTrack>", "Link to mother Track", OffsetOf(MyVertex,m_mother));
  meta4->addField("daughters", "SmartRefVector<MyTrack>", "Link to daughter Tracks", OffsetOf(MyVertex,m_daughters));
}

