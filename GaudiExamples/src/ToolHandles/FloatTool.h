#pragma once

#include "GaudiAlg/GaudiTool.h"
#include "GaudiKernel/AnyDataHandle.h"

namespace Gaudi
{
  namespace Examples
  {

    struct FloatTool : public GaudiTool {

      FloatTool( const std::string& type, const std::string& name, const IInterface* parent )
          : GaudiTool( type, name, parent )
      {
      }
      float getFloat() const;

      AnyDataHandle<float> m_float{"/Event/MyFloat", Gaudi::DataHandle::Reader, this};
    };
  }
}
