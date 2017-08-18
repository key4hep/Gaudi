#include "PrecedenceSvc.h"
#include "EventSlot.h"
#include "PRGraphVisitors.h"
#include "AlgResourcePool.h"

#include "GaudiKernel/SvcFactory.h"

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

  // Get the algo resource pool
  m_algResourcePool = serviceLocator()->service( "AlgResourcePool" );
  if (!m_algResourcePool.isValid()) {
    fatal() << "Error retrieving AlgoResourcePool" << endmsg;
    return StatusCode::FAILURE;
  }
  const AlgResourcePool* algPool = dynamic_cast<const AlgResourcePool*>(m_algResourcePool.get());
  m_PRGraph = algPool->getPRGraph();

  if (!m_PRGraph) {
    fatal() << "No graph supplied, the service failed to initialize" << endmsg;
    return StatusCode::FAILURE;
  }

  sc = m_PRGraph->buildAugmentedDataDependenciesRealm();
  if (sc.isFailure()) {
    fatal() << "Could not assemble the data dependency realm." << endmsg;
    return sc;
  }

  // Rank algorithms if a prioritization rule is supplied
  if (m_mode == "PCE") {
    auto ranker = concurrency::RankerByProductConsumption();
    m_PRGraph->rankAlgorithms(ranker);
  } else if (m_mode == "COD") {
    auto ranker = concurrency::RankerByCummulativeOutDegree();
    m_PRGraph->rankAlgorithms(ranker);
  } else if (m_mode == "E") {
    auto ranker = concurrency::RankerByEccentricity();
    m_PRGraph->rankAlgorithms(ranker);
  } else if (m_mode == "T") {
    auto ranker = concurrency::RankerByTiming();
    m_PRGraph->rankAlgorithms(ranker);
  } else if (m_mode == "DRE") {
    auto ranker = concurrency::RankerByDataRealmEccentricity();
    m_PRGraph->rankAlgorithms(ranker);
  } else if (!m_mode.empty()) {
    error() << "Requested prioritization rule '" << m_mode << "' is unknown." << endmsg;
    sc = StatusCode::FAILURE;
  }

  ON_DEBUG debug() << m_PRGraph->dumpDataFlow() << endmsg;
  info() << "PrecedenceSvc initialized successfully" << endmsg;

  return sc;
}

// ============================================================================
StatusCode PrecedenceSvc::iterate(EventSlot& slot, const Cause& cause) {

  if (Cause::source::Task == cause.m_source) {
    ON_VERBOSE verbose() << "Triggering bottom-up traversal at node '"
                         << cause.m_sourceName <<"'" << endmsg;
    auto visitor = concurrency::DecisionUpdater(slot,cause);
    m_PRGraph->getAlgorithmNode(cause.m_sourceName)->accept(visitor);
  } else {
    ON_VERBOSE verbose() << "Triggering top-down traversal at the root node" << endmsg;
    auto visitor = concurrency::Supervisor(slot,cause);
    m_PRGraph->getHeadNode()->accept(visitor);
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
    m_PRGraph->getHeadNode()->accept(visitor);
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
  return (-1 != slot.controlFlowState[m_PRGraph->getHeadNode()->getNodeIndex()] ? true : false);
}

// ============================================================================
void PrecedenceSvc::dumpControlFlow() const {

  info() << std::endl
         << "==================== Control Flow Configuration =================="
         << std::endl << std::endl;
  info() << m_PRGraph->dumpControlFlow() << endmsg;

}
// ============================================================================
void PrecedenceSvc::dumpDataFlow() const {
  info() << std::endl
         << "===================== Data Flow Configuration ===================="
         << std::endl;
  info() << m_PRGraph->dumpDataFlow() << endmsg;
}

// ============================================================================
const std::string PrecedenceSvc::printState(EventSlot& slot) const {

  std::stringstream ss;
  m_PRGraph->printState(ss, slot.algsStates, slot.controlFlowState, 0);
  return ss.str();
}

// ============================================================================
void PrecedenceSvc::dumpPrecedenceTrace() const {
  m_PRGraph->dumpExecutionPlan();
}

// ============================================================================
// Finalize
// ============================================================================
StatusCode PrecedenceSvc::finalize() {
  return Service::finalize();
}
