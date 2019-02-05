#pragma once

#include <string>

namespace Gaudi {
  namespace Parsers {
    /// Helper class to enable ADL for parsers
    struct InputData : std::string {
      InputData( const std::string& s ) : std::string{s} {}
      using std::string::string;
      using std::string::operator=;
    };
  } // namespace Parsers
} // namespace Gaudi
