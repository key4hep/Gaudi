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
#include <GaudiKernel/FunctionalFilterDecision.h>
#include <tuple>

namespace Gaudi::Functional {

  namespace details {

    template <typename T, typename Traits_, bool isLegacy>
    struct FilterPredicate;

    template <typename... In, typename Traits_>
    struct FilterPredicate<bool( const In&... ), Traits_, true>
        : DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_> {
      using DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute() override final {
        return details::execute( *this, [&] {
          return filter_evtcontext_t<In...>::apply( *this, Gaudi::Hive::currentContext(), this->m_inputs )
                     ? FilterDecision::PASSED
                     : FilterDecision::FAILED;
        } );
      }
      // ... instead, they must implement the following operator
      virtual bool operator()( const In&... ) const = 0;
    };

    template <typename... In, typename Traits_>
    struct FilterPredicate<bool( const In&... ), Traits_, false>
        : DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_> {
      using DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute( const EventContext& ctx ) const override final {
        return details::execute( *this, [&] {
          return filter_evtcontext_t<In...>::apply( *this, ctx, this->m_inputs ) ? FilterDecision::PASSED
                                                                                 : FilterDecision::FAILED;
        } );
      }

      // ... instead, they must implement the following operator
      virtual bool operator()( const In&... ) const = 0;
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using FilterPredicate = details::FilterPredicate<Signature, Traits_, details::isLegacy<Traits_>>;

} // namespace Gaudi::Functional
