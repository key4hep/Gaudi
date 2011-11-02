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
  MemoryAuditor(name, pSvcLocator), m_vSize(-1.)
{
}

MemStatAuditor::~MemStatAuditor() {}

StatusCode MemStatAuditor::initialize() {
  StatusCode sc = CommonAuditor::initialize();
  if (UNLIKELY(sc.isFailure())) return sc;

  m_stat = serviceLocator()->service("ChronoStatSvc");
  if (UNLIKELY(!m_stat.get())) {
    MsgStream log(msgSvc(), name());
    log << MSG::ERROR << "Cannot get ChronoStatSvc" << endmsg;
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

  procInfo info;
  if (getProcInfo(info)) {
    MsgStream log(msgSvc(), name());

    if (info.vsize > 0) {
      if (m_vSize > 0){
        deltaVSize = info.vsize - m_vSize;
      }
      // store the current VSize to be able to monitor the increment
      m_vSize = info.vsize;
    }

    log << MSG::INFO << msg << " " << caller << " " << evt <<
        " \tvirtual size = " << info.vsize << " MB"  <<
        " \tresident set size = " << info.rss << " MB" <<
        " deltaVsize = " << deltaVSize << "  MB" << endmsg;
  }
  // fill the stat for every call, not just when there is a change
  // only monitor the increment in VSize
  // Stat stv(statSvc(), caller + ":VMemUsage", info.vsize);
  // Stat str(statSvc(), caller + ":RMemUsage", info.rss);
  Stat sts(statSvc(), caller + ":VMem", deltaVSize);
}
