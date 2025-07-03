/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiKernel/ContainedObject.h>
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/NTupleItems.h>

typedef const std::string&    CSTR;
typedef const std::type_info& CTYPE;

/// Create instance
template <class TYP>
NTuple::_Item<TYP>* NTuple::_Item<TYP>::create( INTuple* tup, const std::string& name, const std::type_info& info,
                                                TYP min, TYP max, TYP def ) {
  return new _ItemImp<TYP>( tup, name, info, min, max, def );
}

/// Create instance
template <class TYP>
NTuple::_Array<TYP>* NTuple::_Array<TYP>::create( INTuple* tup, const std::string& name, const std::type_info& info,
                                                  const std::string& idx, long len, TYP min, TYP max, TYP def ) {
  return new _ArrayImp<TYP>( tup, name, info, idx, len, min, max, def );
}

/// Create instance
template <class TYP>
NTuple::_Matrix<TYP>* NTuple::_Matrix<TYP>::create( INTuple* tup, const std::string& name, const std::type_info& info,
                                                    const std::string& idx, long ncol, long nrow, TYP min, TYP max,
                                                    TYP def ) {
  return new _MatrixImp<TYP>( tup, name, info, idx, ncol, nrow, min, max, def );
}

#define INSTANTIATE( TYP )                                                                                             \
  template class NTuple::_Item<TYP>;                                                                                   \
  template class NTuple::_Array<TYP>;                                                                                  \
  template class NTuple::_Matrix<TYP>;
// This makes the stuff accessible from outside
// (instruct the compiler to explicitly instantiate specific instances....)
INSTANTIATE( bool )
INSTANTIATE( char )
INSTANTIATE( unsigned char )
INSTANTIATE( short )
INSTANTIATE( unsigned short )
INSTANTIATE( int )
INSTANTIATE( unsigned int )
INSTANTIATE( long )
INSTANTIATE( long long )
INSTANTIATE( unsigned long )
INSTANTIATE( unsigned long long )
INSTANTIATE( float )
INSTANTIATE( double )
INSTANTIATE( void* )
INSTANTIATE( IOpaqueAddress* )
INSTANTIATE( std::string )

#undef INSTANTIATE
