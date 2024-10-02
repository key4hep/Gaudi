/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include <Gaudi/Rust/AlgWrapper.h>

extern "C" Gaudi::Rust::details::WrappedAlg* my_rust_counting_alg_factory();

namespace Gaudi {
  namespace PluginService {
    GAUDI_PLUGIN_SERVICE_V2_INLINE namespace v2 {
      namespace Details {
        template <>
        struct DefaultFactory<Gaudi::Rust::AlgWrapper, Gaudi::Algorithm::Factory> {
          inline typename Gaudi::Algorithm::Factory::ReturnType operator()( const std::string& name,
                                                                            ISvcLocator*       svcLoc ) {
            return std::make_unique<Gaudi::Rust::AlgWrapper>( name, svcLoc, my_rust_counting_alg_factory() );
          }
        };
      } // namespace Details
    }
  } // namespace PluginService
} // namespace Gaudi

DECLARE_COMPONENT_WITH_ID( Gaudi::Rust::AlgWrapper, "Gaudi::Examples::MyRustCountingAlg" )

// namespace { ::Gaudi::PluginService::v2::DeclareFactory<Gaudi::Rust::AlgWrapper, RustFactory> _register_25{
// ::Gaudi::PluginService::v2::Details::stringify_id( "Gaudi::Examples::MyRustCountingAlg" ) }; }
