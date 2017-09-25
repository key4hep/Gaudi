#ifndef SCALAR_TRANSFORMER_H
#define SCALAR_TRANSFORMER_H

#include "GaudiAlg/Transformer.h"
#include <utility>

namespace Gaudi
{
  namespace Functional
  {

    // Scalar->Vector adapted N->1 algorithm
    template <typename ScalarOp, typename TransformerSignature, typename Traits_ = Traits::useDefaults>
    class ScalarTransformer;
    template <typename ScalarOp, typename Out, typename... In, typename Traits_>
    class ScalarTransformer<ScalarOp, Out( const In&... ), Traits_> : public Transformer<Out( const In&... ), Traits_>
    {

      /// Access the scalar operator
      const ScalarOp& scalarOp() const { return static_cast<const ScalarOp&>( *this ); }

    private:
      /// Call the scalar operator with the objects obtained from the given tuple
      template <typename Tuple, typename Scalar, std::size_t... I>
      inline decltype( auto ) getScalar( const Tuple& t, const Scalar& s, std::index_sequence<I...> ) const
      {
        return s( details::deref( std::get<I>( t ) )... );
      }

    public:
      using Transformer<Out( const In&... ), Traits_>::Transformer;

      /// The main operator
      Out operator()( const In&... in ) const override final
      {
        const auto inrange = details::zip::const_range( in... );
        Out out;
        out.reserve( inrange.size() );
        auto& scalar = scalarOp();
        for ( const auto&& i : inrange ) {
          details::insert( out, getScalar( i, scalar, std::index_sequence_for<In...>{} ) );
        }
        details::applyPostProcessing( scalar, out ); // awaiting a post-processor call
        return out;
      }
    };

    // Scalar->Vector adapted N->M algorithm
    template <typename ScalarOp, typename TransformerSignature, typename Traits_ = Traits::useDefaults>
    class MultiScalarTransformer;
    template <typename ScalarOp, typename... Out, typename... In, typename Traits_>
    class MultiScalarTransformer<ScalarOp, std::tuple<Out...>( const In&... ), Traits_>
        : public MultiTransformer<std::tuple<Out...>( const In&... ), Traits_>
    {

      /// Access the scalar operator
      const ScalarOp& scalarOp() const { return static_cast<const ScalarOp&>( *this ); }

    private:
      /// Reserve the given size in all output containers
      template <typename Tuple, std::size_t... I>
      inline void reserve( Tuple& t, const std::size_t resSize, std::index_sequence<I...> ) const
      {
        std::initializer_list<long unsigned int>{( std::get<I>( t ).reserve( resSize ), I )...};
      }

      /// Call the scalar operator with the objects obtained from the given tuple
      template <typename Tuple, typename Scalar, std::size_t... I>
      inline decltype( auto ) getScalar( const Tuple& t, const Scalar& s, std::index_sequence<I...> ) const
      {
        return s( details::deref( std::get<I>( t ) )... );
      }

      /// Insert the returned tuple of objects into the correct containers
      template <typename InTuple, typename OutTuple, std::size_t... I>
      void insert( InTuple&& in, OutTuple& out, std::index_sequence<I...> ) const
      {
        if ( in ) {
          std::initializer_list<long unsigned int>{
              ( details::insert( std::get<I>( out ), std::move( std::get<I>( *in ) ) ), I )...};
        }
      }

    public:
      using MultiTransformer<std::tuple<Out...>( const In&... ), Traits_>::MultiTransformer;

      /// The main operator
      std::tuple<Out...> operator()( const In&... in ) const final
      {
        const auto inrange = details::zip::const_range( in... );
        std::tuple<Out...> out;
        reserve( out, inrange.size(), std::index_sequence_for<Out...>{} );
        auto& scalar = scalarOp();
        for ( const auto&& i : inrange ) {
          insert( getScalar( i, scalar, std::index_sequence_for<In...>{} ), out, std::index_sequence_for<Out...>{} );
        }
        details::applyPostProcessing( scalar, out ); // awaiting a post-processor call
        return out;
      }
    };
  }
}

#endif
