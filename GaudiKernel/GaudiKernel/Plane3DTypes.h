
//---------------------------------------------------------------------------------
/** @file Plane3DTypes.h
 *
 *  3D plane typedefs
 *
 *  CVS Log :-
 *  $Id: Plane3DTypes.h,v 1.1 2006/12/08 15:00:00 hmd Exp $
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
//---------------------------------------------------------------------------------

#ifndef GAUDIKERNEL_PLANE3DTYPES_H 
#define GAUDIKERNEL_PLANE3DTYPES_H 1

// Include files
#include "Math/Plane3D.h"

/** @namespace Gaudi
 *  
 *  General Gaudi namespace
 * 
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
namespace Gaudi 
{
  
  // NB : Please remember to give a simple doxygen comment for each tyedef

  typedef ROOT::Math::Plane3D Plane3D; ///< 3D plane (double)
};

#endif // KERNEL_PLANE3DTYPES_H
