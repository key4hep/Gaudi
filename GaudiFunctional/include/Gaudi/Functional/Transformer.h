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
#include <GaudiKernel/ThreadLocalContext.h>
#include <tuple>

// Adapt an Algorithm (by default, Gaudi::Algorithm) so that derived classes
//   a) do not need to access the event store, and have to explicitly
//      state their data dependencies
//   b) are encouraged not to have state which depends on the events
//      (eg. histograms, counters will have to be mutable)

namespace Gaudi ::Functional {

  namespace details {

    template <typename Signature, typename Traits_, bool isLegacy>
    struct Transformer;

    template <typename Signature, typename Traits_, bool isLegacy>
    struct MultiTransformer;

    // general N -> 1 algorithms
    template <typename Out, typename... In, typename Traits_>
    struct Transformer<Out( const In&... ), Traits_, true>
        : DataHandleMixin<type_list<Out>, type_list<In...>, Traits_> {
      using DataHandleMixin<type_list<Out>, type_list<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute() override final {
        return execute_single_output( *this, Gaudi::Hive::currentContext(), this->m_outputs );
      }

      // instead they MUST implement this operator
      virtual Out operator()( const In&... ) const = 0;
    };

    template <typename Out, typename... In, typename Traits_>
    struct Transformer<Out( const In&... ), Traits_, false>
        : DataHandleMixin<type_list<Out>, type_list<In...>, Traits_> {
      using DataHandleMixin<type_list<Out>, type_list<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        return execute_single_output( *this, ctx, this->m_outputs );
      }

      // instead they MUST implement this operator
      virtual Out operator()( const In&... ) const = 0;
    };

    //
    // general N -> M algorithms
    //
    template <typename... Out, typename... In, typename Traits_>
    struct MultiTransformer<std::tuple<Out...>( const In&... ), Traits_, true>
        : DataHandleMixin<type_list<Out...>, type_list<In...>, Traits_> {
      using DataHandleMixin<type_list<Out...>, type_list<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute() override final {
        return execute_outputs( *this, Gaudi::Hive::currentContext(), this->m_outputs );
      }

      // instead they MUST implement this operator
      virtual std::tuple<Out...> operator()( const In&... ) const = 0;
    };

    template <typename... Out, typename... In, typename Traits_>
    struct MultiTransformer<std::tuple<Out...>( const In&... ), Traits_, false>
        : DataHandleMixin<type_list<Out...>, type_list<In...>, Traits_> {
      using DataHandleMixin<type_list<Out...>, type_list<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        return execute_outputs( *this, ctx, this->m_outputs );
      }

      // instead they MUST implement this operator
      virtual std::tuple<Out...> operator()( const In&... ) const = 0;
    };

    //
    // general N -> M algorithms with filter functionality
    //
    template <typename Signature, typename Traits_>
    struct MultiTransformerFilter;

    template <typename... Out, typename... In, typename Traits_>
    struct MultiTransformerFilter<std::tuple<Out...>( const In&... ), Traits_>
        : DataHandleMixin<type_list<Out...>, type_list<In...>, Traits_> {
      using DataHandleMixin<type_list<Out...>, type_list<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        return execute_filtered_outputs( *this, ctx, this->m_outputs );
      }

      // instead they MUST implement this operator
      virtual std::tuple<bool, Out...> operator()( const In&... ) const = 0;
    };
  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using Transformer = details::Transformer<Signature, Traits_, details::isLegacy<Traits_>>;

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using MultiTransformer = details::MultiTransformer<Signature, Traits_, details::isLegacy<Traits_>>;

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using MultiTransformerFilter = details::MultiTransformerFilter<Signature, Traits_>;

} // namespace Gaudi::Functional
