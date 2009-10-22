#ifndef ABORTEVENTALG_H_
#define ABORTEVENTALG_H_
// $Id: AbortEventAlg.h,v 1.1 2007/11/16 18:34:56 marcocle Exp $

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

  virtual ~AbortEventAlg( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

private:
  /// Pointer to the incident service.
  IIncidentSvc *m_incidentSvc;
  /// Event at which to abort.
  long m_count;
  /// Counter of events.
  long m_counter;
};

#endif /*ABORTEVENTALG_H_*/
