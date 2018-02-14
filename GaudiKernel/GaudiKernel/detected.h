#ifndef DETECTED_H
#define DETECTED_H
// implementation of Library Fundamentals TS V2 detected idiom,
// based on http://en.cppreference.com/w/cpp/experimental/is_detected
// and the libstdc++ source, specificially libstdc++-v3/include/std/type_traits

namespace Gaudi
{
  namespace cpp17
  {
    namespace details
    {
      template <typename...>
      struct void_t_ {
        using type = void;
      };
      template <typename... T>
      using void_t = typename void_t_<T...>::type;

      /// Implementation of the detection idiom (negative case).
      template <typename Default, typename AlwaysVoid, template <typename...> class Op, typename... Args>
      struct detector {
        using type = Default;
      };

      /// Implementation of the detection idiom (positive case).
      template <typename Default, template <typename...> class Op, typename... Args>
      struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
        using type = Op<Args...>;
      };
    }

    // Op<Args...> if that is a valid type, otherwise Default.
    template <typename Default, template <typename...> class Op, typename... Args>
    using detected_or_t = typename details::detector<Default, void, Op, Args...>::type;
  }
}

#endif
