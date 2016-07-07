#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include <utility>
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiAlg/Algorithm_details.h"

namespace Gaudi { namespace Functional {

   //
   //
   // Adapt a GaudiAlgorithm so that derived classes
   //   a) do not need to access the event store, and have to
   //      state their data dependencies
   //   b) are encouraged not to have state which depends on the events
   //      (eg. histograms, counters will have to be mutable)
   //

   template <typename T> class Transformer;

   // general N -> 1 algorithms

   template <typename Out, typename... In>
   class Transformer<Out(const In&...)> : public GaudiAlgorithm {
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

       template <typename KeyValues, std::size_t... I>
       void declare_input(const KeyValues& inputs, std::index_sequence<I...>) {
           std::initializer_list<int>{
               (this->declareProperty( std::get<I>(inputs).first,
                                       std::get<I>(m_inputs)      ),0)...
           };
       }

       std::tuple<DataObjectHandle<In>...>  m_inputs;
       DataObjectHandle<Out>                m_output;
   };

   namespace Transformer_detail {

      template <std::size_t N, typename Tuple >
      using Out_t = typename std::tuple_element<N, Tuple>::type;

      template <typename... T, typename KeyValues, std::size_t... I>
      auto make_tuple_of_handles_helper( IDataHandleHolder* o, const KeyValues& initvalue, Gaudi::DataHandle::Mode m, std::index_sequence<I...> ) {
          return std::make_tuple( DataObjectHandle<T>(std::get<I>(initvalue).second, m, o) ... );
      }
      template <typename... T, typename KeyValues>
      auto make_tuple_of_handles( IDataHandleHolder* owner, const KeyValues& initvalue, Gaudi::DataHandle::Mode mode) {
          return make_tuple_of_handles_helper<T...>( owner, initvalue, mode, std::make_index_sequence<sizeof...(T)>{} );
      }

      template <typename KeyValues, typename Properties,  std::size_t... I>
      void declare_tuple_of_properties_helper(Algorithm* owner, const KeyValues& inputs, Properties& props,  std::index_sequence<I...>) {
          std::initializer_list<int>{
              (owner->declareProperty( std::get<I>(inputs).first,
                                       std::get<I>(props)         ),0)...
          };
      }

      template <typename KeyValues, typename Properties>
      void declare_tuple_of_properties(Algorithm* owner, const KeyValues& inputs, Properties& props)
      {
          static_assert( std::tuple_size<KeyValues>::value == std::tuple_size<Properties>::value, "Inconsistent lengths" );
          constexpr auto N = std::tuple_size<KeyValues>::value;
          declare_tuple_of_properties_helper( owner, inputs, props, std::make_index_sequence<N>{} );
      }

   }

   template <typename Out, typename... In>
   Transformer<Out(const In&...)>::Transformer( const std::string& name,
                                                ISvcLocator* pSvcLocator,
                                                const std::array<KeyValue,N>& inputs,
                                                const KeyValue& output )
     : GaudiAlgorithm ( name , pSvcLocator ),
       m_inputs( Transformer_detail::make_tuple_of_handles<In...>( this, inputs, Gaudi::DataHandle::Reader ) ),
       m_output( output.second,  Gaudi::DataHandle::Writer, this )
   {
       using Transformer_detail::declare_tuple_of_properties;
       declare_tuple_of_properties( this, inputs, m_inputs);
       declareProperty( output.first, m_output );
   }

   template <typename Out, typename... In>
   template <std::size_t... I>
   StatusCode
   Transformer<Out(const In&...)>::invoke(std::index_sequence<I...>) {
     try {
         using detail::as_const;
         m_output.put( new Out( as_const(*this)( as_const(*std::get<I>(m_inputs).get())... ) ) );
         return StatusCode::SUCCESS;
     } catch ( GaudiException& e ) {
         warning() << "Error during transform: " << e.message() << " returning " << e.code() << endmsg;
         return e.code();
     }
   }


//// TODO: should this be called a MultiTransformer instead??
//
// general N -> M algorithms
//
   template <typename T> class MultiTransformer;

   template <typename ... Out, typename... In>
   class MultiTransformer<std::tuple<Out...>(const In&...)> : public GaudiAlgorithm {
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
         try {
             using Transformer_detail::Out_t;
             using detail::as_const;
             auto out = as_const(*this)( as_const(*std::get<I>(m_inputs).get())... );
             std::initializer_list<int> { 
                  (std::get<O>(m_outputs).put( new Out_t<O,decltype(out)>{ std::move( std::get<O>(out)) } ),0)... 
             };
             return StatusCode::SUCCESS;
         } catch ( GaudiException& e ) {
             warning() << "Error during transform: " << e.message() << " returning " << e.code() << endmsg;
             return e.code();
         }
       }

       std::tuple<DataObjectHandle<In>...>  m_inputs;
       std::tuple<DataObjectHandle<Out>...> m_outputs;
   };


   template <typename... Out, typename... In>
   MultiTransformer<std::tuple<Out...>(const In&...)>::MultiTransformer( const std::string& name,
                                                               ISvcLocator* pSvcLocator,
                                                               const std::array<KeyValue,N_in>& inputs,
                                                               const std::array<KeyValue,N_out>& outputs )
     : GaudiAlgorithm ( name , pSvcLocator ),
       m_inputs( Transformer_detail::make_tuple_of_handles<In...>( this, inputs, Gaudi::DataHandle::Reader ) ),
       m_outputs( Transformer_detail::make_tuple_of_handles<Out...>( this, outputs, Gaudi::DataHandle::Writer ) )
   {
       using Transformer_detail::declare_tuple_of_properties;
       declare_tuple_of_properties( this, inputs, m_inputs );
       declare_tuple_of_properties( this, outputs, m_outputs );
   }

}}

#endif
