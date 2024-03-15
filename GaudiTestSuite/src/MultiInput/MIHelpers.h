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

  namespace TestSuite {
    namespace MultiInput {
      typedef std::tuple<long, CLID, std::string, std::string, unsigned long, unsigned long> RootAddressArgs;
      inline std::istream& operator>>( std::istream& i, RootAddressArgs& addr ) {
        long          s{ 0 };
        CLID          c{ 0 };
        std::string   p1, p2;
        unsigned long ip1{ 0 }, ip2{ 0 };
        i >> s >> c >> p1 >> p2 >> ip1 >> ip2;
        addr = RootAddressArgs{ s, c, p1, p2, ip1, ip2 };
        return i;
      }
      inline Gaudi::RootAddress* make_address( RootAddressArgs args ) {
        long          s{ 0 };
        CLID          c{ 0 };
        std::string   p1, p2;
        unsigned long ip1{ 0 }, ip2{ 0 };
        std::tie( s, c, p1, p2, ip1, ip2 ) = args;
        return new Gaudi::RootAddress( s, c, p1, p2, ip1, ip2 );
      }
    } // namespace MultiInput
  }   // namespace TestSuite
} // namespace Gaudi
#endif
