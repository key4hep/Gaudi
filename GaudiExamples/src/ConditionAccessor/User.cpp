#include <Gaudi/Algorithm.h>
#include <Gaudi/Examples/Conditions/ConditionAccessorHolder.h>

namespace Gaudi::Examples::Conditions {
  /// Example of a simple algorithm using conditions via the ConditionAccessorHolder
  /// pattern.
  class UserAlg : public ConditionAccessorHolder<Algorithm> {
    /// Delegate to base class constructor.
    using base_class::base_class;

    /// Data member to access condition values for the current event.
    ConditionAccessor<int> m_cond{this, "MyCondition", "condition/key"};

    StatusCode initialize() override {
      auto status = base_class::initialize();
      if ( !status ) return status;

      info() << "initialize" << endmsg;

      return status;
    }

    StatusCode execute( const EventContext& ctx ) const override {
      // get the condition slot for the current event
      const auto& condCtx = getConditionContext( ctx );
      // get the condition value from the current condition slot.
      const auto& cond = m_cond.get( condCtx );

      info() << "got condition value " << cond << endmsg;

      return StatusCode::SUCCESS;
    }

    StatusCode finalize() override {
      info() << "finalize" << endmsg;

      return base_class::finalize();
    }
  };
  DECLARE_COMPONENT( UserAlg )
} // namespace Gaudi::Examples::Conditions
