#ifndef VALIDATORS_H_
#define VALIDATORS_H_

#include "IGraphVisitor.h"

#include <sstream>

namespace concurrency {

  //--------------------------------------------------------------------------
  class NodePropertiesValidator : public IGraphVisitor {
  public:
    using IGraphVisitor::visit;
    using IGraphVisitor::visitEnter;

    bool visitEnter( AlgorithmNode& ) const override { return false; };
    bool visitEnter( DataNode& ) const override { return false; };
    bool visitEnter( ConditionNode& ) const override { return false; };

    bool visit( DecisionNode& ) override;

    std::string reply() const { return m_status.str(); };
    bool        passed() const { return !m_foundViolations; };

  private:
    std::ostringstream m_status{"  No 'Concurrent'/'Prompt' contradictions found"};
    bool               m_foundViolations{false};
  };

} // namespace concurrency

#endif /* VALIDATORS_H_ */
