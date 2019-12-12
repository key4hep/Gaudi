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
// Dear emacs, this is -*- c++ -*-
#ifndef GAUDIUTILS_CREATEGUIDASSTRING_H
#define GAUDIUTILS_CREATEGUIDASSTRING_H

// System include(s):
#include <string>

namespace Gaudi {

  /// Helper function creating file identifier using the UUID mechanism
  std::string createGuidAsString();

} // namespace Gaudi

#endif // GAUDIUTILS_CREATEGUIDASSTRING_H
