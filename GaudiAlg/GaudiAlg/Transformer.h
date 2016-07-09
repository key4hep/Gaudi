#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include <utility>
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"

   // Adapt a GaudiAlgorithm so that derived classes
   //   a) do not need to access the event store, and have to
   //      state their data dependencies
   //   b) are encouraged not to have state which depends on the events
   //      (eg. histograms, counters will have to be mutable)
   //

namespace Gaudi { namespace Functional {

   template <typename Signature,typename Traits=useDataObjectHandle> class Transformer;

   // general N -> 1 algorithms

   template <typename Out, typename... In, typename Traits>
   class Transformer<Out(const In&...),Traits> : public GaudiAlgorithm {
   public:
       using KeyValue = std::pair<std::string, std::string>;
       constexpr static std::size_t N = sizeof...(In);

       Transformer(const std::string& name, ISvcLocator* locator,
                   const std::array<KeyValue,N>& inputs,
                   const KeyValue& output);

       // derived classes can NOT implement execute
       StatusCode execute() override final
       { return invoke(std::make_index_sequence<N>{}); }

       // instead they MUST implement this operator
       virtual Out operator()(const In&...) const = 0;

   private:
       template <std::size_t... I>
       StatusCode invoke(std::index_sequence<I...>);

       template <typename T> using InputHandle = typename Traits::template InputHandle<T>;
       template <typename T> using OutputHandle = typename Traits::template OutputHandle<T>;

       std::tuple<InputHandle<In>...>  m_inputs;
       OutputHandle<Out>               m_output;
   };


   template <typename Out, typename... In, typename Traits>
   Transformer<Out(const In&...),Traits>::Transformer( const std::string& name,
                                                       ISvcLocator* pSvcLocator,
                                                       const std::array<KeyValue,N>& inputs,
                                                       const KeyValue& output )
     : GaudiAlgorithm ( name , pSvcLocator ),
       m_inputs( details::make_tuple_of_handles< typename Traits::template InputHandle<In>...>( this, inputs, Gaudi::DataHandle::Reader ) ),
       m_output( output.second,  Gaudi::DataHandle::Writer, this )
   {
       using details::declare_tuple_of_properties;
       declare_tuple_of_properties( this, inputs, m_inputs);
       declareProperty( output.first, m_output );
   }

   template <typename Out, typename... In, typename Traits>
   template <std::size_t... I>
   StatusCode
   Transformer<Out(const In&...),Traits>::invoke(std::index_sequence<I...>) {
       using details::as_const; using details::put;
       try {
           put( m_output,  as_const(*this)( as_const(*std::get<I>(m_inputs).get())... ) );
       } catch ( GaudiException& e ) {
           error() << "Error during transform: " << e.message() << " returning " << e.code() << endmsg;
           return e.code();
       }
       return StatusCode::SUCCESS;
   }



//
// general N -> M algorithms
//
   template <typename T> class MultiTransformer;

   template <typename ... Out, typename... In>
   class MultiTransformer<std::tuple<Out...>(const In&...)> : public GaudiAlgorithm {
       template <typename T> using Handle = DataObjectHandle<T>;
   public:
       using KeyValue = std::pair<std::string, std::string>;
       constexpr static std::size_t N_in = sizeof...(In);
       constexpr static std::size_t N_out = sizeof...(Out);

       MultiTransformer(const std::string& name, ISvcLocator* locator,
                        const std::array<KeyValue,N_in>& inputs,
                        const std::array<KeyValue,N_out>& outputs);

       // derived classes can NOT implement execute
       StatusCode execute() override final
       { return invoke(std::make_index_sequence<N_in>{},std::make_index_sequence<N_out>{}); }

       // instead they MUST implement this operator
       virtual std::tuple<Out...> operator()(const In&...) const = 0;

   private:
       template <std::size_t... I, std::size_t... O>
       StatusCode invoke(std::index_sequence<I...>,std::index_sequence<O...>) {
           using details::as_const; using details::Out_t; using details::put;
           try {
               auto out = as_const(*this)( as_const(*std::get<I>(m_inputs).get())... );
               std::initializer_list<int> {
                    (put(std::get<O>(m_outputs),std::move(std::get<O>(out))),0)...
               };
           } catch ( GaudiException& e ) {
               warning() << "Error during transform: " << e.message() << " returning " << e.code() << endmsg;
               return e.code();
           }
           return StatusCode::SUCCESS;
       }

       std::tuple<Handle<In>...>  m_inputs;
       std::tuple<Handle<Out>...> m_outputs;
   };


   template <typename... Out, typename... In>
   MultiTransformer<std::tuple<Out...>(const In&...)>::MultiTransformer( const std::string& name,
                                                               ISvcLocator* pSvcLocator,
                                                               const std::array<KeyValue,N_in>& inputs,
                                                               const std::array<KeyValue,N_out>& outputs )
     : GaudiAlgorithm ( name , pSvcLocator ),
       m_inputs( details::make_tuple_of_handles< DataObjectHandle<In>...>( this, inputs, Gaudi::DataHandle::Reader ) ),
       m_outputs( details::make_tuple_of_handles< DataObjectHandle<Out>...>( this, outputs, Gaudi::DataHandle::Writer ) )
   {
       using details::declare_tuple_of_properties;
       declare_tuple_of_properties( this, inputs, m_inputs );
       declare_tuple_of_properties( this, outputs, m_outputs );
   }

}}

#endif
