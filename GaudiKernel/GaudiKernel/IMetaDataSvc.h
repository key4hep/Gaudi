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
struct GAUDI_API IMetaDataSvc : extend_interfaces<IService> {
  DeclareInterfaceID( IMetaDataSvc, 1, 0 );

  virtual MetaData* getMetaData() = 0;
  virtual std::map<std::string, std::string> getMetaDataMap() = 0;
};
#endif /* GAUDIKERNEL_GAUDIKERNEL_IMETADATASVC_H_ */
