/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
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
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace Gaudi::Functional {

  template <typename Container>
  using vector_of_ = std::vector<Container>;
  template <typename Container>
  using vector_of_optional_ = std::vector<std::optional<Container>>;

  namespace details {

    template <typename Signature, typename Traits_, bool isLegacy>
    class SplittingTransformer;

    ////// N -> Many of the same one (value of Many not known at compile time, but known at configuration time)
    template <typename Out, typename... In, typename Traits_>
    class SplittingTransformer<vector_of_<Out>( const In&... ), Traits_, true>
        : public details::DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_> {
      using base_class = details::DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_>;

    public:
      constexpr static std::size_t N = base_class::N_in;
      using KeyValue                 = typename base_class::KeyValue;
      using KeyValues                = typename base_class::KeyValues;

      SplittingTransformer( std::string name, ISvcLocator* locator, const RepeatValues_<KeyValue, N>& inputs,
                            const KeyValues& outputs )
          : base_class( std::move( name ), locator, inputs )
          , m_outputLocations(
                this, outputs.first, details::to_DataObjID( outputs.second ),
                [=]( Gaudi::Details::PropertyBase& ) {
                  this->m_outputs =
                      details::make_vector_of_handles<decltype( this->m_outputs )>( this, m_outputLocations );
                  if constexpr ( details::is_optional_v<Out> ) { // handle constructor does not (yet) allow to
                                                                 // set optional flag... so
                                                                 // do it explicitly here...
                    std::for_each( this->m_outputs.begin(), this->m_outputs.end(),
                                   []( auto& h ) { h.setOptional( true ); } );
                  }
                },
                Gaudi::Details::Property::ImmediatelyInvokeHandler{ true } ) {}

      SplittingTransformer( std::string name, ISvcLocator* locator, const KeyValue& input, const KeyValues& output )
          : SplittingTransformer( std::move( name ), locator, std::forward_as_tuple( input ), output ) {
        static_assert( N == 1, "single input argument requires single input signature" );
      }

      // accessor to output Locations
      const std::string& outputLocation( unsigned int n ) const { return m_outputLocations.value()[n].key(); }
      unsigned int       outputLocationSize() const { return m_outputLocations.value().size(); }

      // derived classes can NOT implement execute
      StatusCode execute() override final {
        try {
          // TODO:FIXME: how does operator() know the number and order of expected outputs?
          auto out = details::filter_evtcontext_t<In...>::apply( *this, this->m_inputs );
          if ( out.size() != m_outputs.size() ) {
            throw GaudiException( "Error during transform: expected " + std::to_string( m_outputs.size() ) +
                                      " containers, got " + std::to_string( out.size() ) + " instead",
                                  this->name(), StatusCode::FAILURE );
          }
          for ( unsigned i = 0; i != out.size(); ++i ) details::put( m_outputs[i], std::move( out[i] ) );
          return FilterDecision::PASSED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.tag() << " : " << e.message() << endmsg;
          return e.code();
        }
      }

      // TODO/FIXME: how does the callee know in which order to produce the outputs?
      //             (note: 'missing' items can be specified by making Out an std::optional<Out>,
      //              and only those entries which contain an Out are stored)
      virtual vector_of_<Out> operator()( const In&... ) const = 0;

    private:
      template <typename T>
      using OutputHandle = details::OutputHandle_t<Traits_, details::remove_optional_t<T>>;
      std::vector<OutputHandle<Out>>          m_outputs;
      Gaudi::Property<std::vector<DataObjID>> m_outputLocations; // TODO/FIXME  for now: use a call-back to update the
                                                                 // actual handles!
    };

    template <typename Out, typename... In, typename Traits_>
    class SplittingTransformer<vector_of_<Out>( const In&... ), Traits_, false>
        : public details::DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_> {
      using base_class = details::DataHandleMixin<std::tuple<>, filter_evtcontext<In...>, Traits_>;

    public:
      constexpr static std::size_t N = base_class::N_in;
      using KeyValue                 = typename base_class::KeyValue;
      using KeyValues                = typename base_class::KeyValues;

      SplittingTransformer( std::string name, ISvcLocator* locator, const RepeatValues_<KeyValue, N>& inputs,
                            const KeyValues& outputs )
          : base_class( std::move( name ), locator, inputs )
          , m_outputLocations(
                this, outputs.first, details::to_DataObjID( outputs.second ),
                [=]( Gaudi::Details::PropertyBase& ) {
                  this->m_outputs =
                      details::make_vector_of_handles<decltype( this->m_outputs )>( this, m_outputLocations );
                  if constexpr ( details::is_optional_v<Out> ) { // handle constructor does not (yet) allow to
                                                                 // set optional flag... so
                                                                 // do it explicitly here...
                    std::for_each( this->m_outputs.begin(), this->m_outputs.end(),
                                   []( auto& h ) { h.setOptional( true ); } );
                  }
                },
                Gaudi::Details::Property::ImmediatelyInvokeHandler{ true } ) {}

      SplittingTransformer( std::string name, ISvcLocator* locator, const KeyValue& input, const KeyValues& output )
          : SplittingTransformer( std::move( name ), locator, std::forward_as_tuple( input ), output ) {
        static_assert( N == 1, "single input argument requires single input signature" );
      }

      // accessor to output Locations
      const std::string& outputLocation( unsigned int n ) const { return m_outputLocations.value()[n].key(); }
      unsigned int       outputLocationSize() const { return m_outputLocations.value().size(); }

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        try {
          // TODO:FIXME: how does operator() know the number and order of expected outputs?
          auto out = details::filter_evtcontext_t<In...>::apply( *this, ctx, this->m_inputs );
          if ( out.size() != m_outputs.size() ) {
            throw GaudiException( "Error during transform: expected " + std::to_string( m_outputs.size() ) +
                                      " containers, got " + std::to_string( out.size() ) + " instead",
                                  this->name(), StatusCode::FAILURE );
          }
          for ( unsigned i = 0; i != out.size(); ++i ) details::put( m_outputs[i], std::move( out[i] ) );
          return FilterDecision::PASSED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.tag() << " : " << e.message() << endmsg;
          return e.code();
        }
      }

      // TODO/FIXME: how does the callee know in which order to produce the outputs?
      //             (note: 'missing' items can be specified by making Out an std::optional<Out>,
      //              and only those entries which contain an Out are stored)
      virtual vector_of_<Out> operator()( const In&... ) const = 0;

    private:
      template <typename T>
      using OutputHandle = details::OutputHandle_t<Traits_, details::remove_optional_t<T>>;
      std::vector<OutputHandle<Out>>          m_outputs;
      Gaudi::Property<std::vector<DataObjID>> m_outputLocations; // TODO/FIXME  for now: use a call-back to update the
                                                                 // actual handles!
    };

  } // namespace details

  template <typename Signature, typename Traits_ = Traits::useDefaults>
  using SplittingTransformer = details::SplittingTransformer<Signature, Traits_, details::isLegacy<Traits_>>;

} // namespace Gaudi::Functional
