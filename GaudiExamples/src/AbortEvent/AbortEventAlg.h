#ifndef ABORTEVENTALG_H_
#define ABORTEVENTALG_H_
// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

class IIncidentSvc;

/** @class AbortEventAlg AbortEventAlg.h
 *  
 *  Small algorithm that after a given number of events triggers an AbortEvent.
 *
 *  @author Marco Clemencic
 *  @date   Nov 16, 2007
 */
class AbortEventAlg : public GaudiAlgorithm {
public: 
  /// Standard constructor
  AbortEventAlg( const std::string& name, ISvcLocator* pSvcLocator );

  ~AbortEventAlg() override; ///< Destructor

  StatusCode initialize() override;    ///< Algorithm initialization
  StatusCode execute   () override;    ///< Algorithm execution
  StatusCode finalize  () override;    ///< Algorithm finalization

protected:

private:
  /// Pointer to the incident service.
  SmartIF<IIncidentSvc> m_incidentSvc;
  /// Event at which to abort.
  long m_count;
  /// Counter of events.
  long m_counter;
};

#endif /*ABORTEVENTALG_H_*/
