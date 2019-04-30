#ifndef GAUDI_FUNCTIONAL_CONSUMER_H
#define GAUDI_FUNCTIONAL_CONSUMER_H

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"
#include <utility>

namespace Gaudi::Functional {

  namespace details {

    template <typename Signature, typename Traits_, bool isLegacy>
    class Consumer;

    template <typename... In, typename Traits_>
    class Consumer<void( const In&... ), Traits_, true>
        : public details::DataHandleMixin<void, details::filter_evtcontext<In...>, Traits_> {
    public:
      using details::DataHandleMixin<void, details::filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute() override final {
        try {
          details::filter_evtcontext_t<In...>::apply( *this, this->m_inputs );
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
        return StatusCode::SUCCESS;
      }

      // ... instead, they must implement the following operator
      virtual void operator()( const In&... ) const = 0;
    };

    template <typename... In, typename Traits_>
    class Consumer<void( const In&... ), Traits_, false>
        : public details::DataHandleMixin<void, details::filter_evtcontext<In...>, Traits_> {
    public:
      using details::DataHandleMixin<void, details::filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute( const EventContext& ctx ) const override final {
        try {
          details::filter_evtcontext_t<In...>::apply( *this, ctx, this->m_inputs );
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
        return StatusCode::SUCCESS;
      }

      // ... instead, they must implement the following operator
      virtual void operator()( const In&... ) const = 0;
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using Consumer = details::Consumer<Signature, Traits_, details::isLegacy<Traits_>>;

} // namespace Gaudi::Functional

#endif
