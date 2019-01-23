#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include "GaudiAlg/FunctionalDetails.h"
#include "GaudiAlg/FunctionalUtilities.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/apply.h"
#include <type_traits>
#include <utility>

// Adapt an Algorithm (by default, GaudiAlgorithm) so that derived classes
//   a) do not need to access the event store, and have to explicitly
//      state their data dependencies
//   b) are encouraged not to have state which depends on the events
//      (eg. histograms, counters will have to be mutable)

namespace Gaudi
{
  namespace Functional
  {

    template <typename Signature, typename Traits_ = Traits::useDefaults>
    class Transformer;

    // general N -> 1 algorithms
    template <typename Out, typename... In, typename Traits_>
    class Transformer<Out( const In&... ), Traits_>
        : public details::DataHandleMixin<std::tuple<Out>, std::tuple<In...>, Traits_>
    {
    public:
      using details::DataHandleMixin<std::tuple<Out>, std::tuple<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute() override final
      {
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

    //
    // general N -> M algorithms
    //
    template <typename Signature, typename Traits_ = Traits::useDefaults>
    class MultiTransformer;

    template <typename... Out, typename... In, typename Traits_>
    class MultiTransformer<std::tuple<Out...>( const In&... ), Traits_>
        : public details::DataHandleMixin<std::tuple<Out...>, std::tuple<In...>, Traits_>
    {
    public:
      using details::DataHandleMixin<std::tuple<Out...>, std::tuple<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute() override final
      {
        try {
          Gaudi::apply(
              [this]( auto&... ohandle ) {

#if defined( __clang__ ) && ( __clang_major__ < 6 )
// clang-5 gives a spurious warning about not using the captured `ohandle`
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-lambda-capture"
#endif

                Gaudi::apply(
                    [&ohandle...]( auto&&... data ) {
#if __cplusplus < 201703L
                      (void)std::initializer_list<int>{
                          ( details::put( ohandle, std::forward<decltype( data )>( data ) ), 0 )...};
#else
                      ( details::put( ohandle, std::forward<decltype( data )>( data ) ), ... );
#endif
                    },
                    details::filter_evtcontext_t<In...>::apply( *this, this->m_inputs ) );

#if defined( __clang__ ) && ( __clang_major__ < 6 )
#pragma clang diagnostic pop
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
    template <typename Signature, typename Traits_ = Traits::useDefaults>
    class MultiTransformerFilter;

    template <typename... Out, typename... In, typename Traits_>
    class MultiTransformerFilter<std::tuple<Out...>( const In&... ), Traits_>
        : public details::DataHandleMixin<std::tuple<Out...>, std::tuple<In...>, Traits_>
    {
    public:
      using details::DataHandleMixin<std::tuple<Out...>, std::tuple<In...>, Traits_>::DataHandleMixin;

      // derived classes can NOT implement execute
      StatusCode execute() override final
      {
        try {
          Gaudi::apply(
              [&]( auto&... ohandle ) {
                Gaudi::apply(
                    [&ohandle..., this]( bool passed, auto&&... data ) {
                      this->setFilterPassed( passed );
#if __cplusplus < 201703L
                      (void)std::initializer_list<int>{
                          ( details::put( ohandle, std::forward<decltype( data )>( data ) ), 0 )...};
#else
                      ( details::put( ohandle, std::forward<decltype( data )>( data ) ), ... );
#endif
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
  }
}

#endif
