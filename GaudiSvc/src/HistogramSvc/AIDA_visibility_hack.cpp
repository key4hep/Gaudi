/*
 * AIDA_visibility_hack.cpp
 *
 *  Created on: Jan 27, 2009
 *      Author: Marco Clemencic
 */

/// @FIXME: AIDA interfaces visibility
#include "AIDA_visibility_hack.h"

#include <typeinfo>

using namespace AIDA;

#define TI(x) const std::type_info& x##Info = typeid(x)
namespace {
  TI(IBaseHistogram);
  TI(IHistogram);
  TI(IHistogram1D);
  TI(IHistogram2D);
  TI(IHistogram3D);
  TI(IProfile);
  TI(IProfile1D);
  TI(IProfile2D);
  TI(IAnnotation);
  TI(IAxis);
}
