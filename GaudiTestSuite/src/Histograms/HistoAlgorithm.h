/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef HISTOGRAMS_HISTOALGORITHM_H
#define HISTOGRAMS_HISTOALGORITHM_H 1

// Include files
#include <GaudiKernel/Algorithm.h>

#include <AIDA/IAxis.h>
#include <AIDA/IHistogram1D.h>
#include <AIDA/IHistogram2D.h>
#include <AIDA/IHistogram3D.h>
#include <AIDA/IHistogramFactory.h>
#include <AIDA/IProfile1D.h>
#include <AIDA/IProfile2D.h>

using namespace AIDA;
// Forward declarations
class HistogramSvc;

class HistoAlgorithm : public Algorithm {

public:
  // Constructor of this form must be provided
  HistoAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );

  // Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  IHistogram1D* m_h1D;
  IHistogram1D* m_h1DVar;
  IHistogram2D* m_h2D;
  IHistogram2D* m_h2DVar;
  IHistogram3D* m_h3D;
  IHistogram3D* m_h3DVar;
  IProfile1D*   m_p1D;
  IProfile2D*   m_p2D;
};

#endif // HISTOGRAMS_HISTOALGORITHM_H
