/*
 * AIDA_visibility_hack.h
 *
 *  Created on: Jan 27, 2009
 *      Author: Marco Clemencic
 */

#ifndef AIDA_VISIBILITY_HACK_H_
#define AIDA_VISIBILITY_HACK_H_
/// @FIXME: AIDA interfaces visibility

// For the GAUDI_API macro
#include "GaudiKernel/Kernel.h"

// To avoid breaking STL
#include <string>
#include <vector>

// Force visibility of the classes
#define class class GAUDI_API
#include "AIDA/IBaseHistogram.h"
#include "AIDA/IHistogram1D.h"
#include "AIDA/IHistogram2D.h"
#include "AIDA/IHistogram3D.h"
#include "AIDA/IProfile1D.h"
#include "AIDA/IProfile2D.h"
#include "AIDA/IAnnotation.h"
#include "AIDA/IAxis.h"
#undef class


#endif /* AIDA_VISIBILITY_HACK_H_ */
