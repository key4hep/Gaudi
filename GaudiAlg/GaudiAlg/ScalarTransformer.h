#ifndef SCALAR_TRANSFORMER_H
#define SCALAR_TRANSFORMER_H

#include <utility>
#include "GaudiAlg/Transformer.h"

namespace Gaudi { namespace Functional {

// Scalar->Vector adapted N->1 algorithm
template <typename ScalarOp,
          typename TransformerSignature,
          typename Traits_ = Traits::useDefaults> class ScalarTransformer;
    template <typename ScalarOp, typename Out, typename... In, typename Traits_>
    class ScalarTransformer<ScalarOp,Out(const In&...),Traits_> : public Transformer<Out(const In&...),Traits_> 
    {
    
      const ScalarOp& scalarOp() const { return static_cast<const ScalarOp&>(*this); }

    private:

      template < typename OneIn >
      inline Out processOne( const OneIn& in ) const
      {
        Out out; 
        out.reserve(in.size());
        auto& scalar = scalarOp();
        for (const auto& i : in) details::insert( out, scalar( details::deref(i) ) );
        details::apply( scalar, out );
        return out;
      }

      template< typename Tuple, std::size_t... I >
      inline decltype(auto) getScalar( const Tuple & t, std::index_sequence<I...> ) const
      {
        return scalarOp()( std::get<I>(t)... );
      }

      template < typename... ManyIn,
                 typename Indices = std::make_index_sequence<sizeof...(In)> >
      inline Out processMany( const ManyIn&... in ) const
      {
        const auto inrange = details::zip::const_range(in...); 
        Out out; 
        out.reserve(inrange.size());
        auto& scalar = scalarOp();
        for ( const auto && i : inrange ) { details::insert( out, getScalar( i, Indices() ) ); }
        details::apply( scalar, out );
        return out;
      }
 
      template < typename... TIn >
      inline typename std::enable_if< 1 == sizeof...(TIn), Out >::type
      process( const TIn&... in ) const { return processOne(in...); }

      template < typename... TIn >
      inline typename std::enable_if< 1 != sizeof...(TIn), Out >::type
      process( const TIn&... in ) const { return processMany(in...); }
   
    public:
      
      using Transformer<Out(const In&...)>::Transformer;

      Out operator()(const In&... in) const final { return process(in...); }

    };

}}

#endif
