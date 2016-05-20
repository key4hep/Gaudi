#ifndef SCALAR_TRANSFORM_ALGORITHM_H
#define SCALAR_TRANSFORM_ALGORITHM_H
#include "GaudiAlg/TransformAlgorithm.h"
#include "boost/optional.hpp"
#include <type_traits>

// Scalar->Vector adapted 1->1 algoritm
// TODO: make variadic, then check that # of elems in each input container is the same...
template <typename ExternalSignature, typename ScalarFunctor> class ScalarTransformAlgorithm;

namespace detail {

   // we want 'user code' to never get pointers as input -- so adapt such that
   // if we get a pointer, we derefence it before calling the 'user code'.

   template <typename In, typename Fun>
   auto adapt_in( const Fun& f, const In& in,
                  typename std::enable_if< !std::is_pointer<In>::value>::type* = nullptr )
   { return f(in); }

   template <typename In, typename Fun>
   auto adapt_in( const Fun& f, const In* in ) { return f(*in); }

   // we always receive a T, but sometimes we must output a T* -- so adapt if neccessary...
   // (backwards compatibility: until we get a 'move aware' AnyDataHandle)

   template <typename Out>
   Out adapt_out( Out&& in ) { return std::forward<Out>(in); }

   template <typename Out>
   Out adapt_out( typename std::remove_pointer<Out>::type&& in )
   { return new typename std::remove_pointer<Out>::type( std::forward<typename std::remove_pointer<Out>::type>(in) ); }

   // adapt to differences between eg. std::vector (which has push_back) and KeyedContainer (which has insert)

   template <typename> struct void_t { typedef void type; };
   template <typename T> using is_valid_t = typename void_t<T>::type;

   template <typename T, typename SFINAE = void> struct has_push_back : std::false_type {};
   template <typename T>                         struct has_push_back<T, is_valid_t<decltype(std::declval<T>().push_back( std::declval<typename T::value_type>() ))>> : std::true_type {};

   template <typename T, typename SFINAE = void> struct has_insert : std::false_type {};
   template <typename T>                         struct has_insert<T, is_valid_t<decltype(std::declval<T>().insert( std::declval<typename T::value_type>() ))>> : std::true_type {};

   template <typename Container, typename Value>
   typename std::enable_if< has_push_back<Container>::value, void >::type
   adapt_insert(Container& c, Value&& v) { c.push_back( std::forward<Value>(v) ); }

   template <typename Container, typename Value>
   typename std::enable_if< has_insert<Container>::value, void >::type
   adapt_insert(Container& c, Value&& v) { c.insert( std::forward<Value>(v) ); }
}

template <typename Out, typename In, typename ScalarFunctor>
class ScalarTransformAlgorithm<Out(const In&),ScalarFunctor> : public TransformAlgorithm<Out(const In&)> {
    ScalarFunctor m_f;
public:
    using KeyValue = typename TransformAlgorithm<Out(const In&)>::KeyValue;

    template <typename Functor>
    ScalarTransformAlgorithm(const std::string& name, ISvcLocator* locator,
                             KeyValue&& input,
                             KeyValue&& output,
                             Functor&& scalar_functor)
        : TransformAlgorithm<Out(const In&)> ( name , locator,
                                             { std::forward<KeyValue>(input) },
                                             std::forward<KeyValue>(output) ),
        m_f( std::forward<Functor>(scalar_functor) ) {
    }

    Out operator()(const In& in) const override {
        Out out; out.reserve(in.size());
        for (const auto& i : in) {
            auto o = detail::adapt_in( m_f, i );
            if (o) detail::adapt_insert(out, detail::adapt_out<typename Out::value_type>( std::move( *o ) ) );
        }
        return out;
    }
};

#endif
