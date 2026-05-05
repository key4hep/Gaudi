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
#include <optional>
#include <string>
#include <vector>

namespace Gaudi::Functional {

  template <typename Container>
  using vector_of_ = std::vector<Container>;
  template <typename Container>
  using vector_of_optional_ = std::vector<std::optional<Container>>;

  namespace details {

    template <typename Signature, typename Traits_>
    class SplittingTransformer;

    template <typename Out, typename... In, typename Traits_>
    class SplittingTransformer<vector_of_<Out>( const In&... ), Traits_>
        : public details::DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_> {
      using base_class = details::DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_>;

      template <typename T>
      using OutputHandle = details::OutputHandle_t<Traits_, details::remove_optional_t<T>>;
      details::HandleVector<OutputHandle, Out> m_outs;

    public:
      constexpr static std::size_t N = base_class::N_in;
      using KeyValue                 = typename base_class::KeyValue;
      using KeyValues                = typename base_class::KeyValues;

      SplittingTransformer( std::string name, ISvcLocator* locator, const RepeatValues_<KeyValue, N>& inputs,
                            const KeyValues& outputs )
          : base_class{ std::move( name ), locator, inputs }, m_outs{ this, outputs } {}

      SplittingTransformer( std::string name, ISvcLocator* locator, const KeyValue& input, const KeyValues& output )
        requires( N == 1 )
          : SplittingTransformer( std::move( name ), locator, std::forward_as_tuple( input ), output ) {}

      // accessor to output Locations
      const std::string& outputLocation( unsigned int n ) const { return m_outs.at( n ).key(); }
      unsigned int       outputLocationSize() const { return m_outs.size(); }

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        return details::execute( *this, [&] {
          // TODO:FIXME: how does operator() know the number and order of expected outputs?
          m_outs.put( details::filter_evtcontext_t<In...>::apply( *this, ctx, this->m_inputs ) );
          return FilterDecision::PASSED;
        } );
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
