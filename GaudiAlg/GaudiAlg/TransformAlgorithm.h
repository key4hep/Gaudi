#ifndef TRANSFORM_ALGORITHM_H
#define TRANSFORM_ALGORITHM_H
#include "GaudiKernel/index_sequence.h"
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
                       std::array<KeyValue,N> inputs,
                       KeyValue output);
    StatusCode execute() override final; // derived classes can NOT implement execute

    virtual Out operator()(const In&...) const = 0;

private:
    template <std::size_t... I>
    StatusCode invoke(utility::index_sequence<I...>);
    std::array<StringProperty,N>  m_inputs;
    StringProperty                m_output;
};

template <typename Out, typename... In>
TransformAlgorithm<Out(const In&...)>::TransformAlgorithm( const std::string& name,
                                                           ISvcLocator* pSvcLocator,
                                                           std::array<KeyValue,N> inputs,
                                                           KeyValue output )
  : GaudiAlgorithm ( name , pSvcLocator )
{
    auto prop = std::begin(m_inputs);
    for (const auto& arg : inputs) declareProperty( arg.first, *prop++ = arg.second );
    declareProperty( output.first, m_output = output.second );
}

template <typename Out, typename... In>
template <std::size_t... I>
StatusCode
TransformAlgorithm<Out(const In&...)>::invoke(utility::index_sequence<I...>) {
    auto in = std::make_tuple( get<In>(m_inputs[I])... );
    auto result = detail::as_const(*this)( detail::as_const(*std::get<I>(in))... );
    //@TODO: add policy for setFilterPassed
    //       a) just set to true, defer filtering to a dedicated filter algorithm
    //       b) set to true if m_output is not empty
    //       c) have it returned together with result
    //       d) ask derived class, passing it the value of result.size()
    setFilterPassed(!result.empty());
    put( evtSvc(), new Out( std::move(result) ), m_output );
    return StatusCode::SUCCESS;
}

template <typename Out, typename... In>
StatusCode
TransformAlgorithm<Out(const In&...)>::execute() {
    return invoke(utility::make_index_sequence<N>{});
}
#endif
