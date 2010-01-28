// ChronoAuditor:
//  An auditor that monitors memory usage

#include "ChronoAuditor.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AudFactory.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/Chrono.h"

DECLARE_AUDITOR_FACTORY(ChronoAuditor)

ChronoAuditor::ChronoAuditor(const std::string& name, ISvcLocator* pSvcLocator)
: Auditor(name, pSvcLocator) {
  service( "ChronoStatSvc", m_chronoSvc, true).ignore();
  declareProperty("CustomEventTypes",m_types);
}

ChronoAuditor::~ChronoAuditor(){
  m_chronoSvc->release();
}

void ChronoAuditor::beforeInitialize(INamedInterface* alg) {
  chronoSvc( )->chronoStart( alg->name() + ":initialize" ) ;
}
void ChronoAuditor:: afterInitialize(INamedInterface* alg){
  chronoSvc( )->chronoStop( alg->name() + ":initialize" ) ;
}

void ChronoAuditor::beforeReinitialize(INamedInterface* alg) {
  chronoSvc( )->chronoStart( alg->name() + ":reinitialize" ) ;
}
void ChronoAuditor:: afterReinitialize(INamedInterface* alg){
  chronoSvc( )->chronoStop( alg->name() + ":reinitialize" ) ;
}

void ChronoAuditor:: beforeExecute(INamedInterface* alg){
  chronoSvc( )->chronoStart( alg->name() + ":execute" ) ;
}
void ChronoAuditor:: afterExecute(INamedInterface* alg, const StatusCode& ) {
  chronoSvc( )->chronoStop( alg->name() + ":execute" ) ;
}

void ChronoAuditor::beforeBeginRun(INamedInterface* alg) {
  chronoSvc( )->chronoStart( alg->name() + ":beginRun" ) ;
}
void ChronoAuditor:: afterBeginRun(INamedInterface* alg){
  chronoSvc( )->chronoStop( alg->name() + ":beginRun" ) ;
}
void ChronoAuditor::beforeEndRun(INamedInterface* alg) {
  chronoSvc( )->chronoStart( alg->name() + ":endRun" ) ;
}
void ChronoAuditor:: afterEndRun(INamedInterface* alg){
  chronoSvc( )->chronoStop( alg->name() + ":endRun" ) ;
}


void ChronoAuditor:: beforeFinalize(INamedInterface* alg) {
  chronoSvc( )->chronoStart( alg->name() + ":finalize" ) ;
}
void ChronoAuditor:: afterFinalize(INamedInterface* alg){
  chronoSvc( )->chronoStop( alg->name() + ":finalize" ) ;
}

void
ChronoAuditor::before(CustomEventTypeRef evt, const std::string& caller) {

  if (m_types.value().size() != 0) {
    if ( (m_types.value())[0] == "none") {
      return;
    }

    if ( find(m_types.value().begin(), m_types.value().end(), evt) ==
	 m_types.value().end() ) {
      return;
    }
  }

  chronoSvc( )->chronoStart( caller + ":" + evt ) ;

}

void
ChronoAuditor::after(CustomEventTypeRef evt, const std::string& caller, const StatusCode&) {

  if (m_types.value().size() != 0) {
    if ( (m_types.value())[0] == "none") {
      return;
    }

    if ( find(m_types.value().begin(), m_types.value().end(), evt) ==
	 m_types.value().end() ) {
      return;
    }
  }

  chronoSvc( )->chronoStop( caller + ":" + evt ) ;

}
