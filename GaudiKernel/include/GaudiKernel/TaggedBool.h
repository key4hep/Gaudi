// Copyright (C) 2016 Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See http://www.boost.org/LICENSE_1_0.txt)
//
// copied from https://github.com/akrzemi1/explicit/tree/master/include/ak_toolkit
// last commit: bd572fe05a700cc6766b3d09f8916c9975ccbb45

#ifndef GAUDIKERNEL_TAGGEDBOOL_H
#define GAUDIKERNEL_TAGGEDBOOL_H

namespace Gaudi {
  namespace tagged_bool_ns { // artificial namespace to prevent ADL lookups in namespace Gaudi

    template <typename Tag>
    class tagged_bool {
      bool value;
      template <typename /*OtherTag*/>
      friend class tagged_bool;

    public:
      constexpr explicit tagged_bool( bool v ) : value{ v } {}

      constexpr explicit tagged_bool( int )    = delete;
      constexpr explicit tagged_bool( double ) = delete;
      constexpr explicit tagged_bool( void* )  = delete;

      template <typename OtherTag>
      constexpr explicit tagged_bool( tagged_bool<OtherTag> b ) : value{ b.value } {}

      constexpr explicit operator bool() const { return value; }
      constexpr tagged_bool operator!() const { return tagged_bool{ !value }; }

      friend constexpr bool operator==( tagged_bool l, tagged_bool r ) { return l.value == r.value; }
      friend constexpr bool operator!=( tagged_bool l, tagged_bool r ) { return l.value != r.value; }
    };
  } // namespace tagged_bool_ns

  using tagged_bool_ns::tagged_bool; // with this tagged_bool is in namespace xplicit but with disabled ADL
} // namespace Gaudi

#endif
