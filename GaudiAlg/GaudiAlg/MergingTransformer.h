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
   :   public details::DataHandleMixin<std::tuple<Out>,void,Traits_>
   {
       using base_class = details::DataHandleMixin<std::tuple<Out>,void,Traits_>;
   public:
       using KeyValue  = std::pair<std::string, std::string>;
       using KeyValues = std::pair<std::string, std::vector<std::string>>;

       MergingTransformer(const std::string& name, ISvcLocator* locator,
                          const KeyValues& inputs, const KeyValue& output);

       // derived classes can NOT implement execute
       StatusCode execute() final;

       virtual Out operator()(const vector_of_const_<In>& inputs) const = 0;

   private:
       std::vector<std::string>     m_inputLocations; // TODO/FIXME: remove this duplication...
       std::vector<details::InputHandle_t<Traits_,In>> m_inputs;  //   and make the handles properties instead...
   };

   template <typename Out, typename In, typename Traits_>
   MergingTransformer<Out(const vector_of_const_<In>&),Traits_>
   ::MergingTransformer( const std::string& name, ISvcLocator* pSvcLocator,
                         const KeyValues& inputs, const KeyValue& output )
   :   base_class( name , pSvcLocator, output )
   ,   m_inputLocations( inputs.second )
   {
       // TODO/FIXME: replace vector of string property + call-back with a
       //             vector<handle> property ... as soon as declareProperty can deal with that.
       auto p = this->declareProperty( inputs.first, m_inputLocations );
       p->declareUpdateHandler( [=](Property&) {
           //@FIXME: if any handles, de-register ('retract') them first!
           // std::for_each( this->m_inputs.begin(), this->m_inputs.end(), [&](auto& h) { this->retractInput(&h); } );
           if (!this->m_inputs.empty()) {
              this->warning() << "DataHandle property about to be updated, some DataHandles are about to run out of scope. This will cause a crash later..." << endmsg;
           }
           this->m_inputs = details::make_vector_of_handles<decltype(this->m_inputs)>
                            (this, m_inputLocations, Gaudi::DataHandle::Reader);
           std::for_each( this->m_inputs.begin(), this->m_inputs.end(), [&](auto& h) { this->declareInput(&h); } );
       } );
       p->useUpdateHandler(); // invoke call-back now, to be sure the input handles are synced with the property...
   }

   template <typename Out, typename In, typename Traits_>
   StatusCode
   MergingTransformer<Out(const vector_of_const_<In>&),Traits_>::execute()
   {
       vector_of_const_<In> ins; ins.reserve(m_inputs.size());
       std::transform(m_inputs.begin(),m_inputs.end(),std::back_inserter(ins),
                      [&](const auto& handle) -> const In&
                      { return *handle.get(); } );
       try {
           using details::as_const;
           details::put( std::get<0>(this->m_outputs), as_const(*this)( as_const(ins) ) );
       } catch ( GaudiException& e ) {
           this->error() << "Error during transform: " << e.message() << " returning " << e.code() << endmsg;
           return e.code();
       }
       return StatusCode::SUCCESS;
   }

}}

#endif
