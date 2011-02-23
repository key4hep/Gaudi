
//---------------------------------------------------------------------------------
/** @file Transform4DTypes.h
 *
 *  4D transformation typedefs
 *
 *  CVS Log :-
 *  $Id: Transform4DTypes.h,v 1.1 2006/12/08 15:00:02 hmd Exp $
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
//---------------------------------------------------------------------------------

#ifndef GAUDIKERNEL_TRANSFORM4DTYPES_H 
#define GAUDIKERNEL_TRANSFORM4DTYPES_H 1

// Include files
#include "Math/LorentzRotation.h"

/** @namespace Gaudi 
 *  
 *  General Gaudi namespace
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-23
 */
namespace Gaudi 
{
  // NB : Please remember to give a simple doxygen comment for each tyedef

  typedef ROOT::Math::LorentzRotation LorentzRotation; ///< Lorentz rotation

};
#endif // KERNEL_TRANSFORM4DTYPES_H
