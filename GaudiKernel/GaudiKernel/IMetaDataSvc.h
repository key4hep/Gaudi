/*
 * IMetaDataSvc.h
 *
 *  Created on: Mar 30, 2015
 *      Author: Ana Trisovic
 */

#ifndef GAUDIKERNEL_GAUDIKERNEL_IMETADATASVC_H_
#define GAUDIKERNEL_GAUDIKERNEL_IMETADATASVC_H_

#include "GaudiKernel/IService.h"
#include "GaudiKernel/MetaData.h"
#include "GaudiKernel/System.h"
#include "TFile.h"
class GAUDI_API IMetaDataSvc : virtual public IService
{
public:
  DeclareInterfaceID( IMetaDataSvc, 1, 0 );

  virtual MetaData* getMetaData() = 0;
  virtual std::map<std::string, std::string> getMetaDataMap() = 0;
};
#endif /* GAUDIKERNEL_GAUDIKERNEL_IMETADATASVC_H_ */
