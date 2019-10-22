#include "GaudiKernel/FunctionalFilterDecision.h"

namespace Gaudi::Functional {

  struct FilterDecisionErrorCategory : StatusCode::Category {
    const char* name() const override { return "FilterDecision"; }
    bool        isSuccess( StatusCode::code_t code ) const override {
      return ( static_cast<FilterDecision>( code ) == FilterDecision::PASSED or
               static_cast<FilterDecision>( code ) == FilterDecision::FAILED );
    }
    bool        isRecoverable( StatusCode::code_t ) const override { return false; }
    std::string message( StatusCode::code_t code ) const override {
      switch ( static_cast<FilterDecision>( code ) ) {
      case FilterDecision::PASSED:
        return "Filter decision: PASSED";
      case FilterDecision::FAILED:
        return "Filter decision: FAILED";
      default:
        return StatusCode::default_category().message( code );
      }
    }
  };
} // namespace Gaudi::Functional

STATUSCODE_ENUM_IMPL( Gaudi::Functional::FilterDecision, Gaudi::Functional::FilterDecisionErrorCategory )
