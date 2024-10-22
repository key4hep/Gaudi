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
#include <GaudiKernel/Algorithm.h>

#include <GaudiKernel/ThreadLocalContext.h>

namespace Gaudi {
  namespace details {
    bool LegacyAlgorithmAdapter::isExecuted() const {
      return execState( Gaudi::Hive::currentContext() ).state() == AlgExecState::State::Done;
    }

    void LegacyAlgorithmAdapter::setExecuted( bool state ) const {
      execState( Gaudi::Hive::currentContext() )
          .setState( state ? AlgExecState::State::Done : AlgExecState::State::None );
    }

    bool LegacyAlgorithmAdapter::filterPassed() const {
      return execState( Gaudi::Hive::currentContext() ).filterPassed();
    }

    void LegacyAlgorithmAdapter::setFilterPassed( bool state ) const {
      execState( Gaudi::Hive::currentContext() ).setFilterPassed( state );
    }
  } // namespace details
} // namespace Gaudi
