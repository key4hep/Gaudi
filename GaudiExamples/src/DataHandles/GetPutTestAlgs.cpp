#include <memory>
#include <string>
#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/EventDataHandle.h"

// ============================================================================
/** @file
 *  Test harness checking that new DataHandles access the whiteboard correctly
 *
 *  @author Hadrien GRASLAND
 *  @date 2017-10-05
 */
// ============================================================================

// === TEST DATA ===

/// Handles must correctly pass around plain-of-data, non-DataObject types
const int TEST_INT = 042;
const std::string TEST_STRING = "Should work as well";

/// Since the current Handle implementation treats DataObjects specially, it
/// must also be tested with payloads that inherit from DataObject, whether
/// that data is movable and normally passed around by value...
class DoubleObject : public DataObject {
  public:
    DoubleObject(double value) : m_value(value) {}

    bool operator!=(const DoubleObject& other) const {
      return (m_value != other.m_value);
    }

  private:
    double m_value;
};
const DoubleObject TEST_DOUBLE_OBJ = DoubleObject(713705.37);

/// ...or not movable, and passed around using a special pointer-to-DataObject
/// code path (that will be removed after code migration is finished).
class NonMovableObject : public DataObject {
  public:
    // This type is neither movable nor copyable, as happens sometimes
    NonMovableObject(NonMovableObject&&) = delete;
    NonMovableObject(const NonMovableObject&) = delete;
    NonMovableObject& operator=(NonMovableObject&&) = delete;
    NonMovableObject& operator=(const NonMovableObject&) = delete;

    // Fundamentally, all it does is to wrap a simpler value type
    NonMovableObject(size_t value) : m_value(value) {}
    size_t value() const { return m_value; }

  private:
    size_t m_value;
};
const size_t TEST_NON_MOVABLE_VALUE = 0xbadbeef;


// === TEST ALGORITHMS ===

/// This algorithm tests supported whiteboard writing scenarios
class WriteHandleTestAlg : public Gaudi::experimental::Algorithm
{
  using Super = Gaudi::experimental::Algorithm;
public:
  // NOTE: Cannot use "using Gaudi::experimental::Algorithm::Algorithm;"
  //       due to a GCC 6 bug
  template<typename... Args>
  WriteHandleTestAlg( Args&&... args )
    : Super( std::forward<Args>(args)... )
  {}

  StatusCode execute() override
  {
    // TODO: Once reentrant Algorithms land in Gaudi, use their explicit context
    auto eventContext = getContext();

    // Test that putting integers in works
    info() << "Writing an int" << endmsg;
    const auto& intVal = m_intWriter.put(eventContext, TEST_INT);
    if(intVal != TEST_INT) {
      throw std::runtime_error("Bad output from writing an int");
    }

    // Test that putting strings in works
    info() << "Writing a string" << endmsg;
    const auto& stringVal = m_stringWriter.put(eventContext, TEST_STRING);
    if(stringVal != TEST_STRING) {
      throw std::runtime_error("Bad output from writing a string");
    }

    // Test that putting DataObject values in works
    info() << "Writing a DataObject by value" << endmsg;
    const auto& doubleVal = m_objWriter.put(eventContext, TEST_DOUBLE_OBJ);
    if(doubleVal != TEST_DOUBLE_OBJ) {
      throw std::runtime_error("Bad output from writing a DataObject value");
    }

    // Test that putting non-movable DataObjects in via a pointer works
    info() << "Writing a DataObject by pointer" << endmsg;
    auto objPtr = std::make_unique<NonMovableObject>(TEST_NON_MOVABLE_VALUE);
    const auto& objVal = m_objPtrWriter.put(eventContext, std::move(objPtr));
    if(objVal.value() != TEST_NON_MOVABLE_VALUE) {
      throw std::runtime_error("Bad output from writing a DataObject pointer");
    }

    return StatusCode::SUCCESS;
  }

private:
  template<typename T>
  using WriteHandle = Gaudi::experimental::EventWriteHandle<T>;

