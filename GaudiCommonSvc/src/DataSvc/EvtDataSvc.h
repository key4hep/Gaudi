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
#ifndef DATASVC_EVTDATASVC_H
#define DATASVC_EVTDATASVC_H

#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/IConversionSvc.h"

/** @class EvtDataSvc EvtDataSvc.h
 *
 *   A EvtDataSvc is the base class for event services
 *
 *  @author M.Frank
 */
class EvtDataSvc : public DataSvc {
public:
  using DataSvc::DataSvc;
  StatusCode initialize() override;
  StatusCode reinitialize() override;
  StatusCode finalize() override;

private:
  SmartIF<IConversionSvc> m_cnvSvc;
};
#endif // DATASVC_EVTDATASVC_H
