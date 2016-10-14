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
  /// Property: incident name of records service
  std::string       m_incidentName;

public:
  /// Constructor: A constructor of this form must be provided.
  ReadAlg(const std::string& nam, ISvcLocator* pSvc)
    : Algorithm(nam, pSvc), m_recordSvc(0), m_incidentSvc(0) { 
    declareProperty("IncidentName",m_incidentName="");
  }
  /// Standard Destructor
  ~ReadAlg() override = default;
  /// Initialize
  StatusCode initialize() override;
  /// Finalize
  StatusCode finalize() override;
  /// Event callback
  StatusCode execute() override;
  /// IIncidentListener override: Inform that a new incident has occured
  void handle(const Incident& incident) override;
};

#endif // GAUDIEXAMPLES_READALG_H
