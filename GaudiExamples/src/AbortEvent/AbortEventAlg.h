/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
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
  using GaudiAlgorithm::GaudiAlgorithm;

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode finalize() override;   ///< Algorithm finalization

private:
  Gaudi::Property<long> m_count{ this, "AbortedEventNumber", 3, "At which event to trigger an abort" };

  /// Pointer to the incident service.
  SmartIF<IIncidentSvc> m_incidentSvc;
  /// Counter of events.
  long m_counter = 0;
};

#endif /*ABORTEVENTALG_H_*/
