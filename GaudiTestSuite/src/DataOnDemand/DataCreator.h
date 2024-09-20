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
#ifndef GAUDIEXAMPLE_DATACREATOR_H
#define GAUDIEXAMPLE_DATACREATOR_H 1

// Include files
#include <GaudiKernel/Algorithm.h>

/** @class DataCreator
    Trivial Algorithm for test purposes
    @author nobody
*/
class DataCreator : public Algorithm {
public:
  /// Constructor of this form must be provided
  using Algorithm::Algorithm;

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  Gaudi::Property<std::string> m_data{ this, "Data", "/Event/Unknown" };
};

#endif // GAUDIEXAMPLE_HELLOWORLD_H
