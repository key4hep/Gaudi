/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Examples/TestVector.h>

namespace Gaudi::Examples {
  double TestVector::x() const { return m_data.x(); }
  double TestVector::y() const { return m_data.y(); }
  double TestVector::z() const { return m_data.z(); }

  void TestVector::setX( double x ) { m_data.SetX( x ); }
  void TestVector::setY( double y ) { m_data.SetY( y ); }
  void TestVector::setZ( double z ) { m_data.SetZ( z ); }
} // namespace Gaudi::Examples
