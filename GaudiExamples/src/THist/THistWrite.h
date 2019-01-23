#ifndef THIST_THISTWRITE_H
#define THIST_THISTWRITE_H 1

// Include files
#include "GaudiKernel/Algorithm.h"

class TH1F;
class ITHistSvc;
class TDirectory;

class THistWrite : public Algorithm
{

public:
  // Constructor of this form must be provided
  THistWrite( const std::string& name, ISvcLocator* pSvcLocator );

  // Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  ITHistSvc* m_ths;

  void listKeys( TDirectory* td );
};

#endif // THIST_HISTOALGORITHM_H
