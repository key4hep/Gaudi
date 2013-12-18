// ============================================================================
// Include files
// ============================================================================
// local
// ============================================================================
#include "TupleTool.h"
// ============================================================================

/** @file
 * Implementation file for class : TupleTool
 *
 * @date 2004-06-28
 * @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 */

// ============================================================================
// Declaration of the Tool Factory
// ============================================================================
DECLARE_COMPONENT(TupleTool)
// ============================================================================

// ============================================================================
/** Standard constructor
 *  @see GaudiTupleTool
 *  @see GaudiHistoTool
 *  @see      GaudiTool
 *  @see        AlgTool
 *  @param type   tool type (?)
 *  @param name   tool name
 *  @param parent pointer to parent component
 */
// ============================================================================
TupleTool::TupleTool( const std::string& type,
                      const std::string& name,
                      const IInterface* parent )
  : GaudiTupleTool ( type, name , parent )
{
  declareInterface<ITupleTool>(this);
}


// ============================================================================
/// destructor: virtual and protected
// ============================================================================
TupleTool::~TupleTool() {}
// ============================================================================
