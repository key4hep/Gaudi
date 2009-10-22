// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IMagneticFieldSvc.h,v 1.8 2006/05/29 08:46:06 hmd Exp $
#ifndef GAUDIKERNEL_IMAGNETICFIELDSVC_H
#define GAUDIKERNEL_IMAGNETICFIELDSVC_H


// Include files
#include "GaudiKernel/IService.h"
#include "GaudiKernel/StatusCode.h"


// Forward declarations
template <class TYPE> class SvcFactory;
namespace ROOT {
  namespace Math {
    class DefaultCoordinateSystemTag;

    // from Math/Point3Dfwd.h
    template<class CoordSystem, class Tag> class PositionVector3D;
    // from Math/Vector3Dfwd.h
    template<class CoordSystem, class Tag> class DisplacementVector3D;
    // from Math/Point3Dfwd.h
    template<typename T> class Cartesian3D;
    // from Math/Point3Dfwd.h
    typedef PositionVector3D< Cartesian3D<double>,DefaultCoordinateSystemTag> XYZPoint;
    // from Math/Vector3Dfwd.h
    typedef DisplacementVector3D< Cartesian3D<double>,DefaultCoordinateSystemTag > XYZVector;
  }
}

/** @class IMagneticFieldSvc IMagneticFieldSvc.h GaudiKernel/IMagneticFieldSvc.h

    The interface to the MagneticFieldSvc

    @author Iain Last
*/
class GAUDI_API IMagneticFieldSvc: virtual public IService {

public:
  /// InterfaceID
  DeclareInterfaceID(IMagneticFieldSvc,2,0);

  // Get the magnetic field vector at a given point in space.
  // Input: XYZPoint - Point at which magnetic field vector is to be given.
  // Output: XYZVector - Magnetic field vector.
  // Return: StatusCode SUCCESS if calculation was performed.
  virtual StatusCode fieldVector( const ROOT::Math::XYZPoint& xyz,
				  ROOT::Math::XYZVector& fvec ) const = 0;

};

#endif  // GAUDIKERNEL_IMAGNETICFIELDSVC_H
