// FIXME: Add missing C++ standard library includes
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/EventDataHandle.h"

// ============================================================================
/** @file
 *  Test harness checking that the new DataHandles declare data flow correctly
 *
 *  The following data flow graph is being tested. It happens to be one of
 *  the simplest graphs that checks all useful combinations of 0, 1, and 2
 *  inputs and outputs (ie anything but no inputs and no outputs).
 *
 *  In the schematic below, named boxes are algorithms and numbers are the
 *  values that get passed around between them, which double as graph edge
 *  identifiers. This simplifies implementation, at the cost of breaking
 *  reconfiguration of the data flow graph from the Python test script. For
 *  testing code, I think this is a fine trade-off.
 *
 *     +---+ 1 +---+     2    +---+ 3 +---+
 *     | A |-->|   |--------->|   |-->| C |     A: 0 inputs and 1 output
 *     +---+   |   |          |   |   +---+     B: 0 inputs and 2 outputs
 *             | H |          | E |             C: 1 input  and 0 outputs
 *     +---+ 4 |   | 5 +---+  |   | 6 +---+     D: 1 input  and 1 output
 *     |   |-->|   |-->|   |  |   |-->|   |     E: 1 input  and 2 outputs
 *     |   |   +---+   |   |  +---+   |   |     F: 2 inputs and 0 outputs
 *     | B |           | G |          | F |     G: 2 inputs and 1 output
 *     |   | 7 +---+ 8 |   |    9     |   |     H: 2 inputs and 2 outputs
 *     |   |-->| D |-->|   |--------->|   |
 *     +---+   +---+   +---+          +---+
 *
 *  @author Hadrien GRASLAND
 *  @date 2017-10-05
 */
// ============================================================================

// You may notice that in the graph above, all algorithms will be implemented
// in a similar way. All they do is to make sure that inputs and outputs are
// declared properly, and send/check dummy data around.
//
// So ideally, we would like to write a single algorithm template that we can
// instantiate at will for each of our eight test algorithms, like this:
//
//    using AlgE = DataflowTestAlg<{2}, {3, 6}>;
//
// However, this is not valid C++ today, and we will need to go through some
// ugly metaprogramming tricks to get something similar.
//
// Let's start with what C++ does provides today, namely std::integer_sequence.
// That name is too long for humans to type repeatedly, so let's shorten it.
//
template <int... ids> using IDs = std::integer_sequence<int, ids...>;
//
// Can we now make sure that algorithms get instantiated using syntax like
// "using AlgE = DataflowTestAlg<IDs<2>, IDs<3, 6>>"? Yes, we can. But due to
// more unfortunate C++ limitations, we will also need to use specialization
// for this purpose instead of a pure template declaration.
//
template<typename Inputs, typename Outputs> class DataHandleFlowAlg;
//
template<int... inputIDs,
         int... outputIDs>
