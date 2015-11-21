#ifndef SCALAR_TRANSFORM_ALGORITHM_H
#define SCALAR_TRANSFORM_ALGORITHM_H
#include "GaudiAlg/TransformAlgorithm.h"
#include "boost/optional.hpp"
#include <type_traits>

// Scalar->Vector adapted 1->1 algoritm
// TODO: make variadic, then check that # of elems in each input container is the same...
template <typename ExternalSignature, typename ScalarFunctor> class ScalarTransformAlgorithm;

namespace detail {

   template <typename In, typename Fun>
   auto adapt_in( const Fun& f, const In& in,
                  typename std::enable_if< !std::is_pointer<In>::value>::type* = nullptr )
   { return f(in); }

   template <typename In, typename Fun>
   auto adapt_in( const Fun& f, const In* in ) { return f(*in); }

   template <typename Out>
   Out adapt_out( Out&& in ) { return std::forward<Out>(in); }

   template <typename Out>
   Out adapt_out( typename std::remove_pointer<Out>::type&& in )
   { return new typename std::remove_pointer<Out>::type( std::forward<typename std::remove_pointer<Out>::type>(in) ); }
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
            //@FIXME: this requires container::insert(container::value_type&&)  to exist
            //     -- must be made more generic!!
            if (o) out.insert( detail::adapt_out<typename Out::value_type>( std::move( *o ) ) );
        }
        return out;
    }
};

#endif
