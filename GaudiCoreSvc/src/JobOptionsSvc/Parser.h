// Copyright 2011 alexander.mazurov@gmail.com
#pragma once

#include "Position.h"

namespace Gaudi {
  namespace Parsers {
    class Messages;
    struct Node;
    class IncludedFiles;

    bool Parse( std::string_view filename, std::string_view search_path, IncludedFiles* included, Messages* messages,
                Node* root );

    bool Parse( const Position& from, std::string_view filename, std::string_view search_path, IncludedFiles* included,
                Messages* messages, Node* root );

    bool ParseUnits( const Position& from, std::string_view filename, std::string_view search_path,
                     IncludedFiles* included, Messages* messages, Node* root );
  } // namespace Parsers
} // namespace Gaudi
