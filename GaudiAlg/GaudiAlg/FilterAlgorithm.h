#include "GaudiKernel/index_sequence.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/Algorithm_details.h"

template <typename T> class FilterAlgorithm;

template <typename... In>
class FilterAlgorithm<void(const In&...)> : public GaudiAlgorithm {
public:
    using KeyValue = std::pair<std::string, std::string>;
    constexpr static std::size_t N = sizeof...(In);

    FilterAlgorithm(const std::string& name, ISvcLocator* locator,
                    std::array<KeyValue,N> inputs);
    StatusCode execute() override final; // derived classes can NOT implement execute

    virtual bool operator()(const In&...) const = 0;

private:
    template <std::size_t... I>
    StatusCode invoke(utility::index_sequence<I...>);
    std::array<StringProperty,N>  m_inputs;
};


template <typename... In>
FilterAlgorithm<bool(const In&...)>::FilterAlgorithm( const std::string& name,
                                        ISvcLocator* pSvcLocator,
                                        std::array< KeyValue,N> inputs )
  : GaudiAlgorithm ( name , pSvcLocator )
{
    auto prop = std::begin(m_inputs);
    for (const auto& arg : inputs) declareProperty( arg.first, *prop++ = arg.second );
}

template <typename... In>
template <std::size_t... I>
StatusCode
FilterAlgorithm<void(const In&...)>::invoke(utility::index_sequence<I...>) {
    auto in = std::make_tuple( getIfExists<In>(m_inputs[I])... );
    if ( detail::awol( { pair_t(std::get<I>(in),m_inputs[I])... } ) ) {
      //@TODO: add policy / virtual function for what to do if input does not exist:
      //        a) return StatusCode::FAILURE
      //        b) do setFilterPassed(false), and return FAILURE
      //        c) do setFilterPassed(false), and return SUCCESS
      //       could be done by calling a virtual function, or policy template argument
      //       as it is independent of the 'event' state of the algorithm
      setFilterPassed(false);
    } else {
      setFilterPassed(detail::as_const(*this)( detail::as_const(*std::get<I>(in))... ));
    }
    return StatusCode::SUCCESS;
}

