#include "PrecedenceSvc.h"
#include "EventSlot.h"
#include "PRGraphVisitors.h"
#include "AlgResourcePool.h"

#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/Algorithm.h"

#define ON_DEBUG if (msgLevel(MSG::DEBUG))
#define ON_VERBOSE if (msgLevel(MSG::VERBOSE))

DECLARE_SERVICE_FACTORY(PrecedenceSvc)

// ============================================================================
// Standard constructor, initializes variables
// ============================================================================

PrecedenceSvc::PrecedenceSvc(const std::string& name, ISvcLocator* svcLoc)
: base_class(name,svcLoc) {}


// ============================================================================
// Initialization
// ============================================================================
StatusCode PrecedenceSvc::initialize() {

  auto sc = Service::initialize(); // parent class must be initialized first
  if ( sc.isFailure() ) {
    fatal() << "Base class failed to initialize" << endmsg;
    return sc;
  }

  ON_DEBUG {
    // prepare a directory to dump precedence analysis files to.
    if (m_dumpPrecTrace or m_dumpPrecRules) {
      if(!boost::filesystem::create_directory(m_dumpDirName)) {
        error() << "Could not create directory " << m_dumpDirName << "required "
                   "for task precedence tracing" << endmsg;
        return StatusCode::FAILURE;
      }
    }
  }

  // Get the algo resource pool
  m_algResourcePool = serviceLocator()->service("AlgResourcePool");
   if (!m_algResourcePool.isValid()) {
     fatal() << "Error retrieving AlgoResourcePool" << endmsg;
     return StatusCode::FAILURE;
   }

  info() << "Assembling CF and DF task precedence rules" << endmsg;

  ON_DEBUG debug() << "Assembling CF precedence realm:" << endmsg;
  // create the root CF node
  m_PRGraph.addHeadNode("RootDecisionHub",true,false,true,true);
  // assemble the CF rules
  for (const auto& ialgoPtr : m_algResourcePool->getTopAlgList()) {
    auto algorithm = dynamic_cast<Algorithm*> (ialgoPtr);
    if (!algorithm) fatal() << "Conversion from IAlgorithm to Algorithm failed" << endmsg;
    sc = assembleCFRules(algorithm, "RootDecisionHub");
    if (sc.isFailure()) {
      fatal() << "Could not assemble the CF precedence realm" << endmsg;
      return sc;
    }
  }

  if (m_ignoreDFRules) {
    warning() << "Ignoring DF precedence rules, disabling all associated features"
              << endmsg;
    return StatusCode::SUCCESS;
  }

  ON_DEBUG debug() << "Assembling DF precedence realm:" << endmsg;
  sc = m_PRGraph.initialize();
  if (sc.isFailure()) {
    fatal() << "Could not assemble the DF precedence realm" << endmsg;
    return sc;
  }

  // Rank algorithms if a prioritization rule is supplied
  if (m_mode == "PCE") {
    auto ranker = concurrency::RankerByProductConsumption();
    m_PRGraph.rankAlgorithms(ranker);
  } else if (m_mode == "COD") {
    auto ranker = concurrency::RankerByCummulativeOutDegree();
    m_PRGraph.rankAlgorithms(ranker);
  } else if (m_mode == "E") {
    auto ranker = concurrency::RankerByEccentricity();
    m_PRGraph.rankAlgorithms(ranker);
  } else if (m_mode == "T") {
    auto ranker = concurrency::RankerByTiming();
    m_PRGraph.rankAlgorithms(ranker);
  } else if (m_mode == "DRE") {
    auto ranker = concurrency::RankerByDataRealmEccentricity();
    m_PRGraph.rankAlgorithms(ranker);
  } else if (!m_mode.empty()) {
    error() << "Requested prioritization rule '" << m_mode << "' is unknown" << endmsg;
    return StatusCode::FAILURE;
  }

  ON_DEBUG debug() << m_PRGraph.dumpDataFlow() << endmsg;

  if (sc.isSuccess()) info() << "PrecedenceSvc initialized successfully" << endmsg;

  return sc;
}

