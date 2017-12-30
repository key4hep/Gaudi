#pragma once

#include "GaudiAlg/GaudiTool.h"
#include "GaudiKernel/DataObjectHandle.h"

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

      DataObjectReadHandle<float> m_float{this, "Input", "/Event/MyFloat"};
    };
  }
}
