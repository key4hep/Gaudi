#ifndef SCALAR_TRANSFORMER_H
#define SCALAR_TRANSFORMER_H

#include "GaudiAlg/Transformer.h"
#include "boost/optional.hpp"
#include <algorithm>
#include <type_traits>

namespace Gaudi { namespace Functional {

   // Scalar->Vector adapted 1->1 algoritm
   // TODO: make variadic, then check that # of elems in each input container is the same,
   //       and use a 'zip' iterator (dot product) for the input...
   // TODO: add a 'tensor' variant
   //
   template <typename ScalarSignature, typename TransformerSignature, typename Traits_ = Traits::useDefaults> class ScalarTransformer;

   namespace details {

      constexpr struct invoke_t {
          template <typename In, typename = typename std::enable_if< !std::is_pointer<In>::value>::type>
          const In& operator()( const In& in ) const { return in; }

          template <typename In>
          const In& operator()( const In* in ) const { return *in; }
      } adapt_in {};

      // adapt to differences between eg. std::vector (which has push_back) and KeyedContainer (which has insert)
      // adapt to getting a T, and a container wanting T* by doing new T{ std::move(out) }
      // adapt to getting a boost::optional<T>

      constexpr struct insert_t {
          // for Container<T*>, return T
          template <typename Container>
          using c_remove_ptr_t = typename std::remove_pointer<typename Container::value_type>::type;

          template <typename Container, typename Value>
          auto operator()(Container& c, Value&& v) const -> decltype( c.push_back(v) ) { return c.push_back( std::forward<Value>(v) ); }

          template <typename Container, typename Value>
          auto operator()(Container& c, Value&& v) const -> decltype( c.insert(v) ) { return c.insert( std::forward<Value>(v) ); }

          // Container<T*> with T&& as argument
          template <typename Container, typename = typename std::enable_if< std::is_pointer<typename Container::value_type>::value >::type >
          auto operator()(Container& c, c_remove_ptr_t<Container>&& v) const
          { return operator()( c, new c_remove_ptr_t<Container>{ std::move(v) } ); }

          template <typename Container, typename Value>
          void operator()(Container& c, boost::optional<Value>&& v) const { if (v) operator()(c,std::move(*v)); }
      } insert_scalar {};

   }


   template <typename ScalarOp, typename Out, typename In, typename Traits_>
   class ScalarTransformer<ScalarOp,Out(const In&),Traits_> : public Transformer<Out(const In&),Traits_> {
   public:
     using Transformer<Out(const In&)>::Transformer;
                                  //add 'chunking' properties (when we go task based),
                                  // would be much easier in Marco's property design,
                                  // as then we just declare properties and do not have
                                  // to explicitly write a c'tor (body)...
     Out operator()(const In& in) const final {
       Out out; out.reserve(in.size());
       const auto& scalar = scalarOp();
       for (const auto& i : in) {
          details::insert_scalar( out, scalar( details::adapt_in(i)) );
       }
       return out;
     }
   private:
     const ScalarOp& scalarOp() const { return static_cast<const ScalarOp&>(*this); }
   };

}}

#endif
