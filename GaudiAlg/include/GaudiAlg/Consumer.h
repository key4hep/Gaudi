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

#pragma once

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"
#include "GaudiKernel/FunctionalFilterDecision.h"
#include <utility>

namespace Gaudi::Functional {

  namespace details {

    template <typename Signature, typename Traits_, bool isLegacy>
    struct Consumer;

    template <typename... In, typename Traits_>
    struct Consumer<void( const In&... ), Traits_, true>
        : DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_> {
      using DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute() override final {
        try {
          filter_evtcontext_t<In...>::apply( *this, this->m_inputs );
          return FilterDecision::PASSED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.tag() << " : " << e.message() << endmsg;
          return e.code();
        }
      }

      // ... instead, they must implement the following operator
      virtual void operator()( const In&... ) const = 0;
    };

    template <typename... In, typename Traits_>
    struct Consumer<void( const In&... ), Traits_, false>
        : DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_> {
      using DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute( const EventContext& ctx ) const override final {
        try {
          filter_evtcontext_t<In...>::apply( *this, ctx, this->m_inputs );
          return FilterDecision::PASSED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.tag() << " : " << e.message() << endmsg;
          return e.code();
        }
      }

      // ... instead, they must implement the following operator
      virtual void operator()( const In&... ) const = 0;
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using Consumer = details::Consumer<Signature, Traits_, details::isLegacy<Traits_>>;

} // namespace Gaudi::Functional
