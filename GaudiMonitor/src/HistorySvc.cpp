#include "HistorySvc.h"

#include "GaudiKernel/HistoryObj.h"
#include "GaudiKernel/IVersHistoryObj.h"

#include "GaudiKernel/JobHistory.h"
#include "GaudiKernel/AlgorithmHistory.h"
#include "GaudiKernel/AlgToolHistory.h"
#include "GaudiKernel/DataHistory.h"
#include "GaudiKernel/ServiceHistory.h"

#include "GaudiKernel/ISvcLocator.h"

#include "GaudiKernel/System.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/ServiceHandle.h"

#include "GaudiKernel/IAlgContextSvc.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>

#define ON_DEBUG if (UNLIKELY(outputLevel() <= MSG::DEBUG))
#define ON_VERBOSE if (UNLIKELY(outputLevel() <= MSG::VERBOSE))

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
DECLARE_COMPONENT(HistorySvc)

using namespace std;

//
///////////////////////////////////////////////////////////////////////////
//

namespace {
  template <typename MAP, typename SET>
  inline void map_to_set(const MAP& m, SET& s) {
    std::transform(std::begin(m), std::end(m),
                   std::inserter(s, s.begin()),
                   [](typename MAP::const_reference p){ return p.second; });
  }
}

struct DHH {
  CLID id;
  std::string key;

  DHH(const CLID& i, const std::string& k):id(i), key(k) {}

  bool operator < ( DHH const &rhs ) const {
    if (id != rhs.id) {
      return (id < rhs.id);
    } else {
      return (key < rhs.key);
    }
  }
};

void fenv_dummy(char**) {}

HistorySvc::HistorySvc( const std::string& name, ISvcLocator* svc )
  : base_class( name, svc ),
    m_isInitialized(false),
    m_dump(false),
    p_algCtxSvc(0),
    m_jobHistory(0),
    m_outputFile(""),
    m_incidentSvc(0),
    m_log(msgSvc(), name ),
    m_outputFileTypeXML(false)

