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
