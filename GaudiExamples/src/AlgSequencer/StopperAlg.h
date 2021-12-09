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
#ifndef GAUDIEXAMPLE_STOPPERALG_H
#define GAUDIEXAMPLE_STOPPERALG_H 1

// Include files
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/MsgStream.h"
#include <Gaudi/Property.h>

/** @class ParentAlg
    Trivial Algorithm for tutotial purposes

    @author nobody
*/
class StopperAlg : public GaudiAlgorithm {
public:
  /// Constructor of this form must be provided
  using GaudiAlgorithm::GaudiAlgorithm;

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  Gaudi::Property<int> m_stopcount{ this, "StopCount", 3 };
};

#endif // GAUDIEXAMPLE_STOPPERALG_H
