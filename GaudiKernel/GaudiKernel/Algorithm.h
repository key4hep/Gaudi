#pragma once
#include <Gaudi/Algorithm.h>
#include <GaudiKernel/ThreadLocalContext.h>

namespace Gaudi {
  namespace details {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
    class LegacyAlgorithmAdapter : public Gaudi::Algorithm {
    public:
      LegacyAlgorithmAdapter( const std::string& name, ISvcLocator* svcloc,
                              const std::string& version = PACKAGE_VERSION )
          : Gaudi::Algorithm( name, svcloc, version ) {
        // default cardinality for non-reentrant algorithms
        setProperty( "Cardinality", 1 );
      }

      virtual StatusCode execute() = 0;

      const EventContext& getContext() const { return Gaudi::Hive::currentContext(); }

      /// Has this algorithm been executed since the last reset?
      bool isExecuted() const;

      /// Set the executed flag to the specified state
      void setExecuted( bool state ) const;

      /** Reset the executed state of the Algorithm for the duration
       *  of the current event.
       */
      [[deprecated( "resetExecuted should be triggered globally via the AlgExecStateSvc, not individually for each "
                    "Algorithm" )]] void
      resetExecuted();

      /// Did this algorithm pass or fail its filter criterion for the last event?
      bool filterPassed() const;

      /// Set the filter passed flag to the specified state
      void setFilterPassed( bool state ) const;

    protected:
      bool isReEntrant() const override { return false; }

    private:
      StatusCode execute( const EventContext& ) const override final {
        return const_cast<LegacyAlgorithmAdapter*>( this )->execute();
      }
    };
#pragma GCC diagnostic pop
  } // namespace details
} // namespace Gaudi

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
/// Alias for backward compatibility
class Algorithm : public Gaudi::details::LegacyAlgorithmAdapter {
public:
  using Gaudi::details::LegacyAlgorithmAdapter::LegacyAlgorithmAdapter;
  // this is to explicitly hide the other signature of `execute`
  StatusCode execute() override = 0;
};
#pragma GCC diagnostic pop
