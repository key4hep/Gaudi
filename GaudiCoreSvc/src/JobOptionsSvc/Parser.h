// Copyright 2011 alexander.mazurov@gmail.com
#pragma once
// ============================================================================
// Include files
// ============================================================================
// STD & STL:
// ============================================================================
#include <map>
#include <string>
#include <vector>

#include <fstream>
// ============================================================================
#include "Position.h"
// ============================================================================
namespace Gaudi {
  namespace Parsers {
    // ============================================================================
    // Forward declarations:
    // ============================================================================
    class Messages;
    struct Node;
    class IncludedFiles;
    // ============================================================================

    bool Parse( std::string_view filename, std::string_view search_path, IncludedFiles* included, Messages* messages,
                Node* root );

    bool Parse( const Position& from, std::string_view filename, std::string_view search_path, IncludedFiles* included,
                Messages* messages, Node* root );

    bool ParseUnits( const Position& from, std::string_view filename, std::string_view search_path,
                     IncludedFiles* included, Messages* messages, Node* root );
    // ============================================================================
  } // namespace Parsers
} // namespace Gaudi
// ============================================================================
