// MemoryAuditor:
//  An auditor that monitors memory usage

#include "MemoryAuditor.h"
#include "ProcStats.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AudFactory.h"

DECLARE_AUDITOR_FACTORY(MemoryAuditor)

MemoryAuditor::MemoryAuditor(const std::string& name, ISvcLocator* pSvcLocator) :
  Auditor(name, pSvcLocator)
{
  declareProperty("CustomEventTypes",m_types);

}

MemoryAuditor::~MemoryAuditor(){
}

void MemoryAuditor::beforeInitialize(INamedInterface* alg) {
  std::string theString = "Memory usage before ";
  theString += alg->name() ;
  theString += " Initialization Method";
  printinfo(theString);
}
void MemoryAuditor:: afterInitialize(INamedInterface* alg){
  std::string theString = "Memory usage has changed after ";
  theString += alg->name() ;
  theString += " Initialization Method";
  printinfo(theString);
}
void MemoryAuditor::beforeReinitialize(INamedInterface* alg) {
  std::string theString = "Memory usage before ";
  theString += alg->name() ;
  theString += " Reinitialization Method";
  printinfo(theString);
}
void MemoryAuditor:: afterReinitialize(INamedInterface* alg){
  std::string theString = "Memory usage has changed after ";
  theString += alg->name() ;
  theString += " Reinitialization Method";
  printinfo(theString);
}
void MemoryAuditor:: beforeExecute(INamedInterface* alg){
  std::string theString = "Memory usage has changed before ";
  theString += alg->name() ;
  theString += " Execute Method";
  printinfo(theString);
}
void MemoryAuditor:: afterExecute(INamedInterface* alg, const StatusCode& ) {
  std::string theString = "Memory usage has changed after ";
  theString += alg->name() ;
  theString += " Execute Method";
  printinfo(theString);
}
void MemoryAuditor::beforeBeginRun(INamedInterface* ini) {
  std::string theString = "Memory usage before ";
  theString += ini->name() ;
  theString += " BeginRun Method";
  printinfo(theString);
}
void MemoryAuditor:: afterBeginRun(INamedInterface* ini){
  std::string theString = "Memory usage has changed after ";
  theString += ini->name() ;
  theString += " BeginRun Method";
  printinfo(theString);
}
void MemoryAuditor::beforeEndRun(INamedInterface* ini) {
  std::string theString = "Memory usage before ";
  theString += ini->name() ;
  theString += " EndRun Method";
  printinfo(theString);
}
void MemoryAuditor:: afterEndRun(INamedInterface* ini){
  std::string theString = "Memory usage has changed after ";
  theString += ini->name() ;
  theString += " EndRun Method";
  printinfo(theString);
}
void MemoryAuditor:: beforeFinalize(INamedInterface* alg) {
  std::string theString = "Memory usage has changed before ";
  theString += alg->name() ;
  theString += " Finalize Method";
  printinfo(theString);
}
void MemoryAuditor:: afterFinalize(INamedInterface* alg){
  std::string theString = "Memory usage has changed after ";
  theString += alg->name() ;
  theString += " Finalize Method";
  printinfo(theString);
}

void
MemoryAuditor::before(CustomEventTypeRef evt, const std::string& caller) {
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
  printinfo(theString);

}

void
MemoryAuditor::after(CustomEventTypeRef evt, const std::string& caller, const StatusCode&) {

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
  printinfo(theString);

}


StatusCode MemoryAuditor::sysFinalize( )
{
  return StatusCode::SUCCESS;
}

bool MemoryAuditor::printinfo(std::string theString)
{
   ProcStats* p = ProcStats::instance();
   procInfo info;

   // The fetch method returns true if memory usage has changed...
   if( p->fetch(info) == true) {
     MsgStream log(msgSvc(), name());
     log << MSG::INFO << theString <<
       " virtual size = " << info.vsize << " MB"  <<
       " resident set size = " << info.rss << " MB" << endmsg;
     return true;
    }
   else {
      return false;
   }
}
