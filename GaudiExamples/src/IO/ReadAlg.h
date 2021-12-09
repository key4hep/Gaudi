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
#ifndef GAUDIEXAMPLES_READALG_H
#define GAUDIEXAMPLES_READALG_H

// Framework include files
#include "GaudiKernel/Algorithm.h"         // Required for inheritance
#include "GaudiKernel/IIncidentListener.h" // Required for inheritance

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
  SmartIF<IIncidentSvc>        m_incidentSvc;
  Gaudi::Property<std::string> m_incidentName{ this, "IncidentName", "", "incident name of records service" };

public:
  /// Constructor: A constructor of this form must be provided.
  using Algorithm::Algorithm;
  /// Initialize
  StatusCode initialize() override;
  /// Finalize
  StatusCode finalize() override;
  /// Event callback
  StatusCode execute() override;
  /// IIncidentListener override: Inform that a new incident has occured
  void handle( const Incident& incident ) override;
};

#endif // GAUDIEXAMPLES_READALG_H
