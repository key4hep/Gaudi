// $Id: SolidUnion.cpp,v 1.16 2009-04-17 08:54:24 cattanem Exp $ 
// ===========================================================================
/** STD & STL  */
#include <iostream> 
#include <string>
#ifdef __INTEL_COMPILER         // Disable ICC remark
  #pragma warning(disable:1572) // Floating-point equality and inequality comparisons are unreliable
#endif
/** DetDesc */ 
#include "DetDesc/Solid.h"
#include "DetDesc/SolidUnion.h"
#include "DetDesc/SolidException.h"
#include "DetDesc/SolidBox.h"

// ============================================================================
/** @file
 *
 *  implementation of class SolidUnion
 * 
 *  @author Vanya Belyaev Ivan.Belyaev@itep.ru
 *  @date   xx/xx/xxxx
 */
// ============================================================================

// ============================================================================
/** constructor 
 *  @param name name of the intersection
 *  @param first pointer to first/main solid 
 */
// ============================================================================
SolidUnion::SolidUnion( const std::string& name  , 
                        ISolid*            first )
  : SolidBase    ( name         )
  , SolidBoolean ( name , first )
  , m_coverTop   ( 0 ) 
{
  if( 0 == first ) 
    { throw SolidException(" SolidUnion:: ISolid* points to NULL!"); }
}
// ============================================================================

// ============================================================================
/** constructor 
 *  @param name name of the solid union 
 */
// ============================================================================
SolidUnion::SolidUnion( const std::string& name )
  : SolidBase    ( name )
  , SolidBoolean ( name )
  , m_coverTop( 0 ) 
{}
// ============================================================================

// ============================================================================
/// destructor 
// ============================================================================
SolidUnion::~SolidUnion()
{ 
  if( 0 != m_coverTop ) { delete m_coverTop ; }
}
// ============================================================================

// ============================================================================
/** - check for the given 3D-point. 
 *    Point coordinates are in the local reference 
 *    frame of the solid.   
 *  - implementation of ISolid absstract interface  
 *  @see ISolid 
 *  @param point point (in local reference system of the solid)
 *  @return true if the point is inside the solid
 */
// ============================================================================
bool SolidUnion::isInside( const Gaudi::XYZPoint   & point ) const 
{
  return isInsideImpl(point);
}
// ============================================================================
bool SolidUnion::isInside( const Gaudi::Polar3DPoint& point ) const 
{
  return isInsideImpl(point);
}
// ============================================================================
bool SolidUnion::isInside( const Gaudi::RhoZPhiPoint   & point ) const 
{
  return isInsideImpl(point);
}
// ============================================================================
template <class aPoint>
bool SolidUnion::isInsideImpl( const aPoint   & point ) const 
{ 
  /// check bounding box 
  if ( isOutBBox( point )         ) { return false ; }
  ///  is point inside the "main" volume?  
  if ( first()->isInside( point ) ) { return true  ; }
  /// find the first daughter in which the given point is placed   
  SolidUnion::SolidChildrens::const_iterator ci = 
    std::find_if( childBegin () , 
                  childEnd   () , 
                  Solid::IsInside<aPoint>( point ) ) ;
  ///
  return ( childEnd() == ci ? false : true );   
}

// ============================================================================
/** add child solid to the solid union
 *  @param solid pointer to child solid 
 *  @param mtrx  pointer to transformation 
 *  @return status code 
 */
// ============================================================================
StatusCode  SolidUnion::unite( ISolid*                solid    , 
                               const Gaudi::Transform3D*  mtrx     )
{  
  StatusCode sc = addChild( solid , mtrx ); 
  if( sc.isFailure() ) { return sc ; }
  return updateBP();
}

// ============================================================================
/** add child solid to the solid union
 *  @param child pointer to child solid 
 *  @param position position  
 *  @return status code 
 */
// ============================================================================
StatusCode  SolidUnion::unite ( ISolid*                  child    , 
                                const Gaudi::XYZPoint&   position , 
                                const Gaudi::Rotation3D& rotation )
{
  StatusCode sc = addChild( child , position , rotation ); 
  if( sc.isFailure() ) { return sc ; }
  return updateBP();
}
// ============================================================================

// ============================================================================
/** create the cover top box 
 */
// ============================================================================
const ISolid* SolidUnion::coverTop() const 
{
  if( 0 != m_coverTop ) { return m_coverTop ; }
  
  const double x =  
    fabs( xMax() ) > fabs( xMin() ) ? fabs( xMax() ) : fabs( xMin() ) ;
  const double y =  
    fabs( yMax() ) > fabs( yMin() ) ? fabs( yMax() ) : fabs( yMin() ) ;
  const double z =  
    fabs( zMax() ) > fabs( zMin() ) ? fabs( zMax() ) : fabs( zMin() ) ;
  
  m_coverTop = new SolidBox ("CoverTop for " + name () , x , y, z  ) ;
  // 
  return m_coverTop;
}
// ============================================================================

// ============================================================================
/** update bonding parameters 
 *  @return status code 
 */
// ============================================================================
StatusCode SolidUnion::updateBP()
{
  if( childBegin() == childEnd() ) { return StatusCode::SUCCESS ; }
  // get last child 
  SolidChild* child = 
    *( childBegin() + ( childEnd() - childBegin() - 1 ) );
  // cast it!
  SolidBase* base = dynamic_cast<SolidBase*> (child);
  if( 0 == base ) { return StatusCode::FAILURE ; }
  //
  setXMin   ( base->xMin   () < xMin   () ? base->xMin   () : xMin   () );
  setXMax   ( base->xMax   () > xMax   () ? base->xMax   () : xMax   () );

  setYMin   ( base->yMin   () < yMin   () ? base->yMin   () : yMin   () );
  setYMax   ( base->yMax   () > yMax   () ? base->yMax   () : yMax   () );

  setZMin   ( base->zMin   () < zMin   () ? base->zMin   () : zMin   () );
  setZMax   ( base->zMax   () > zMax   () ? base->zMax   () : zMax   () );

  setRMax   ( base->rMax   () > rMax   () ? base->rMax   () : rMax   () );
  setRhoMax ( base->rhoMax () > rhoMax () ? base->rhoMax () : rhoMax () );
  //
  checkBP();
  return StatusCode::SUCCESS;
}
// ============================================================================

// ============================================================================
// The END 
// ============================================================================

