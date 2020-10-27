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
#ifndef LISTS_TRANSFORMER_H
#define LISTS_TRANSFORMER_H

#include "GaudiAlg/MergingTransformer.h"

namespace Gaudi {
  namespace Functional {
    template <typename Container>
    using vector_of_ = details::template vector_of_const_<Container>;
    template <typename Signature, typename Traits_ = Traits::useDefaults>
    struct [[deprecated( "use MergingTransformer instead" )]] ListTransformer : MergingTransformer<Signature, Traits_> {
      using MergingTransformer<Signature, Traits_>::MergingTransformer;
    };
  } // namespace Functional
} // namespace Gaudi

#endif
