// $Id: MyClass1A.h,v 1.1 2006/02/14 15:07:07 hmd Exp $
// ============================================================================
// CVS tag $Name:  $ , version $Revison:$
// ============================================================================
#ifndef TEST_MYCLASS1A_H 
#define TEST_MYCLASS1A_H 1
// ============================================================================
// Include files
// ============================================================================
// #include "Kernel/Point3DTypes.h"
// #include "Kernel/Point4DTypes.h"
// #include "Kernel/GenericMatrixTypes.h"
// ============================================================================

/** @class MyClass1A MyClass1A.h test/MyClass1A.h
 *
 *  Simple class, equepped with "Allocator", 
 *  to be compared with class MyClass1
 *
 *  @author Vanya BELYAEV
 *  @date   2006-02-14
 */
class MyClass1A 
{
public:
  MyClass1A() ;
  virtual ~MyClass1A();
public:
  void* operator new    ( size_t    ) ;
  void  operator delete ( void *hit ) ;
private:
  //Empty class:                  // gain is ~3.5
  //Gaudi::XYZPoint   m_point   ; // gain is ~3 
  //Gaudi::XYZTPoint  m_point4  ; // gain is ~2.5
  //Gaudi::Matrix4x3  m_3x4     ; // gain is ~1.8
  //Gaudi::Matrix7x7  m_7x7     ; // gain is ~1.8
  //double            m_double  ; // gain is ~3.5 
  //int               m_int[10] ; // gain is ~1.8
  //int               m_int[3]  ; // gain is ~4.0
  //int               m_int2[2] ; // gain is ~3.0 
  //int               m_int100[100] ; // gain is ~2.5 
  int                 m_int0    ; // gain is ~3.0
};

// ============================================================================
// The END 
// ============================================================================
#endif // TEST_MYCLASS1A_
// ============================================================================