class DataHandleFlowAlg<IDs<inputIDs...>, IDs<outputIDs...>>
  : public Gaudi::experimental::Algorithm
{
  using Super = Gaudi::experimental::Algorithm;
public:
  // NOTE: Cannot use "using Gaudi::experimental::Algorithm::Algorithm;"
  //       due to a GCC 6 bug
  template<typename... Args>
  DataHandleFlowAlg( Args&&... args )
    : Super( std::forward<Args>(args)... )
  {}

  /// Run the algorithm, checking the data flow graph and I/O values
  StatusCode execute() override
  {
    // TODO: Once reentrant Algorithms land in Gaudi, use their explicit context
    auto eventContext = getContext();

    // Check that inputs were declared correctly
    info() << "Checking input keys" << endmsg;
    DataObjIDColl expectedInputKeys;
    for(const auto& inputHandle: m_readers) {
      expectedInputKeys.emplace(inputHandle.id());
    }
    if(eventInputKeys() != expectedInputKeys) {
      throw std::runtime_error("Unexpected input keys");
    }

    // Check that outputs were declared correctly
    info() << "Checking output keys" << endmsg;
    DataObjIDColl expectedOutputKeys;
    for(const auto& outputHandle: m_writers) {
      expectedOutputKeys.emplace(outputHandle.id());
    }
    if(eventOutputKeys() != expectedOutputKeys) {
      throw std::runtime_error("Unexpected output keys");
    }

    // Check that we receive the expected input values
    info() << "Checking input values" << endmsg;
    for(size_t i = 0; i < N_inputs; ++i) {
      if(m_readers[i].get(eventContext) != m_inputValues[i]) {
        throw std::runtime_error("Unexpected input #" + std::to_string(i));
      }
    }

    // Send our output values
    info() << "Sending output values" << endmsg;
    for(size_t i = 0; i < N_outputs; ++i) {
      const auto& output = m_outputValues[i];
      if(m_writers[i].put(eventContext, output) != output) {
        throw std::runtime_error("Unexpected output #" + std::to_string(i));
      }
    }

    return StatusCode::SUCCESS;
  }

private:
  // Count our inputs and outputs
  static constexpr size_t N_inputs = sizeof...(inputIDs);
  static constexpr size_t N_outputs = sizeof...(outputIDs);

  // Record our input and output IDs for future use as input and output values
  std::array<int, N_inputs> m_inputValues{inputIDs...};
  std::array<int, N_outputs> m_outputValues{outputIDs...};

  // Declare our inputs using ReadHandles
  using IntReadHandle = Gaudi::experimental::EventReadHandle<int>;
  std::array<IntReadHandle, N_inputs> m_readers{
    IntReadHandle{
      this,
      "Input" + std::to_string(inputIDs),
      DataObjID( "/Event/Int" + std::to_string(inputIDs) )
    }...
  };

  // Declare our outputs using WriteHandles
  using IntWriteHandle = Gaudi::experimental::EventWriteHandle<int>;
  std::array<IntWriteHandle, N_outputs> m_writers{
    IntWriteHandle{
      this,
      "Output" + std::to_string(outputIDs),
      DataObjID( "/Event/Int" + std::to_string(outputIDs) )
    }...
  };
};


// clang-format off

// Generate the algorithms used by the test | Inputs   | Outputs   |
using DataHandleFlowAlgA = DataHandleFlowAlg< IDs<>,     IDs<1>    >;
using DataHandleFlowAlgB = DataHandleFlowAlg< IDs<>,     IDs<4, 7> >;
using DataHandleFlowAlgC = DataHandleFlowAlg< IDs<3>,    IDs<>     >;
using DataHandleFlowAlgD = DataHandleFlowAlg< IDs<7>,    IDs<8>    >;
using DataHandleFlowAlgE = DataHandleFlowAlg< IDs<2>,    IDs<3, 6> >;
using DataHandleFlowAlgF = DataHandleFlowAlg< IDs<6, 9>, IDs<>     >;
using DataHandleFlowAlgG = DataHandleFlowAlg< IDs<5, 8>, IDs<9>    >;
using DataHandleFlowAlgH = DataHandleFlowAlg< IDs<1, 4>, IDs<2, 5> >;

// clang-format on


// Now, make the algorithms accessible via Python
# define DECLARE_COMPONENT_ALIAS( Alg ) DECLARE_COMPONENT_WITH_ID( Alg, #Alg )
DECLARE_COMPONENT_ALIAS( DataHandleFlowAlgA )
DECLARE_COMPONENT_ALIAS( DataHandleFlowAlgB )
DECLARE_COMPONENT_ALIAS( DataHandleFlowAlgC )
DECLARE_COMPONENT_ALIAS( DataHandleFlowAlgD )
DECLARE_COMPONENT_ALIAS( DataHandleFlowAlgE )
DECLARE_COMPONENT_ALIAS( DataHandleFlowAlgF )
DECLARE_COMPONENT_ALIAS( DataHandleFlowAlgG )
DECLARE_COMPONENT_ALIAS( DataHandleFlowAlgH )
