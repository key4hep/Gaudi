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

#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/Property.h"

class GAUDI_API Prescaler : public GaudiAlgorithm {

public:
  /**
   ** Constructor(s)
   **/
  Prescaler( std::string name, ISvcLocator* pSvcLocator );

  /*****************************
   ** Public Function Members **
   *****************************/

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  /**
   ** Percentage of events that should be passed
   **/
  Gaudi::CheckedProperty<double> m_percentPass{this, "PercentPass", 100.0,
                                               "percentage of events that should be passed"};

  /**
   ** Number of events passed
   **/
  int m_pass = 0;

  /**
   ** Number of events seen
   **/
  int m_seen = 0;
};
