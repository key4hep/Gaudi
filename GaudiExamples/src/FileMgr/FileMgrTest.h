#ifndef GAUDIEXAMPLES_FILEMGRTEST_H
#define GAUDIEXAMPLES_FILEMGRTEST_H

//<<<<<< INCLUDES                                                       >>>>>>
#include "GaudiKernel/Algorithm.h"
//<<<<<< CLASS DECLARATIONS                                             >>>>>>
/** @class FileMgrTest
 * @brief  an algorithm to test the FileMgr
 */


#include <string>
#include <stdio.h>
#include "GaudiKernel/IFileMgr.h"

class IFileMgr;
class TFile;


class FileMgrTest : public Algorithm {

public:
  FileMgrTest (const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
  
private:

  IFileMgr* p_fileMgr;
  Io::Fd fd_1, fd_2, fd_3, fd_4, fd_r1, fd_r2;
  FILE *fp_1, *fp_2, *fp_3, *fp_4;
  TFile *fp_r1, *fp_r2, *fp_r3;
  std::string m_f1, m_f2, m_f3, m_f4, m_fr1, m_fr2, m_fr3;

  StatusCode PosixOpenAction(FILEMGR_CALLBACK_ARGS);
  StatusCode allCloseAction(FILEMGR_CALLBACK_ARGS);



};

#endif // GAUDIEXAMPLES_FILEMGRTEST_H
