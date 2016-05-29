#ifndef FILTER_ALGORITHM_H
#define FILTER_ALGORITHM_H
#include <utility>
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/Algorithm_details.h" // for details::as_const (which is C++17)

template <typename T> class FilterAlgorithm;

template <typename... In>
class FilterAlgorithm<bool(const In&...)> : public GaudiAlgorithm {
public:
    using KeyValue = std::pair<std::string, std::string>; // (name,value) of the  data handle property
    constexpr static std::size_t N = sizeof...(In);       // the number of inputs
    using KeyValues = std::array<KeyValue,N>;

    FilterAlgorithm(const std::string& name, ISvcLocator* locator,
                    const KeyValues& inputs);

    // derived classes are NOT allowed to implement execute ...
    StatusCode execute() override final
    { return invoke(std::make_index_sequence<N>{}); }

    // ... instead, they must implement the following operator
    virtual bool operator()(const In&...) const = 0;

private:

    // note: invoke is not const, as DataObjectHandle<In>::get is not const
    template <std::size_t... I>
    StatusCode invoke(std::index_sequence<I...>) {
        auto pass = detail::as_const(*this)( detail::as_const(*std::get<I>(m_inputs).get())... );
        setFilterPassed( pass );
        return StatusCode::SUCCESS;
    }

    template <typename KeyValues, std::size_t... I>
    void declare(const KeyValues& inputs, std::index_sequence<I...>) {
        std::initializer_list<int>{
            (this->declareProperty( std::get<I>(inputs).first,
                                    std::get<I>(m_inputs)      ),0)...
        };
    }

    std::tuple< DataObjectHandle<In>... > m_inputs;
};

namespace FilterAlgorithm_detail {

   template <typename...  In, typename KeyValues, std::size_t... I>
   auto make_read_handles( IDataHandleHolder* o, const KeyValues& initvalue, std::index_sequence<I...> ) {
       return std::make_tuple( DataObjectHandle<In>(std::get<I>(initvalue).second, Gaudi::DataHandle::Reader, o) ... );
   }

}

template <typename... In>
FilterAlgorithm<bool(const In&...)>::FilterAlgorithm( const std::string& name,
                                        ISvcLocator* pSvcLocator,
                                        const KeyValues& inputs )
  : GaudiAlgorithm ( name , pSvcLocator ),
    m_inputs( FilterAlgorithm_detail::make_read_handles<In...>( this, inputs, std::make_index_sequence<N>{} ) )
{
    declare( inputs, std::make_index_sequence<N>{} );
}

#endif
