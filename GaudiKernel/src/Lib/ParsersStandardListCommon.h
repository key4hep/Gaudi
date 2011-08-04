#ifndef PARSERS_STANDARD_LIST_COMMON_H
#define PARSERS_STANDARD_LIST_COMMON_H 1
// ============================================================================
// Include files
// ============================================================================
#include "GaudiKernel/Parsers.h"
#include "GaudiKernel/ParsersFactory.h"
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
#include <vector>
// ============================================================================
// ============================================================================
#define PARSERS_DEF_FOR_LIST(InnerType)\
    StatusCode Gaudi::Parsers::parse(std::vector<InnerType>& result,\
                                     const std::string& input) {\
      return Gaudi::Parsers::parse_(result, input);\
    }
// ============================================================================
#endif /* PARSERS_STANDARD_LIST_COMMON_H */
