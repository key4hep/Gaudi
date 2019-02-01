#ifndef GAUDIEXAMPLES_MULTIINPUT_COMMON
#define GAUDIEXAMPLES_MULTIINPUT_COMMON

#include "RootCnv/RootAddress.h"
#include <istream>
#include <ostream>
#include <tuple>

namespace Gaudi {
  inline std::ostream& operator<<( std::ostream& o, const RootAddress& addr ) {
    return o << addr.svcType() << ' ' << addr.clID() << ' ' << addr.par()[0] << ' ' << addr.par()[1] << ' '
             << addr.ipar()[0] << ' ' << addr.ipar()[1];
  }

  namespace Examples {
    namespace MultiInput {
      typedef std::tuple<long, CLID, std::string, std::string, unsigned long, unsigned long> RootAddressArgs;
      inline std::istream& operator>>( std::istream& i, RootAddressArgs& addr ) {
        long          s{0};
        CLID          c{0};
        std::string   p1, p2;
        unsigned long ip1{0}, ip2{0};
        i >> s >> c >> p1 >> p2 >> ip1 >> ip2;
        addr = RootAddressArgs{s, c, p1, p2, ip1, ip2};
        return i;
      }
      inline Gaudi::RootAddress* make_address( RootAddressArgs args ) {
        long          s{0};
        CLID          c{0};
        std::string   p1, p2;
        unsigned long ip1{0}, ip2{0};
        std::tie( s, c, p1, p2, ip1, ip2 ) = args;
        return new Gaudi::RootAddress( s, c, p1, p2, ip1, ip2 );
      }
    } // namespace MultiInput
  }   // namespace Examples
} // namespace Gaudi
#endif
