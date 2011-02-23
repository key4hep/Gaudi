// $Id: MyClass1.h,v 1.1 2006/02/14 15:07:07 hmd Exp $
// ============================================================================
// CVS tag $Name:  $ version $Revision: 1.1 $
// ============================================================================
#ifndef TEST_MYCLASS1_H 
#define TEST_MYCLASS1_H 1
// ============================================================================
// Include files
// ============================================================================
// #include "Kernel/Point3DTypes.h"
// #include "Kernel/Point4DTypes.h"
// #include "Kernel/GenericMatrixTypes.h"
// ============================================================================

/** @class MyClass1 MyClass1.h test/MyClass1.h
 *
 *  Trivial "almost empty" class. to be copared with
 *  class MyClass1A
 *
 *  @author Vanya BELYAEV
 *  @date   2006-02-14
 */
class MyClass1 
{
public:
  MyClass1() ;
  virtual ~MyClass1();
private:
  //Empty class 
  //Gaudi::XYZPoint   m_point  ; 
  //Gaudi::XYZTPoint  m_point4 ; 
  //Gaudi::Matrix4x3  m_3x4    ;
  //Gaudi::Matrix7x7    m_7x7   ;
  //double m_double ;
  //int m_int[10];
  //int m_int[3];
  //int m_int2[2];
  //int m_int100[100];
  int m_int0 ;
};

// ============================================================================
// The END 
// ============================================================================
#endif // TEST_MYCLASS1_H
// ============================================================================