  /// We can write plain old data, like ints...
  WriteHandle<int> m_intWriter{this,
                               "IntOutput",
                               DataObjID( "/Event/MyInt" ),
                               "Integer output"};

  /// ...or strings (note that docstrings are optional)...
  WriteHandle<std::string> m_stringWriter{this,
                                          "StringOutput",
                                          DataObjID( "/Event/MyString" )};

  /// ...or data objects...
  WriteHandle<DoubleObject> m_objWriter{this,
                                        "ObjOutput",
                                        DataObjID( "/Event/MyObj" ),
                                        "Data object output (movable)"};

  /// ...or even non-movable data objects...
  WriteHandle<NonMovableObject> m_objPtrWriter{this,
                                               "ObjPtrOutput",
                                               DataObjID( "/Event/MyObjPtr" ),
                                               "Data object output (via ptr)"};
};

DECLARE_COMPONENT( WriteHandleTestAlg )


/// This algorithm tests supported whiteboard reading scenarios
class ReadHandleTestAlg : public Gaudi::experimental::Algorithm
{
  using Super = Gaudi::experimental::Algorithm;
public:
  // NOTE: Cannot use "using Gaudi::experimental::Algorithm::Algorithm;"
  //       due to a GCC 6 bug
  template<typename... Args>
  ReadHandleTestAlg( Args&&... args )
    : Super( std::forward<Args>(args)... )
  {}

  StatusCode execute() override
  {
    // TODO: Once reentrant Algorithms land in Gaudi, use their explicit context
    auto eventContext = getContext();

    // Test that fetching integers works
    info() << "Reading an int" << endmsg;
    const auto& intVal = m_intReader.get(eventContext);
    if(intVal != TEST_INT) {
      throw std::runtime_error("Bad output from reading an int");
    }

    // Test that fetching strings works
    info() << "Reading a string" << endmsg;
    const auto& stringVal = m_stringReader.get(eventContext);
    if(stringVal != TEST_STRING) {
      throw std::runtime_error("Bad output from reading a string");
    }

    // Test that fetching DataObject values works
    info() << "Reading a DataObject that was inserted by value" << endmsg;
    const auto& doubleVal = m_objReader.get(eventContext);
    if(doubleVal != TEST_DOUBLE_OBJ) {
      throw std::runtime_error("Bad output from reading a DataObject value");
    }

    // Test that putting non-movable DataObjects in via a pointer works
    info() << "Reading a DataObject that was inserted by pointer" << endmsg;
    const auto& objVal = m_objPtrReader.get(eventContext);
    if(objVal.value() != TEST_NON_MOVABLE_VALUE) {
      throw std::runtime_error("Bad output from reading a DataObject pointer");
    }

    // TODO: Once special handling of ranges is implemented, test it as well

    return StatusCode::SUCCESS;
  }

private:
  template<typename T>
  using ReadHandle = Gaudi::experimental::EventReadHandle<T>;

  /// We can read plain old data, like ints...
  ReadHandle<int> m_intReader{this,
                              "IntInput",
                              DataObjID( "/Event/MyInt" ),
                              "Integer input"};

  /// ...or strings...
  ReadHandle<std::string> m_stringReader{this,
                                         "StringInput",
                                         DataObjID( "/Event/MyString" ),
                                         "String input"};

  /// ...or data objects (note that docstrings are optional)...
  ReadHandle<DoubleObject> m_objReader{this,
                                       "ObjInput",
                                       DataObjID( "/Event/MyObj" )};

  /// ...or even non-movable data objects...
  ReadHandle<NonMovableObject> m_objPtrReader{this,
                                              "ObjPtrInput",
                                              DataObjID( "/Event/MyObjPtr" ),
                                              "Data object input (via ptr)"};
};

DECLARE_COMPONENT( ReadHandleTestAlg )
