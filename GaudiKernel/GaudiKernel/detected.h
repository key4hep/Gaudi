#ifndef DETECTED_H
#define DETECTED_H
// implementation of Library Fundamentals TS V2 detected idiom,
// taken from http://en.cppreference.com/w/cpp/experimental/is_detected
// and http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4436.pdf

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
        constexpr static bool value = false;
        using type                  = Default;
      };

      /// Implementation of the detection idiom (positive case).
      template <typename Default, template <typename...> class Op, typename... Args>
      struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
        constexpr static bool value = true;
        using type                  = Op<Args...>;
      };
    }

    template <template <class...> class Op, class... Args>
    using is_detected = details::detector<void, void, Op, Args...>;

    template <template <class...> class Op, class... Args>
    using detected_t = typename is_detected<Op, Args...>::type;

    // Op<Args...> if that is a valid type, otherwise Default.
    template <typename Default, template <typename...> class Op, typename... Args>
    using detected_or_t = typename details::detector<Default, void, Op, Args...>::type;
  }
}

#endif
