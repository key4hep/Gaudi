/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDI_FUNCTIONAL_PRODUCER_H
#define GAUDI_FUNCTIONAL_PRODUCER_H

#include "GaudiAlg/Transformer.h"
#include <utility>

namespace Gaudi::Functional {

  namespace details {

    template <typename Derived, typename Signature, typename Traits_, bool isLegacy>
    struct Producer;

    template <typename Derived, typename... Out, typename Traits_, bool legacy>
    struct Producer<Derived, std::tuple<Out...>(), Traits_, legacy>
        : MultiTransformer<Derived, std::tuple<Out...>(), Traits_, legacy> {
      using MultiTransformer<Derived, std::tuple<Out...>(), Traits_, legacy>::MultiTransformer;
    };

    template <typename Derived, typename Out, typename Traits_, bool legacy>
    struct Producer<Derived, Out(), Traits_, legacy> : Transformer<Derived, Out(), Traits_, legacy> {
      using Transformer<Derived, Out(), Traits_, legacy>::Transformer;
    };

  } // namespace details

  template <typename Derived, typename Signature, typename Traits_ = Traits::useDefaults>
  using Producer = details::Producer<Derived, Signature, Traits_, details::isLegacy<Traits_>>;

} // namespace Gaudi::Functional

#endif
