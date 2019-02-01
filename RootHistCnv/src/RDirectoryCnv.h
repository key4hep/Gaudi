#ifndef ROOTHIST_RDIRECTORYCNV_H
#define ROOTHIST_RDIRECTORYCNV_H 1

// Include files
#include "GaudiKernel/NTuple.h"
#include "RConverter.h"

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

#endif // RootHistCnv_RDirectORYCNV_H
