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
#include <Gaudi/Accumulators/HistogramArray.h>
#include <fmt/format.h>

#if FMT_VERSION < 80000
namespace fmt {
  template <typename T>
  const T& runtime( const T& v ) {
    return v;
  }
} // namespace fmt
#endif

std::string Gaudi::Accumulators::details::FormatHistDefault::operator()( size_t n ) {
  return fmt::format( fmt::runtime( text ), n );
}
