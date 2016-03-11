#ifdef __ICC
// disable icc warning #654: overloaded virtual function "B::Y" is only partially overridden in class "C"
//   TODO: there is only a partial overload of IAuditor::before and IAuditor::after
#pragma warning(disable:654)
#endif

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IChronoStatSvc.h"

#include "GaudiKernel/Stat.h"

/// local
#include "ProcStats.h"
#include "MemStatAuditor.h"

DECLARE_COMPONENT(MemStatAuditor)

MemStatAuditor::MemStatAuditor(const std::string& name, ISvcLocator* pSvcLocator) :
  MemoryAuditor(name, pSvcLocator)
{
}

StatusCode MemStatAuditor::initialize() {
  StatusCode sc = CommonAuditor::initialize();
  if (UNLIKELY(sc.isFailure())) return sc;

  m_stat = serviceLocator()->service("ChronoStatSvc");
  if (UNLIKELY(!m_stat.get())) {
    error() << "Cannot get ChronoStatSvc" << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

void MemStatAuditor::i_before(CustomEventTypeRef /*evt*/, const std::string& /*caller*/) {
  // It's not interesting to monitor the memory usage before the methods.
}

void MemStatAuditor::i_printinfo(const std::string& msg, CustomEventTypeRef evt, const std::string& caller) {
  // cannot be exactly 0
  double deltaVSize = 0.00001;

  procInfo pInfo;
  if (getProcInfo(pInfo)) {

    if (pInfo.vsize > 0) {
      if (m_vSize > 0){
        deltaVSize = pInfo.vsize - m_vSize;
      }
      // store the current VSize to be able to monitor the increment
      m_vSize = pInfo.vsize;
    }

    info() << msg << " " << caller << " " << evt <<
        " \tvirtual size = " << pInfo.vsize << " MB"  <<
        " \tresident set size = " << pInfo.rss << " MB" <<
        " deltaVsize = " << deltaVSize << "  MB" << endmsg;
  }
  // fill the stat for every call, not just when there is a change
  // only monitor the increment in VSize
  // Stat stv(statSvc(), caller + ":VMemUsage", pInfo.vsize);
  // Stat str(statSvc(), caller + ":RMemUsage", pInfo.rss);
  Stat sts(statSvc(), caller + ":VMem", deltaVSize);
}
