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
#include <Gaudi/Algorithm.h>
#include <GaudiKernel/ThreadLocalContext.h>
#include <tuple>

namespace Gaudi::Functional {

  namespace details {

    template <typename Sig>
    constexpr bool is_void_fun_v = false;
    template <typename... Args>
    constexpr bool is_void_fun_v<void( Args... )> = true;
    template <typename Sig>
    concept is_void_fun = is_void_fun_v<Sig>;

    template <typename Signature, typename Traits_, bool isLegacy>
    struct MergingTransformer;

    ////// Many of the same -> 1 or 0
    template <typename Out, typename... Ins, typename Traits_>
    struct MergingTransformer<Out( const vector_of_const_<Ins>&... ), Traits_, true>
        : DataHandleMixin<type_list<Out>, type_list<vector_of_input_<Ins>...>, Traits_> {

      using DataHandleMixin<type_list<Out>, type_list<vector_of_input_<Ins>...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute() override final {
        return execute_single_output( *this, Gaudi::Hive::currentContext(), this->m_outputs );
      }

      virtual Out operator()( const vector_of_const_<Ins>&... inputs ) const = 0;
    };

    ////// Many of the same -> 1 or 0
    template <typename Out, typename... Ins, typename Traits_>
    struct MergingTransformer<Out( const vector_of_const_<Ins>&... ), Traits_, false>
        : DataHandleMixin<type_list<Out>, type_list<vector_of_input_<Ins>...>, Traits_> {

      using DataHandleMixin<type_list<Out>, type_list<vector_of_input_<Ins>...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        return execute_single_output( *this, ctx, this->m_outputs );
      }

      virtual Out operator()( const vector_of_const_<Ins>&... inputs ) const = 0;
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using MergingTransformer = details::MergingTransformer<Signature, Traits_, details::isLegacy<Traits_>>;

  // more meaningful alias for cases where the return type in Signature is void
  template <details::is_void_fun Signature, typename Traits_ = Traits::useDefaults>
  using MergingConsumer = details::MergingTransformer<Signature, Traits_, details::isLegacy<Traits_>>;

  using details::vector_of_const_;

  // M vectors of the same -> N
  template <typename Signature, typename Traits_ = Traits::BaseClass_t<Gaudi::Algorithm>>
  struct MergingMultiTransformer;

  template <typename... Outs, typename... Ins, typename Traits_>
  struct MergingMultiTransformer<std::tuple<Outs...>( details::vector_of_const_<Ins> const&... ), Traits_>
      : details::DataHandleMixin<details::type_list<Outs...>, details::type_list<details::vector_of_input_<Ins>...>,
                                 Traits_> {

    using details::DataHandleMixin<details::type_list<Outs...>, details::type_list<details::vector_of_input_<Ins>...>,
                                   Traits_>::DataHandleMixin;

    // derived classes can NOT implement execute
    StatusCode execute( EventContext const& ctx ) const override final {
      return details::execute_outputs( *this, ctx, this->m_outputs );
    }

    virtual std::tuple<Outs...> operator()( const vector_of_const_<Ins>&... inputs ) const = 0;
  };

  // Many of the same -> N with filter functionality
  template <typename Signature, typename Traits_ = Traits::BaseClass_t<Gaudi::Algorithm>>
  struct MergingMultiTransformerFilter;

  template <typename... Outs, typename In, typename Traits_>
  struct MergingMultiTransformerFilter<std::tuple<Outs...>( vector_of_const_<In> const& ), Traits_>
      : details::DataHandleMixin<details::type_list<Outs...>, details::type_list<details::vector_of_input_<In>>,
                                 Traits_> {

    using details::DataHandleMixin<details::type_list<Outs...>, details::type_list<details::vector_of_input_<In>>,
                                   Traits_>::DataHandleMixin;

    // derived classes can NOT implement execute
    StatusCode execute( EventContext const& ctx ) const override final {
      return details::execute_filtered_outputs( *this, ctx, this->m_outputs );
    }

    virtual std::tuple<bool, Outs...> operator()( const vector_of_const_<In>& inputs ) const = 0;
  };

} // namespace Gaudi::Functional
