#ifndef SCALAR_TRANSFORMER_H
#define SCALAR_TRANSFORMER_H

#include "GaudiAlg/Transformer.h"

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

    public:
      using Transformer<Out( const In&... ), Traits_>::Transformer;

      /// The main operator
      Out operator()( const In&... in ) const override final
      {
        const auto inrange = details::zip::const_range( in... );
        Out out;
        out.reserve( inrange.size() );
        auto& scalar = scalarOp();
        for ( const auto&& tuple : inrange ) {
          /// Call the scalar operator with the objects obtained from the given tuple as arguments
          details::insert(
              out, Gaudi::apply( [&]( const auto&... i ) { return scalar( details::deref( i )... ); }, tuple ) );
        }
        details::applyPostProcessing( scalar, out );
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

    public:
      using MultiTransformer<std::tuple<Out...>( const In&... ), Traits_>::MultiTransformer;

      /// The main operator
      std::tuple<Out...> operator()( const In&... in ) const final
      {
        const auto inrange = details::zip::const_range( in... );
        std::tuple<Out...> out;
        Gaudi::apply(
            [sz = inrange.size()]( auto&&... o ) {
#if __cplusplus < 201703L
              (void)std::initializer_list<int>{( o.reserve( sz ), 0 )...};
#else
              ( o.reserve( sz ), ... );
#endif
            },
            out );
        auto& scalar = scalarOp();
        for ( const auto&& tuple : inrange ) {
          Gaudi::apply(
              [&scalar, &tuple]( auto&... out ) {
                /// Call the scalar operator with the objects obtained from the given tuple
                auto data = Gaudi::apply(
                    [&scalar]( const auto&... args ) { return scalar( details::deref( args )... ); }, tuple );
                if ( data ) {
                  Gaudi::apply(
                      [&out...]( auto&&... data ) {
#if __cplusplus < 201703L
                        (void)std::initializer_list<int>{
                            ( details::insert( out, std::forward<decltype( data )>( data ) ), 0 )...};
#else
                        ( details::insert( out, std::forward<decltype( data )>( data ) ), ... );
#endif
                      },
                      std::move( *data ) );
                }
              },
              out );
        }
        details::applyPostProcessing( scalar, out ); // awaiting a post-processor call
        return out;
      }
    };
  }
}

#endif
