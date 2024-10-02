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
#pragma once

#include <Gaudi/Algorithm.h>

namespace Gaudi::Rust {
  namespace details {
    // forward declarations
    struct WrappedAlg;
  } // namespace details
  /// Gaudi::Algorithm specialization that wraps an algorithm implemented in Rust.
  ///
  /// An algorithm implemented in Rust will have to provide a factory function (in Rust)
  /// with signature `#[no_mangle] extern "C" fn my_alg_factory() -> *mut WrappedAlg` and
  /// add to a C++ file compiled in a Gaudi module library the following line:
  /// ```cpp
  /// DECLARE_RUST_ALG(my_alg_factory, "Some::Name::Space::MyAlg")
  /// ```
  class AlgWrapper : public Gaudi::Algorithm {
  public:
    AlgWrapper( std::string const& name, ISvcLocator* svcLoc, details::WrappedAlg* dyn_alg_ptr );
    ~AlgWrapper();

    StatusCode initialize() override;
    StatusCode start() override;
    StatusCode execute( const EventContext& ctx ) const override;
    StatusCode stop() override;
    StatusCode finalize() override;

#define common_msg_forward( level )                                                                                    \
  using Algorithm::level;                                                                                              \
  void level( std::string const& msg ) const { level() << msg << endmsg; }
    common_msg_forward( verbose );
    common_msg_forward( debug );
    common_msg_forward( info );
    common_msg_forward( warning );
    common_msg_forward( error );
    common_msg_forward( fatal );
#undef common_msg_forward
    void trace( std::string const& msg ) const { verbose( msg ); }

  private:
    details::WrappedAlg* m_dyn_alg_ptr;
  };
} // namespace Gaudi::Rust

#define DECLARE_RUST_ALG( factory_function, id )                                                                       \
  extern "C" Gaudi::Rust::details::WrappedAlg* factory_function();                                                     \
  namespace {                                                                                                          \
    struct factory_function##_class : Gaudi::Rust::AlgWrapper {                                                        \
      using AlgWrapper::AlgWrapper;                                                                                    \
    };                                                                                                                 \
  }                                                                                                                    \
  namespace Gaudi {                                                                                                    \
    namespace PluginService {                                                                                          \
      GAUDI_PLUGIN_SERVICE_V2_INLINE namespace v2 {                                                                    \
        namespace Details {                                                                                            \
          template <>                                                                                                  \
          struct DefaultFactory<factory_function##_class, Gaudi::Algorithm::Factory> {                                 \
            inline typename Gaudi::Algorithm::Factory::ReturnType operator()( const std::string& name,                 \
                                                                              ISvcLocator*       svcLoc ) {                  \
              return std::make_unique<factory_function##_class>( name, svcLoc, factory_function() );                   \
            }                                                                                                          \
          };                                                                                                           \
        }                                                                                                              \
      }                                                                                                                \
    }                                                                                                                  \
  }                                                                                                                    \
  DECLARE_COMPONENT_WITH_ID( factory_function##_class, id )
