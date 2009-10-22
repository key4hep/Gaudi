// $Id: RDirectoryCnv.h,v 1.7 2006/11/13 15:19:36 hmd Exp $
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
    virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& refpObj);
    /// Update the transient object from the other representation.
    virtual StatusCode fillObjRefs(IOpaqueAddress* pAddr, DataObject* refpObj);
    /// Convert the transient object to the requested representation.
    virtual StatusCode createRep( DataObject* pObj, IOpaqueAddress*& refpAddr);
    /// Convert the transient object to the requested representation.
    virtual StatusCode updateRep( IOpaqueAddress* pAddr, DataObject* pObject );
    /// Inquire class type
    static const CLID& classID()      {      return CLID_NTupleDirectory;    }
    /// Standard constructor
    RDirectoryCnv(ISvcLocator* svc) : RConverter(CLID_NTupleDirectory, svc) {}
    /// Standard constructor
    RDirectoryCnv(ISvcLocator* svc,const CLID& clid) : RConverter(clid, svc){}
    /// Standard destructor
    virtual ~RDirectoryCnv()  {}
  };
}    // namespace RootHistCnv

#endif    // RootHistCnv_RDirectORYCNV_H 
