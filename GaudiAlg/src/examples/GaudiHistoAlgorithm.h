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
#include "GaudiAlg/GaudiHistoAlg.h"

/** @class GaudiHistoAlgorithm GaudiHistoAlgorithm.h Histograms/GaudiHistoAlgorithm.h
 *
 *  Example algorithm for the GaudiHistoAlg class
 *  The same functionality is available in GaudiHistoTool
 *
 *  @author Chris Jones
 *  @date   2005-08-12
 */

class GaudiHistoAlgorithm : public GaudiHistoAlg {

public:
  /// Standard constructor
  GaudiHistoAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );

  ~GaudiHistoAlgorithm() override; ///< Destructor

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode finalize() override;   ///< Algorithm finalization
};

#endif // HISTOGRAMS_GAUDIHISTOALGORITHM_H
