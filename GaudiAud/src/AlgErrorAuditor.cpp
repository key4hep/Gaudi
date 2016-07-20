#include "AlgErrorAuditor.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/GaudiException.h"

DECLARE_COMPONENT(AlgErrorAuditor)

AlgErrorAuditor::AlgErrorAuditor(const std::string& name, ISvcLocator* pSvcLocator)
  : Auditor(name, pSvcLocator), m_error(0), m_fatal(0) {

  declareProperty( "Abort", m_abort = false,
		   "Abort job upon illegal Algorithm return code");
  declareProperty( "Throw", m_throw = false,
		   "Throw GaudiException upon illegal Algorithm return code");
}

void
AlgErrorAuditor:: beforeExecute(INamedInterface* ){
  m_error = msgSvc()->messageCount(MSG::ERROR);
  m_fatal = msgSvc()->messageCount(MSG::FATAL);
}

StatusCode
AlgErrorAuditor:: initialize() {

  if (m_abort && m_throw) {
    info() << "Both \"Throw\" and \"Abort\" options have been set."
	<< " Abort takes precedence." << endmsg;
  }

  return StatusCode::SUCCESS;
}


void
AlgErrorAuditor:: afterExecute(INamedInterface* alg, const StatusCode& sc) {

  bool fail(false);
  if (msgSvc()->messageCount(MSG::ERROR) != m_error && ! sc.isRecoverable() ) {
    std::ostringstream os;
    os << "Illegal Return Code: Algorithm " << alg->name()
       << " reported an ERROR, but returned a StatusCode \"" << sc << "\"";
    os << std::endl << "Error policy described in "
	 << "https://twiki.cern.ch/twiki/bin/view/AtlasComputing/ReportingErrors";

    error() << os.str() << endmsg;
    incrMap(alg->name(), 0);
    fail = true;

    if (m_throw && ! m_abort) {
      throw GaudiException(os.str(),"AlgErrorAuditor",0);
    }
  }

  if (msgSvc()->messageCount(MSG::FATAL) != m_fatal &&
      sc != StatusCode::FAILURE ) {
    std::ostringstream os;
    os << "Illegal Return Code: Algorithm " << alg->name()
       << " reported a FATAL, but returned a StatusCode \"" << sc << "\"";
    os << std::endl << "Error policy described in "
	 << "https://twiki.cern.ch/twiki/bin/view/AtlasComputing/ReportingErrors";

    error() << os.str() << endmsg;
    incrMap(alg->name(), 1);
    fail = true;

    if (m_throw && ! m_abort) {
      throw GaudiException(os.str(),"AlgErrorAuditor",0);
    }

  }

  if (fail && m_abort) {
    abort();
  }

}

StatusCode
AlgErrorAuditor::finalize() {


  if (m_algMap[0].size() != 0) {
    info() << "Found " << m_algMap[0].size()
	<< " instances where an Algorithm::execute() produced an ERROR "
	<< "but returned a SUCCESS:" << std::endl;

    for (const auto&  i : m_algMap[0] ) {
      msgStream() << i.first << ": " << i.second << std::endl;
    }

    msgStream() << endmsg;
  }

  if (m_algMap[1].size() != 0) {
    info() << "Found " << m_algMap[1].size()
	<< " instances where an Algorithm::execute() produced a FATAL "
	<< "but returned a SUCCESS:" << std::endl;

    for (const auto& i : m_algMap[1]) {
      msgStream() << i.first << ": " << i.second << std::endl;
    }

    msgStream() << endmsg;
  }
  return StatusCode::SUCCESS;
}

void
AlgErrorAuditor::incrMap(const std::string& alg, int level) {
  auto i=m_algMap[level].find(alg);
  if ( i != m_algMap[level].end()) {
    i->second++;
  } else {
    m_algMap[level].emplace( alg,1 );
  }
}
