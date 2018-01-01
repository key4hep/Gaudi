#ifndef FILTER_PREDICATE_H
#define FILTER_PREDICATE_H

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"
#include <type_traits>
#include <utility>

namespace Gaudi
{
  namespace Functional
  {

    template <typename T, typename Traits_ = Traits::useDefaults>
    class FilterPredicate;

    template <typename... In, typename Traits_>
    class FilterPredicate<bool( const In&... ), Traits_>
        : public details::DataHandleMixin<void, std::tuple<In...>, Traits_>
    {
    public:
      using details::DataHandleMixin<void, std::tuple<In...>, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute() override final
      {
        try {
          invoke( std::index_sequence_for<In...>{} );
          return StatusCode::SUCCESS;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // ... instead, they must implement the following operator
      virtual bool operator()( const In&... ) const = 0;

    private:
      // note: invoke is not const, as setFilterPassed is not (yet!) const
      template <std::size_t... I>
      void invoke( std::index_sequence<I...> )
      {
        auto pass = details::as_const( *this )( details::deref( std::get<I>( this->m_inputs ).get() )... );
        this->setFilterPassed( pass );
      }
    };
  }
}

#endif
