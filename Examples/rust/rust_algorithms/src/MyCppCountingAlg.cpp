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
#include <Gaudi/Algorithm.h>
#include <atomic>

namespace Gaudi::Examples {
  class MyCppCountingAlg : public Algorithm {
  public:
    using Algorithm::Algorithm;

    StatusCode initialize() override {
      return Algorithm::initialize().andThen( [this] {
        info() << "Initialize " << name() << " (C++ )" << endmsg;
        m_count.store( 0 );
      } );
    }
    StatusCode execute( const EventContext& ) const override {
      ++m_count;
      info() << "counted " << m_count.load() << " events " << endmsg;
      return StatusCode::SUCCESS;
    }
    StatusCode finalize() override {
      info() << "Finalize " << name() << ": count = " << m_count.load() << endmsg;
      return Algorithm::finalize();
    }

  private:
    mutable std::atomic<std::size_t> m_count{ 0 };
  };
  DECLARE_COMPONENT( MyCppCountingAlg )
} // namespace Gaudi::Examples
