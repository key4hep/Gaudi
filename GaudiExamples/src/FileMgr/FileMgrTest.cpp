#include "FileMgrTest.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/AlgFactory.h"
#include "TFile.h"
#include "TSSLSocket.h"

#include <fstream>
#include <stdio.h>
#include <ext/stdio_filebuf.h>  // __gnu_cxx::stdio_filebuf

#include "boost/bind.hpp"

// Static Factory declaration

DECLARE_ALGORITHM_FACTORY(FileMgrTest)


///////////////////////////////////////////////////////////////////////////

FileMgrTest::FileMgrTest( const std::string& name, ISvcLocator* pSvcLocator ) :
  Algorithm(name, pSvcLocator), p_fileMgr(0)
{

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode FileMgrTest::initialize() {

  StatusCode st(StatusCode::SUCCESS);

  MsgStream log(msgSvc(), name());

  m_f1 = "/etc/redhat-release";
  m_f2 = "t2.txt";
  m_f3 = "t3.txt";
  m_f4 = "t4.txt";

  m_fr1 = "tuple1.rt";
  m_fr2 = "http://annwm.lbl.gov/~leggett/tuple2.rt";
  m_fr3 = "https://t-dpm.grid.sinica.edu.tw/dpm/grid.sinica.edu.tw/home/atlas/atlasppsscratchdisk/mc10_7TeV/AOD/e574_s1110_s1100_r1655_r1700/mc10_7TeV.105805.filtered_minbias6.merge.AOD.e574_s1110_s1100_r1655_r1700_tid261524_00/AOD.261524._032551.pool.root.1";

  if (service("FileMgr",p_fileMgr,true).isFailure()) {
    log << MSG::ERROR << "unable to get the FileMgr" << endmsg;
    st = StatusCode::FAILURE;
  } else {
    log << MSG::DEBUG << "got the FileMgr" << endmsg;
  }


  Io::bfcn_action_t boa = 
    boost::bind(&FileMgrTest::PosixOpenAction, this, _1,_2);
  if (p_fileMgr->regAction(boa, Io::OPEN, Io::POSIX, 
			   "FileMgrTest::POSIXOpenAction").isFailure()) {
    log << MSG::ERROR
	  << "unable to register POSIX file open action with FileMgr"
	  << endmsg;
  }
  
  if (p_fileMgr->regAction(boa, Io::OPEN_ERR, Io::POSIX, 
			   "FileMgrTest::PosixOpenAction").isFailure()) {
    log << MSG::ERROR
	  << "unable to register POSIX file open ERR action with FileMgr"
	  << endmsg;
  }


  Io::bfcn_action_t bob = 
    boost::bind(&FileMgrTest::allCloseAction, this, _1,_2);
  if (p_fileMgr->regAction(bob, Io::CLOSE
			   ).isFailure()) {
			   // "FileMgrTest::allCloseAction").isFailure()) {
    log << MSG::ERROR
	  << "unable to register all Close action with FileMgr"
	  << endmsg;
  }
  
  int r;

  r = p_fileMgr->open(Io::POSIX,name(),m_f1,Io::READ,fd_1,"ASCII");
  if (r != 0) {
    log << MSG::ERROR << "unable to open " << m_f1 << " for reading"
	<< endmsg;
  } else {
//     fp_1 = fdopen(fd_1, "r");
    fp_1 = (FILE*) p_fileMgr->fptr(fd_1);
    log << MSG::INFO << "opened " << m_f1 << " for reading with FD: " << fd_1
	<< "  FILE* " << fp_1
	<< endmsg;
  }
  
  r = p_fileMgr->open(Io::POSIX,name(),m_f2,Io::WRITE|Io::CREATE,fd_2,"ASCII");
  if (r != 0) {
    log << MSG::ERROR << "unable to open " << m_f2 << " for writing"
	<< endmsg;
  } else {
    //    fp_2 = fdopen(fd_2,"w");
    fp_2 = (FILE*) p_fileMgr->fptr(fd_2);
    log << MSG::INFO << "opened " << m_f2 << " for writing with FD: " << fd_2
	<< "  FILE* " << fp_2
	<< endmsg;
  }


  // make sure we have something in m_f3 to append to;
  std::ofstream ofs(m_f3.c_str());
  ofs << "initial line" << std::endl;
  ofs.close();


  r = p_fileMgr->open(Io::POSIX,name(),m_f3,Io::WRITE|Io::APPEND,fd_3,"ASCII");
  if (r != 0) {
    log << MSG::ERROR << "unable to open " << m_f3 << " for write|append"
	<< endmsg;
  } else {
    //    fp_3 = fdopen(fd_3,"a");
    fp_3 = (FILE*) p_fileMgr->fptr(fd_3);
    log << MSG::INFO << "opened " << m_f3 << " for reading with FD: " << fd_3
	<< "  FILE* " << fp_3
	<< endmsg;
  }


  Io::Fd fd;

  log << MSG::ERROR << "the following error is expected" << endmsg;
  r = p_fileMgr->open(Io::POSIX,name(),m_f2,Io::WRITE|Io::CREATE|Io::EXCL,fd,"ASCII");
  if (r != 0) {
    log << MSG::INFO << "unable to open " << m_f2
	<< " for WRITE|CREATE|EXCL - expected as file already exists"
	<< endmsg;
  } else {
    log << MSG::ERROR << "opened " << m_f2 << " for reading with FD: " << fd
	<< "  This should not occur!"
	<< endmsg;
  }



  r = p_fileMgr->open(Io::POSIX,name(),m_f1,Io::READ,fd_4,"ASCII");
  if (r != 0) {
    log << MSG::ERROR << "unable to open " << m_f1 << " again for reading"
	<< endmsg;
  } else {
    fp_4 = (FILE*) p_fileMgr->fptr(fd_4);
    log << MSG::INFO << "opened " << m_f1 << " again for reading with FD: " 
	<< fd_4 << " FILE* " << fp_4
	<< endmsg;
  }


  void *vp(0);
  r = p_fileMgr->open(Io::ROOT,name(),m_fr1,Io::READ,vp,"HIST");
  if (r != 0) {
    log << MSG::ERROR << "unable to open " << m_fr1 << " again for reading"
	<< endmsg;
  } else {
    fp_r1 = (TFile*) vp;
    log << MSG::INFO << "opened " << m_fr1 << " for reading with ptr: " 
	<< fp_r1 
	<< endmsg;
  }

  r = p_fileMgr->open(Io::ROOT,name(),m_fr2,Io::READ,vp,"HIST");
  if (r != 0) {
    log << MSG::ERROR << "unable to open " << m_fr2 << " for reading"
	<< endmsg;
  } else {
    fp_r2 = (TFile*) vp;
    log << MSG::INFO << "opened " << m_fr2 << " for reading with ptr: " 
	<< fp_r2 << " size: " << fp_r2->GetSize()
	<< endmsg;
  }

  // std::string cafile, capath,ucert,ukey;
  // cafile = "/tmp/x509up_u6919";
  // capath = "/afs/cern.ch/project/gd/LCG-share2/certificates";
  // ucert = cafile;
  // ukey = cafile;

  // TSSLSocket::SetUpSSL(cafile.c_str(), capath.c_str(), ucert.c_str(), ukey.c_str());
  r = p_fileMgr->open(Io::ROOT,name(),m_fr3,Io::READ,vp,"HIST");
  if (r != 0) {
    log << MSG::ERROR << "unable to open " << m_fr3 << " for reading"
	<< endmsg;
  } else {
    fp_r3 = (TFile*) vp;
    log << MSG::INFO << "opened " << m_fr3 << " for reading with ptr: " 
	<< fp_r3 << " size: " << fp_r3->GetSize()
	<< endmsg;
  }

  for (int j=0; j<2; ++j) {
    r = p_fileMgr->open(Io::POSIX,name(),"t6.txt",Io::WRITE|Io::CREATE,
			fd,"ASCII");
    if (r != 0) {
      log << MSG::ERROR << "unable to open t6.txt for writing"
	  << endmsg;
    } else {
      log << MSG::INFO << "opened t6.txt for writing, fd: " << fd 
	  << " will now close"
	  << endmsg;
      p_fileMgr->close(fd,name());
    }
  }


  return st;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode FileMgrTest::execute() {

  MsgStream log(msgSvc(), name());

  log << MSG::INFO << "writing to " << p_fileMgr->fname((void*)fp_2) << endmsg;

  std::ofstream ofs;
  __gnu_cxx::stdio_filebuf<char> fb(fp_2, std::ios::out);

  ofs.std::ios::rdbuf(&fb);
  ofs << "Hello World!" << std::endl;


  log << MSG::INFO << "appending to " << p_fileMgr->fname((void*)fp_3) << endmsg;

  std::ofstream ofs2;
  __gnu_cxx::stdio_filebuf<char> fb2(fp_3, std::ios::out);

  ofs2.std::ios::rdbuf(&fb2);
  ofs2 << "Hello World!" << std::endl;



  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode FileMgrTest::finalize() {

  MsgStream log(msgSvc(), name());

  std::vector<std::string> v;
  std::vector<std::string>::const_iterator itr;
  int i = p_fileMgr->getFiles(v);

  log << MSG::INFO << "listing all open files [" <<  i << "]" << std::endl;
  for (itr = v.begin(); itr != v.end(); ++itr) {
    log << "    " << *itr << std::endl;
  }
  log << endmsg;

  i = p_fileMgr->getFiles(v,false);
  log << MSG::INFO << "listing ALL files [" <<  i << "]" << std::endl;
  for (itr = v.begin(); itr != v.end(); ++itr) {
    log << "    " << *itr << std::endl;
  }
  log << endmsg;

  std::vector<const Io::FileAttr*> v2;
  std::vector<const Io::FileAttr*>::const_iterator it2;
  i = p_fileMgr->getFiles(Io::POSIX,v2,false);
  log << MSG::INFO << "listing all POSIX files ever opened [" <<  i << "]" 
      << std::endl;
  for (it2 = v2.begin(); it2 != v2.end(); ++it2) {
    log << "    " << (*it2)->name() << std::endl;
  }
  log << endmsg;



  int r;

  r = p_fileMgr->close(fd_1,name());
  if (r != 0) {
    log << MSG::ERROR << "unable to close " << m_f1 << " with FD " << fd_1
	<< endmsg;
  } else {
    log << MSG::INFO << "closed " << m_f1 << endmsg;
  }

  r = p_fileMgr->close(fd_2,name());
  if (r != 0) {
    log << MSG::ERROR << "unable to close " << m_f2 << " with FD " << fd_2
	<< endmsg;
  } else {
    log << MSG::INFO << "closed " << m_f2 << endmsg;
  }

  r = p_fileMgr->close(fd_3,name());
  if (r != 0) {
    log << MSG::ERROR << "unable to close " << m_f3 << " with FD " << fd_3
	<< endmsg;
  } else {
    log << MSG::INFO << "closed " << m_f3 << endmsg;
  }

  r = p_fileMgr->close(fd_4,name());
  if (r != 0) {
    log << MSG::ERROR << "unable to close " << m_f1 << " with FD " << fd_4
	<< endmsg;
  } else {
    log << MSG::INFO << "closed " << m_f1 << endmsg;
  }

  r = p_fileMgr->close(fp_r1,name());
  if (r != 0) {
    log << MSG::ERROR << "unable to close " << m_fr1 << " with ptr " << fp_r1
	<< endmsg;
  } else {
    log << MSG::INFO << "closed " << m_fr1 << endmsg;
  }

  r = p_fileMgr->close(fp_r2,name());
  if (r != 0) {
    log << MSG::ERROR << "unable to close " << m_fr2 << " with ptr " << fp_r2
	<< endmsg;
  } else {
    log << MSG::INFO << "closed " << m_fr2 << endmsg;
  }

  // r = p_fileMgr->close(fp_r3,name());
  // if (r != 0) {
  //   log << MSG::ERROR << "unable to close " << m_fr3 << " with ptr " << fp_r3
  // 	<< endmsg;
  // } else {
  //   log << MSG::INFO << "closed " << m_fr3 << endmsg;
  // }

  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode 
FileMgrTest::PosixOpenAction(const Io::FileAttr* fa, const std::string& c) {

  MsgStream log(msgSvc(), name());

  log << MSG::INFO << "PosixOpenAction called by " << c << "  for tech "
      << fa->tech() << " on " << fa
	<< endmsg;

  // if (fa.tech() != Io::POSIX) {

  //   // log << MSG::ERROR << "PosixOpenAction called for incorrect tech: "
  //   // 	<< fa.tech() << " on " << fa
  //   // 	<< endmsg;

  //   return StatusCode::SUCCESS;
  // }

  // log << MSG::INFO << "PosixOpenAction for " << fa
  //     << endmsg;


    return StatusCode::SUCCESS;


}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode 
FileMgrTest::allCloseAction(const Io::FileAttr* fa, const std::string& c) {

  MsgStream log(msgSvc(), name());

  log << MSG::INFO << "AllCloseAction called by " << c << "  for tech "
      << fa->tech() << " on " << fa
      << endmsg;

  return StatusCode::SUCCESS;


}
