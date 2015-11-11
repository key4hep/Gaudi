#include "GaudiKernel/index_sequence.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include <type_traits>
#include <utility>
#include <initializer_list>

namespace detail {
    template <typename T>
    constexpr typename std::add_const<T>::type& as_const(T& t) noexcept
    { return t; }

    template <typename T>
    void as_const(T&& t) = delete;

    bool awol(GaudiAlgorithm& parent, std::initializer_list<std::pair<void*,const std::string&>> inputs) {
        bool result = false;
        for ( const auto& i : inputs ) {
            if (!i.first) {
                result = true;
                parent.error() << " Mandatory input at " << i.second << " not found" << endmsg;
            }
        }
        return result;
    }
}

template <typename T> class TransformAlgorithm;

// N -> 1 algorithms
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
                                        std::array< KeyValue,N> inputs,
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
    auto in = std::make_tuple( getIfExists<In>(m_inputs[I])... );
    if ( detail::awol( *this, { { std::get<I>(in),m_inputs[I].value() }... } ) ) {
      //@TODO: add policy / virtual function for what to do if input does not exist:
      //        a) return StatusCode::FAILURE
      //        b) do setFilterPassed(false), and return FAILURE
      //        c) do setFilterPassed(false), and return SUCCESS
      //       could be done by calling a virtual function, or policy template argument
      //       as it is independent of the 'event' state of the algorithm
      setFilterPassed(false);
      return StatusCode::SUCCESS;
    }
    auto result = detail::as_const(*this)( detail::as_const(*std::get<I>(in))... );
    //@TODO: add policy for setFilterPassed
    //       a) just set to true
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


// ---- N -> 0

template <typename... In>
class TransformAlgorithm<void(const In&...)> : public GaudiAlgorithm {
public:
    using KeyValue = std::pair<std::string, std::string>;
    constexpr static std::size_t N = sizeof...(In);

    TransformAlgorithm(const std::string& name, ISvcLocator* locator,
                       std::array<KeyValue,N> inputs);
    StatusCode execute() override final; // derived classes can NOT implement execute

    virtual void operator()(const In&...) const = 0;

private:
    template <std::size_t... I>
    StatusCode invoke(utility::index_sequence<I...>);
    std::array<StringProperty,N>  m_inputs;
};


template <typename... In>
TransformAlgorithm<void(const In&...)>::TransformAlgorithm( const std::string& name,
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
TransformAlgorithm<void(const In&...)>::invoke(utility::index_sequence<I...>) {
    auto in = std::make_tuple( getIfExists<In>(m_inputs[I])... );
    if ( detail::awol( { pair_t(std::get<I>(in),m_inputs[I])... } ) ) {
      //@TODO: add policy / virtual function for what to do if input does not exist:
      //        a) return StatusCode::FAILURE
      //        b) do setFilterPassed(false), and return FAILURE
      //        c) do setFilterPassed(false), and return SUCCESS
      //       could be done by calling a virtual function, or policy template argument
      //       as it is independent of the 'event' state of the algorithm
      setFilterPassed(false);
      return StatusCode::SUCCESS;
    }
    detail::as_const(*this)( detail::as_const(*std::get<I>(in))... );
    //@TODO: add policy for setFilterPassed
    //       a) just set to true
    //       b) set to true if m_output is not empty
    //       c) have it returned together with result
    //       d) ask derived class, passing it the value of result.size()
    setFilterPassed(true);
    return StatusCode::SUCCESS;
}

