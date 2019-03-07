#include "Validators.h"

#include "../PrecedenceRulesGraph.h"

namespace concurrency {
  //---------------------------------------------------------------------------
  bool NodePropertiesValidator::visit( DecisionNode& node ) {

    if ( node.m_modeConcurrent && node.m_modePromptDecision ) {

      if ( !m_foundViolations )
        m_status
            << "  'Concurrent'/'Prompt' contradiction(s) found. Settings are mutually exclusive within a task group. "
               "Discarding 'Prompt' for ";

      m_status << ( m_foundViolations ? ", " : "" ) << node.getNodeName();

      if ( !m_foundViolations ) m_foundViolations = true;

      return false;
    }

    return true;
  }
} // namespace concurrency
