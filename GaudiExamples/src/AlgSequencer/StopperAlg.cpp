/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Algorithm.h>
#include <GaudiKernel/IEventProcessor.h>

struct StopperAlg : Gaudi::Algorithm {
  using Algorithm::Algorithm;

  StatusCode execute( const EventContext& ) const override {
    static int count = 0;

    if ( ++count >= m_stopcount ) {
      info() << "scheduling a event processing stop...." << endmsg;
      auto evt = service<IEventProcessor>( "ApplicationMgr" );
      if ( evt->stopRun().isFailure() ) {
        error() << "unable to schedule a stopRun" << endmsg;
        return StatusCode::FAILURE;
      }
    }

    return StatusCode::SUCCESS;
  }

  Gaudi::Property<int> m_stopcount{ this, "StopCount", 3 };
};

DECLARE_COMPONENT( StopperAlg )
