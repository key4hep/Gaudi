/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Algorithm.h>
#include <example_rust_lib_bridge/lib.h>

namespace Gaudi {
  namespace Examples {
    /// @brief Simple algorithm that invokes Rust code
    class RustyAlg : public Gaudi::Algorithm {
    public:
      using Algorithm::Algorithm;

      StatusCode initialize() override { return Algorithm::initialize(); }

      StatusCode execute( EventContext const& ) const override {
        info() << "entering RustyAlg::execute()" << endmsg;
        auto result = add( 40, 2 ); // some_rust_function();
        info() << "some_rust_function returned " << result << endmsg;
        info() << "leaving RustyAlg::execute()" << endmsg;
        return StatusCode::SUCCESS;
      }

      StatusCode finalize() override { return Algorithm::finalize(); }
    };
    DECLARE_COMPONENT( RustyAlg )
  } // namespace Examples
} // namespace Gaudi
