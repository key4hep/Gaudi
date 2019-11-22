#ifndef GAUDI_FUNCTIONAL_PRODUCER_H
#define GAUDI_FUNCTIONAL_PRODUCER_H

#include "GaudiAlg/Transformer.h"
#include <utility>

namespace Gaudi::Functional {

  namespace details {

    template <typename Signature, typename Traits_, bool isLegacy>
    struct Producer;

    template <typename... Out, typename Traits_, bool legacy>
    struct Producer<std::tuple<Out...>(), Traits_, legacy> : MultiTransformer<std::tuple<Out...>(), Traits_, legacy> {
      using MultiTransformer<std::tuple<Out...>(), Traits_, legacy>::MultiTransformer;
    };

    template <typename Out, typename Traits_, bool legacy>
    struct Producer<Out(), Traits_, legacy> : Transformer<Out(), Traits_, legacy> {
      using Transformer<Out(), Traits_, legacy>::Transformer;
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using Producer = details::Producer<Signature, Traits_, details::isLegacy<Traits_>>;

} // namespace Gaudi::Functional

#endif
