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
#pragma once

// Include files
#include "RConverter.h"
#include <GaudiKernel/NTuple.h>

namespace RootHistCnv {

  /** @class RootHistCnv::RDirectoryCnv RDirectoryCnv.h

      Create persistent and transient representations of
        data store directories
      @author Charles Leggett
  */
  class RDirectoryCnv : public RConverter {
  public:
    /// Create the transient representation of an object.
    StatusCode createObj( IOpaqueAddress* pAddr, DataObject*& refpObj ) override;
    /// Update the transient object from the other representation.
    StatusCode fillObjRefs( IOpaqueAddress* pAddr, DataObject* refpObj ) override;
    /// Convert the transient object to the requested representation.
    StatusCode createRep( DataObject* pObj, IOpaqueAddress*& refpAddr ) override;
    /// Convert the transient object to the requested representation.
    StatusCode updateRep( IOpaqueAddress* pAddr, DataObject* pObject ) override;
    /// Inquire class type
    static const CLID& classID() { return CLID_NTupleDirectory; }
    /// Standard constructor
    RDirectoryCnv( ISvcLocator* svc ) : RConverter( CLID_NTupleDirectory, svc ) {}
    /// Standard constructor
    RDirectoryCnv( ISvcLocator* svc, const CLID& clid ) : RConverter( clid, svc ) {}
  };
} // namespace RootHistCnv
