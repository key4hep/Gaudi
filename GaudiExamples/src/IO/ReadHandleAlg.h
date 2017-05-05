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

  DataObjectHandle<Collision> m_inputHandle;

public:
  /// Constructor: A constructor of this form must be provided.
  ReadHandleAlg(const std::string& nam, ISvcLocator* pSvc);

  bool isClonable() const override { return true; }

  /// Standard Destructor
  ~ReadHandleAlg() override = default;

  /// Event callback
  StatusCode execute() override;
};

#endif // GAUDIEXAMPLES_READALG_H
