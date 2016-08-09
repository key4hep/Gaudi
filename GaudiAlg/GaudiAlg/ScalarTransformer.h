#ifndef SCALAR_TRANSFORMER_H
#define SCALAR_TRANSFORMER_H

#include "GaudiAlg/Transformer.h"
#include "boost/optional.hpp"
#include <algorithm>
#include <type_traits>

namespace Gaudi { namespace Functional {

   // Scalar->Vector adapted 1->1 algoritm
   // TODO: make variadic, then check that # of elems in each input container is the same,
   //       and use a 'zip' iterator (dot product) for the input...
   // TODO: add a 'tensor' variant
   //
   template <typename ScalarSignature, typename TransformerSignature, typename Traits_ = Traits::useDefaults> class ScalarTransformer;

   template <typename ScalarOp, typename Out, typename In, typename Traits_>
   class ScalarTransformer<ScalarOp,Out(const In&),Traits_> : public Transformer<Out(const In&),Traits_> {
   public:
     using Transformer<Out(const In&)>::Transformer;
     Out operator()(const In& in) const final {
       Out out; out.reserve(in.size());
       const auto& scalar = scalarOp();
       for (const auto& i : in) {
          details::insert( out, scalar( details::deref(i) ) );
       }
       return out;
     }
   private:
     const ScalarOp& scalarOp() const { return static_cast<const ScalarOp&>(*this); }
   };

}}

#endif
