#ifndef GAUDIEXAMPLES_HISTORY_H
#define GAUDIEXAMPLES_HISTORY_H

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/IHistorySvc.h"

class History : public Algorithm {
  SmartIF<IHistorySvc> m_histSvc;

public:
  History( const std::string& name, ISvcLocator* pSvcLocator );
  StatusCode initialize() override;
  StatusCode execute() override;
};

#endif // GAUDIEXAMPLES_HISTORY_H
