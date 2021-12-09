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
 *  Trivial "almost empty" class. to be compared with
 *  class MyClass1A
 *
 *  @author Vanya BELYAEV
 *  @date   2006-02-14
 */
class MyClass1 {
public:
  MyClass1();
  virtual ~MyClass1();

private:
  // Empty class
  // Gaudi::XYZPoint   m_point  ;
  // Gaudi::XYZTPoint  m_point4 ;
  // Gaudi::Matrix4x3  m_3x4    ;
  // Gaudi::Matrix7x7    m_7x7   ;
  // double m_double ;
  // int m_int[10];
  // int m_int[3];
  // int m_int2[2];
  // int m_int100[100];
  int m_int0{ 0 };
};

// ============================================================================
// The END
// ============================================================================
#endif // TEST_MYCLASS1_H
