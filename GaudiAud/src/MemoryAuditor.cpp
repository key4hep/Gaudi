// MemoryAuditor:
//  An auditor that monitors memory usage

#ifdef __ICC
// disable icc warning #654: overloaded virtual function "B::Y" is only partially overridden in class "C"
//   TODO: there is only a partial overload of IAuditor::before and IAuditor::after
#pragma warning(disable:654)
#endif

#include "MemoryAuditor.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AudFactory.h"

DECLARE_AUDITOR_FACTORY(MemoryAuditor)

MemoryAuditor::MemoryAuditor(const std::string& name, ISvcLocator* pSvcLocator)
: CommonAuditor(name, pSvcLocator) {
}

void MemoryAuditor::i_before(CustomEventTypeRef evt, const std::string& caller) {
  i_printinfo("Memory usage before", evt, caller);
}

void MemoryAuditor::i_after(CustomEventTypeRef evt, const std::string& caller, const StatusCode&){
  i_printinfo("Memory usage has changed after", evt, caller);
}

void MemoryAuditor::i_printinfo(const std::string& msg, CustomEventTypeRef evt, const std::string& caller)
{
   procInfo info;
   // The fetch method returns true if memory usage has changed...
   if(getProcInfo(info)) {
     MsgStream log(msgSvc(), name());
     log << MSG::INFO << msg << " " << caller << " " << evt <<
       " virtual size = " << info.vsize << " MB"  <<
       " resident set size = " << info.rss << " MB" << endmsg;
   }
}
