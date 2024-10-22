/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-private-field"
#endif
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include <GaudiKernel/Allocator.h>
// ============================================================================
// Local
// ============================================================================
#include "MyClass1A.h"
// ============================================================================

/** @file
 *  Implementation file for class MyClass1
 *  @date 2006-02-14
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 */

MyClass1A::MyClass1A() {}
MyClass1A::~MyClass1A() {}

// ============================================================================
// Anonymous namespace to hide the allocator
// ============================================================================
namespace {
  GaudiUtils::Allocator<MyClass1A> s_Allocator;
}
// ============================================================================

// ============================================================================
/// overloaded 'new' operator
// ============================================================================
void* MyClass1A::operator new( size_t ) {
  void* hit;
  hit = (void*)s_Allocator.MallocSingle();
  return hit;
}
// ============================================================================

// ============================================================================
/// overloaded 'delete' operator
// ============================================================================
void MyClass1A::operator delete( void* hit ) { s_Allocator.FreeSingle( (MyClass1A*)hit ); }
// ============================================================================

// ============================================================================
// The END
// ============================================================================
#ifdef __clang__
#  pragma clang diagnostic pop
#endif
