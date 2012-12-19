#ifndef GAUDIEXAMPLES_READALG_H
#define GAUDIEXAMPLES_READALG_H

// Framework include files
#include "GaudiKernel/Algorithm.h"  // Required for inheritance
#include "GaudiKernel/IIncidentListener.h"  // Required for inheritance

// Forward declarations
class IIncidentSvc;

// Event Model related classes
#include "Event.h"
#include "MyTrack.h"
#include "Counter.h"

using namespace Gaudi::Examples;

/** @class ReadHandleAlg ReadHandleAlg.h
*/

class ReadHandleAlg : public Algorithm {

  DataObjectHandle<Collision>* m_inputHandle;  
  std::string m_inputName;
  
public:
  /// Constructor: A constructor of this form must be provided.
  ReadHandleAlg(const std::string& nam, ISvcLocator* pSvc)
    : Algorithm(nam, pSvc){ 
     declareProperty ( "Input", m_inputName, "The name of the input" );
    }
  /// Standard Destructor
  virtual ~ReadHandleAlg() { }
  /// Initialize
  virtual StatusCode initialize();
  /// Finalize
  virtual StatusCode finalize();
  /// Event callback
  virtual StatusCode execute();
};

#endif // GAUDIEXAMPLES_READALG_H
