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
#ifndef GAUDIEXAMPLES_PARTPROPEXA_H
#define GAUDIEXAMPLES_PARTPROPEXA_H 1

#include "GaudiKernel/Algorithm.h"
#include "HepPDT/CommonParticleData.hh"
#include "HepPDT/ProcessUnknownID.hh"

class IPartPropSvc;

class PartPropExa : public Algorithm {

public:
  using Algorithm::Algorithm;
  StatusCode initialize() override;
  StatusCode execute() override;

private:
  IPartPropSvc* m_pps{ nullptr };
};

namespace HepPDT {
  class TestUnknownID : public ProcessUnknownID {
  public:
    TestUnknownID() = default;

    CommonParticleData* processUnknownID( ParticleID, const ParticleDataTable& pdt ) override;
  };
} // namespace HepPDT

#endif // GAUDIEXAMPLES_PARTPROPEXA_H
