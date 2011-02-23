//$Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiExamples/src/POOLIO/ReadAlg.h,v 1.2 2008/11/04 22:49:24 marcocle Exp $       //
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
  IDataProviderSvc* m_recordSvc;
  /// Reference to incident service
  IIncidentSvc*     m_incidentSvc;
  /// Property: incident name of records service
  std::string       m_incidentName;

public:
  /// Constructor: A constructor of this form must be provided.
  ReadAlg(const std::string& nam, ISvcLocator* pSvc)
    : Algorithm(nam, pSvc), m_recordSvc(0), m_incidentSvc(0) { 
    declareProperty("IncidentName",m_incidentName="");
  }
  /// Standard Destructor
  virtual ~ReadAlg() { }
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