// ============================================================================
StatusCode PrecedenceSvc::assembleCFRules(Algorithm* algo,
                                          const std::string& parentName,
                                          unsigned int recursionDepth) {
  StatusCode sc = StatusCode::SUCCESS;

  ++recursionDepth;

  bool isGaudiSequencer(false);
  bool isAthSequencer(false);

  if (algo->isSequence()) {
    if (algo->hasProperty("ShortCircuit"))
      isGaudiSequencer = true;
    else if (algo->hasProperty("StopOverride"))
      isAthSequencer = true;
  }

  std::vector<Algorithm*>* subAlgorithms = algo->subAlgorithms();
  if ( //we only want to add basic algorithms -> have no subAlgs
       // and exclude the case of empty sequencers
       (subAlgorithms->empty() && !(isGaudiSequencer || isAthSequencer))) {

    ON_DEBUG debug() << std::string(recursionDepth, ' ') << "Algorithm '"
                     << algo->name() << "' discovered" << endmsg;
    sc = m_PRGraph.addAlgorithmNode(algo, parentName, false, false);
    return sc;
  }

  // Recursively unroll
  ON_DEBUG debug() << std::string(recursionDepth, ' ') << "Decision hub '" << algo->name()
                   << "' discovered" << endmsg;
  bool modeOR = false;
  bool allPass = false;
  bool isLazy = false;
  bool isSequential = false;

  if (isGaudiSequencer) {
    modeOR  = (algo->getProperty("ModeOR").toString() == "True")? true : false;
    allPass = (algo->getProperty("IgnoreFilterPassed").toString() == "True")? true : false;
    isLazy  = (algo->getProperty("ShortCircuit").toString() == "True")? true : false;
    if (allPass) isLazy = false; // standard GaudiSequencer behavior on all pass is to execute everything
    isSequential = (algo->hasProperty("Sequential") &&
                   (algo->getProperty("Sequential").toString() == "True"));
  } else if (isAthSequencer) {
    modeOR  = (algo->getProperty("ModeOR").toString() == "True")? true : false;
    allPass = (algo->getProperty("IgnoreFilterPassed").toString() == "True")? true : false;
    isLazy = (algo->getProperty("StopOverride").toString() == "True")? false : true;
    isSequential = (algo->hasProperty("Sequential") &&
                   (algo->getProperty("Sequential").toString() == "True"));
  }
  sc = m_PRGraph.addDecisionHubNode(algo, parentName, !isSequential, isLazy, modeOR, allPass);
  if (sc.isFailure()) {
    error() << "Failed to add DecisionHub " << algo->name() << " to graph of precedence rules" << endmsg;
    return sc;
  }

  for (Algorithm* subalgo : *subAlgorithms) {
    sc = assembleCFRules(subalgo,algo->name(),recursionDepth);
    if (sc.isFailure()) {
      error() << "Algorithm " << subalgo->name() << " could not be flattened" << endmsg;
      return sc;
    }
  }
  return sc;
}

// ============================================================================
StatusCode PrecedenceSvc::iterate(EventSlot& slot, const Cause& cause) {

  bool ifTrace = false;
  ON_DEBUG if (m_dumpPrecTrace) ifTrace = true; // enable precedence analysis

  if (Cause::source::Task == cause.m_source) {
    ON_VERBOSE verbose() << "Triggering bottom-up traversal at node '"
                         << cause.m_sourceName <<"'" << endmsg;
    auto visitor = concurrency::DecisionUpdater(slot,cause,ifTrace);
    m_PRGraph.getAlgorithmNode(cause.m_sourceName)->accept(visitor);
  } else {
    ON_VERBOSE verbose() << "Triggering top-down traversal at the root node" << endmsg;
    auto visitor = concurrency::Supervisor(slot,cause,ifTrace);
    m_PRGraph.getHeadNode()->accept(visitor);
  }

  ON_DEBUG {
    if (m_dumpPrecTrace) if(CFRulesResolved(slot)) dumpPrecedenceTrace(slot);
    if (m_dumpPrecRules) if(CFRulesResolved(slot)) dumpPrecedenceRules(slot);
  }

  return StatusCode::SUCCESS;
}

