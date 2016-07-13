#ifndef LISTS_TRANSFORMER_H
#define LISTS_TRANSFORMER_H

#include <functional>
#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"

namespace Gaudi { namespace Functional {

   template <typename Signature, typename Traits_=Traits::useDefaults> class ListTransformer;


   template <typename Container>
   class vector_of_ {
       std::vector<const Container*> m_containers;
   public:
       using value_type = const Container;
       class iterator {
            typename std::vector<const Container*>::const_iterator m_i;
            friend class vector_of_;
            iterator(typename std::vector<const Container*>::const_iterator iter) : m_i(iter) {}
        public:
            // friend bool operator==(const iterator& lhs, const iterator& rhs) { return lhs.m_i == rhs.m_i; }
            friend bool operator!=(const iterator& lhs, const iterator& rhs) { return lhs.m_i != rhs.m_i; }
            const Container& operator*() const { return **m_i; }
            iterator& operator++() { ++m_i; return *this; }
            iterator& operator--() { --m_i; return *this; }
       };
       void reserve(int size) { m_containers.reserve(size); }
       void push_back(const Container& c) { m_containers.push_back(&c); }
       iterator begin() const { return m_containers.begin(); }
       iterator end() const { return m_containers.end(); }
   };


   ////// Many of the same -> 1 ##TODO/FIXME: rename into 'GatherTransformer'?
   template <typename Out, typename In, typename Traits_>
   class ListTransformer<Out(const vector_of_<In>&),Traits_> : public details::BaseClass_t<Traits_> {
       using base_class = details::BaseClass_t<Traits_>;
       static_assert( std::is_base_of<Algorithm,base_class>::value,
                      "BaseClass must inherit from Algorithm");
   public:
       using KeyValue  = std::pair<std::string, std::string>;
       using KeyValues = std::pair<std::string, std::vector<std::string>>;

       ListTransformer(const std::string& name, ISvcLocator* locator,
                       const KeyValues& inputs,
                       const KeyValue& output);

       // derived classes can NOT implement execute
       StatusCode execute() override final;

       // @TODO: should we not take an iterable (range) instead of a vector of vectors?
       virtual Out operator()(const vector_of_<In>& inputs) const = 0;

   private:
       template <typename T> using  InputHandle = details:: InputHandle_t<Traits_,T>;
       template <typename T> using OutputHandle = details::OutputHandle_t<Traits_,T>;

   //TODO/FIXME    std::vector<InputHandle<In>> m_inputs;
       std::vector<std::string>     m_inputs;
       OutputHandle<Out>            m_output;
   };


   template <typename Out, typename In, typename Traits_>
   ListTransformer<Out(const vector_of_<In>&),Traits_>::ListTransformer( const std::string& name,
                                                                 ISvcLocator* pSvcLocator,
                                                                 const KeyValues&  inputs,
                                                                 const KeyValue& output )
     : base_class ( name , pSvcLocator ),
       //TODO/FIXME m_inputs( details::make_vector_of_handles<decltype(m_inputs)>(this, inputs.second, Gaudi::DataHandle::Reader ) ),
       m_inputs( inputs.second ),
       m_output( output.second,  Gaudi::DataHandle::Writer, this )
   {
       this->declareProperty( inputs.first, m_inputs );
       this->declareProperty( output.first, m_output );
   }

   template <typename Out, typename In, typename Traits_>
   StatusCode
   ListTransformer<Out(const vector_of_<In>&),Traits_>::execute()
   {
       vector_of_<In> ins; ins.reserve(m_inputs.size());
       std::transform(m_inputs.begin(),m_inputs.end(),std::back_inserter(ins),
       // TODO/FIXME    [&](InputHandle<In>& handle) -> const In&
       // TODO/FIXME    { return *handle.get(); } );
                      [&](const std::string& location) -> const In&
                      { return *(this->template get<In>(location)); } );
       try {
           using details::as_const; using details::put;
           put( m_output, as_const(*this)( as_const(ins) ) );
       } catch ( GaudiException& e ) {
           this->error() << "Error during transform: " << e.message() << " returning " << e.code() << endmsg;
           return e.code();
       }
       return StatusCode::SUCCESS;
   }

}}

#endif
