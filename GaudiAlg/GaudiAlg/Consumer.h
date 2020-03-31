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
#ifndef GAUDI_FUNCTIONAL_CONSUMER_H
#define GAUDI_FUNCTIONAL_CONSUMER_H

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"
#include "GaudiKernel/FunctionalFilterDecision.h"
#include <utility>

namespace Gaudi::Functional {

  namespace details {

    template <typename Derived, typename Signature, typename Traits_, bool isLegacy>
    struct Consumer;

    template <typename Derived, typename... In, typename Traits_>
    struct Consumer<Derived, void( const In&... ), Traits_, true>
        : DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_> {
      using DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute() override final {
        try {
          filter_evtcontext_t<In...>::apply( *static_cast<Derived const*>(this), this->m_inputs );
          return FilterDecision::PASSED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }
    };

    template <typename Derived, typename... In, typename Traits_>
    struct Consumer<Derived, void( const In&... ), Traits_, false>
        : DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_> {
      using DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes are NOT allowed to implement execute ...
      StatusCode execute( const EventContext& ctx ) const override final {
        try {
          filter_evtcontext_t<In...>::apply( *static_cast<Derived const*>(this), ctx, this->m_inputs );
          return FilterDecision::PASSED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }
    };

  } // namespace details

  template <typename Derived, typename Signature, typename Traits_ = Traits::useDefaults>
  using Consumer = details::Consumer<Derived, Signature, Traits_, details::isLegacy<Traits_>>;

} // namespace Gaudi::Functional

#endif
