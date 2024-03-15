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

namespace GaudiTesting {
  namespace JIRA {

    /** Special service to expose GAUDI-1185.
     */
    class GAUDI_1185 : public Algorithm {
    public:
      using Algorithm::Algorithm;

      StatusCode initialize() override {
        StatusCode sc = Algorithm::initialize();
        if ( !sc ) return sc;

        auto level = msgLevel();
        always() << "Initial msgLevel(): " << int( level ) << endmsg;
        always() << "Current msgLevel(): " << int( msgLevel() ) << endmsg;

        return sc;
      }
      StatusCode execute() override { return StatusCode::SUCCESS; }
    };

    DECLARE_COMPONENT( GAUDI_1185 )
  } // namespace JIRA
} // namespace GaudiTesting
