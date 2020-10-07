/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/IDataHandleHolder.h"

#include <sstream>

std::string const Gaudi::DataHandle::default_type = "unknown_t";

namespace Gaudi {
  std::string DataHandle::pythonRepr() const {
    std::ostringstream ost;
    ost << "DataHandle(" << fullKey() << ", '";
    switch ( mode() ) {
    case DataHandle::Mode::Reader:
      ost << "R";
      break;
    case DataHandle::Mode::Writer:
      ost << "W";
      break;
    default:
      ost << "UNKNOWN";
      break;
    }
    ost << "', '" + default_type + "')";
    return ost.str();
  }

  std::ostream& operator<<( std::ostream& str, const DataHandle& d ) {
    str << d.fullKey() << "  m: " << d.mode();
    if ( d.owner() ) str << "  o: " << d.owner()->name();
    return str;
  }
} // namespace Gaudi
