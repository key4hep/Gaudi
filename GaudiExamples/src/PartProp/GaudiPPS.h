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
#ifndef PARTPROP_GAUDIPPS_H
#define PARTPROP_GAUDIPPS_H 1

// Include files
// from Gaudi
#include "GaudiKernel/Algorithm.h"

namespace GaudiExamples {
  /** Small algorithm using Gaudi::ParticlePropertySvc.
   *
   *  @author Marco CLEMENCIC
   *  @date   2008-05-23
   */
  class GaudiPPS : public Algorithm {
  public:
    /// Standard constructor
    using Algorithm::Algorithm;

    StatusCode initialize() override; ///< Algorithm initialization
    StatusCode execute() override;    ///< Algorithm execution
  };
} // namespace GaudiExamples
#endif // PARTPROP_GAUDIPPS_H
