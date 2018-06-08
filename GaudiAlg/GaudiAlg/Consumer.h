#ifndef GAUDI_FUNCTIONAL_CONSUMER_H
#define GAUDI_FUNCTIONAL_CONSUMER_H

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"
#include "GaudiKernel/apply.h"
#include <utility>

namespace Gaudi
{
  namespace Functional
  {

    template <typename Signature, typename Traits_ = Traits::useDefaults>
    class Consumer;

    template <typename... In, typename Traits_>
    class Consumer<void( const In&... ), Traits_>
        : public details::DataHandleMixin<void, details::filter_evtcontext<In...>, Traits_>
    {
    public:
      using details::DataHandleMixin<void, details::filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute() override final
      {
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
  }
}

#endif
