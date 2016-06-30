#ifndef GAUDIEXAMPLES_READALG_H
#define GAUDIEXAMPLES_READALG_H

// Framework include files
#include "GaudiKernel/Algorithm.h"  // Required for inheritance
#include "GaudiKernel/IIncidentListener.h"  // Required for inheritance

// Forward declarations
class IIncidentSvc;

/** @class ReadAlg ReadAlg.h

    ReadAlg class for the RootIOExample

    @author Markus Frank
*/

class ReadAlg : public Algorithm, virtual public IIncidentListener {
  /// Reference to run records data service
  SmartIF<IDataProviderSvc> m_recordSvc;
  /// Reference to incident service
  SmartIF<IIncidentSvc>     m_incidentSvc;
  StringProperty m_incidentName{this, "IncidentName", "", "incident name of records service"};

public:
  /// Constructor: A constructor of this form must be provided.
  using Algorithm::Algorithm;
  /// Standard Destructor
  virtual ~ReadAlg() = default;
  /// Initialize
  virtual StatusCode initialize();
  /// Finalize
  virtual StatusCode finalize();
  /// Event callback
  virtual StatusCode execute();
  /// IIncidentListener override: Inform that a new incident has occured
  virtual void handle(const Incident& incident);
};

#endif // GAUDIEXAMPLES_READALG_H
