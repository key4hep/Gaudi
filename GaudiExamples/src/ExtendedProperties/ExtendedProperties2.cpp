// $Id: ExtendedProperties.cpp,v 1.7 2008/01/14 19:45:34 marcocle Exp $
// ============================================================================
// Include files 
// ============================================================================
// STD & STL 
// ============================================================================
#include <vector>
#include <map>
#include <string>
#include <utility>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/Point3DTypes.h"
#include "GaudiKernel/Point4DTypes.h"
#include "GaudiKernel/Vector3DTypes.h"
#include "GaudiKernel/Vector4DTypes.h"
#include "GaudiKernel/GenericVectorTypes.h"
// ============================================================================
#include "GaudiKernel/VectorsAsProperty.h"
#include "GaudiKernel/SVectorAsProperty.h"
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
      // ======================================================================
      /// friend factory for instantiation
      friend class AlgFactory<Gaudi::Examples::ExtendedProperties2>;
      // ======================================================================      
    public:
      // ======================================================================
      StatusCode execute() ;
      // ======================================================================      
    protected:
      // ======================================================================      
      /** standard constructor 
       *  @param name algorithm instance name 
       *  @param pSvc pointer to Service Locator 
       */
      ExtendedProperties2 
      ( const std::string& name , 
        ISvcLocator*       pSvc ) 
        : GaudiAlgorithm ( name , pSvc )
        //
        , m_point3D  ( 0 , 1 , 2     ) 
        , m_vector3D ( 1 , 2 , 3     ) 
#ifndef _WIN32
        , m_vector4D ( 1 , 2 , 3 , 4 ) 
#endif
        , m_vector5  () 
        //
      {
        declareProperty 
          ( "Point3D"  , m_point3D    , "3D-point"       ) ;
        
        declareProperty 
          ( "Vector3D"  , m_vector3D  , "3D-vector"      ) ;

#ifndef _WIN32
        declareProperty 
          ( "Vector4D"  , m_vector4D  , "Lorentz-vector" ) ;
#endif
        
        declareProperty 
          ( "SVector5"  , m_vector5   , "Generic-vector" ) ;
        
        //
        declareProperty 
          ( "Points3D"  , m_points   , "Vector of 3D-points"  ) ;
        declareProperty 
          ( "Vectors3D" , m_vectors  , "Vector of 3D-vectors" ) ;
#ifndef _WIN32
        declareProperty 
          ( "Vectors4D" , m_lvs      , "Vector of 4D-vectors" ) ;
#endif
        
        setProperty ( "PropertiesPrint", true ) . ignore() ; 
      }
      /// destructor: virtual and protected
      virtual ~ExtendedProperties2 (){}
      // ======================================================================
    private:
      // ======================================================================
      /// default constructor is disabled
      ExtendedProperties2 ();                // default constructor is disabled
      /// copy constructor is disabled
      ExtendedProperties2 ( const ExtendedProperties2& ); // no copy constructor
      // assignment operator is disabled
      ExtendedProperties2& operator=( const ExtendedProperties2& );
      // ======================================================================
    private:
      // ======================================================================
      /// 3D-point 
      Gaudi::XYZPoint      m_point3D  ;                       //       3D-point 
      /// 3D-vector 
      Gaudi::XYZVector     m_vector3D ;                       //      3D-vector 
#ifndef _WIN32
      /// Lorentz Vector 
      Gaudi::LorentzVector m_vector4D ;                       // Lorentz Vector
#endif
      /// Generic Vector 
      Gaudi::Vector5       m_vector5  ;                       // Generic Vector 
      // ======================================================================
      std::vector<Gaudi::XYZPoint>       m_points   ;
      std::vector<Gaudi::XYZVector>      m_vectors  ;
#ifndef _WIN32
      std::vector<Gaudi::LorentzVector>  m_lvs      ;
#endif
      // ======================================================================
    };
    // ========================================================================
  } //                                         end of namespace Gaudi::Examples 
  // ==========================================================================
} //                                                     end of namespace Gaudi 
// ============================================================================
StatusCode Gaudi::Examples::ExtendedProperties2::execute() 
{
  always() 
    << "3D-Point : " << Gaudi::Utils::toString( m_point3D  ) << endmsg ;
  always() 
    << "3D-Vector: " << Gaudi::Utils::toString( m_vector3D ) << endmsg ;
#ifndef _WIN32
  always() 
    << "4D-Vector: " << Gaudi::Utils::toString( m_vector4D ) << endmsg ;
#endif
  always() 
    << " 5-Vector: " << Gaudi::Utils::toString( m_vector5  ) << endmsg ;
  
  always()
    << "Vector of 3D-Points  " << Gaudi::Utils::toString( m_points  ) << endmsg ;
  always()
    << "Vector of 3D-Vectors " << Gaudi::Utils::toString( m_vectors ) << endmsg ;
#ifndef _WIN32
  always()
    << "Vector of 4D-Vectors " << Gaudi::Utils::toString( m_lvs     ) << endmsg ;
#endif
  
  return StatusCode::SUCCESS ; 
}
// ============================================================================
// the factory
// ============================================================================
DECLARE_NAMESPACE_ALGORITHM_FACTORY(Gaudi::Examples,ExtendedProperties2)
// ============================================================================
// The END 
// ============================================================================
