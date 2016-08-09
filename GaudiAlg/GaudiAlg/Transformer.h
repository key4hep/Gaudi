#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include <utility>
#include <type_traits>
#include "GaudiKernel/GaudiException.h"
#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"

// Adapt an Algorithm (by default, GaudiAlgorithm) so that derived classes
//   a) do not need to access the event store, and have to explicitly
//      state their data dependencies
//   b) are encouraged not to have state which depends on the events
//      (eg. histograms, counters will have to be mutable)

namespace Gaudi { namespace Functional {

   template <typename Signature,typename Traits_= Traits::useDefaults> class Transformer;

   // general N -> 1 algorithms
   template <typename Out, typename... In, typename Traits_>
   class Transformer<Out(const In&...),Traits_>
   : public details::DataHandleMixin<std::tuple<Out>,std::tuple<In...>,Traits_>
   {
   public:
       using details::DataHandleMixin<std::tuple<Out>,std::tuple<In...>,Traits_>::DataHandleMixin;

       // derived classes can NOT implement execute
       StatusCode execute() final
       { return invoke(std::index_sequence_for<In...>{}); }

       // instead they MUST implement this operator
       virtual Out operator()(const In&...) const = 0;

   private:
       template <std::size_t... I>
       StatusCode invoke(std::index_sequence<I...>) {
           using details::as_const; using details::put;
           try {
               put( std::get<0>(this->m_outputs),  as_const(*this)( as_const(*std::get<I>(this->m_inputs).get())... ) );
           } catch ( GaudiException& e ) {
               this->error() << "Error during transform: " << e.message() << " returning " << e.code() << endmsg;
               return e.code();
           }
           return StatusCode::SUCCESS;
       }
   };

//
// general N -> M algorithms
//
   template <typename Signature,typename Traits_=Traits::useDefaults> class MultiTransformer;

   template <typename ... Out, typename... In, typename Traits_>
   class MultiTransformer<std::tuple<Out...>(const In&...),Traits_>
   : public details::DataHandleMixin<std::tuple<Out...>, std::tuple<In...>,Traits_>
   {
   public:
       using details::DataHandleMixin<std::tuple<Out...>, std::tuple<In...>,Traits_>::DataHandleMixin;

       // derived classes can NOT implement execute
       StatusCode execute() final
       { return invoke(std::index_sequence_for<In...>{},std::index_sequence_for<Out...>{}); }

       // instead they MUST implement this operator
       virtual std::tuple<Out...> operator()(const In&...) const = 0;

   private:
       template <std::size_t... I, std::size_t... O>
       StatusCode invoke(std::index_sequence<I...>,std::index_sequence<O...>) {
           using details::as_const; using details::put;
           try {
               auto out = as_const(*this)( as_const(*std::get<I>(this->m_inputs).get())... );
               std::initializer_list<int> {
                    (put(std::get<O>(this->m_outputs),std::move(std::get<O>(out))),0)...
               };
           } catch ( GaudiException& e ) {
               this->error() << "Error during transform: " << e.message() << " returning " << e.code() << endmsg;
               return e.code();
           }
           return StatusCode::SUCCESS;
       }
   };

}}

#endif
