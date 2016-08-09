#ifndef GAUDI_FUNCTIONAL_CONSUMER_H
#define GAUDI_FUNCTIONAL_CONSUMER_H

#include <utility>
#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"

namespace Gaudi { namespace Functional {

   template <typename Signature, typename Traits_ = Traits::useDefaults> class Consumer;

   template <typename... In, typename Traits_>
   class Consumer<void(const In&...),Traits_>
   : public details::DataHandleMixin<void,std::tuple<In...>,Traits_> {
   public:
       using details::DataHandleMixin<void,std::tuple<In...>,Traits_>::DataHandleMixin;

       // derived classes are NOT allowed to implement execute ...
       StatusCode execute() final
       { return invoke(std::index_sequence_for<In...>{}); }

       // ... instead, they must implement the following operator
       virtual void operator()(const In&...) const = 0;

   private:
       template <std::size_t... I>
       StatusCode invoke(std::index_sequence<I...>) const {
           (*this)( details::as_const(*std::get<I>(this->m_inputs).get())... );
           return StatusCode::SUCCESS;
       }
   };
}}

#endif
