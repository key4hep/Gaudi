#ifndef SCALAR_TRANSFORMER_H
#define SCALAR_TRANSFORMER_H

#include "GaudiAlg/Transformer.h"

namespace Gaudi { namespace Functional {

// Scalar->Vector adapted 1->1 algoritm
template <typename ScalarOp,
          typename TransformerSignature,
          typename Traits_ = Traits::useDefaults> class ScalarTransformer;

template <typename ScalarOp, typename Out, typename In, typename Traits_>
class ScalarTransformer<ScalarOp,Out(const In&),Traits_> : public Transformer<Out(const In&),Traits_> {
    const ScalarOp& scalarOp() const { return static_cast<const ScalarOp&>(*this); }
public:
    using Transformer<Out(const In&)>::Transformer;
    Out operator()(const In& in) const final {
        Out out; out.reserve(in.size());
        auto& scalar = scalarOp();
        for (const auto& i : in) details::insert( out, scalar( details::deref(i) ) );
        details::apply( scalar, out );
        return out;
    }
};

}}

#endif