// ============================================================================
StatusCode PrecedenceSvc::simulate(EventSlot& slot) const {

  Cause cs = {Cause::source::Root, "RootDecisionHub"};
  auto visitor = concurrency::RunSimulator( slot, cs );

  auto& nodeDecisions = slot.controlFlowState;

  std::vector<int> prevNodeDecisions;
  int cntr = 0;
  std::vector<int> counters;

  while (!CFRulesResolved(slot)) {
    cntr += 1;
    int prevAlgosNum = visitor.m_nodesSucceeded;
    debug() << "  Proceeding with iteration #" << cntr << endmsg;
    prevNodeDecisions = slot.controlFlowState;
    m_PRGraph.getHeadNode()->accept(visitor);
    if ( prevNodeDecisions == nodeDecisions) {
      error() << "  No progress on iteration " << cntr
              << " detected, node decisions are:" << nodeDecisions << endmsg;
      return StatusCode::FAILURE;
    }
    info() << "   Iteration #" << cntr << " finished, total algorithms executed: "
           << visitor.m_nodesSucceeded << endmsg;

    std::stringstream s;
    s << cntr << ", " << (visitor.m_nodesSucceeded-prevAlgosNum) << "\n";

    std::ofstream myfile;
    myfile.open("RunSimulation.csv", std::ios::app);
    myfile << s.str();
    myfile.close();


    if (visitor.m_nodesSucceeded != prevAlgosNum)
      counters.push_back(visitor.m_nodesSucceeded);
  }

  info() << "Asymptotical concurrency speedup depth: "
         << (float) visitor.m_nodesSucceeded / (float) counters.size() << endmsg;

  // Reset algorithm states and node decisions
  slot.algsStates.reset();
  nodeDecisions.assign(nodeDecisions.size(),-1);

  return StatusCode::SUCCESS;
}

// ============================================================================
bool PrecedenceSvc::CFRulesResolved(EventSlot& slot) const {
  return (-1 != slot.controlFlowState[m_PRGraph.getHeadNode()->getNodeIndex()] ? true : false);
}

// ============================================================================
void PrecedenceSvc::dumpControlFlow() const {

  info() << std::endl
         << "==================== Control Flow Configuration =================="
         << std::endl << std::endl;
  info() << m_PRGraph.dumpControlFlow() << endmsg;

}
// ============================================================================
void PrecedenceSvc::dumpDataFlow() const {
  info() << std::endl
         << "===================== Data Flow Configuration ===================="
         << std::endl;
  info() << m_PRGraph.dumpDataFlow() << endmsg;
}

// ============================================================================
const std::string PrecedenceSvc::printState(EventSlot& slot) const {

  std::stringstream ss;
  m_PRGraph.printState(ss, slot.algsStates, slot.controlFlowState, 0);
  return ss.str();
}

// ============================================================================
void PrecedenceSvc::dumpPrecedenceRules(EventSlot& slot) {

  ON_DEBUG debug() << "Dumping temporal precedence rules" << endmsg;

  std::string fileName;
  if (m_dumpPrecRulesFile.empty()) {
    const auto& eventID = slot.eventContext->eventID();
    fileName = "rules.evt-" + std::to_string(eventID.event_number()) + "." +
               "run-" + std::to_string(eventID.run_number()) + ".graphml";
  } else {
    fileName = m_dumpPrecRulesFile;
  }

  boost::filesystem::path pth{m_dumpDirName};
  pth.append(fileName);

  m_PRGraph.dumpPrecRules(pth,slot);
}

// ============================================================================
void PrecedenceSvc::dumpPrecedenceTrace(EventSlot& slot) {

  ON_DEBUG debug() << "Dumping temporal precedence trace" << endmsg;

  std::string fileName;
  if (m_dumpPrecTraceFile.empty()) {
    const auto& eventID = slot.eventContext->eventID();
    fileName = "trace.evt-" + std::to_string(eventID.event_number()) + "." +
               "run-" + std::to_string(eventID.run_number()) + ".graphml";
  } else {
    fileName = m_dumpPrecTraceFile;
  }

  boost::filesystem::path pth{m_dumpDirName};
  pth.append(fileName);

  m_PRGraph.dumpPrecTrace(pth);
}

// ============================================================================
// Finalize
// ============================================================================
StatusCode PrecedenceSvc::finalize() {
  return Service::finalize();
}
