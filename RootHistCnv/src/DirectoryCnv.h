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
#ifndef ROOTHISTCNV_DIRECTORYCNV_H
#define ROOTHISTCNV_DIRECTORYCNV_H 1

// Include files
#include "GaudiKernel/DataObject.h"
#include "RConverter.h"

//------------------------------------------------------------------------------
//
// ClassName:   RootHistCnv::DirectoryCnv
//
// Description: Histogram directory converter
//
// Author:      Charles Leggett
//
//------------------------------------------------------------------------------
namespace RootHistCnv {
  class DirectoryCnv : public RConverter {
  public:
    /// Create the transient representation of an object.
    StatusCode createObj( IOpaqueAddress* pAddr, DataObject*& refpObj ) override;
    /// Convert the transient object to the requested representation.
    StatusCode createRep( DataObject* pObj, IOpaqueAddress*& refpAddr ) override;
    /// Inquire class type
    static const CLID& classID() { return CLID_DataObject; }
    /// Standard constructor
    DirectoryCnv( ISvcLocator* svc ) : RConverter( CLID_DataObject, svc ) {}
    /// Standard constructor
    DirectoryCnv( const CLID& clid, ISvcLocator* svc ) : RConverter( clid, svc ) {}
  };
} // namespace RootHistCnv

#endif // ROOTHISTCNV_DIRECTORYCNV_H
