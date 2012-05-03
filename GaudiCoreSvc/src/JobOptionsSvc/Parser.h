// $Id:$
// Copyright 2011 alexander.mazurov@gmail.com
#ifndef JOBOPTIONSVC_FILE_PARSER_H_
#define JOBOPTIONSVC_FILE_PARSER_H_
// ============================================================================
// Include files
// ============================================================================
// STD & STL:
// ============================================================================
#include <string>
#include <vector>
#include <map>

#include <fstream>
// ============================================================================
#include "Position.h"
// ============================================================================
namespace Gaudi { namespace Parsers {
// ============================================================================
// Forward declarations:
// ============================================================================
class Messages;
class Node;
class IncludedFiles;
// ============================================================================

bool Parse(const std::string& filename,
        const std::string& search_path, IncludedFiles* included,
        Messages* messages,  Node* root);

bool Parse(const Position& from, const std::string& filename,
        const std::string& search_path, IncludedFiles* included,
        Messages* messages, Node* root);

bool ParseUnits(const Position& from, const std::string& filename,
        const std::string& search_path, IncludedFiles* included,
        Messages* messages, Node* root);
// ============================================================================
}  /* Gaudi */ }  /* Parsers */
// ============================================================================
#endif  // JOBOPTIONSVC_FILE_PARSER_H_
