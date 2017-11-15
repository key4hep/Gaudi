/*
 * IMetaDataSvc.h
 *
 *  Created on: Mar 30, 2015
 *      Author: Ana Trisovic
 */

#ifndef GAUDIKERNEL_GAUDIKERNEL_IMETADATASVC_H_
#define GAUDIKERNEL_GAUDIKERNEL_IMETADATASVC_H_

#include "GaudiKernel/IService.h"
#include <map>
struct GAUDI_API IMetaDataSvc : extend_interfaces<IService> {
  DeclareInterfaceID( IMetaDataSvc, 2, 0 );
  virtual std::map<std::string, std::string> getMetaDataMap() const = 0;
};
#endif /* GAUDIKERNEL_GAUDIKERNEL_IMETADATASVC_H_ */
