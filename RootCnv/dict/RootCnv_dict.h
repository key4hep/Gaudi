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
#include "RootCnv/RootRefs.h"

#ifdef __POOL_COMPATIBILITY
#  include "RootCnv/PoolClasses.h"
#  if 0
//typedef Gaudi::RootNTupleDescriptor PoolDbNTupleDescriptor;
class PoolDbNTupleDescriptor {
 public:
  /// Description string
  std::string   description;
  /// Optional description
  std::string   optional;
  /// Identifier of description
  std::string   container;
  /// Class ID of the described object
  unsigned long clid;
  /// Standard constructor
  PoolDbNTupleDescriptor() {}
  /// Standard destructor
  virtual ~PoolDbNTupleDescriptor() {}
};
#  endif
#endif

// Add here addition include files for the dictionary generation
namespace RootCnv {
  struct __Instantiations {};
} // namespace RootCnv
