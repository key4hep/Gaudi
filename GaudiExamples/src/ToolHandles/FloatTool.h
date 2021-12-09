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
#pragma once

#include "GaudiAlg/GaudiTool.h"
#include "GaudiKernel/DataObjectHandle.h"

namespace Gaudi {
  namespace Examples {

    struct FloatTool : public GaudiTool {

      FloatTool( const std::string& type, const std::string& name, const IInterface* parent )
          : GaudiTool( type, name, parent ) {}

      float getFloat() const;

      DataObjectReadHandle<float> m_float{ this, "Input", "/Event/MyFloat" };
    };
  } // namespace Examples
} // namespace Gaudi
