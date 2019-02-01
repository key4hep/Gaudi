#ifndef THIST_THISTREAD_H
#define THIST_THISTREAD_H 1

// Include files
#include "GaudiKernel/Algorithm.h"

class TH1F;
class ITHistSvc;
class TDirectory;

class THistRead : public Algorithm {

public:
  // Constructor of this form must be provided
  THistRead( const std::string& name, ISvcLocator* pSvcLocator );

  // Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  ITHistSvc* m_ths;
};

#endif // THIST_HISTREAD_H
