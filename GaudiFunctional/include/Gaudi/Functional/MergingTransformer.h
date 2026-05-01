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

    ////// Merging inputs -> 1 or 0
    template <typename Out, typename... Args, typename Traits_>
      requires has_handle_vector_input<Args...>
    struct MergingTransformer<Out( const Args&... ), Traits_, true>
        : DataHandleVectorMixin<type_list<Out>, Traits_, Args...> {
      using DataHandleVectorMixin<type_list<Out>, Traits_, Args...>::DataHandleVectorMixin;

      // derived classes can NOT implement execute
      StatusCode execute() override final {
        return execute_single_output( *this, this->getContext(), this->m_outputs );
      }

      virtual Out operator()( const Args&... inputs ) const = 0;
    };

    ////// Merging inputs -> 1 or 0
    template <typename Out, typename... Args, typename Traits_>
      requires has_handle_vector_input<Args...>
    struct MergingTransformer<Out( const Args&... ), Traits_, false>
        : DataHandleVectorMixin<type_list<Out>, Traits_, Args...> {
      using DataHandleVectorMixin<type_list<Out>, Traits_, Args...>::DataHandleVectorMixin;

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        return execute_single_output( *this, ctx, this->m_outputs );
      }

      virtual Out operator()( const Args&... inputs ) const = 0;
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using MergingTransformer = details::MergingTransformer<Signature, Traits_, details::isLegacy<Traits_>>;

  // more meaningful alias for cases where the return type in Signature is void
  template <details::is_void_fun Signature, typename Traits_ = Traits::useDefaults>
  using MergingConsumer = details::MergingTransformer<Signature, Traits_, details::isLegacy<Traits_>>;

  using details::vector_of_const_;

  // Merging inputs -> N
  template <typename Signature, typename Traits_ = Traits::BaseClass_t<Gaudi::Algorithm>>
  struct MergingMultiTransformer;

  template <typename... Outs, typename... Args, typename Traits_>
    requires details::has_handle_vector_input<Args...>
  struct MergingMultiTransformer<std::tuple<Outs...>( const Args&... ), Traits_>
      : details::DataHandleVectorMixin<details::type_list<Outs...>, Traits_, Args...> {
    using details::DataHandleVectorMixin<details::type_list<Outs...>, Traits_, Args...>::DataHandleVectorMixin;

    // derived classes can NOT implement execute
    StatusCode execute( EventContext const& ctx ) const override final {
      return details::execute_outputs( *this, ctx, this->m_outputs );
    }

    virtual std::tuple<Outs...> operator()( const Args&... inputs ) const = 0;
  };

  // Merging inputs -> N with filter functionality
  template <typename Signature, typename Traits_ = Traits::BaseClass_t<Gaudi::Algorithm>>
  struct MergingMultiTransformerFilter;

  template <typename... Outs, typename... Args, typename Traits_>
    requires details::has_handle_vector_input<Args...>
  struct MergingMultiTransformerFilter<std::tuple<Outs...>( const Args&... ), Traits_>
      : details::DataHandleVectorMixin<details::type_list<Outs...>, Traits_, Args...> {
    using details::DataHandleVectorMixin<details::type_list<Outs...>, Traits_, Args...>::DataHandleVectorMixin;

    // derived classes can NOT implement execute
    StatusCode execute( EventContext const& ctx ) const override final {
      return details::execute_filtered_outputs( *this, ctx, this->m_outputs );
    }

    virtual std::tuple<bool, Outs...> operator()( const Args&... inputs ) const = 0;
  };

} // namespace Gaudi::Functional
