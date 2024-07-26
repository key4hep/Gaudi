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
#include <Gaudi/Algorithm.h>
#include <Gaudi/FSMCallbackHolder.h>

namespace Gaudi::TestSuite {

  /**
   * Example of usage of callbacks, when using CallbackHolder
   */
  struct FSMCallbackTestAlgo : FSMCallbackHolder<Algorithm> {
    FSMCallbackTestAlgo( const std::string& name, ISvcLocator* pSvcLocator )
        : FSMCallbackHolder<Algorithm>( name, pSvcLocator ) {
      registerCallBack( StateMachine::INITIALIZE, []() { std::cout << "Callback properly called at INITIALIZE\n"; } );
      registerCallBack( StateMachine::START, []() { std::cout << "Callback properly called at START\n"; } );
      registerCallBack( StateMachine::STOP, []() { std::cout << "Callback properly called at STOP\n"; } );
      registerCallBack( StateMachine::FINALIZE, []() { std::cout << "Callback properly called at FINALIZE\n"; } );
    }
    StatusCode execute( const EventContext& ) const override {
      std::cout << "Executing\n";
      return StatusCode::SUCCESS;
    }
  };
  DECLARE_COMPONENT( FSMCallbackTestAlgo );

} // namespace Gaudi::TestSuite
