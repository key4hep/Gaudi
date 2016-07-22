#ifndef FILTER_PREDICATE_H
#define FILTER_PREDICATE_H

#include <utility>
#include <type_traits>
#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"

namespace Gaudi { namespace Functional {

   template <typename T, typename Traits_ = Traits::useDefaults> class FilterPredicate;

   template <typename... In, typename Traits_>
   class FilterPredicate<bool(const In&...),Traits_> : public details::BaseClass_t<Traits_> {
       using base_class = details::BaseClass_t<Traits_>;
       static_assert( std::is_base_of<Algorithm,base_class>::value,
                      "BaseClass must inherit from Algorithm");
   public:
       using KeyValue = std::pair<std::string, std::string>; // (name,value) of the  data handle property
       constexpr static std::size_t N = sizeof...(In);       // the number of inputs
       using KeyValues = std::array<KeyValue,N>;

       // TODO: can we, for N=1, accept KeyValue instead of KeyValues? -- or replace std::array<KeyValue,1> with KeyValue???
       FilterPredicate(const std::string& name, ISvcLocator* locator,
                       const KeyValues& inputs);

       // derived classes are NOT allowed to implement execute ...
       StatusCode execute() override final
       { return invoke(std::make_index_sequence<N>{}); }

       // ... instead, they must implement the following operator
       virtual bool operator()(const In&...) const = 0;

   private:

       // note: invoke is not const, as DataObjectHandle<In>::get is not (yet!) const
       template <std::size_t... I>
       StatusCode invoke(std::index_sequence<I...>) {
           using details::as_const;
           auto pass = as_const(*this)( as_const(*std::get<I>(m_inputs).get())... );
           this->setFilterPassed( pass );
           return StatusCode::SUCCESS;
       }

       template <typename T> using InputHandle = details::InputHandle_t<Traits_,T>;

       std::tuple< InputHandle<In>... > m_inputs;
   };

   template <typename... In, typename Traits_>
   FilterPredicate<bool(const In&...),Traits_>::FilterPredicate( const std::string& name,
                                                                 ISvcLocator* pSvcLocator,
                                                                 const KeyValues& inputs )
     : base_class( name , pSvcLocator ),
       m_inputs( details::make_tuple_of_handles<decltype(m_inputs)>( this, inputs, Gaudi::DataHandle::Reader ) )
   {
       details::declare_tuple_of_properties( this, inputs, m_inputs );
   }

}}

#endif
