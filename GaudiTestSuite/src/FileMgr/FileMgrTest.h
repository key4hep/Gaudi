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
#ifndef GAUDIEXAMPLES_FILEMGRTEST_H
#define GAUDIEXAMPLES_FILEMGRTEST_H

//<<<<<< INCLUDES                                                       >>>>>>
#include <GaudiKernel/Algorithm.h>
//<<<<<< CLASS DECLARATIONS                                             >>>>>>
/** @class FileMgrTest
 * @brief  an algorithm to test the FileMgr
 */

#include <GaudiKernel/IFileMgr.h>
#include <stdio.h>
#include <string>

class IFileMgr;
class TFile;

class FileMgrTest : public Algorithm {

public:
  FileMgrTest( const std::string& name, ISvcLocator* pSvcLocator );
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  IFileMgr*   p_fileMgr;
  Io::Fd      fd_1{ -1 }, fd_2{ -1 }, fd_3{ -1 }, fd_4{ -1 };
  FILE *      fp_1{ nullptr }, *fp_2{ nullptr }, *fp_3{ nullptr }, *fp_4{ nullptr };
  TFile *     fp_r1{ nullptr }, *fp_r2{ nullptr }, *fp_r3{ nullptr };
  std::string m_f1, m_f2, m_f3, m_f4, m_fr1, m_fr2, m_fr3;

  StatusCode PosixOpenAction( FILEMGR_CALLBACK_ARGS );
  StatusCode allCloseAction( FILEMGR_CALLBACK_ARGS );
};

#endif // GAUDIEXAMPLES_FILEMGRTEST_H
