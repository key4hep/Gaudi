#ifndef GAUDIEXAMPLES_READALG_H
#define GAUDIEXAMPLES_READALG_H

// Framework include files
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/IIncidentListener.h"  // Required for inheritance

// Forward declarations
class IIncidentSvc;

// Event Model related classes
#include "GaudiExamples/Event.h"
#include "GaudiExamples/MyTrack.h"
#include "GaudiExamples/Counter.h"

using namespace Gaudi::Examples;

/** @class ReadHandleAlg ReadHandleAlg.h
*/

class ReadHandleAlg : public GaudiAlgorithm {

  DataObjectReadHandle<Collision> m_inputHandle{this, "Input", "/Event/MyCollision"};

public:
  ReadHandleAlg(const std::string& n, ISvcLocator* l): GaudiAlgorithm(n, l) {}

  bool isClonable() const override { return true; }

  /// Event callback
  StatusCode execute() override;
};

#endif // GAUDIEXAMPLES_READALG_H
