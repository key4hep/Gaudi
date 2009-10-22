// $Id: THistWrite.h,v 1.1 2006/09/21 13:22:00 hmd Exp $
#ifndef THIST_THISTWRITE_H
#define THIST_THISTWRITE_H 1

// Include files
#include "GaudiKernel/Algorithm.h"

class TH1F;
class ITHistSvc;
class TDirectory;

class THistWrite : public Algorithm {

public:
  // Constructor of this form must be provided
  THistWrite(const std::string& name, ISvcLocator* pSvcLocator); 

  // Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:

  ITHistSvc* m_ths;

  TH1F* m_h1;

  void listKeys(TDirectory *td);

};


# endif    // THIST_HISTOALGORITHM_H
