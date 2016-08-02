#ifndef MERGING_TRANSFORMER_H
#define MERGING_TRANSFORMER_H

#include <vector>
#include <string>
#include <functional>

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"

namespace Gaudi { namespace Functional {

   template <typename Signature, typename Traits_=Traits::useDefaults> class MergingTransformer;

   using details::vector_of_const_;

   ////// Many of the same -> 1
   template <typename Out, typename In, typename Traits_>
   class MergingTransformer<Out(const vector_of_const_<In>&),Traits_>
   :   public details::BaseClass_t<Traits_>
   {
       using base_class = details::BaseClass_t<Traits_>;
       // the reason to demand that base_class inherits from GaudiAlgorithm
       // is that only it has a working declareProperty for handles, but eg.
       // Algorithm does have a template one that matches, but does the wrong
       // thing...
       static_assert( std::is_base_of<GaudiAlgorithm,base_class>::value,
                      "BaseClass must inherit from GaudiAlgorithm");
   public:
       using KeyValue  = std::pair<std::string, std::string>;
       using KeyValues = std::pair<std::string, std::vector<std::string>>;

       MergingTransformer(const std::string& name, ISvcLocator* locator,
                          const KeyValues& inputs, const KeyValue& output);

       // derived classes can NOT implement execute
       StatusCode execute() final;

       virtual Out operator()(const vector_of_const_<In>& inputs) const = 0;

   private:
       template <typename T> using  InputHandle = details:: InputHandle_t<Traits_,T>;
       template <typename T> using OutputHandle = details::OutputHandle_t<Traits_,T>;

       std::vector<std::string>     m_inputLocations; // TODO/FIXME: remove this duplication...
       std::vector<InputHandle<In>> m_inputs;         //   and make the handles properties instead...
       OutputHandle<Out>            m_output;
   };

   template <typename Out, typename In, typename Traits_>
   MergingTransformer<Out(const vector_of_const_<In>&),Traits_>
   ::MergingTransformer( const std::string& name, ISvcLocator* pSvcLocator,
                         const KeyValues& inputs, const KeyValue& output )
   :   base_class ( name , pSvcLocator )
   ,   m_inputLocations( inputs.second )
   ,   m_output( output.second,  Gaudi::DataHandle::Writer, this )
   {
       // TODO/FIXME: replace vector of string property + call-back with a
       //             vector<handle> property ... as soon as declareProperty can deal with that.
       auto p = this->declareProperty( inputs.first, m_inputLocations );
       p->declareUpdateHandler( [=](Property&) {
           this->m_inputs = details::make_vector_of_handles<decltype(this->m_inputs)>
                            (this, m_inputLocations, Gaudi::DataHandle::Reader);
       } );
       p->useUpdateHandler(); // invoke call-back now, to be sure the input handles are synced with the property...
       std::for_each( this->m_inputs.begin(), this->m_inputs.end(), [&](auto& h) { this->declareInput(&h); } );

       this->declareProperty( output.first, m_output );
   }

   template <typename Out, typename In, typename Traits_>
   StatusCode
   MergingTransformer<Out(const vector_of_const_<In>&),Traits_>::execute()
   {
       vector_of_const_<In> ins; ins.reserve(m_inputs.size());
       std::transform(m_inputs.begin(),m_inputs.end(),std::back_inserter(ins),
                      [&](InputHandle<In>& handle) -> const In&
                      { return *handle.get(); } );
       try {
           using details::as_const;
           details::put( m_output, as_const(*this)( as_const(ins) ) );
       } catch ( GaudiException& e ) {
           this->error() << "Error during transform: " << e.message() << " returning " << e.code() << endmsg;
           return e.code();
       }
       return StatusCode::SUCCESS;
   }

}}

#endif
