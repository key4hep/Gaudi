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
          invoke( std::index_sequence_for<In...>{} );
          return StatusCode::SUCCESS;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // instead they MUST implement this operator
      virtual Out operator()( const In&... ) const = 0;

    private:
      template <std::size_t... I>
      void invoke( std::index_sequence<I...> )
      {
        details::put( std::get<0>( this->m_outputs ),
                      details::as_const( *this )( details::deref( std::get<I>( this->m_inputs ).get() )... ) );
      }
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
          invoke( std::index_sequence_for<In...>{}, std::index_sequence_for<Out...>{} );
          return StatusCode::SUCCESS;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // instead they MUST implement this operator
      virtual std::tuple<Out...> operator()( const In&... ) const = 0;

    private:
      template <std::size_t... I, std::size_t... O>
      void invoke( std::index_sequence<I...>, std::index_sequence<O...> )
      {
        auto out = details::as_const( *this )( details::deref( std::get<I>( this->m_inputs ).get() )... );
        (void)std::initializer_list<int>{
            ( details::put( std::get<O>( this->m_outputs ), std::get<O>( std::move( out ) ) ), 0 )...};
      }
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
          invoke( std::index_sequence_for<In...>{}, std::index_sequence_for<Out...>{} );
          return StatusCode::SUCCESS;
        } catch ( GaudiException& e ) {
          ( e.code() ? this->warning() : this->error() ) << e.message() << endmsg;
          return e.code();
        }
      }

      // instead they MUST implement this operator
      virtual std::tuple<bool, Out...> operator()( const In&... ) const = 0;

    private:
      template <std::size_t... I, std::size_t... O>
      void invoke( std::index_sequence<I...>, std::index_sequence<O...> )
      {
        auto out = details::as_const( *this )( details::deref( std::get<I>( this->m_inputs ).get() )... );
        this->setFilterPassed( std::get<0>( out ) );
        (void)std::initializer_list<int>{
            ( details::put( std::get<O>( this->m_outputs ), std::get<O + 1>( std::move( out ) ) ), 0 )...};
      }
    };
  }
}

#endif
