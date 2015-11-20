#ifndef SCALAR_TRANSFORM_ALGORITHM_H
#define SCALAR_TRANSFORM_ALGORITHM_H
#include "GaudiAlg/TransformAlgorithm.h"
#include "boost/optional.hpp"

// Scalar->Vector adapted 1->1 algoritm
// TODO: make variadic, then check that # of elems in each input container is the same...
template <typename Vector,typename Scalar> class ScalarTransformAlgorithm;

template <typename Out, typename In, typename ScalarOut, typename ScalarIn >
class ScalarTransformAlgorithm<Out(const In&), ScalarOut(const ScalarIn&)> : public TransformAlgorithm<Out(const In&)> {
public:
    using KeyValue = typename TransformAlgorithm<Out(const In&)>::KeyValue;
    ScalarTransformAlgorithm(const std::string& name, ISvcLocator* locator,
                             KeyValue&& input,
                             KeyValue&& output);
    Out operator()(const In&) const override;
    virtual typename boost::optional<ScalarOut> operator()(const ScalarIn&) const = 0;
};

template <typename Out, typename In, typename ScalarOut, typename ScalarIn>
ScalarTransformAlgorithm<Out(const In&), ScalarOut(const ScalarIn&)>
::ScalarTransformAlgorithm( const std::string& name, ISvcLocator* pSvcLocator,
                            KeyValue&& inputs,
                            KeyValue&& output )
  : TransformAlgorithm<Out(const In&)> ( name , pSvcLocator,
                                         { std::forward<KeyValue>(inputs) },
                                         std::forward<KeyValue>(output) )
{
}

template <typename Out, typename In,typename ScalarOut, typename ScalarIn>
Out ScalarTransformAlgorithm<Out(const In&),ScalarOut(const ScalarIn&)>::operator()(const In& in) const {
    Out out; out.reserve(in.size());
    for (const auto& i : in) {
        auto o = (*this)(*i);
        if (o) out.insert( new ScalarOut(std::move( *o )) );
    }
    return out;
}
#endif
