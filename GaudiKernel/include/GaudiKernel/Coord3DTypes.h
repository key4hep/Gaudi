/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

//---------------------------------------------------------------------------------
/** @file Coord3DTypes.h
 *
 *  3D coordinate typedefs
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
//---------------------------------------------------------------------------------

#pragma once

/** @namespace Gaudi
 *
 *  General Gaudi namespace
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
#include <Math/Cartesian3D.h>
#include <Math/Cylindrical3D.h>
#include <Math/CylindricalEta3D.h>
#include <Math/Polar3D.h>
namespace Gaudi {

  // NB : Please remember to give a simple doxygen comment for each typedef

  typedef ROOT::Math::Cartesian3D<double>      Cartesian3D;  ///< Cartesian 3D point (double)
  typedef ROOT::Math::Cartesian3D<float>       Cartesian3DF; ///< Cartesian 3D point (float)
  typedef ROOT::Math::Polar3D<double>          Polar3D;      ///< Polar 3D point (double)
  typedef ROOT::Math::Polar3D<float>           Polar3DF;     ///< Polar 3D point (float)
  typedef ROOT::Math::Cylindrical3D<double>    RhoPhiZ3D;    ///< Cylindrical 3D point (double)
  typedef ROOT::Math::Cylindrical3D<float>     RhoPhiZ3DF;   ///< Cylindrical 3D point (float)
  typedef ROOT::Math::CylindricalEta3D<double> RhoEtaPhi3D;  ///< Cylindrical Eta 3D point (double)
  typedef ROOT::Math::CylindricalEta3D<float>  RhoEtaPhi3DF; ///< Cylindrical Eta 3D point (float)
} // namespace Gaudi
