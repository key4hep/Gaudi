/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <string_view>

namespace Gaudi {
  namespace Parsers {
    class Messages;
    class Catalog;
    class Units;
    struct Node;
    class IncludedFiles;
    class PragmaOptions;

    /** Parse and analyze filename, save all messages and properties. Also output
     *  AST tree (http://en.wikipedia.org/wiki/Abstract_syntax_tree).
     *  Returns true if there were no errors during analysis.
     */
    bool ReadOptions( std::string_view filename, std::string_view search_path, Messages* messages, Catalog* catalog,
                      Units* units, PragmaOptions* pragma, Node* root );
  } // namespace Parsers
} // namespace Gaudi
