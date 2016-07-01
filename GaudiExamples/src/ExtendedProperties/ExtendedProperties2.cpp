// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <map>
#include <string>
#include <utility>
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/GenericVectorTypes.h"
#include "GaudiKernel/Point3DTypes.h"
#include "GaudiKernel/Point4DTypes.h"
#include "GaudiKernel/Vector3DTypes.h"
#include "GaudiKernel/Vector4DTypes.h"
// ============================================================================
#include "GaudiKernel/SVectorAsProperty.h"
#include "GaudiKernel/VectorsAsProperty.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
// ============================================================================

/** @file
 *  simple DEMO-file for "extended vector properties",
 *  implementation file for class Gaudi::Examples::ExtendedProperties2
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date  2009-09-05
 */
// ============================================================================
namespace Gaudi
{
  // ==========================================================================
  namespace Examples
  {
    // ========================================================================
    /** @class ExtendedProperties2
     *  simple DEMO-file for "extended vector properties",
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date  2009-09-05
     */
    class ExtendedProperties2 : public GaudiAlgorithm
    {
    public:
      // ======================================================================
      StatusCode execute();
      // ======================================================================
    public:
      // ======================================================================
      /** standard constructor
       *  @param name algorithm instance name
       *  @param pSvc pointer to Service Locator
       */
      ExtendedProperties2( const std::string& name, ISvcLocator* pSvc ) : GaudiAlgorithm( name, pSvc )
      {
        setProperty( "PropertiesPrint", true ).ignore();
      }
      /// destructor
      ~ExtendedProperties2() override = default;
      // ======================================================================
    private:
      // ======================================================================
      PropertyWithValue<Gaudi::XYZPoint> m_point3D{this, "Point3D", {0, 1, 2}, "3D-point"};
      PropertyWithValue<Gaudi::XYZVector> m_vector3D{this, "Vector3D", {1, 2, 3}, "3D-vector"};
      PropertyWithValue<Gaudi::LorentzVector> m_vector4D{this, "Vector4D", {1, 2, 3, 4}, "Lorentz-vector"};
      PropertyWithValue<Gaudi::Vector5> m_vector5{this, "SVector5", {}, "Generic-vector"};
      PropertyWithValue<std::vector<Gaudi::XYZPoint>> m_points{this, "Points3D", {}, "Vector of 3D-points"};
      PropertyWithValue<std::vector<Gaudi::XYZVector>> m_vectors{this, "Vectors3D", {}, "Vector of 3D-vectors"};
      PropertyWithValue<std::vector<Gaudi::LorentzVector>> m_lvs{this, "Vectors4D", {}, "Vector of 4D-vectors"};
      // ======================================================================
    };
    // ========================================================================
  } //                                         end of namespace Gaudi::Examples
  // ==========================================================================
} //                                                     end of namespace Gaudi
// ============================================================================
StatusCode Gaudi::Examples::ExtendedProperties2::execute()
{
  always() << "3D-Point : " << Gaudi::Utils::toString( m_point3D.value() ) << endmsg;
  always() << "3D-Vector: " << Gaudi::Utils::toString( m_vector3D.value() ) << endmsg;
  always() << "4D-Vector: " << Gaudi::Utils::toString( m_vector4D.value() ) << endmsg;
  always() << " 5-Vector: " << Gaudi::Utils::toString( m_vector5.value() ) << endmsg;

  always() << "Vector of 3D-Points  " << Gaudi::Utils::toString( m_points.value() ) << endmsg;
  always() << "Vector of 3D-Vectors " << Gaudi::Utils::toString( m_vectors.value() ) << endmsg;
  always() << "Vector of 4D-Vectors " << Gaudi::Utils::toString( m_lvs.value() ) << endmsg;

  return StatusCode::SUCCESS;
}
// ============================================================================
// the factory
// ============================================================================
using Gaudi::Examples::ExtendedProperties2;
DECLARE_COMPONENT( ExtendedProperties2 )
// ============================================================================
// The END
// ============================================================================
