#ifndef FILTER_PREDICATE_H
#define FILTER_PREDICATE_H

#include <utility>
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"

namespace Gaudi { namespace Functional {

   template <typename T, typename Traits = useDataObjectHandle> class FilterPredicate;

   template <typename... In, typename Traits>
   class FilterPredicate<bool(const In&...),Traits> : public GaudiAlgorithm {
   public:
       using KeyValue = std::pair<std::string, std::string>; // (name,value) of the  data handle property
       constexpr static std::size_t N = sizeof...(In);       // the number of inputs
       using KeyValues = std::array<KeyValue,N>;

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
           setFilterPassed( pass );
           return StatusCode::SUCCESS;
       }

       template <typename T> using InputHandle = typename Traits::template InputHandle<T>;

       std::tuple< InputHandle<In>... > m_inputs;
   };

   template <typename... In, typename Traits>
   FilterPredicate<bool(const In&...),Traits>::FilterPredicate( const std::string& name,
                                                                ISvcLocator* pSvcLocator,
                                                                const KeyValues& inputs )
     : GaudiAlgorithm ( name , pSvcLocator ),
       m_inputs( details::make_tuple_of_handles<DataObjectHandle<In>...>( this, inputs, Gaudi::DataHandle::Reader ) )
   {
       details::declare_tuple_of_properties( this, inputs, m_inputs );
   }

}}

#endif