{
    declareProperty("Dump",m_dump);
    declareProperty("Activate", m_activate=true);
    declareProperty("OutputFile",m_outputFile);

    // hack to bring in environ
    vector<string> envtmp = System::getEnv();
    envtmp.size(); // prevent icc remark #177: X declared but never referenced
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

HistorySvc::~HistorySvc() {
  delete m_jobHistory;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::reinitialize() {

  clearState();

  m_state = Gaudi::StateMachine::OFFLINE;

  return initialize();

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace {
  template <typename T1, typename T2>
  void clear(std::pair<const T1* const, T2*>& p) {
    const_cast<T1*>(p.first)->release();
    delete p.second;
  }
  template<typename M>
  void clear(M& m) {
    std::for_each(std::begin(m), std::end(m),
                  [](typename M::value_type& alg){ clear(alg); });
    m.clear();
  }
}
void HistorySvc::clearState() {
  m_algs.clear();
  clear(m_algmap);

  m_ialgtools.clear();
  m_algtools.clear();
  clear(m_algtoolmap);

  m_svcs.clear();
  clear(m_svcmap);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::initialize() {

  StatusCode status = Service::initialize();
  m_log.setLevel( m_outputLevel.value() );

  if (status.isFailure()) {

    ON_DEBUG
      m_log << MSG::DEBUG << "Failed to initialize the base class (Service)"
	    << endmsg;
    return status;
  }

  ON_DEBUG
    m_log << MSG::DEBUG << "Initializing HistorySvc" << endmsg;

  if (!m_activate) return StatusCode::SUCCESS;

  static const bool CREATEIF(true);

  if ( service("AlgContextSvc",p_algCtxSvc,CREATEIF).isFailure() ) {
    m_log << MSG::ERROR << "unable to get the AlgContextSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  if (service("IncidentSvc", m_incidentSvc, CREATEIF).isFailure()) {
    m_log << MSG::ERROR << "unable to get the IncidentSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  // create a weak dependency on the ToolSvc, so that it doesn't get deleted
  // before we're done with it in finalize
  m_toolSvc = serviceLocator()->service("ToolSvc");
  if (! m_toolSvc)  {
    m_log << MSG::ERROR << "could not retrieve the ToolSvc handle !"
	  << endmsg;
    return StatusCode::FAILURE;
  }

  // add listener to be triggered by first BeginEvent with low priority
  // so it gets called first
  const bool rethrow = false;
  const bool oneShot = true; // make the listener called only once
  m_incidentSvc->addListener(this,IncidentType::BeginEvent,
			     std::numeric_limits<long>::min(),rethrow,oneShot);

  if (m_outputFile.find(".XML") != string::npos || m_outputFile.find(".xml") != string::npos) {
    ON_DEBUG
      m_log << MSG::DEBUG << "output format is XML" << endmsg;
    m_outputFileTypeXML = true;
  }

  m_isInitialized = true;

  return StatusCode::SUCCESS;

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::captureState() {

  if (m_jobHistory == 0) {
    m_jobHistory = new JobHistory;
    IJobOptionsSvc *jo;
    if (service("JobOptionsSvc",jo).isFailure()) {
       m_log << MSG::ERROR
	     << "Could not get jobOptionsSvc - "
	     << "not adding properties to JobHistory" << endmsg;
    } else {

      bool foundAppMgr(false);

      for (auto& client: jo->getClients()) {
        if (client == "ApplicationMgr") foundAppMgr = true;
        for (auto prop: *jo->getProperties(client)) {
          m_jobHistory->addProperty(client, prop);
        }
      }

      if (!foundAppMgr) {
	IProperty *ap;
	if (service("ApplicationMgr", ap).isFailure()) {
	  m_log << MSG::ERROR << "could not get the ApplicationMgr" << endmsg;
	} else {
	  for (auto prop: ap->getProperties()) {
	    m_jobHistory->addProperty("ApplicationMgr", prop);
	  }
	}
      }

    }
  }

  /// Get all the Algorithms

  StatusCode sc;
  IAlgManager* algMgr = 0;
  sc = Gaudi::svcLocator()->queryInterface( IAlgManager::interfaceID(),
                                            pp_cast<void>(&algMgr) );
  if (sc.isFailure()) {
    m_log << MSG::ERROR << "Could not get AlgManager" << endmsg;
    return StatusCode::FAILURE;
  } else {
    size_t count = 0;
    for (auto ialg: algMgr->getAlgorithms()) {
      Algorithm* alg = dynamic_cast<Algorithm*>(ialg);
      if (alg == 0) {
	m_log << MSG::WARNING << "Algorithm " << ialg->name()
	      << " does not inherit from Algorithm. Not registering it."
	      << endmsg;
      } else {
        ++count;
	registerAlg( *alg ).ignore();
      }
    }

    m_log << MSG::INFO << "Registered " << count << " Algorithms" << endmsg;

  }

  /// Get all the AlgTools

  m_isInitialized = true;
  for (auto algtool: m_ialgtools) {
    (const_cast<IAlgTool*>(algtool))->addRef();
    registerAlgTool(*algtool).ignore();
  }

  m_log << MSG::INFO << "Registered " << m_algtools.size() << " AlgTools"
	<< endmsg;

  /// Get all the Services

  for (auto svc: Gaudi::svcLocator()->getServices()) {
    svc->addRef();
    registerSvc(*svc).ignore();
  }

  m_log << MSG::INFO;
  m_log << "Registered " << Gaudi::svcLocator()->getServices().size() << " Services" << endmsg;

  return StatusCode::SUCCESS;


}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::stop() {

  if (!m_activate) return StatusCode::SUCCESS;

  if (m_dump) {
    listProperties().ignore();
  }

  if (m_outputFile != "") {
    std::ofstream ofs;
    ofs.open(m_outputFile.c_str());
    if (!ofs) {
      m_log << MSG::ERROR << "Unable to open output file \"m_outputFile\""
	    << endmsg;
    } else {

      //      dumpProperties(ofs);
      dumpState(ofs);

      ofs.close();
    }
  }

  clearState();

  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode HistorySvc::finalize() {

  ON_VERBOSE
    m_log << MSG::VERBOSE << "HistorySvc::finalize()" << endmsg;


  clearState();

  StatusCode status = Service::finalize();

  if ( status.isSuccess() )
    m_log << MSG::INFO << "Service finalised successfully" << endmsg;

  return status;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
HistorySvc::registerAlg(const Algorithm &alg) {

  JobHistory *job = getJobHistory();
  if (m_algmap.find(&alg) != m_algmap.end()) {
    m_log << MSG::WARNING << "Algorithm " << alg.name()
	  << " already registered with HistorySvc" << endmsg;
    return StatusCode::SUCCESS;
  }

  (const_cast<Algorithm*>(&alg))->addRef();

  m_algs.insert(&alg);

  AlgorithmHistory *algHist = new AlgorithmHistory(alg, job);
  m_algmap[&alg] = algHist;

  ON_DEBUG {
    m_log << MSG::DEBUG << "Registering algorithm: ";
    m_log.setColor(MSG::CYAN);
    m_log << alg.name() << endmsg;
    m_log.resetColor();
  }

  return StatusCode(StatusCode::SUCCESS,true);

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
HistorySvc::listProperties(const Algorithm &alg) const {

  m_log << MSG::INFO << "Dumping properties for " << alg.name() << endl;

  AlgorithmHistory *hist = getAlgHistory( alg );

  if (hist == 0) {
    return StatusCode::FAILURE;
  }

  ostringstream ost;
  ost << *hist;

  std::string str = ost.str();

  m_log << MSG::INFO << alg.name() << " --> " << endl << str << endmsg;

  return StatusCode(StatusCode::SUCCESS,true);

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
HistorySvc::dumpProperties(const Algorithm &alg, std::ofstream& ofs) const {

  AlgorithmHistory *hist = getAlgHistory( alg );

  if (hist == 0) {
    return;
  }

  PropertyList::const_iterator itr;
  for (auto prop: hist->properties()) {
    ofs << alg.name() << "  " << dumpProp(prop) << std::endl;
  }

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

AlgorithmHistory*
HistorySvc::getAlgHistory(const Algorithm &alg) const {

  const Algorithm *palg = &alg;
  set<const Algorithm*>::const_iterator itr = m_algs.find(palg);
  if ( itr == m_algs.end() ) {
    m_log << MSG::WARNING << "Algorithm " << alg.name() << " not registered"
          << endmsg;
    return 0;
  }

  map<const Algorithm*, AlgorithmHistory*>::const_iterator itr2;
  itr2 = m_algmap.find( *itr );

  return ( itr2->second );

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
HistorySvc::getAlgHistory(std::set<AlgorithmHistory*>& algs) const {
  map_to_set(m_algmap, algs);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode
HistorySvc::registerJob() {


  return StatusCode(StatusCode::SUCCESS,true);


}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


StatusCode
HistorySvc::listProperties() const {

  m_log << MSG::INFO;
  m_log.setColor(MSG::CYAN);
  m_log << "Dumping properties for all Algorithms (" << m_algmap.size()
	<< ")" << endmsg;

  for (auto& alg: m_algmap) {
    listProperties(*alg.first).ignore();
  }

  m_log << MSG::INFO;
  m_log.setColor(MSG::CYAN);
  m_log << "Dumping properties for all AlgTools (" << m_algtoolmap.size()
	<< ")" << endmsg;

  for (auto& algtool: m_algtoolmap) {
    ON_DEBUG
      m_log << MSG::DEBUG << " --> " << algtool.second->algtool_name() << endmsg;
    listProperties(*algtool.first).ignore();
  }

  m_log << MSG::INFO;
  m_log.setColor(MSG::CYAN);
  m_log << "Dumping properties for all Services (" << m_svcmap.size()
	<< ")" << endmsg;

  for (auto& svc: m_svcmap) {
    listProperties(*svc.first).ignore();
  }

  m_log << MSG::INFO;
  m_log.setColor(MSG::CYAN);
  m_log << "Dumping properties for Job";
  m_log.resetColor();

  ostringstream ost;
  ost << *m_jobHistory;
  std::string str = ost.str();

  m_log << std::endl << str << endmsg;

  return StatusCode(StatusCode::SUCCESS,true);

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
HistorySvc::dumpProperties(std::ofstream& ofs) const {


  ofs << "GLOBAL" << std::endl;
  const JobHistory::PropertyPairList props = m_jobHistory->propertyPairs();
  JobHistory::PropertyPairList::const_iterator itrj;
  for (itrj=props.begin(); itrj != props.end(); ++itrj) {
    std::string client = itrj->first;
    const Property* prp = itrj->second;
    ofs << client << "  " << dumpProp(prp) << std::endl;
  }

  ofs << std::endl << "SERVICES" << std::endl;
  std::map<const IService*, ServiceHistory*>::const_iterator itr_s;
  for (itr_s=m_svcmap.begin(); itr_s != m_svcmap.end(); ++itr_s) {
    const IService* svc = itr_s->first;

    dumpProperties( *svc, ofs );

  }

  ofs << std::endl << "ALGORITHMS" << std::endl;
  std::map<const Algorithm*, AlgorithmHistory*>::const_iterator itr;
  for (itr=m_algmap.begin(); itr != m_algmap.end(); ++itr) {
    const Algorithm* alg = itr->first;

    dumpProperties( *alg, ofs );

  }

  ofs << std::endl << "ALGTOOLS" << std::endl;
  std::map<const AlgTool*, AlgToolHistory*>::const_iterator itr_a;
  for (itr_a=m_algtoolmap.begin(); itr_a != m_algtoolmap.end(); ++itr_a) {
    const AlgTool* alg = itr_a->first;

    dumpProperties( *alg, ofs );

  }


}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

JobHistory*
HistorySvc::getJobHistory() const {

  return m_jobHistory;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
IAlgorithm*
HistorySvc::getCurrentIAlg() const {
  if (p_algCtxSvc == 0) {
    m_log << MSG::WARNING << "trying to create DataHistoryObj before "
	  << "HistorySvc has been initialized" << endmsg;
    return 0;

  } else {
    return p_algCtxSvc->currentAlg();
  }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

DataHistory*
HistorySvc::createDataHistoryObj(const CLID& id, const std::string& key,
				 const std::string& /* storeName */) {

  if (!m_activate) return 0;


  AlgorithmHistory *algHist;

  IAlgorithm* ialg = getCurrentIAlg();
  if (ialg == 0) {
    ON_DEBUG
      m_log << MSG::DEBUG
	    << "Could not discover current Algorithm:" << endl
	    << "          object CLID: " << id << "  key: \"" << key
	    << "\"" << endmsg;
    algHist = 0;
  } else {
    Algorithm* alg = dynamic_cast<Algorithm*>(ialg);
    if (alg != 0) {
      algHist = getAlgHistory( *alg );
    } else {
      m_log << MSG::WARNING
            << "Could not extract concrete Algorithm:"
            << endl
            << "          object CLID: " << id << "  key: \"" << key
            << "\"" << endmsg;
      algHist = 0;
    }
  }

  DataHistory *hist = new DataHistory(id, key, algHist);

  return hist;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode
HistorySvc::registerDataHistory(const CLID& id, const std::string& key,
				const std::string& storeName) {

  DHH dhh(id,key);

  auto boundaries = m_datMap.equal_range(dhh);
  auto match = boundaries.second;

  if (boundaries.first != boundaries.second) {
    // there is something in the map, let's look for the specific entry

    std::string algName;
    IAlgorithm *ialg = getCurrentIAlg();
    if (ialg) {
      algName = ialg->name();
    } else {
      algName = "UNKNOWN";
    }

    match = std::find_if(boundaries.first, boundaries.second,
                         [&algName](decltype(boundaries)::first_type::reference p) -> bool {
      return p.second->algorithmHistory()->algorithm_name() == algName;
    });
  }

  if (match == boundaries.second) { // not found, crete the entry
    DataHistory *dh = createDataHistoryObj(id, key, storeName);
    m_datMap.insert(pair<DHH, DataHistory*>(dhh,dh));
  }

  return StatusCode::SUCCESS;

}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
DataHistory*
HistorySvc::getDataHistory(const CLID& id, const std::string& key,
			   const std::string& /*storeName*/) const {

  DHH dhh(id,key);

  pair<DHMCitr,DHMCitr> mitr = m_datMap.equal_range(dhh);

  if(mitr.first == mitr.second) {
    return 0;
  }

  return mitr.first->second;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
HistorySvc::getDataHistory(const CLID& id, const std::string& key,
			   const std::string& /*storeName*/,
			   std::list<DataHistory*>& dhlist) const {

  DHH dhh(id,key);

  int n(0);

  pair<DHMCitr,DHMCitr> mitr = m_datMap.equal_range(dhh);

  for (DHMCitr itr=mitr.first; itr != mitr.second; ++itr) {
    dhlist.push_back(itr->second);
    n++;
  }

  return n;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
HistorySvc::registerSvc(const IService &svc) {

  if ( svc.name() == "HistoryStore" ) {
    //    m_log << MSG::WARNING << "not registering store" << endmsg;
    return StatusCode(StatusCode::SUCCESS,true);
  }

  JobHistory *job = getJobHistory();
  const IService* psvc = &svc;
  map<const IService*, ServiceHistory*>::const_iterator itr =
    m_svcmap.find(psvc);
  if (itr == m_svcmap.end()) {

    if (m_log.level() <= MSG::DEBUG) {
      m_log << MSG::DEBUG << "Registering Service: ";
      m_log.setColor(MSG::CYAN);
      m_log << svc.name() << endmsg;
    }

    m_svcs.insert(psvc);

    ServiceHistory *svcHist = new ServiceHistory(&svc, job);
    m_svcmap[psvc] = svcHist;

    (const_cast<IService*>(psvc))->addRef();

  }

  m_log.resetColor();
  return StatusCode(StatusCode::SUCCESS,true);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

ServiceHistory*
HistorySvc::getServiceHistory(const IService &svc) const {

  const IService *psvc = &svc;
  map<const IService*, ServiceHistory*>::const_iterator itr =
    m_svcmap.find(psvc);
  if ( itr == m_svcmap.end() ) {
    m_log << MSG::WARNING << "Service " << svc.name() << " not registered"
          << endmsg;
    return 0;
  }

  return ( itr->second );

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
HistorySvc::getServiceHistory(std::set<ServiceHistory*>& svcs) const {
  map_to_set(m_svcmap, svcs);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
HistorySvc::listProperties(const IService &svc) const {

  m_log << MSG::INFO << "Dumping properties for " << svc.name() << endl;

  ServiceHistory *hist = getServiceHistory( svc );

  if (hist == 0) {
    return StatusCode::FAILURE;
  }

  ostringstream ost;
  ost << *hist;

  std::string str = ost.str();

  m_log << MSG::INFO << svc.name() << " --> " << endl << str << endmsg;



  return StatusCode(StatusCode::SUCCESS,true);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
HistorySvc::dumpProperties(const IService &svc, std::ofstream &ofs) const {

  ServiceHistory *hist = getServiceHistory( svc );

  if (hist == 0) {
    return;
  }

  for (auto &prop: hist->properties()) {
    ofs << svc.name() << "  " << dumpProp(prop) << std::endl;
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
HistorySvc::registerAlgTool(const IAlgTool& ialg) {

  if (! m_isInitialized) {
    if (p_algCtxSvc == 0) {
      if ( service("AlgContextSvc",p_algCtxSvc,true).isFailure() ) {
	m_log << MSG::ERROR << "unable to get the AlgContextSvc" << endmsg;
	return StatusCode::FAILURE;
      }
    }

    m_ialgtools.insert(&ialg);
    return StatusCode::SUCCESS;
  }

  const AlgTool *alg = dynamic_cast<const AlgTool*>( &ialg );
  if ( alg == 0 ) {
    m_log << MSG::ERROR << "Could not dcast IAlgTool \"" << ialg.name()
	  << "\" to an AlgTool" << endmsg;
    return StatusCode::FAILURE;
  }

  if (m_algtools.find(alg) != m_algtools.end()) {
    m_log << MSG::WARNING << "AlgTool " << ialg.name()
	  << " already registered in HistorySvc" << endmsg;
    return StatusCode::SUCCESS;
  }

  m_algtools.insert(alg);
  (const_cast<AlgTool*>(alg))->addRef();

  const JobHistory *job = getJobHistory();
  AlgToolHistory *algHist = new AlgToolHistory(*alg, job);
  m_algtoolmap[alg] = algHist;

  if (m_log.level() <= MSG::DEBUG) {
    m_log << MSG::DEBUG << "Registering algtool: ";
    m_log.setColor(MSG::CYAN);
    m_log << alg->name() << endmsg;
    m_log.resetColor();
  }

  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode
HistorySvc::listProperties(const IAlgTool& alg) const {

  m_log << MSG::INFO << "Dumping properties for " << alg.name() << endl;

  AlgToolHistory *hist = getAlgToolHistory( alg );

  if (hist == 0) {
    return StatusCode::FAILURE;
  }

  ostringstream ost;
  ost << *hist;

  std::string str = ost.str();

  m_log << MSG::INFO << alg.name() << " --> " << endl << str << endmsg;

  return StatusCode::SUCCESS;

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
HistorySvc::dumpProperties(const IAlgTool& alg, std::ofstream &ofs) const {

  AlgToolHistory *hist = getAlgToolHistory( alg );

  if (hist == 0) {
    return;
  }

  for (auto& prop: hist->properties()) {
    ofs << alg.name() << "  " << dumpProp(prop) << std::endl;
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

AlgToolHistory*
HistorySvc::getAlgToolHistory(const IAlgTool& alg) const {

  const AlgTool *palg = dynamic_cast<const AlgTool*>(&alg);
  set<const AlgTool*>::const_iterator itr = m_algtools.find(palg);
  if ( itr == m_algtools.end() ) {
    m_log << MSG::WARNING << "AlgTool " << alg.name() << " not registered"
          << endmsg;
    return 0;
  }

  map<const AlgTool*, AlgToolHistory*>::const_iterator itr2;
  itr2 = m_algtoolmap.find( *itr );

  return ( itr2->second );

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
HistorySvc::getAlgToolHistory(std::set<AlgToolHistory*>& algs) const {
  map_to_set(m_algtoolmap, algs);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
HistorySvc::handle(const Incident& incident) {

  if (incident.type() == IncidentType::BeginEvent) {
    if (captureState().isFailure()) {
      m_log << MSG::WARNING << "Error capturing state." << endl
            << "Will try again at next BeginEvent incident" << endmsg;
    }
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


std::string
HistorySvc::dumpProp(const Property* prop, bool isXML, int ind) const {
  std::ostringstream ost;
  if (isXML) {
    while (ind > 0) {
      ost << " ";
      ind --;
    }
    ost << "<PROPERTY name=\"" << prop->name()
	<< "\" value=\"" << HistoryObj::convert_string(prop->toString())
	<< "\" documentation=\"" << HistoryObj::convert_string(prop->documentation())
	<< "\">";
  } else {
    prop->fillStream(ost);
  }
  return ost.str();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
HistorySvc::dumpState(std::ofstream& ofs) const {


  if(m_outputFileTypeXML) {
    //xml header
    ofs << "<?xml version=\"1.0\" ?> " << std::endl;
    ofs << "<!--Test-xml-->" << std::endl;
    ofs << "<SETUP>" << std::endl;
    ofs << "  <GLOBAL>" << std::endl;
  } else {
    ofs << "GLOBAL" << std::endl;
  }

  std::string client_currently_open = "start";
  for (auto& item: m_jobHistory->propertyPairs()) {
    // client is the name of the component of the current property
    const std::string& client = item.first;
    const Property* prp = item.second;

    if (m_outputFileTypeXML) {

      if (client != client_currently_open) {
	if(client_currently_open!="start") ofs << "    </COMPONENT>" << endl;
	ofs << "    <COMPONENT name=\""
	    << client << "\" class=\"undefined\">" << std::endl;
      }
    } else {
      ofs << client << "  ";
    }

    ofs << dumpProp(prp,m_outputFileTypeXML,6) << endl;

    client_currently_open = client;

    if (m_outputFileTypeXML)
  ofs << "    </COMPONENT>" << endl;
  }


  if(m_outputFileTypeXML) {
    ofs << "</GLOBAL>" << endl << "<SERVICES>" << endl;
  } else {
    ofs << "SERVICES" << std::endl;
  }

  for (auto& item: m_svcmap) {
    const IService* svc = item.first;
    dumpState(svc, ofs);
  }

  if(m_outputFileTypeXML) {
    ofs << "</SERVICES>" << endl << "<ALGORITHMS> " << endl;
  } else {
    ofs << "ALGORITHMS" << std::endl;
  }

  for (auto& item: m_algmap) {
    const Algorithm* alg = item.first;
    dumpState(alg, ofs);
  }


  if(m_outputFileTypeXML) {
    ofs << "</ALGORITHMS>" << endl << "<ALGTOOLS> " << endl;
  } else {
    ofs << "ALGTOOLS" << std::endl;
  }

  for (auto& item: m_algtoolmap) {
    const AlgTool* algtool = item.first;
    dumpState(algtool, ofs);
  }

  if(m_outputFileTypeXML) {
    ofs << "</ALGTOOLS>" << endl << "</SETUP>" << endl;
  }

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void
HistorySvc::dumpState(const INamedInterface *in, std::ofstream& ofs) const {

  HistoryObj *hist(0);
  IVersHistoryObj *vhist(0);

  const IService* is(0);
  const Algorithm* ia(0);
  const IAlgTool* it(0);
  if ( (is=dynamic_cast<const IService*>(in)) != 0) {
    ON_VERBOSE
      m_log << MSG::VERBOSE << in->name() << " is Service" << endmsg;
    ServiceHistory* o = getServiceHistory( *is );
    hist = dynamic_cast<HistoryObj*>( o );
    vhist = dynamic_cast<IVersHistoryObj*>( o );
  } else if ( (ia = dynamic_cast<const Algorithm*>(in)) != 0 ) {
    ON_VERBOSE
      m_log << MSG::VERBOSE << in->name() << " is Alg" << endmsg;
    AlgorithmHistory *o = getAlgHistory( *ia );
    hist = dynamic_cast<HistoryObj*>( o );
    vhist = dynamic_cast<IVersHistoryObj*>( o );
  } else if ( (it = dynamic_cast<const IAlgTool*>(in)) != 0 ) {
    ON_VERBOSE
      m_log << MSG::VERBOSE << in->name() << " is AlgTool" << endmsg;
    AlgToolHistory *o = getAlgToolHistory( *it );
    hist = dynamic_cast<HistoryObj*>( o );
    vhist = dynamic_cast<IVersHistoryObj*>( o );
  } else {
    m_log << MSG::ERROR
	  << "Could not dcast interface to accepted History Obj type for "
	  << in->name() << endmsg;
    return;
  }

  if (hist == 0 || vhist == 0) {
    m_log << MSG::ERROR << "Could not dcast recognized object to HistoryObj or IVersHistoryObj. This should never happen."
	  << endmsg;
    return;
  }

  if (m_outputFileTypeXML) {
    hist->dump(ofs,true);
  } else {
    ofs << ">> " << vhist->name() << endl << *hist << endl;
  }

}
