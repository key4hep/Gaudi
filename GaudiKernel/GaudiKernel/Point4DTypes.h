
//---------------------------------------------------------------------------------
/** @file Point4DTypes.h
 *
 *  4D point typedefs
 *
 *  CVS Log :-
 *  $Id: Point4DTypes.h,v 1.2 2007/01/24 14:55:00 hmd Exp $
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
//---------------------------------------------------------------------------------

#ifndef GAUDIKERNEL_POINT4DTYPES_H 
#define GAUDIKERNEL_POINT4DTYPES_H 1

// Include files
#include "GaudiKernel/Vector4DTypes.h"

/** @namespace Gaudi
 *  
 *  General Gaudi namespace
 * 
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
namespace Gaudi 
{
  
  /** 4D point (space-time) 
   *  @todo use Point4D from MathCore
   */
  typedef LorentzVector XYZTPoint; 

}

#endif // KERNEL_POINT4DTYPES_H
