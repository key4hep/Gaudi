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
#ifndef SCS_SCSALG_H
#define SCS_SCSALG_H

#include "GaudiKernel/Algorithm.h"

/**
 * @brief  an algorithm to test the StatusCodeSvc
 * @author Charles Leggett, Marco Clemencic
 */
class SCSAlg : public Algorithm {

public:
  using Algorithm::Algorithm;
  StatusCode execute() override;

  StatusCode test();

  void fncChecked();
  void fncUnchecked();
  void fncUnchecked2();
  void fncIgnored();

private:
};

#endif
