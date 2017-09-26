/*
 * AIDA_visibility_hack.cpp
 *
 *  Created on: Jan 27, 2009
 *      Author: Marco Clemencic
 */

#ifdef __ICC
// disable icc remark #177: declared but never referenced
#pragma warning( disable : 177 )
#elif __clang__
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

// For the GAUDI_API macro
#include "GaudiKernel/Kernel.h"

// To avoid breaking STL
#include <string>
#include <vector>

// Force visibility of the classes
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-macro"
#endif
#define class class GAUDI_API
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include <AIDA/IAnnotation.h>
#include <AIDA/IAxis.h>
#include <AIDA/IBaseHistogram.h>
#include <AIDA/IHistogram1D.h>
#include <AIDA/IHistogram2D.h>
#include <AIDA/IHistogram3D.h>
#include <AIDA/IProfile1D.h>
#include <AIDA/IProfile2D.h>
#undef class

#include <typeinfo>

using namespace AIDA;

#define TI( x ) const std::type_info& x##Info = typeid( x )
namespace
{
  TI( IBaseHistogram );
  TI( IHistogram );
  TI( IHistogram1D );
  TI( IHistogram2D );
  TI( IHistogram3D );
  TI( IProfile );
  TI( IProfile1D );
  TI( IProfile2D );
  TI( IAnnotation );
  TI( IAxis );
}
