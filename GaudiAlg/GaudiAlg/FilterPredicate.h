#ifndef FILTER_PREDICATE_H
#define FILTER_PREDICATE_H

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"
#include "GaudiKernel/apply.h"
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
          this->setFilterPassed( details::filter_evtcontext_t<In...>::apply( *this, this->m_inputs ) );
          return StatusCode::SUCCESS;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // ... instead, they must implement the following operator
      virtual bool operator()( const In&... ) const = 0;
    };
  }
}

#endif
