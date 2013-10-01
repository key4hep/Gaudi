// Include files

// From Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataStoreAgent.h"
// local
#include "ReplayOutputStream.h"
#include "RecordOutputStream.h"

#include <algorithm>
#include <functional>
#include <list>

// ----------------------------------------------------------------------------
// Implementation file for class: ReplayOutputStream
//
// 30/08/2013: Marco Clemencic
// ----------------------------------------------------------------------------
DECLARE_ALGORITHM_FACTORY(ReplayOutputStream)

// ============================================================================
// Standard constructor, initializes variables
// ============================================================================
ReplayOutputStream::ReplayOutputStream(const std::string& name, ISvcLocator* pSvcLocator)
  : GaudiAlgorithm(name, pSvcLocator)
{
  declareProperty("OutputStreams", m_outputStreamNames,
      "OutputStream instances that can be called.");
}

// ============================================================================
// Destructor
// ============================================================================
ReplayOutputStream::~ReplayOutputStream() {}

namespace {

  /// Helper class to call trigger the transitions in an OutputStream
  template <Gaudi::StateMachine::Transition TR>
  class OutStreamTransition {
  public:
    typedef ReplayOutputStream::OutStreamsMapType::value_type ItemType;
    OutStreamTransition(MsgStream &msg):
      m_msg(msg),
      m_code(StatusCode::SUCCESS, false)
    {}

    void operator() (ItemType &item);

    StatusCode result() const { return m_code; }
  private:
    MsgStream &m_msg;
    StatusCode m_code;
  };

  template <>
  void OutStreamTransition<Gaudi::StateMachine::INITIALIZE>::operator() (ItemType &item) {
    const StatusCode sc = item.second->sysInitialize();
    if (sc.isFailure()) {
      m_msg << MSG::WARNING << "Failed to initialize " << item.first << endmsg;
      m_code = sc;
    }
  }
  template <>
  void OutStreamTransition<Gaudi::StateMachine::START>::operator() (ItemType &item) {
    const StatusCode sc = item.second->sysStart();
    if (sc.isFailure()) {
      m_msg << MSG::WARNING << "Failed to start " << item.first << endmsg;
      m_code = sc;
    }
  }
  template <>
  void OutStreamTransition<Gaudi::StateMachine::STOP>::operator() (ItemType &item) {
    const StatusCode sc = item.second->sysStop();
    if (sc.isFailure()) {
      m_msg << MSG::WARNING << "Failed to stop " << item.first << endmsg;
      m_code = sc;
    }
  }
  template <>
  void OutStreamTransition<Gaudi::StateMachine::FINALIZE>::operator() (ItemType &item) {
    const StatusCode sc = item.second->sysFinalize();
    if (sc.isFailure()) {
      m_msg << MSG::WARNING << "Failed to finalize " << item.first << endmsg;
      m_code = sc;
    }
  }

}

template <Gaudi::StateMachine::Transition TR>
StatusCode ReplayOutputStream::i_outStreamTransition() {
  OutStreamTransition<TR> trans(msg());
  std::for_each(m_outputStreams.begin(), m_outputStreams.end(), trans);
  return trans.result();
}

// ============================================================================
// Initialization
// ============================================================================
StatusCode ReplayOutputStream::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc; // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;

  m_algMgr = service("ApplicationMgr");
  if (UNLIKELY(!m_algMgr.isValid())) {
    return Error("cannot retrieve IAlgManager");
  }

  m_evtMgr = evtSvc();
  if (UNLIKELY(!m_algMgr.isValid())) {
    return Error("cannot retrieve IDataManagerSvc ");
  }

  std::for_each(m_outputStreamNames.begin(), m_outputStreamNames.end(),
                OutStreamAdder(this));

  return i_outStreamTransition<Gaudi::StateMachine::INITIALIZE>();
}

StatusCode ReplayOutputStream::start() {
  StatusCode sc = GaudiAlgorithm::start(); // must be executed first
  if ( sc.isFailure() ) return sc; // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Start" << endmsg;

  return i_outStreamTransition<Gaudi::StateMachine::START>();
}


namespace {
  /// Helper class to collect the names of the subnodes of an entry in the
  /// Transient Event Store.
  struct OutputStreamsCollector: public IDataStoreAgent {
    std::list<std::string> names;
    virtual bool analyse(IRegistry* pRegistry, int lvl) {
      if (lvl > 0)
        names.push_back(pRegistry->name());
      return true;
    }
  };
}

// ============================================================================
// Main execution
// ============================================================================
StatusCode ReplayOutputStream::execute() {
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;

  OutputStreamsCollector collector;
  m_evtMgr->traverseSubTree(RecordOutputStream::locationRoot(), &collector);

  std::for_each(collector.names.begin(), collector.names.end(),
                OutStreamTrigger(this));

  return StatusCode::SUCCESS;
}

// ============================================================================
// Finalize
// ============================================================================
StatusCode ReplayOutputStream::finalize() {
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

  StatusCode sc = i_outStreamTransition<Gaudi::StateMachine::FINALIZE>();

  // release interfaces
  m_outputStreams.clear();
  m_algMgr.reset();
  m_evtMgr.reset();

  StatusCode fsc = GaudiAlgorithm::finalize(); // must be called after all other actions
  if (sc.isSuccess()) sc = fsc;
  return sc;
}

StatusCode ReplayOutputStream::stop() {
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Stop" << endmsg;

  StatusCode sc = i_outStreamTransition<Gaudi::StateMachine::STOP>();

  StatusCode ssc = GaudiAlgorithm::stop(); // must be called after all other actions
  if (sc.isSuccess()) sc = ssc;
  return sc;
}

void ReplayOutputStream::i_addOutputStream(const Gaudi::Utils::TypeNameString &outStream) {
  // we prepend '/' to the name of the algorithm to simplify the handling in
  // OutputStreamsCollector
  const std::string algId = "/" + outStream.name();
  if (m_outputStreams.find(algId) == m_outputStreams.end()) {
    if (!(m_outputStreams[algId] = m_algMgr->algorithm(outStream)).isValid()){
      throw GaudiException(name(), "Could not get algorithm " + outStream.name(),
                           StatusCode::FAILURE);
    }
  } else {
    warning() << "OutputStream instance " << outStream.name()
        << " already added, ignoring " << outStream << endmsg;
  }
}

// ============================================================================
