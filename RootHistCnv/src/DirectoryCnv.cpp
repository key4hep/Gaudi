// Include files
#include "DirectoryCnv.h"
//------------------------------------------------------------------------------
//
// Implementation of class :  RootHistCnv::DirectoryCnv
//
// Author :                   Charles Leggett
//
//------------------------------------------------------------------------------
DECLARE_NAMESPACE_CONVERTER_FACTORY(RootHistCnv, DirectoryCnv)
//------------------------------------------------------------------------------
StatusCode RootHistCnv::DirectoryCnv::createObj(IOpaqueAddress* /* pAddress */,
                                                DataObject*& refpObject)
{
  refpObject = new DataObject();
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode
RootHistCnv::DirectoryCnv::createRep(DataObject* pObj, IOpaqueAddress*& refpAddr) {
  if ( changeDirectory(pObj) ) {
    const char* d_nam = pObj->name().c_str()+1;
    if (! gDirectory->GetKey(d_nam) ) {
      gDirectory->mkdir(d_nam);
    }
    gDirectory->cd(d_nam);
    return createAddress(pObj, gDirectory, 0, refpAddr);
  }
  refpAddr = 0;
  return StatusCode::FAILURE;
}
