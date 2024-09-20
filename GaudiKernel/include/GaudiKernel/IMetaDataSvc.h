/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
/*
 * IMetaDataSvc.h
 *
 *  Created on: Mar 30, 2015
 *      Author: Ana Trisovic
 */

#ifndef GAUDIKERNEL_GAUDIKERNEL_IMETADATASVC_H_
#define GAUDIKERNEL_GAUDIKERNEL_IMETADATASVC_H_

#include <GaudiKernel/IService.h>
#include <map>
struct GAUDI_API IMetaDataSvc : extend_interfaces<IService> {
  DeclareInterfaceID( IMetaDataSvc, 2, 0 );
  virtual std::map<std::string, std::string> getMetaDataMap() const = 0;
};
#endif /* GAUDIKERNEL_GAUDIKERNEL_IMETADATASVC_H_ */
