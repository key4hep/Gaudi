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
/*
 * MetaDataSvc.h
 *
 *  Created on: Mar 24, 2015
 *      Author: Ana Trisovic
 */

#ifndef GAUDISVC_SRC_METADATASVC_METADATASVC_H_
#define GAUDISVC_SRC_METADATASVC_METADATASVC_H_

#include "GaudiKernel/IMetaDataSvc.h"
#include "GaudiKernel/Service.h"

namespace Gaudi {
  class MetaDataSvc : public extends<Service, IMetaDataSvc> {
  public:
    using extends::extends;

    StatusCode start() override;

    bool isEnabled() const { return m_isEnabled; }

    StatusCode collectData();

    std::map<std::string, std::string> getMetaDataMap() const override;

  private:
    Gaudi::Property<bool> m_isEnabled{ this, "Enabled", true };

    std::map<std::string, std::string> m_metadata;
  };
} // namespace Gaudi

#endif /* GAUDISVC_SRC_METADATASVC_METADATASVC_H_ */
