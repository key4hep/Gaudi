#include <algorithm>
#include <vector>
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/DataHandle.h"

/// Experimental Algorithm that can work with new-style DataHandles
class ExtendedAlg : public Algorithm {
public:
  using Algorithm::Algorithm;

  /// (For handles) Register a data handle as an input of the algorithm
  void registerInputHandle(Gaudi::experimental::DataHandle& handle) {
    m_inputHandles.push_back(&handle);
  }

  /// (For handles) Register a data handle as an output of the algorithm
  void registerOutputHandle(Gaudi::experimental::DataHandle& handle) {
    m_outputHandles.push_back(&handle);
  }

  /// (For scheduler) Tell which keys the algorithm will be reading from
  DataObjIDColl getInputKeys() const {
    return getKeys(m_inputHandles);
  }

  /// (For scheduler) Tell which keys the algorithm will be writing from
  DataObjIDColl getOutputKeys() const {
    return getKeys(m_outputHandles);
  }

  /// Initialize handles once the whiteboard is ready
  StatusCode initialize() override {
    initializeHandles(m_inputHandles);
    initializeHandles(m_outputHandles);
    return StatusCode::SUCCESS;
  }

private:
  using HandlePtr = Gaudi::experimental::DataHandle*;
  using HandleList = std::vector<HandlePtr>;
  HandleList m_inputHandles;
  HandleList m_outputHandles;

  /// Query the keys of a set of handles
  static DataObjIDColl getKeys(const HandleList& handles) {
    DataObjIDColl result;
    result.reserve(handles.size());
    for(auto handlePtr: handles) {
      result.insert(handlePtr->id());
    }
    return result;
  }

  /// Initialize a set of handles
  static void initializeHandles(HandleList& handles) {
    for(auto& handlePtr: handles) {
      handlePtr->initialize();
    }
  }
};


/// Experimental algorithm using new-style DataHandles
class DataHandleTestAlg : public ExtendedAlg
{
public:
  // NOTE: Cannot use "using ExtendedAlg::ExtendedAlg;" due to a GCC 6 bug
  template<typename... Args>
  DataHandleTestAlg( Args&&... args )
    : ExtendedAlg( std::forward<Args>(args)... )
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
