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
#pragma once

#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/Vector3DTypes.h>

namespace Gaudi::Examples {
  class TestVector : public DataObject {
  public:
    TestVector()                    = default;
    TestVector( const TestVector& ) = delete;
    TestVector( TestVector&& )      = default;
    ~TestVector()                   = default;

    double x() const;
    double y() const;
    double z() const;

    void setX( double x );
    void setY( double y );
    void setZ( double z );

  private:
    Gaudi::XYZVector m_data;
  };
} // namespace Gaudi::Examples
