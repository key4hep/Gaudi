#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"
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
        : details::DataHandleMixin<std::tuple<Out>, details::filter_evtcontext<In...>, Traits_> {
      using details::DataHandleMixin<std::tuple<Out>, details::filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute() override final {
        try {
          details::put( std::get<0>( this->m_outputs ),
                        details::filter_evtcontext_t<In...>::apply( *this, this->m_inputs ) );
          return StatusCode::SUCCESS;
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
        : details::DataHandleMixin<std::tuple<Out>, details::filter_evtcontext<In...>, Traits_> {
      using details::DataHandleMixin<std::tuple<Out>, details::filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        try {
          details::put( std::get<0>( this->m_outputs ),
                        details::filter_evtcontext_t<In...>::apply( *this, ctx, this->m_inputs ) );
          return StatusCode::SUCCESS;
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
        : details::DataHandleMixin<std::tuple<Out...>, details::filter_evtcontext<In...>, Traits_> {
      using details::DataHandleMixin<std::tuple<Out...>, details::filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute() override final {
        try {
          std::apply(
              [this]( auto&... ohandle ) {

#if defined( __clang__ ) && ( __clang_major__ < 9 )
// clang-8 gives a spurious warning about not using the captured `ohandle`
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-lambda-capture"
#endif
                std::apply(
                    [&ohandle...]( auto&&... data ) {
                      ( details::put( ohandle, std::forward<decltype( data )>( data ) ), ... );
                    },
                    details::filter_evtcontext_t<In...>::apply( *this, this->m_inputs ) );

#if defined( __clang__ ) && ( __clang_major__ < 9 )
#  pragma clang diagnostic pop
#endif
              },
              this->m_outputs );
          return StatusCode::SUCCESS;
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
        : details::DataHandleMixin<std::tuple<Out...>, details::filter_evtcontext<In...>, Traits_> {
      using details::DataHandleMixin<std::tuple<Out...>, details::filter_evtcontext<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        try {
          std::apply(
              [this, &ctx]( auto&... ohandle ) {

#if defined( __clang__ ) && ( __clang_major__ < 6 )
// clang-5 gives a spurious warning about not using the captured `ohandle`
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-lambda-capture"
#endif
                std::apply(
                    [&ohandle...]( auto&&... data ) {
                      ( details::put( ohandle, std::forward<decltype( data )>( data ) ), ... );
                    },
                    details::filter_evtcontext_t<In...>::apply( *this, ctx, this->m_inputs ) );

#if defined( __clang__ ) && ( __clang_major__ < 6 )
#  pragma clang diagnostic pop
#endif
              },
              this->m_outputs );
          return StatusCode::SUCCESS;
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
        : details::DataHandleMixin<std::tuple<Out...>, std::tuple<In...>, Traits_> {
      using details::DataHandleMixin<std::tuple<Out...>, std::tuple<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute() override final {
        try {
          std::apply(
              [&]( auto&... ohandle ) {
                std::apply(
                    [&ohandle..., this]( bool passed, auto&&... data ) {
                      this->setFilterPassed( passed );
                      ( details::put( ohandle, std::forward<decltype( data )>( data ) ), ... );
                    },
                    details::filter_evtcontext_t<In...>::apply( *this, this->m_inputs ) );
              },
              this->m_outputs );
          return StatusCode::SUCCESS;
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
        : details::DataHandleMixin<std::tuple<Out...>, std::tuple<In...>, Traits_> {
      using details::DataHandleMixin<std::tuple<Out...>, std::tuple<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute( const EventContext& ctx ) const override final {
        try {
          std::apply(
              [&]( auto&... ohandle ) {
                std::apply(
                    [&ohandle..., &ctx, this]( bool passed, auto&&... data ) {
                      this->execState( ctx ).setFilterPassed( passed );
                      ( details::put( ohandle, std::forward<decltype( data )>( data ) ), ... );
                    },
                    details::filter_evtcontext_t<In...>::apply( *this, ctx, this->m_inputs ) );
              },
              this->m_outputs );
          return StatusCode::SUCCESS;
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
