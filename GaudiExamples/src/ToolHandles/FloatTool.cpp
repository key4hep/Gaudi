#include "FloatTool.h"

float Gaudi::Examples::FloatTool::getFloat() const { return *m_float.get(); }

DECLARE_COMPONENT( Gaudi::Examples::FloatTool )
