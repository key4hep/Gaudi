#include "CLHEP/Geometry/BasicVector3D.h"
#include "CLHEP/Geometry/Normal3D.h"
#include "CLHEP/Geometry/Plane3D.h"
#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Geometry/Transform3D.h"
#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Matrix/DiagMatrix.h"
#include "CLHEP/Matrix/GenMatrix.h"
#include "CLHEP/Matrix/Matrix.h"
#include "CLHEP/Matrix/Pile.h"
#include "CLHEP/Matrix/SymMatrix.h"
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Vector/EulerAngles.h"
#include "CLHEP/Vector/LorentzRotation.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Vector/Rotation.h"
#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Vector/TwoVector.h"

typedef MyHEP::FloatVector FloatVector;
typedef MyHEP::FloatPoint  MyHepPoint;

typedef CLHEP::Hep2Vector              Hep2Vector;
typedef CLHEP::Hep3Vector              Hep3Vector;
typedef CLHEP::HepLorentzVector        HepLorentzVector;
typedef CLHEP::HepRotation             HepRotation;
typedef CLHEP::HepLorentzRotation      HepLorentzRotation;
typedef CLHEP::HepEulerAngles          HepEulerAngles;
typedef CLHEP::HepVector               HepVector;
typedef CLHEP::HepMatrix               HepMatrix;
typedef CLHEP::HepSymMatrix            HepSymMatrix;
typedef CLHEP::HepGenMatrix            HepGenMatrix;
typedef CLHEP::HepPile                 HepPile;
typedef CLHEP::HepDiagMatrix           HepDiagMatrix;
typedef HepGeom::Vector3D<double>      HepVector3D;
typedef HepGeom::BasicVector3D<double> BasicVector3D;
typedef HepGeom::Transform3D           Transform3D;
typedef HepGeom::Point3D<double>       HepPoint3D;
typedef HepGeom::Plane3D<double>       HepPlane3D;
typedef HepGeom::Normal3D<double>      HepNormal3D;

