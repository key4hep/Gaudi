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

namespace Gaudi::Functional {

  namespace details {

    template <typename T, typename Traits_, bool isLegacy>
    struct FilterPredicate;

    template <typename... In, typename Traits_, bool isLegacy>
    StatusCode execute_filter_predicate( const FilterPredicate<bool( const In&... ), Traits_, isLegacy>& algo,
                                         const EventContext&                                             ctx ) {
      return details::execute(
          algo, [&] { return algo.invoke( algo, ctx ) ? FilterDecision::PASSED : FilterDecision::FAILED; } );
    }

    template <typename... In, typename Traits_>
    struct FilterPredicate<bool( const In&... ), Traits_, true>
        : DataHandleMixin<type_list<>, type_list<In...>, Traits_> {
      using DataHandleMixin<type_list<>, type_list<In...>, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute() override final { return execute_filter_predicate( *this, this->getContext() ); }
      // ... instead, they must implement the following operator
      virtual bool operator()( const In&... ) const = 0;
    };

    template <typename... In, typename Traits_>
    struct FilterPredicate<bool( const In&... ), Traits_, false>
        : DataHandleMixin<type_list<>, type_list<In...>, Traits_> {
      using DataHandleMixin<type_list<>, type_list<In...>, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute( const EventContext& ctx ) const override final {
        return execute_filter_predicate( *this, ctx );
      }

      // ... instead, they must implement the following operator
      virtual bool operator()( const In&... ) const = 0;
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using FilterPredicate = details::FilterPredicate<Signature, Traits_, details::isLegacy<Traits_>>;

} // namespace Gaudi::Functional
