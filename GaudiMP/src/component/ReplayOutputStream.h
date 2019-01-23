#ifndef COMPONENT_REPLAYOUTPUTSTREAM_H
#define COMPONENT_REPLAYOUTPUTSTREAM_H 1
// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/ThreadLocalContext.h"

class IAlgManager;
struct IDataManagerSvc;

/** @class ReplayOutputStream ReplayOutputStream.h component/ReplayOutputStream.h
  *
  *
  * @author Marco Clemencic
  * @date 30/08/2013
  */
class ReplayOutputStream : public GaudiAlgorithm
{
public:
  /// Inherited constructor
  using GaudiAlgorithm::GaudiAlgorithm;

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode start() override;      ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode stop() override;       ///< Algorithm finalization
  StatusCode finalize() override;   ///< Algorithm finalization

  /// Class used to hold the OutputStream instances
  typedef GaudiUtils::HashMap<std::string, SmartIF<IAlgorithm>> OutStreamsMapType;

private:
  /// Add a new algorithm to the list of OutputStreams
  void i_addOutputStream( const Gaudi::Utils::TypeNameString& outStream );

  /// Helper class to fill the internal map of OutputStreams.
  class OutStreamAdder
  {
  public:
    OutStreamAdder( ReplayOutputStream* ptr ) : m_ptr( ptr ) {}
    inline void operator()( const Gaudi::Utils::TypeNameString& outStream ) { m_ptr->i_addOutputStream( outStream ); }

  private:
    ReplayOutputStream* m_ptr;
  };

  /// Helper function to call the transition on the contained OutputStreams.
  /// Returns StatusCode::FAILURE if any of the OutputStreams returned a failure.
  template <Gaudi::StateMachine::Transition TR>
  StatusCode                                i_outStreamTransition();

  Gaudi::Property<std::vector<std::string>> m_outputStreamNames{
      this, "OutputStreams", {}, "OutputStream instances that can be called."};

  /// Internal storage for the OutputStreams to call.
  OutStreamsMapType m_outputStreams;

  SmartIF<IAlgManager>     m_algMgr;
  SmartIF<IDataManagerSvc> m_evtMgr;
};

#endif // COMPONENT_REPLAYOUTPUTSTREAM_H
