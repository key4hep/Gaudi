// MemoryAuditor:
//  An auditor that monitors memory usage

#ifdef __ICC
// disable icc warning #654: overloaded virtual function "B::Y" is only partially overridden in class "C"
//   TODO: there is only a partial overload of IAuditor::before and IAuditor::after
#pragma warning( disable : 654 )
#endif

#include "MemoryAuditor.h"
#include "GaudiKernel/MsgStream.h"

DECLARE_COMPONENT( MemoryAuditor )

void MemoryAuditor::i_before( CustomEventTypeRef evt, const std::string& caller )
{
  i_printinfo( "Memory usage before", evt, caller );
}

void MemoryAuditor::i_after( CustomEventTypeRef evt, const std::string& caller, const StatusCode& )
{
  i_printinfo( "Memory usage has changed after", evt, caller );
}

void MemoryAuditor::i_printinfo( const std::string& msg, CustomEventTypeRef evt, const std::string& caller )
{
  procInfo pInfo;
  // The fetch method returns true if memory usage has changed...
  if ( getProcInfo( pInfo ) ) {
    info() << msg << " " << caller << " " << evt << " virtual size = " << pInfo.vsize << " MB"
           << " resident set size = " << pInfo.rss << " MB" << endmsg;
  }
}
