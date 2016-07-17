#ifndef SPLITTING_TRANSFORMER_H
#define SPLITTING_TRANSFORMER_H

#include <vector>
#include <string>
#include <functional>

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"

namespace Gaudi { namespace Functional {

   template <typename Signature, typename Traits_=Traits::useDefaults> class SplittingTransformer;

   template <typename Container> using details::vector_of_<Container>;

   ////// N -> Many of the same one (value of Many not known at compile time, but known at configuration time)
   template <typename Out, typename... In, typename Traits_>
   class SplittingTransformer<vector_of_<Out>(const In&...),Traits_>
   :   public details::BaseClass_t<Traits_> 
   {
       using base_class = details::BaseClass_t<Traits_>;
       static_assert( std::is_base_of<Algorithm,base_class>::value,
                      "BaseClass must inherit from Algorithm");
   public:
       constexpr static std::size_t N_in = sizeof...(In);
       using KeyValue  = std::pair<std::string, std::string>;
       using KeyValues = std::pair<std::string, std::vector<std::string>>;

       SplittingTransformer(const std::string& name, ISvcLocator* locator,
                            const std::array<KeyValue,N_in>& inputs,
                            const KeyValues& output);

       // derived classes can NOT implement execute
       StatusCode execute() override final
       { return invoke(std::make_index_sequence<N_in>{},std::make_index_sequence<N_out>{}); }

       // TODO/FIXME: how does the callee know how many Out to produce, 
       //             and in which order???
       virtual vector_of_<Out> operator()(const In&... ) const = 0;

   private:
       template <std::size_t... I> StatusCode invoke(std::index_sequence<I...>);

       template <typename T> using  InputHandle = details:: InputHandle_t<Traits_,T>;
       template <typename T> using OutputHandle = details::OutputHandle_t<Traits_,T>;

       std::tuple<InputHandle<Out>...> m_inputs;
       std::vector<std::string>        m_outputLocations; //TODO/FIXME  for now: use a call-back to update the actual handles!
       std::vector<OutputHandle<Out>>  m_outputs;
   };


   template <typename Out, typename... In, typename Traits_>
   SplittingTransformer<vector_of_<Out>(const In&...),Traits_>::SplittingTransformer( const std::string& name,
                                                                 ISvcLocator* pSvcLocator,
                                                                 const KeyValues&  inputs,
                                                                 const KeyValue& output )
   :   base_class ( name , pSvcLocator )
   ,   m_inputs( details::make_tuple_of_handles<decltype(m_inputs)>( this, inputs, Gaudi::DataHandle::Reader ) )
   ,   m_outputLocations( outputs.second ),
   {
       this->declareProperty( inputs.first, m_inputs );
       auto p = this->declareProperty( output.first, m_outputLocations );
       p->declareUpdateHandler( [=](Property&) {
           this->m_outputs = details::make_vector_of_handles<decltype(this->m_outputs)>(this, m_outputLocations, Gaudi::DataHandle::Writer);
       } );
       p->useUpdateHandler(); // invoke now, to be sure the input handles are synced with the property...
   }

   template <typename Out, typename... In, typename Traits_>
   template <std::size_t... I>
   StatusCode
   SplittingTransformer<vector_of_<Out>(const In&...),Traits_>::invoke(std::index_sequence<I...>)
   {
       try {
           //TODO:FIXME: how does operator() know the number and order of expected outputs?
           //(note: the MergingTransformer is much easier in this respect, as there the input
           //       is variable instead of the output...)
           using details::as_const;
           auto out = as_const(*this)( as_const(*std::get<I>(m_inputs).get())... );
           if (out.size()!=m_outputs.size()) {
               throw GaudiException( "Error during transform: expected " + std::to_string(m_outputs.size())
                                     " containers, got " + std::to_string(out.size()) + " instead",
                                     this->name(),
                                     StatusCode::FAILURE );
           }
           for (int i=0;i!=out.size();++i) details::put( m_output[i], std::move(out[i]) );
       } catch ( GaudiException& e ) {
           this->error() << "Error during transform: " << e.message() << " returning " << e.code() << endmsg;
           return e.code();
       }
       return StatusCode::SUCCESS;
   }

}}

#endif
