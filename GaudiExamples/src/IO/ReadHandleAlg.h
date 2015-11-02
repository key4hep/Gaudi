#ifndef GAUDIEXAMPLES_READALG_H
#define GAUDIEXAMPLES_READALG_H

// Framework include files
#include "GaudiKernel/Algorithm.h"  // Required for inheritance
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

class ReadHandleAlg : public Algorithm {

  DataObjectHandle<Collision> m_inputHandle;

public:
  /// Constructor: A constructor of this form must be provided.
  ReadHandleAlg(const std::string& nam, ISvcLocator* pSvc)
    : Algorithm(nam, pSvc){
     declareInput ( "Input", m_inputHandle);
    }
  /// Standard Destructor
  virtual ~ReadHandleAlg() { }

  /// Event callback
  virtual StatusCode execute();
};

#endif // GAUDIEXAMPLES_READALG_H
