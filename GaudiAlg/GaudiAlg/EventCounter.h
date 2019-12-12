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
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"

class GAUDI_API EventCounter : public Algorithm {
public:
  /**
   ** Constructor(s)
   **/
  EventCounter( const std::string& name, ISvcLocator* pSvcLocator );

  /*****************************
   ** Public Function Members **
   *****************************/

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  /**************************
   ** Private Data Members **
   **************************/

  /**
   ** The frequency with which the number of events
   ** should be reported. The default is 1, corresponding
   ** to every event.
   **/
  Gaudi::CheckedProperty<int> m_frequency{this, "Frequency", 1};

  /**
   ** The number of events skipped since the last time
   ** the count was reported.
   **/
  int m_skip = 0;

  /**
   ** The total events seen.
   **/
  int m_total = 0;
};
