// $Id:$
#ifndef JOBOPTIONSVC_ANALYZER_H_
#define JOBOPTIONSVC_ANALYZER_H_
// ============================================================================
// Include files
// ============================================================================
// STD & STL:
// ============================================================================
#include <string>
#include <vector>
// ============================================================================
namespace Gaudi { namespace Parsers {
// ============================================================================
// Forward declarations
// ============================================================================
class Messages;
class Catalog;
class Units;
class Node;
class IncludedFiles;
class PragmaOptions;
// ============================================================================


/** Parse and analyze filename, save all messages and properties. Also output
 *  AST tree (http://en.wikipedia.org/wiki/Abstract_syntax_tree).
 *  Returns true if there were no errors during analysis.
 */
bool ReadOptions(const std::string& filename, const std::string& search_path,
        Messages* messages, Catalog* catalog, Units* units,
        PragmaOptions* pragma, Node* root);
// ============================================================================
}}
#endif  // JOBOPTIONSVC_ANALYZER_H_
