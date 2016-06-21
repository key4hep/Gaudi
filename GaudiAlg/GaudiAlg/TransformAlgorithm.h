#ifndef TRANSFORM_ALGORITHM_H
#define TRANSFORM_ALGORITHM_H
#include <utility>
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/Algorithm_details.h"
//
//
// Adapt a GaudiAlgorithm so that derived classes
//   a) do not need to access the event store, and have to
//      state their data dependencies
//   b) are encouraged not to have state which depends on the events
//      (eg. histograms, counters will have to be mutable)
//

template <typename T> class TransformAlgorithm;

// general N -> 1 algorithms

template <typename Out, typename... In>
class TransformAlgorithm<Out(const In&...)> : public GaudiAlgorithm {
public:
    using KeyValue = std::pair<std::string, std::string>;
    constexpr static std::size_t N = sizeof...(In);

    TransformAlgorithm(const std::string& name, ISvcLocator* locator,
                       const std::array<KeyValue,N>& inputs,
                       const KeyValue& output);

    // derived classes can NOT implement execute
    StatusCode execute() override final { return invoke(std::make_index_sequence<N>{}); }

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

namespace TransformAlgorithm_detail {

   template <typename...  In, typename KeyValues, std::size_t... I>
   auto make_read_handles( IDataHandleHolder* o, const KeyValues& initvalue, std::index_sequence<I...> ) {
       return std::make_tuple( DataObjectHandle<In>(std::get<I>(initvalue).second, Gaudi::DataHandle::Reader, o) ... );
   }

}

template <typename Out, typename... In>
TransformAlgorithm<Out(const In&...)>::TransformAlgorithm( const std::string& name,
                                                           ISvcLocator* pSvcLocator,
                                                           const std::array<KeyValue,N>& inputs,
                                                           const KeyValue& output )
  : GaudiAlgorithm ( name , pSvcLocator ),
    m_inputs( TransformAlgorithm_detail::make_read_handles<In...>( this, inputs, std::make_index_sequence<N>{} ) ),
    m_output( output.second,  Gaudi::DataHandle::Writer, this )
{
    declare_input( inputs, std::make_index_sequence<N>{} );
    declareProperty( output.first, m_output );
}

template <typename Out, typename... In>
template <std::size_t... I>
StatusCode
TransformAlgorithm<Out(const In&...)>::invoke(std::index_sequence<I...>) {
    auto result = detail::as_const(*this)( detail::as_const(*std::get<I>(m_inputs).get())... );
    m_output.put( new Out( std::move(result) ) );
    return StatusCode::SUCCESS;
}

#endif
