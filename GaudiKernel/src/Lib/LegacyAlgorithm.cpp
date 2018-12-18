#include <GaudiKernel/Algorithm.h>

#include "GaudiKernel/ThreadLocalContext.h"

namespace Gaudi
{
  namespace details
  {
    bool LegacyAlgorithmAdapter::isExecuted() const
    {
      return execState( Gaudi::Hive::currentContext() ).state() == AlgExecState::State::Done;
    }

    void LegacyAlgorithmAdapter::setExecuted( bool state ) const
    {
      execState( Gaudi::Hive::currentContext() )
          .setState( state ? AlgExecState::State::Done : AlgExecState::State::None );
    }

    void LegacyAlgorithmAdapter::resetExecuted() { execState( Gaudi::Hive::currentContext() ).reset(); }

    bool LegacyAlgorithmAdapter::filterPassed() const
    {
      return execState( Gaudi::Hive::currentContext() ).filterPassed();
    }

    void LegacyAlgorithmAdapter::setFilterPassed( bool state ) const
    {
      execState( Gaudi::Hive::currentContext() ).setFilterPassed( state );
    }
  }
}
