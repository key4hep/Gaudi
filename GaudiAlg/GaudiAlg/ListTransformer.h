#ifndef LISTS_TRANSFORMER_H
#define LISTS_TRANSFORMER_H

#include "GaudiAlg/MergingTransformer.h"

namespace Gaudi
{
  namespace Functional
  {
    template <typename Container>
    using vector_of_ = details::template vector_of_const_<Container>;
    template <typename Signature, typename Traits_ = Traits::useDefaults>
    struct[[deprecated( "use MergingTransformer instead" )]] ListTransformer : MergingTransformer<Signature, Traits_>
    {
      using MergingTransformer<Signature, Traits_>::MergingTransformer;
    };
  }
}

#endif
