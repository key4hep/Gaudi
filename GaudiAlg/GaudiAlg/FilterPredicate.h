#ifndef FILTER_PREDICATE_H
#define FILTER_PREDICATE_H

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"
#include "GaudiKernel/FunctionalFilterDecision.h"
#include <type_traits>
#include <utility>

namespace Gaudi::Functional {

  namespace details {

    template <typename T, typename Traits_, bool isLegacy>
    struct FilterPredicate;

    template <typename... In, typename Traits_>
    struct FilterPredicate<bool( const In&... ), Traits_, true>
        : DataHandleMixin<std::tuple<>, std::tuple<In...>, Traits_> {
      using DataHandleMixin<std::tuple<>, std::tuple<In...>, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute() override final {
        try {
          if ( filter_evtcontext_t<In...>::apply( *this, this->m_inputs ) ) return FilterDecision::PASSED;
          return FilterDecision::FAILED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // ... instead, they must implement the following operator
      virtual bool operator()( const In&... ) const = 0;
    };

    template <typename... In, typename Traits_>
    struct FilterPredicate<bool( const In&... ), Traits_, false>
        : DataHandleMixin<std::tuple<>, std::tuple<In...>, Traits_> {
      using DataHandleMixin<std::tuple<>, std::tuple<In...>, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute( const EventContext& ctx ) const override final {
        try {
          if ( filter_evtcontext_t<In...>::apply( *this, ctx, this->m_inputs ) ) return FilterDecision::PASSED;
          return FilterDecision::FAILED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // ... instead, they must implement the following operator
      virtual bool operator()( const In&... ) const = 0;
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using FilterPredicate = details::FilterPredicate<Signature, Traits_, details::isLegacy<Traits_>>;

} // namespace Gaudi::Functional

#endif
