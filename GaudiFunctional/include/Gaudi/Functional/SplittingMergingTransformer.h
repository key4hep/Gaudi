/*****************************************************************************\
* (c) Copyright 2022-2026 CERN for the benefit of the LHCb Collaboration      *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
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
  using details::vector_of_const_;

  namespace details {

    template <typename Signature, typename Traits_>
    struct SplittingMergingTransformer;

    template <typename Out, typename In, typename Traits_>
    struct SplittingMergingTransformer<vector_of_<Out>( const vector_of_const_<In>& ), Traits_>
        : DataHandleMixin<type_list<vector_of_output_<Out>>, type_list<vector_of_input_<In>>, Traits_> {
      using DataHandleMixin<type_list<vector_of_output_<Out>>, type_list<vector_of_input_<In>>,
                            Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        return details::execute_single_output( *this, ctx, this->m_outputs );
      }

      // TODO/FIXME: how does the callee know in which order to produce the outputs?
      //             (note: 'missing' items can be specified by making Out an std::optional<Out>,
      //              and only those entries which contain an Out are stored)
      virtual vector_of_<Out> operator()( const vector_of_const_<In>& ) const = 0;
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using SplittingMergingTransformer = details::SplittingMergingTransformer<Signature, Traits_>;

} // namespace Gaudi::Functional
