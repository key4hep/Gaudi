#include "GaudiKernel/Service.h"

#include "MyTool.h"

namespace GaudiTesting {

  /** Special service that issue a failure in one of the transitions (for testing).
   */
  class SvcWithTool : public Service {
  public:
    using Service::Service;

  private:
    PublicToolHandle<IMyTool> m_tool{this, "MyTool", "MyTool"};
  };

  DECLARE_COMPONENT( SvcWithTool )
} // namespace GaudiTesting
