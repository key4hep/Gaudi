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
       // if In is a pointer, it signals optional (as opposed to mandatory) input
       template <typename T> using InputHandle_t = details::InputHandle_t<Traits_,typename std::remove_pointer<T>::type>;
       std::vector<std::string>     m_inputLocations; // TODO/FIXME: remove this duplication...
       std::vector<InputHandle_t<In>> m_inputs;  //   and make the handles properties instead...
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
       p->declareUpdateHandler( [=](Gaudi::Details::PropertyBase&) {
           this->m_inputs = details::make_vector_of_handles<decltype(this->m_inputs)>
                            (this, m_inputLocations, Gaudi::DataHandle::Reader);
           if (std::is_pointer<In>::value) { // handle constructor does not (yet) allow to set optional flag... so do it explicitly here...
               std::for_each( this->m_inputs.begin(), this->m_inputs.end(), [](auto& h) { h.setOptional(true); } );
           }
       } );
       p->useUpdateHandler(); // invoke call-back now, to be sure the input handles are synced with the property...
   }

   template <typename Out, typename In, typename Traits_>
   StatusCode
   MergingTransformer<Out(const vector_of_const_<In>&),Traits_>::execute()
   {
       vector_of_const_<In> ins; ins.reserve(m_inputs.size());
       std::transform(m_inputs.begin(),m_inputs.end(),
                      std::back_inserter(ins),
                      details::details2::get_from_handle<In>{} );
       try {
           using details::as_const;
           details::put( std::get<0>(this->m_outputs), as_const(*this)( as_const(ins) ) );
       } catch ( GaudiException& e ) {
           (e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
           return e.code();
       }
       return StatusCode::SUCCESS;
   }

}}

#endif
