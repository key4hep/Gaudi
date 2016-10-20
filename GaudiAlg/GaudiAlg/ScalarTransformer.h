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
    
      /// Access the scalar operator
      const ScalarOp& scalarOp() const { return static_cast<const ScalarOp&>(*this); }

    private:

      /// Process a single input container
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

      /// Call the scalar operator with the objects obtained from the given tuple
      template< typename Tuple, typename Scalar, std::size_t... I >
      inline decltype(auto) getScalar( const Tuple & t, 
                                       const Scalar & s, 
                                       std::index_sequence<I...> ) const
      {
        return s( std::get<I>(t)... );
      }

      /// Process more than one container
      template < typename... ManyIn >
      inline Out processMany( const ManyIn&... in ) const
      {
        const auto inrange = details::zip::const_range(in...); 
        Out out; 
        out.reserve(inrange.size());
        auto& scalar = scalarOp();
        for ( const auto && i : inrange ) 
        { details::insert( out, getScalar( i, scalar, std::index_sequence_for<In...>{} ) ); }
        details::apply( scalar, out );
        return out;
      }
 
      /// specialise for 1 container
      template < typename... TIn >
      inline typename std::enable_if< 1 == sizeof...(TIn), Out >::type
      process( const TIn&... in ) const { return processOne(in...); }

      /// specialise for >1 container
      template < typename... TIn >
      inline typename std::enable_if< 1 != sizeof...(TIn), Out >::type
      process( const TIn&... in ) const { return processMany(in...); }
   
    public:
      
      using Transformer<Out(const In&...),Traits_>::Transformer;

      /// The main operator
      Out operator()(const In&... in) const final { return process(in...); }

    };

// Scalar->Vector adapted N->M algorithm
template <typename ScalarOp,
          typename TransformerSignature,
          typename Traits_ = Traits::useDefaults> class MultiScalarTransformer;
    template <typename ScalarOp, typename... Out, typename... In, typename Traits_>
    class MultiScalarTransformer<ScalarOp,std::tuple<Out...>(const In&...),Traits_> : public MultiTransformer<std::tuple<Out...>(const In&...),Traits_> 
    {
    
      /// Access the scalar operator
      const ScalarOp& scalarOp() const { return static_cast<const ScalarOp&>(*this); }

    private:

      /// Reserve the given size in all output containers
      template< typename Tuple, std::size_t... I >
      inline void reserve( Tuple & t, 
                           const std::size_t resSize, 
                           std::index_sequence<I...> ) const
      {
        std::initializer_list<long unsigned int> { (std::get<I>(t).reserve(resSize),I)... };
      }

      /// Process a single input container
      template < typename OneIn >
      inline std::tuple<Out...> processOne( const OneIn& in ) const
      {
        std::tuple<Out...> out; 
        reserve( out, in.size(), std::index_sequence_for<Out...>{} );
        auto& scalar = scalarOp();
        for ( const auto& i : in ) { details::insert( out, scalar( details::deref(i) ) ); }
        details::apply( scalar, out );
        return out;
      }

      /// Call the scalar operator with the objects obtained from the given tuple
      template< typename Tuple, typename Scalar, std::size_t... I >
      inline decltype(auto) getScalar( const Tuple & t, 
                                       const Scalar & s, 
                                       std::index_sequence<I...> ) const
      {
        return s( std::get<I>(t)... );
      }

      /// Insert the returned tuple of objects into the correct containers
      template< typename InTuple, typename OutTuple, std::size_t... I >
      void insert( const InTuple & in, OutTuple & out, std::index_sequence<I...> ) const
      {
        if ( in )
        { std::initializer_list<long unsigned int> 
          { ( details::insert( std::get<I>(out), std::get<I>(*in) ), I )... }; }
      }

      /// Process more than one container
      template < typename... ManyIn >
      inline std::tuple<Out...> processMany( const ManyIn&... in ) const
      {
        const auto inrange = details::zip::const_range(in...); 
        std::tuple<Out...> out; 
        reserve( out, inrange.size(), std::index_sequence_for<Out...>{} );
        auto& scalar = scalarOp();
        for ( const auto && i : inrange ) 
        {
          insert( getScalar( i, scalar, std::index_sequence_for<In...>{} ), out,
                  std::index_sequence_for<Out...>{} );
        }
        details::apply( scalar, out );
        return out;
      }
 
      /// specialise for 1 container
      template < typename... TIn >
      inline typename std::enable_if< 1 == sizeof...(TIn), std::tuple<Out...> >::type
      process( const TIn&... in ) const { return processOne(in...); }

      /// specialise for >1 container
      template < typename... TIn >
      inline typename std::enable_if< 1 != sizeof...(TIn), std::tuple<Out...> >::type
      process( const TIn&... in ) const { return processMany(in...); }
   
    public:
      
      using MultiTransformer<std::tuple<Out...>(const In&...),Traits_>::MultiTransformer;
 
      /// The main operator
      std::tuple<Out...> operator()(const In&... in) const final { return process(in...); }

    };

}}

#endif
