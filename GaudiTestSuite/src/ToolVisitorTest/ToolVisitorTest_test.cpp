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
#include "GaudiEnv.h"
#include <Gaudi/Algorithm.h>
#include <Gaudi/PluginService.h>
#include <GaudiKernel/Bootstrap.h>
#include <GaudiKernel/IAlgorithm.h>

#include <regex>
#include <typeinfo>

int main() {
  GaudiTesting::GaudiEnv gaudi;
  const auto&            reg = Gaudi::PluginService::Details::Registry::instance();

  {
    const auto& info  = reg.getInfo( "GaudiTesting::TestAlg", true );
    using Traits      = Gaudi::PluginService::Details::Traits<IAlgorithm*( const std::string&, ISvcLocator* )>;
    using FactoryType = typename Traits::FactoryType;

    DEBUG_TRACE( std::cout << "DEBUG get factory   " << info.library << "\n type : " << info.factory.type().name()
                           << "\n exp. : " << typeid( FactoryType ).name() << std::endl );
    auto alg = std::any_cast<FactoryType>( info.factory )( "TestAlg", &( *( gaudi.m_svcLoc ) ) );
    std::cout << "INFO got alg " << alg->name()
              << " with some inputs renounced. Should not \"have\" implicit circular dependencies." << std::endl;
    alg->addRef();
    if ( alg->sysInitialize().isFailure() ) return 1;
    DEBUG_TRACE( std::cout << "DEBUG finalize " << alg->name() << std::endl );
    if ( alg->sysFinalize().isFailure() ) return 1;
    DEBUG_TRACE( std::cout << "DEBUG FINALIZED " << alg->name() << std::endl );
    alg.reset();
    alg = std::any_cast<FactoryType>( info.factory )( "TestAlgBug", &( *( gaudi.m_svcLoc ) ) );
    alg->addRef();
    std::cout
        << "INFO Now check that initialize will fail when there are circular dependencies which should be the case for "
        << alg->name() << std::endl;
    if ( !alg->sysInitialize().isFailure() ) {
      DEBUG_TRACE( std::cout << "DEBUG finalize " << alg->name() << std::endl );
      if ( alg->sysFinalize().isFailure() ) return 1;
      DEBUG_TRACE( std::cout << "DEBUG FINALIZED " << alg->name() << std::endl );
      return 1;
    }
    std::cout << "INFO Good. Initialize of " << alg->name() << " failed as it should." << std::endl;
    alg.reset();
  }
  std::cout << "INFO Test passed." << std::endl;
  return 0;
}
