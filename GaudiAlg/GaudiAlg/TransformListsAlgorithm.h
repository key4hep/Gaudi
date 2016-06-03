#ifndef TRANSFORMLISTSALGORITHM_H
#define TRANSFORMLISTSALGORITHM_H
#include <functional>
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/Algorithm_details.h"
template <typename T> class TransformListsAlgorithm;

template <typename In>
using cref_t = std::reference_wrapper<const In>;

template <typename In>
using vec_cref_t = std::vector<cref_t<In>>;

////// Many of the same -> 1
template <typename Out, typename In>
class TransformListsAlgorithm<Out(const std::vector<cref_t<In>>&)> : public GaudiAlgorithm {
public:
    using KeyValue  = std::pair<std::string, std::string>;
    using KeyValues = std::pair<std::string, std::vector<std::string>>;

    TransformListsAlgorithm(const std::string& name, ISvcLocator* locator,
                            KeyValues inputs,
                            KeyValue output);
    StatusCode execute() override final; // derived classes can NOT implement execute

    // @TODO: should we not take an iterable instead of a container?
    virtual Out operator()(const vec_cref_t<In>& inputs) const = 0;

private:
    StringArrayProperty     m_inputs;
    StringProperty          m_output;
};

template <typename Out, typename In>
TransformListsAlgorithm<Out(const vec_cref_t<In>&)>::TransformListsAlgorithm( const std::string& name,
                                                           ISvcLocator* pSvcLocator,
                                                           KeyValues inputs,
                                                           KeyValue output )
  : GaudiAlgorithm ( name , pSvcLocator )
{
    declareProperty( inputs.first, m_inputs = inputs.second );
    declareProperty( output.first, m_output = output.second );
}

template <typename Out, typename In>
StatusCode
TransformListsAlgorithm<Out(const vec_cref_t<In>&)>::execute()
{
    std::vector<cref_t<In>> in; in.reserve(m_inputs.value().size());
    //@NOTE: we explicitly use 'get' so we don't have to worry about missing inputs,
    //       and instead defer its handling to 'get'
    //@FIXME: add an option which skips non-existing locations without error
    std::transform(m_inputs.value().begin(),m_inputs.value().end(),std::back_inserter(in),
                   [&](const std::string& location)   { return std::cref(*this->get<In>(location)); } );
    auto result = detail::as_const(*this)( detail::as_const(in) );
    //@NOTE: we do NOT set filterPassed. This should be done in a seperate, dedicated
    //       algorithm.
    put( evtSvc(), new Out( std::move(result) ), m_output );
    return StatusCode::SUCCESS;
}
#endif
