#ifndef COMPONENT_REPLAYOUTPUTSTREAM_H
#define COMPONENT_REPLAYOUTPUTSTREAM_H 1
// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/SmartIF.h"

class IAlgManager;
class IDataManagerSvc;

/** @class ReplayOutputStream ReplayOutputStream.h component/ReplayOutputStream.h
  *
  *
  * @author Marco Clemencic
  * @date 30/08/2013
  */
class ReplayOutputStream: public GaudiAlgorithm {
public:
  /// Standard constructor
  ReplayOutputStream(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~ReplayOutputStream(); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode start     ();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode stop      ();    ///< Algorithm finalization
  virtual StatusCode finalize  ();    ///< Algorithm finalization

  /// Class used to hold the OutputStream instances
  typedef GaudiUtils::HashMap<std::string, SmartIF<IAlgorithm> > OutStreamsMapType;

protected:
private:

  /// Add a new algorithm to the list of OutputStreams
  void i_addOutputStream(const Gaudi::Utils::TypeNameString &outStream);

  /// Helper class to fill the internal map of OutputStreams.
  class OutStreamAdder {
  public:
    OutStreamAdder(ReplayOutputStream *ptr): m_ptr(ptr) {}
    inline void operator() (const Gaudi::Utils::TypeNameString &outStream) {
      m_ptr->i_addOutputStream(outStream);
    }
  private:
    ReplayOutputStream *m_ptr;
  };

  /// Helper class to call the required OutputStream.
  class OutStreamTrigger {
  public:
    OutStreamTrigger(ReplayOutputStream *ptr): m_ptr(ptr) {}
    inline void operator() (const std::string &name) const {
      SmartIF<IAlgorithm> &alg = m_ptr->m_outputStreams[name];
      if (alg.isValid()) {
        if (!alg->isExecuted()) {
          alg->sysExecute();
        } else {
          m_ptr->warning() << name
              << " already executed for the current event" << endmsg;
        }
      } else {
        m_ptr->warning() << "invalid OuputStream " << name << endmsg;
      }
    }
  private:
    ReplayOutputStream *m_ptr;
  };

  /// Helper function to call the transition on the contained OutputStreams.
  /// Returns StatusCode::FAILURE if any of the OutputStreams returned a failure.
  template <Gaudi::StateMachine::Transition TR>
  StatusCode i_outStreamTransition();

  /// (property) Type/Name list of OutputStream we have to call.
  std::vector<std::string> m_outputStreamNames;
  /// Internal storage for the OutputStreams to call.
  OutStreamsMapType m_outputStreams;

  SmartIF<IAlgManager> m_algMgr;
  SmartIF<IDataManagerSvc> m_evtMgr;
};

#endif // COMPONENT_REPLAYOUTPUTSTREAM_H
