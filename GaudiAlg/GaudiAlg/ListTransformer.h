#ifndef LISTS_TRANSFORMER_H
#define LISTS_TRANSFORMER_H

#include <functional>
#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"

namespace Gaudi { namespace Functional {

   template <typename Signature, typename Traits_=Traits::useDefaults> class ListTransformer;

   template <typename In>
   using cref_t = std::reference_wrapper<const In>;

   template <typename In>
   using vec_cref_t = std::vector<cref_t<In>>;

   ////// Many of the same -> 1
   template <typename Out, typename In>
   class ListTransformer<Out(const std::vector<cref_t<In>>&)> : public details::BaseClass_t<Traits_> {
       using base_class = details::BaseClass_t<Traits_>;
       static_assert( std::is_base_of<Algorithm,base_class>::value,
                      "BaseClass must inherit from Algorithm");
   public:
       using KeyValue  = std::pair<std::string, std::string>;
       using KeyValues = std::pair<std::string, std::vector<std::string>>;

       ListTransformer(const std::string& name, ISvcLocator* locator,
                       KeyValues inputs,
                       KeyValue output);

       // derived classes can NOT implement execute
       StatusCode execute() override final;

       // @TODO: should we not take an iterable instead of a container?
       virtual Out operator()(const vec_cref_t<In>& inputs) const = 0;

   private:
       template <typename T> using OutputHandle = details::OutputHandle_t<Traits_,T>;

       StringArrayProperty     m_inputs;       //TODO: migrate to data handles array...
       OutputHandle<Out>       m_output;
   };

   template <typename Out, typename In>
   ListTransformer<Out(const vec_cref_t<In>&)>::ListTransformer( const std::string& name,
                                                                 ISvcLocator* pSvcLocator,
                                                                 KeyValues inputs,
                                                                 KeyValue output )
     : base_class ( name , pSvcLocator ),
       m_output( output.second,  Gaudi::DataHandle::Writer, this )
   {
       this->declareProperty( inputs.first, m_inputs = inputs.second );
       this->declareProperty( output.first, m_output );
   }

   template <typename Out, typename In>
   StatusCode
   ListTransformer<Out(const vec_cref_t<In>&)>::execute()
   {
       using details::as_const; using details::put;
       std::vector<cref_t<In>> in; in.reserve(m_inputs.value().size());
       //@NOTE: we explicitly use 'get' so we don't have to worry about missing inputs,
       //       and instead defer its handling to 'get'
       //@FIXME: add an option which skips non-existing locations without error
       std::transform(m_inputs.value().begin(),m_inputs.value().end(),std::back_inserter(in),
                      [&](const std::string& location)   { return std::cref(*this->get<In>(location)); } );
       try {
           put( m_output,  new Out( detail::as_const(*this)( detail::as_const(in) ) ) );
           //@NOTE: we do NOT set filterPassed. This should be done in a seperate, dedicated
           //       algorithm.
       } catch ( GaudiException& e ) {
           this->error() << "Error during transform: " << e.message() << " returning " << e.code() << endmsg;
           return e.code();
       }
       return StatusCode::SUCCESS;
   }

}}

#endif
