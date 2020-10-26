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

//---------------------------------------------------------------------------------
/** @file Plane3DTypes.h
 *
 *  3D plane typedefs
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
//---------------------------------------------------------------------------------

#ifndef GAUDIKERNEL_PLANE3DTYPES_H
#define GAUDIKERNEL_PLANE3DTYPES_H 1

#ifdef __INTEL_COMPILER // Disable ICC remark from Math headers
#  pragma warning( push )
#  pragma warning( disable : 1572 ) // Floating-point equality and inequality comparisons are unreliable
#endif

// Include files
#include "Math/Plane3D.h"

#ifdef __INTEL_COMPILER
#  pragma warning( pop )
#endif

/** @namespace Gaudi
 *
 *  General Gaudi namespace
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
namespace Gaudi {

  // NB : Please remember to give a simple doxygen comment for each tyedef

  typedef ROOT::Math::Plane3D Plane3D; ///< 3D plane (double)
} // namespace Gaudi

#endif // KERNEL_PLANE3DTYPES_H
