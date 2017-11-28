/// Test file to ensure that the Plugin Service can work as a drop in
/// replacement for the old one.

#include "GaudiKernel/Algorithm.h"

template <class T>
class AlgFactory;

namespace GaudiTesting
{
  namespace PluginService
  {
    class Algorithm1 : public Algorithm
    {
    private:
      using Algorithm::Algorithm;
      friend class AlgFactory<Algorithm1>;

    public:
      StatusCode execute() override { return StatusCode::SUCCESS; }
    };

    DECLARE_ALGORITHM_FACTORY( Algorithm1 )
    DECLARE_NAMED_ALGORITHM_FACTORY( Algorithm1, Named1 )
  }
}
