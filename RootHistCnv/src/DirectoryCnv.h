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
    virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& refpObj);
    /// Convert the transient object to the requested representation.
    virtual StatusCode createRep(DataObject* pObj, IOpaqueAddress*& refpAddr);
    /// Inquire class type
    static const CLID& classID()      {  return CLID_DataObject;    }
    /// Standard constructor
    DirectoryCnv(ISvcLocator* svc) : RConverter(CLID_DataObject, svc) {}
    /// Standard constructor
    DirectoryCnv(const CLID& clid, ISvcLocator* svc) : RConverter(clid, svc) {}
    /// Standard destructor
    virtual ~DirectoryCnv()   {}
  };
}    // namespace RootHistCnv


#endif    // ROOTHISTCNV_DIRECTORYCNV_H 
