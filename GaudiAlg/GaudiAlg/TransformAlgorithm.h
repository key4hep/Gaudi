#include "GaudiKernel/index_sequence.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include <type_traits>

namespace detail {
    template <typename T> constexpr typename std::add_const<T>::type& as_const(T& t) noexcept { return t; }    
    template <typename T> void as_const(T&& t) = delete;
}

template <typename T> class TransformAlgorithm;

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
    StatusCode invoke_transformer(utility::index_sequence<I...>) const;

    using pair_t = std::pair<void*,std::string>;
    bool awol( std::array< pair_t,N > inputs ) const;

    std::array<StringProperty,N>  m_inputs;
    StringProperty                m_output;
};

template <typename Out, typename... In>
TransformAlgorithm<Out(const In&...)>::TransformAlgorithm( const std::string& name,
                                        ISvcLocator* pSvcLocator, 
                                        std::array< KeyValue,N> inputs, 
                                        KeyValue output )
  : GaudiAlgorithm ( name , pSvcLocator )
{
    auto prop = std::begin(m_inputs);
    for (const auto& arg : inputs) declareProperty( arg.first, *prop++ = arg.second );
    declareProperty( output.first, m_output = output.second );
}

template <typename Out, typename... In>
bool TransformAlgorithm<Out(const In&...)>::awol( std::array<pair_t, N> inputs ) const {
    bool result = false;
    for ( const auto& i : inputs ) { 
        if (!i.first) {
            result = true;
            error() << " Mandatory input at " << i.second << " not found" << endmsg;
        }
    }
    return result;
}

template <typename Out, typename... In>
template <std::size_t... I> 
StatusCode 
TransformAlgorithm<Out(const In&...)>::invoke_transformer(utility::index_sequence<I...>) const {
    auto in = std::make_tuple( getIfExists<In>(m_inputs[I])... );
#if 0
    if ( awol( { pair_t(std::get<I>(in),m_inputs[I])... ) ) {
      //@TODO: add policy for what to do if input does not exist: 
      //        a) return StatusCode::FAILURE
      //        b) do setFilterPassed(false), and return FAILURE
      //        c) do setFilterPassed(false), and return SUCCESS
      //       could be done by calling a virtual function, or policy template argument
      //       as it is independent of the 'event' state of the algorithm
      setFilterPassed(false);
      return StatusCode::SUCCESS;
    }
#endif
    auto result = (*this)( detail::as_const(*std::get<I>(in))... );
    put( evtSvc(), new Out( std::move(result) ), m_output );
    //@TODO: add policy for setFilterPassed
    //       a) just set to true
    //       b) set to true if m_output is not empty
    //       c) have it returned together with result
    //       d) query m_transformer and forward? -- needs to be obtained 'atomically' from operator()...
    return StatusCode::SUCCESS;
}

template <typename Out, typename... In>
StatusCode 
TransformAlgorithm<Out(const In&...)>::execute() {
    return invoke_transformer(utility::make_index_sequence<N>{});
}
