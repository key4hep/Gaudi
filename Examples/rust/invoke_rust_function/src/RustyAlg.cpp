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

      StatusCode initialize() override {
        return Algorithm::initialize().andThen( [this] { info() << "RustyAlg::initialize()" << endmsg; } );
      }

      StatusCode execute( EventContext const& ) const override {
        info() << "entering RustyAlg::execute()" << endmsg;
        m_stats->increment();                                  // Rust function
        info() << "event count -> " << m_stats->events_count() /* Rust function */
               << endmsg;
        info() << "leaving RustyAlg::execute()" << endmsg;
        return StatusCode::SUCCESS;
      }

      StatusCode finalize() override {
        info() << "total event count -> " << m_stats->events_count() /* Rust function */ << endmsg;
        return Algorithm::finalize();
      }

      rust::Box<JobStats> m_stats = init_job_stats(); // Rust function
    };
    DECLARE_COMPONENT( RustyAlg )
  } // namespace Examples
} // namespace Gaudi
