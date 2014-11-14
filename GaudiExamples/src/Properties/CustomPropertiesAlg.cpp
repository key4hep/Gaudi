// ----------------------------------------------------------------------------
//  Allow to use std::unordered_map as a property
// ----------------------------------------------------------------------------
// Includes needed for the custom type
#include <unordered_map>
#include <string>
// A typedef may save a lot of mistakes
typedef std::unordered_map<std::string, std::string> MyCustomType;

// Define the parser
#include "GaudiKernel/ParsersFactory.h"

namespace Gaudi {
  namespace Parsers {

    // Parser grammar
    template <typename Iterator, typename Skipper>
    struct Grammar_<Iterator, MyCustomType, Skipper > {
      // In this case, the type is a mapping type, so it requires the MapGrammar.
      // For other grammars see GaudiKernel/GrammarsV2.h
      typedef MapGrammar<Iterator, MyCustomType, Skipper> Grammar;
    };

    // Parse function... nothing special, but it must be done explicitely.
    StatusCode parse(MyCustomType& result, const std::string& input) {
        return parse_(result, input);
    }
  }
}

// We also need to be able to print an object of our type as a string that both
// Python and our parser can understand,
#include "GaudiKernel/ToStream.h"
#include <map>
namespace std {
  // This is an example valid for any mapping type.
  ostream& operator<<(ostream& s, const MyCustomType& m) {
    bool first = true;
    s << '{';
    // this is not strictly needed, but it makes the output sorted, which is
    // always nice (in particular for tests)
    const map<string, string> m1(m.begin(), m.end());
    for(const auto& i: m1) {
      if (first) first = false;
      else s << ", ";
      Gaudi::Utils::toStream(i.first, s) << ": ";
      Gaudi::Utils::toStream(i.second, s);
    }
    s << '}';
    return s;
  }
}

// This must be included after the grammar definitions
#include "CustomPropertiesAlg.h"

// ----------------------------------------------------------------------------
// Implementation file for class: CustomPropertiesAlg
//
// 14/11/2014: Marco Clemencic
// ----------------------------------------------------------------------------
namespace Gaudi { namespace Examples {
  DECLARE_COMPONENT(CustomPropertiesAlg)

  // ============================================================================
  // Standard constructor, initializes variables
  // ============================================================================
  CustomPropertiesAlg::CustomPropertiesAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : GaudiAlgorithm(name, pSvcLocator)
  {
    declareProperty("UnorderedMap", m_unorderedMap);
  }

  // ============================================================================
  // Destructor
  // ============================================================================
  CustomPropertiesAlg::~CustomPropertiesAlg() {}

  // ============================================================================
  // Initialization
  // ============================================================================
  StatusCode CustomPropertiesAlg::initialize() {
    StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
    if ( sc.isFailure() ) return sc; // error printed already by GaudiAlgorithm

    if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;

    // TODO initialization procedure

    return StatusCode::SUCCESS;
  }

  // ============================================================================
  // Main execution
  // ============================================================================
  StatusCode CustomPropertiesAlg::execute() {
    if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;

    // TODO execution logic

    return StatusCode::SUCCESS;
  }

  // ============================================================================
  // Finalize
  // ============================================================================
  StatusCode CustomPropertiesAlg::finalize() {
    if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

    // TODO Implement finalize

    return GaudiAlgorithm::finalize(); // must be called after all other actions
  }
}}
// ============================================================================
