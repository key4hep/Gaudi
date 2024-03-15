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
#ifndef HISTOGRAMS_GAUDIHISTOALGORITHM_H
#define HISTOGRAMS_GAUDIHISTOALGORITHM_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

// Event Model related classes
#include "GaudiTestSuite/Event.h"
#include "GaudiTestSuite/MyTrack.h"

/** @class GaudiCommonTests GaudiCommonTests.h
 *
 *  Example algorithm for the GaudiCommon class, to test various features.
 *
 *  @author Chris Jones
 *  @date   2005-08-12
 */

class GaudiCommonTests : public GaudiAlgorithm {

public:
  /// Standard constructor
  GaudiCommonTests( const std::string& name, ISvcLocator* pSvcLocator );

  ~GaudiCommonTests() override; ///< Destructor

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode finalize() override;   ///< Algorithm finalization
};

#endif // HISTOGRAMS_GAUDIHISTOALGORITHM_H
