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
#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"
#include "GaudiKernel/FunctionalFilterDecision.h"
#include "GaudiKernel/GaudiException.h"
#include <type_traits>
#include <utility>

// Adapt an Algorithm (by default, GaudiAlgorithm) so that derived classes
//   a) do not need to access the event store, and have to explicitly
//      state their data dependencies
//   b) are encouraged not to have state which depends on the events
//      (eg. histograms, counters will have to be mutable)

namespace Gaudi ::Functional {

  namespace details {

    template <typename Signature, typename Traits_, bool isLegacy>
    struct Transformer;

    // general N -> 1 algorithms
    template <typename Out, typename... In, typename Traits_>
    struct Transformer<Out( const In&... ), Traits_, true>
        : DataHandleMixin<std::tuple<Out>, filter_evtcontext<In...>, Traits_> {
      using DataHandleMixin<std::tuple<Out>, filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute() override final {
        try {
          if constexpr ( sizeof...( In ) == 0 ) {
            put( std::get<0>( this->m_outputs ), ( *this )() );
          } else if constexpr ( std::tuple_size_v<filter_evtcontext<In...>> == 0 ) {
            put( std::get<0>( this->m_outputs ), ( *this )( Gaudi::Hive::currentContext() ) );
          } else {
            put( std::get<0>( this->m_outputs ), filter_evtcontext_t<In...>::apply( *this, this->m_inputs ) );
          }
          return FilterDecision::PASSED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // instead they MUST implement this operator
      virtual Out operator()( const In&... ) const = 0;
    };

    template <typename Out, typename... In, typename Traits_>
    struct Transformer<Out( const In&... ), Traits_, false>
        : DataHandleMixin<std::tuple<Out>, filter_evtcontext<In...>, Traits_> {
      using DataHandleMixin<std::tuple<Out>, filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        try {
          if constexpr ( sizeof...( In ) == 0 ) {
            put( std::get<0>( this->m_outputs ), ( *this )() );
          } else if constexpr ( std::tuple_size_v<filter_evtcontext<In...>> == 0 ) {
            put( std::get<0>( this->m_outputs ), ( *this )( ctx ) );
          } else {
            put( std::get<0>( this->m_outputs ), filter_evtcontext_t<In...>::apply( *this, ctx, this->m_inputs ) );
          }
          return FilterDecision::PASSED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // instead they MUST implement this operator
      virtual Out operator()( const In&... ) const = 0;
    };

    //
    // general N -> M algorithms
    //
    template <typename Signature, typename Traits_, bool isLegacy>
    struct MultiTransformer;

    template <typename... Out, typename... In, typename Traits_>
    struct MultiTransformer<std::tuple<Out...>( const In&... ), Traits_, true>
        : DataHandleMixin<std::tuple<Out...>, filter_evtcontext<In...>, Traits_> {
      using DataHandleMixin<std::tuple<Out...>, filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute() override final {
        try {
          std::apply(
              [this]( auto&... ohandle ) {
                GF_SUPPRESS_SPURIOUS_CLANG_WARNING_BEGIN

                if constexpr ( sizeof...( In ) == 0 ) {
                  std::apply( [&ohandle...](
                                  auto&&... data ) { ( put( ohandle, std::forward<decltype( data )>( data ) ), ... ); },
                              std::as_const( *this )() );
                } else {
                  std::apply( [&ohandle...](
                                  auto&&... data ) { ( put( ohandle, std::forward<decltype( data )>( data ) ), ... ); },
                              filter_evtcontext_t<In...>::apply( std::as_const( *this ), this->m_inputs ) );
                }
                GF_SUPPRESS_SPURIOUS_CLANG_WARNING_END
              },
              this->m_outputs );
          return FilterDecision::PASSED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // instead they MUST implement this operator
      virtual std::tuple<Out...> operator()( const In&... ) const = 0;
    };

    template <typename... Out, typename... In, typename Traits_>
    struct MultiTransformer<std::tuple<Out...>( const In&... ), Traits_, false>
        : DataHandleMixin<std::tuple<Out...>, filter_evtcontext<In...>, Traits_> {
      using DataHandleMixin<std::tuple<Out...>, filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        try {
          GF_SUPPRESS_SPURIOUS_CLANG_WARNING_BEGIN
          std::apply(
              [this, &ctx]( auto&... ohandle ) {
                if constexpr ( sizeof...( In ) == 0 ) {
                  std::apply( [&ohandle...](
                                  auto&&... data ) { ( put( ohandle, std::forward<decltype( data )>( data ) ), ... ); },
                              ( *this )() );
                } else if constexpr ( std::tuple_size_v<filter_evtcontext<In...>> == 0 ) {
                  std::apply( [&ohandle...](
                                  auto&&... data ) { ( put( ohandle, std::forward<decltype( data )>( data ) ), ... ); },
                              ( *this )( ctx ) );
                } else {
                  std::apply( [&ohandle...](
                                  auto&&... data ) { ( put( ohandle, std::forward<decltype( data )>( data ) ), ... ); },
                              filter_evtcontext_t<In...>::apply( *this, ctx, this->m_inputs ) );
                }
              },
              this->m_outputs );
          GF_SUPPRESS_SPURIOUS_CLANG_WARNING_END
          return FilterDecision::PASSED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // instead they MUST implement this operator
      virtual std::tuple<Out...> operator()( const In&... ) const = 0;
    };

    //
    // general N -> M algorithms with filter functionality
    //
    template <typename Signature, typename Traits_, bool isLegacy>
    struct MultiTransformerFilter;

    template <typename... Out, typename... In, typename Traits_>
    struct MultiTransformerFilter<std::tuple<Out...>( const In&... ), Traits_, true>
        : DataHandleMixin<std::tuple<Out...>, filter_evtcontext<In...>, Traits_> {
      using DataHandleMixin<std::tuple<Out...>, filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute() override final {
        try {
          return std::apply(
                     [&]( auto&... ohandle ) {
                       GF_SUPPRESS_SPURIOUS_CLANG_WARNING_BEGIN
                       return std::apply(
                           [&ohandle..., this]( bool passed, auto&&... data ) {
                             ( put( ohandle, std::forward<decltype( data )>( data ) ), ... );
                             return passed;
                           },
                           filter_evtcontext_t<In...>::apply( *this, this->m_inputs ) );
                       GF_SUPPRESS_SPURIOUS_CLANG_WARNING_END
                     },
                     this->m_outputs )
                     ? FilterDecision::PASSED
                     : FilterDecision::FAILED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // instead they MUST implement this operator
      virtual std::tuple<bool, Out...> operator()( const In&... ) const = 0;
    };

    template <typename... Out, typename... In, typename Traits_>
    struct MultiTransformerFilter<std::tuple<Out...>( const In&... ), Traits_, false>
        : DataHandleMixin<std::tuple<Out...>, filter_evtcontext<In...>, Traits_> {
      using DataHandleMixin<std::tuple<Out...>, filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        try {
          return std::apply(
                     GF_SUPPRESS_SPURIOUS_CLANG_WARNING_BEGIN[&]( auto&... ohandle ) {
                       return std::apply(
                           [&ohandle..., &ctx, this]( bool passed, auto&&... data ) {
                             ( put( ohandle, std::forward<decltype( data )>( data ) ), ... );
                             return passed;
                           },
                           filter_evtcontext_t<In...>::apply( *this, ctx, this->m_inputs ) );
                     },
                     GF_SUPPRESS_SPURIOUS_CLANG_WARNING_END

                     this->m_outputs )
                     ? FilterDecision::PASSED
                     : FilterDecision::FAILED;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
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
  using MultiTransformerFilter = details::MultiTransformerFilter<Signature, Traits_, details::isLegacy<Traits_>>;

} // namespace Gaudi::Functional

#endif
