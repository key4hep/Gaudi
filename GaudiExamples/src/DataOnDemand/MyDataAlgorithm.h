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
#ifndef GAUDIEXAMPLE_MyDataAlgorithm_H
#define GAUDIEXAMPLE_MyDataAlgorithm_H 1

// Include files
#include "GaudiKernel/Algorithm.h"

/** @class MyDataAlgorithm
    Trivial Algorithm for tutotial purposes
        @author nobody
*/

class MyDataAlgorithm : public Algorithm {
public:
  /// Constructor of this form must be provided
  MyDataAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
};

#endif // GAUDIEXAMPLE_MyDataAlgorithm_H
