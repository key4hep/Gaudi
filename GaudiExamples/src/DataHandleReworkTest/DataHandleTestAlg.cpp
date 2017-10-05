#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/DataHandle.h"

/// Experimental algorithm using new-style DataHandles
class DataHandleTestAlg : public Gaudi::experimental::Algorithm
{
  using Super = Gaudi::experimental::Algorithm;
public:
  // NOTE: Cannot use "using Gaudi::experimental::Algorithm::Algorithm;"
  //       due to a GCC 6 bug
  template<typename... Args>
  DataHandleTestAlg( Args&&... args )
    : Super( std::forward<Args>(args)... )
  {}

  StatusCode execute() override;

private:
  Gaudi::experimental::WriteHandle<int>
    m_intWriteHandle{this,
                     "MyIntWrite",
                     DataObjID( "/Event/DefaultMyIntPath" ),
                     "Data being written to"};

  Gaudi::experimental::ReadHandle<int>
    m_intReadHandle{this,
                    "MyIntRead",
                    DataObjID( "/Event/DefaultMyIntPath" ),
                    "Data being read from"};
};

DECLARE_COMPONENT( DataHandleTestAlg )

StatusCode DataHandleTestAlg::execute()
{
  // FIXME: Once reentrant algorithms land in Gaudi, use an explicit context
  auto eventContext = getContext();

  // Try reading and writing data, see what happens
  const int TEST_VALUE = 042;
  const int& r1 = m_intWriteHandle.put(eventContext, TEST_VALUE);
  if(r1 != TEST_VALUE) throw std::runtime_error("This is stupid!");
  const int& r2 = m_intReadHandle.get(eventContext);
  if(r2 != TEST_VALUE) throw std::runtime_error("What is this I don't even");

  // This wouldn't be Gaudi without status codes
  return StatusCode::SUCCESS;
}
