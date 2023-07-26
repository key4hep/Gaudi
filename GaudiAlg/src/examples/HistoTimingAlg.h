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
#ifndef HISTOGRAMS_HistoTimingAlg_H
#define HISTOGRAMS_HistoTimingAlg_H 1

#include <map>
#include <sstream>

// GaudiKernel
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"

// from Gaudi
#include "GaudiAlg/GaudiHistoAlg.h"

/** @class HistoTimingAlg HistoTimingAlg.h Histograms/HistoTimingAlg.h
 *
 *  Timing tests for various histogramming methods
 *
 *  @author Chris Jones
 *  @date   2005-08-12
 */

class HistoTimingAlg : public GaudiHistoAlg {

public:
  /// Standard constructor
  using GaudiHistoAlg::GaudiHistoAlg;

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution

private:
  Rndm::Numbers m_rand;

  typedef std::map<IHistogram1D*, std::string> HistoMap;

  HistoMap m_histos;

  Gaudi::Property<bool>         m_useGaudiAlg{ this, "UseLookup", false, "" };
  Gaudi::Property<unsigned int> m_nHistos{ this, "NumHistos", 20, "" };
  Gaudi::Property<unsigned int> m_nTracks{ this, "NumTracks", 30, "" };
};

#endif // HISTOGRAMS_HistoTimingAlg_H
