/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include "details.h"
#include "utilities.h"
#include <optional>
#include <vector>

namespace Gaudi::Functional {

  template <typename Container>
  using vector_of_ = std::vector<Container>;
  template <typename Container>
  using vector_of_optional_ = std::vector<std::optional<Container>>;

  namespace details {

    template <typename Signature, typename Traits_>
    struct SplittingTransformer;

    template <typename Out, typename... In, typename Traits_>
    struct SplittingTransformer<vector_of_<Out>( const In&... ), Traits_>
        : details::DataHandleMixin<type_list<details::vector_of_output_<Out>>, type_list<In...>, Traits_> {
      using details::DataHandleMixin<type_list<details::vector_of_output_<Out>>, type_list<In...>,
                                     Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        return details::execute_single_output( *this, ctx, this->m_outputs );
      }

      // TODO/FIXME: how does the callee know in which order to produce the outputs?
      //             (note: 'missing' items can be specified by making Out an std::optional<Out>,
      //              and only those entries which contain an Out are stored)
      virtual vector_of_<Out> operator()( const In&... ) const = 0;
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using SplittingTransformer = details::SplittingTransformer<Signature, Traits_>;

} // namespace Gaudi::Functional
