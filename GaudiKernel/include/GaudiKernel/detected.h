/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef DETECTED_H
#define DETECTED_H
// implementation of Library Fundamentals TS V2 detected idiom,
// taken from http://en.cppreference.com/w/cpp/experimental/is_detected
// and http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4436.pdf

#include <type_traits>

namespace Gaudi::cpp17 {
  namespace details {

    /// Implementation of the detection idiom (negative case).
    template <typename Default, typename AlwaysVoid, template <typename...> class Op, typename... Args>
    struct detector {
      constexpr static bool value = false;
      using type                  = Default;
      using value_t               = std::false_type;
    };

    /// Implementation of the detection idiom (positive case).
    template <typename Default, template <typename...> class Op, typename... Args>
    struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
      constexpr static bool value = true;
      using type                  = Op<Args...>;
      using value_t               = std::true_type;
    };
  } // namespace details

  template <template <class...> class Op, class... Args>
  using is_detected = details::detector<void, void, Op, Args...>;

  template <template <class...> class Op, class... Args>
  inline constexpr bool is_detected_v = is_detected<Op, Args...>::value;

  template <template <class...> class Op, class... Args>
  using detected_t = typename is_detected<Op, Args...>::type;

  // Op<Args...> if that is a valid type, otherwise Default.
  template <typename Default, template <typename...> class Op, typename... Args>
  using detected_or_t = typename details::detector<Default, void, Op, Args...>::type;

} // namespace Gaudi::cpp17

#endif
