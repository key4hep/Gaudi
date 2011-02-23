#ifdef __ICC
// disable icc warning #654: overloaded virtual function "B::Y" is only partially overridden in class "C"
//   TODO: there is only a partial overload of IAuditor::before and IAuditor::after
#pragma warning(disable:654)
#endif

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AudFactory.h"
#include "GaudiKernel/IChronoStatSvc.h"

#include "GaudiKernel/Stat.h"

/// local
#include "ProcStats.h"
#include "MemStatAuditor.h"

DECLARE_AUDITOR_FACTORY(MemStatAuditor)

MemStatAuditor::MemStatAuditor(const std::string& name, ISvcLocator* pSvcLocator) :
  Auditor(name, pSvcLocator), m_vSize(-1.)
{
  declareProperty("CustomEventTypes",m_types);

  m_stat = serviceLocator()->service("ChronoStatSvc");
}

MemStatAuditor::~MemStatAuditor(){
}

void MemStatAuditor::beforeInitialize(INamedInterface*) {
  //DR not useful
  //   std::string theString = "Memory usage before ";
  //   theString += alg->name() ;
  //   theString += " \tInitialization Method";
  //   printinfo(theString, alg->name() );
}
void MemStatAuditor:: afterInitialize(INamedInterface* ini){
  std::string theString = "Memory usage has changed after  ";
  theString += ini->name() ;
  theString += " \tInitialization Method";
  printinfo(theString, ini->name() );
}
void MemStatAuditor::beforeReinitialize(INamedInterface*) {
  //DR not useful
  //   std::string theString = "Memory usage before ";
  //   theString += alg->name() ;
  //   theString += " \tReinitialization Method";
  //   printinfo(theString, alg->name() );
}
void MemStatAuditor:: afterReinitialize(INamedInterface* ini){
  std::string theString = "Memory usage has changed after  ";
  theString += ini->name() ;
  theString += " \tReinitialization Method";
  printinfo(theString, ini->name() );
}
void MemStatAuditor:: beforeExecute(INamedInterface*){
  //DR not useful
  //   std::string theString = "Memory usage has changed before ";
  //   theString += alg->name() ;
  //   theString += " \tBefExecute Method";
  //   printinfo(theString, alg->name() );
}
void MemStatAuditor:: afterExecute(INamedInterface* alg, const StatusCode& ) {
  std::string theString = "Memory usage has changed after  ";
  theString += alg->name() ;
  theString += " \tExecute Method";
  printinfo(theString, alg->name() );

}
void MemStatAuditor::beforeBeginRun(INamedInterface*) {
  //DR not useful
  //   std::string theString = "Memory usage before ";
  //   theString += alg->name() ;
  //   theString += " \tBeginRun Method";
  //   printinfo(theString, alg->name() );
}
void MemStatAuditor:: afterBeginRun(INamedInterface* ini){
  std::string theString = "Memory usage has changed after  ";
  theString += ini->name() ;
  theString += " \tBeginRun Method";
  printinfo(theString, ini->name() );
}
void MemStatAuditor::beforeEndRun(INamedInterface*) {
  //DR not useful
  //   std::string theString = "Memory usage before ";
  //   theString += alg->name() ;
  //   theString += " \tEndRun Method";
  //   printinfo(theString, alg->name() );
}
void MemStatAuditor:: afterEndRun(INamedInterface* ini){
  std::string theString = "Memory usage has changed after  ";
  theString += ini->name() ;
  theString += " \tEndRun Method";
  printinfo(theString, ini->name() );
}
void MemStatAuditor:: beforeFinalize(INamedInterface*) {
  //DR not useful
  //   std::string theString = "Memory usage has changed before ";
  //   theString += alg->name() ;
  //   theString += " \tFinalize Method";
  //   printinfo(theString, alg->name() );
}
void MemStatAuditor:: afterFinalize(INamedInterface*){
  //DR not useful
  //   std::string theString = "Memory usage has changed after  ";
  //   theString += alg->name() ;
  //   theString += " \tFinalize Method";
  //   printinfo(theString, alg->name() );

}

void
MemStatAuditor::before(CustomEventTypeRef evt, const std::string& caller) {

  if (m_types.value().size() != 0) {
    if ( (m_types.value())[0] == "none") {
      return;
    }

    if ( find(m_types.value().begin(), m_types.value().end(), evt) ==
	 m_types.value().end() ) {
      return;
    }
  }

  std::string theString = "Memory usage before ";
  theString += caller + " with auditor trigger " + evt;
  printinfo(theString, caller);

}

void
MemStatAuditor::after(CustomEventTypeRef evt, const std::string& caller, const StatusCode&) {

  if (m_types.value().size() != 0) {
    if ( (m_types.value())[0] == "none") {
      return;
    }

    if ( find(m_types.value().begin(), m_types.value().end(), evt) ==
	 m_types.value().end() ) {
      return;
    }
  }

  std::string theString = "Memory usage has changed after ";
  theString += caller + " with auditor trigger " + evt;
  printinfo(theString, caller);

}

StatusCode MemStatAuditor::sysFinalize( )
{
  return StatusCode::SUCCESS;
}

bool MemStatAuditor::printinfo(const std::string& theString, const std::string& tag )
{
  bool status(false);
  ProcStats* p = ProcStats::instance();
  procInfo info;
  ///cannot be eaxactly 0
  double deltaVSize =0.00001;


  if( p->fetch(info) == true)
    {
      MsgStream log(msgSvc(), name());

      if (m_vSize>0 && info.vsize >0 ){
	 deltaVSize=info.vsize-m_vSize;
      }

      // store the current VSize to be able to monitor the increment
      if (info.vsize>0) {
	 m_vSize=info.vsize;
      }

      log << MSG::INFO << theString <<
	" \tvirtual size = " << info.vsize << " MB"  <<
	" \tresident set size = " << info.rss << " MB" <<
	" deltaVsize = " << deltaVSize << "  MB " << endmsg;
      ///

      //      Stat stv( statSvc() , tag+":VMemUsage" , info.vsize );
      //   Stat str( statSvc() , tag+":RMemUsage" , info.rss   );
      ///
      status=true;
    }
  // fill the stat for every call, not just when there is a change
  // only monitor the increment in VSize
  Stat sts( statSvc() , tag+":VMem" , deltaVSize );


  ///
  return status; //FIXME
}








