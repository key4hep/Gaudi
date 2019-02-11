#ifndef PARSERS_STANDARD_LIST_COMMON_H
#define PARSERS_STANDARD_LIST_COMMON_H 1
// ============================================================================
// Include files
// ============================================================================
#include <Gaudi/Parsers/CommonParsers.h>
#include <Gaudi/Parsers/Factory.h>
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
#include <vector>
// ============================================================================
// ============================================================================
#define PARSERS_DEF_FOR_LIST( InnerType )                                                                              \
  StatusCode Gaudi::Parsers::parse( std::vector<InnerType>& result, const std::string& input ) {                       \
    return Gaudi::Parsers::sparse<>::parse_( result, input );                                                                    \
  }
// ============================================================================
#endif /* PARSERS_STANDARD_LIST_COMMON_H */
