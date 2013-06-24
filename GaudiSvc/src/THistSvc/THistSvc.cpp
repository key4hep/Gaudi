#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, since it comes from ROOT TMathBase.h
#pragma warning(disable:2259)
#endif

#include "THistSvc.h"

#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Tokenizer.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/FileIncident.h"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/IIoComponentMgr.h"
#include "GaudiKernel/IFileMgr.h"

#include "boost/bind.hpp"


#include "TROOT.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TKey.h"
#include "TError.h"
#include "TGraph.h"

#include <sstream>
#include <streambuf>
#include <cstdio>

using namespace std;


DECLARE_SERVICE_FACTORY(THistSvc)

inline void toupper(std::string &s)
{
  std::transform(s.begin(), s.end(), s.begin(),
                 (int(*)(int)) toupper);
}


//*************************************************************************//

THistSvc::THistSvc( const std::string& name, ISvcLocator* svc )
  : base_class(name, svc), m_log(msgSvc(), name ), signaledStop(false),
    m_delayConnect(false),m_okToConnect(false),
    p_incSvc(0), p_fileMgr(0) {

  declareProperty ("AutoSave", m_autoSave=0 );
  declareProperty ("AutoFlush", m_autoFlush=0 );
  declareProperty ("PrintAll", m_print=false);
  declareProperty ("MaxFileSize", m_maxFileSize=10240,
		   "maximum file size in MB. if exceeded, will cause an abort. -1 to never check.");
  declareProperty ("CompressionLevel", m_compressionLevel=1 )->declareUpdateHandler( &THistSvc::setupCompressionLevel, this );
  declareProperty ("Output", m_outputfile )->declareUpdateHandler( &THistSvc::setupOutputFile, this );
  declareProperty ("Input", m_inputfile )->declareUpdateHandler ( &THistSvc::setupInputFile,  this );


}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

THistSvc::~THistSvc() {

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::initialize() {
  GlobalDirectoryRestore restore;

  // Super ugly hack to make sure we have the OutputLevel set first, so we
  // can see DEBUG printouts in update handlers.
  IJobOptionsSvc* jos(0);
  if( serviceLocator()->service( "JobOptionsSvc", jos, true ).isSuccess() ) {
    const std::vector<const Property*> *props = jos->getProperties( name() );

    if (props != NULL) {
      for ( std::vector<const Property*>::const_iterator cur = props->begin();
            cur != props->end(); cur++) {
        if ( (*cur)->name() == "OutputLevel" ) {
          setProperty( **cur ).ignore();
          m_log.setLevel( m_outputLevel.value() );
          break;
        }
      }
    }
  }


  StatusCode status = Service::initialize();
  m_log.setLevel( m_outputLevel.value() );

  if (status.isFailure()) {
    m_log << MSG::ERROR << "initializing service" << endmsg;
    return status;
  }

  vector<string>::const_iterator itr;
  StatusCode st(StatusCode::SUCCESS);

  try {
    setupOutputFile( m_outputfile );
  } catch ( GaudiException& err ) {
    m_log << MSG::ERROR
          << "Caught: " << err << endmsg;
    st = StatusCode::FAILURE;
  }

  try {
    setupInputFile( m_inputfile );
  } catch ( GaudiException& err ) {
    m_log << MSG::ERROR
          << "Caught: " << err << endmsg;
    st = StatusCode::FAILURE;
  }

  // Protect against multiple instances of TROOT
  if ( 0 == gROOT )   {
    static TROOT root("root","ROOT I/O");
    //    gDebug = 99;
  } else {
    if (m_log.level() <= MSG::VERBOSE)
      m_log << MSG::VERBOSE << "ROOT already initialized, debug = "
            << gDebug<< endmsg;
  }

  if (service("IncidentSvc", p_incSvc, true).isFailure()) {
    m_log << MSG::ERROR << "unable to get the IncidentSvc" << endmsg;
    st = StatusCode::FAILURE;
  } else {
    p_incSvc->addListener( this, "EndEvent", 100, true);
  }

  if (service("FileMgr",p_fileMgr,true).isFailure()) {
    m_log << MSG::ERROR << "unable to get the FileMgr" << endmsg;
    st = StatusCode::FAILURE;
  } else {
    m_log << MSG::DEBUG << "got the FileMgr" << endmsg;
  }


  // Register open/close callback actions

  Io::bfcn_action_t boa = boost::bind(&THistSvc::rootOpenAction, this, _1,_2);
  if (p_fileMgr->regAction(boa, Io::OPEN, Io::ROOT).isFailure()) {
    m_log << MSG::ERROR
	  << "unable to register ROOT file open action with FileMgr"
	  << endmsg;
  }
  Io::bfcn_action_t bea = boost::bind(&THistSvc::rootOpenErrAction, this, _1,_2);
  if (p_fileMgr->regAction(bea, Io::OPEN_ERR, Io::ROOT).isFailure()) {
    m_log << MSG::ERROR
	  << "unable to register ROOT file open Error action with FileMgr"
	  << endmsg;
  }


  m_okToConnect = true;

  if (m_delayConnect == true) {
    if (m_inputfile.value().size() > 0) { setupInputFile(m_inputfile); }
    if (m_outputfile.value().size() > 0) { setupOutputFile(m_outputfile); }

    m_delayConnect = false;

  }
  m_alreadyConnectedOutFiles.clear();
  m_alreadyConnectedInFiles.clear();


  IIoComponentMgr* iomgr(0);

  if (service("IoComponentMgr", iomgr, true).isFailure()) {
    m_log << MSG::ERROR << "unable to get the IoComponentMgr" << endmsg;
    st = StatusCode::FAILURE;
  } else {

    if ( !iomgr->io_register (this).isSuccess() ) {
      m_log << MSG::ERROR
            << "could not register with the I/O component manager !"
            << endmsg;
      st = StatusCode::FAILURE;
    } else {
      bool all_good = true;
      typedef std::map<std::string, std::pair<TFile*,Mode> > Registry_t;
      // register input/output files...
      for ( Registry_t::const_iterator
              ireg = m_files.begin(),
              iend = m_files.end();
            ireg != iend;
            ++ireg ) {
        const std::string fname = ireg->second.first->GetName();
        const IIoComponentMgr::IoMode::Type iomode =
          ( ireg->second.second==THistSvc::READ
	    ? IIoComponentMgr::IoMode::READ
	    : IIoComponentMgr::IoMode::WRITE );
        if ( !iomgr->io_register (this, iomode, fname).isSuccess () ) {
          m_log << MSG::WARNING << "could not register file ["
                << fname << "] with the I/O component manager..." << endmsg;
          all_good = false;
        } else {
          m_log << MSG::INFO << "registered file [" << fname << "]... [ok]"
              << endmsg;
        }
      }
      if (!all_good) {
        m_log << MSG::ERROR
              << "problem while registering input/output files with "
              << "the I/O component manager !" << endmsg;
        st = StatusCode::FAILURE;
      }
    }

  }

  if (st.isFailure()) {
    m_log << MSG::FATAL << "Unable to initialize THistSvc" << endmsg;
  }

  return st;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::reinitialize() {

  GlobalDirectoryRestore restore;

  m_log << MSG::WARNING << "reinitialize not implemented" << endmsg;


  return StatusCode::SUCCESS;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::finalize() {

  GlobalDirectoryRestore restore;

  if (m_log.level() <= MSG::DEBUG)
    m_log << MSG::DEBUG << "THistSvc::finalize" << endmsg;

#ifndef NDEBUG
  if (m_log.level() <= MSG::DEBUG) {
  uidMap::const_iterator uitr;
  for (uitr=m_uids.begin(); uitr != m_uids.end(); ++uitr) {

    TObject* to = uitr->second.obj;

    string dirname("none");
      if (to && to->IsA()->InheritsFrom("TTree")) {
      TTree* tr = dynamic_cast<TTree*>(to);
      if (tr->GetDirectory() != 0) {
        dirname = tr->GetDirectory()->GetPath();
      }
      } else if (to && to->IsA()->InheritsFrom("TGraph")) {
      if (!uitr->second.temp) {
        dirname = uitr->second.file->GetPath();
        string id2(uitr->second.id);
        id2.erase(0,id2.find("/",1));
        id2.erase(id2.rfind("/"), id2.length());
        if (id2.find("/") == 0) {
          id2.erase(0,1);
        }
        dirname += id2;
      } else {
        dirname = "/tmp";
      }
      } else if (to && to->IsA()->InheritsFrom("TH1")) {
      TH1* th = dynamic_cast<TH1*>(to);
      if (th == 0) {
        m_log << MSG::ERROR << "Couldn't dcast: " << uitr->first << endmsg;
      } else {
        if (th->GetDirectory() != 0) {
          dirname = th->GetDirectory()->GetPath();
        }
      }
      } else if (! to ) {
	m_log << MSG::WARNING << uitr->first << " has NULL TObject ptr"
	      << endmsg;
    }

      m_log << MSG::DEBUG << "uid: \"" << uitr->first << "\"  temp: "
            << uitr->second.temp << "  dir: " << dirname
            << endmsg;
  }
  }
#endif

  StatusCode sc = write();
  if (sc.isFailure()) {
    m_log << MSG::ERROR << "problems writing histograms" << endmsg;
  }

  if (m_print) {
    m_log << MSG::INFO << "Listing contents of ROOT files: " << endmsg;
  }
  vector<TFile*> deleted_files;
  map<string, pair<TFile*,Mode> >::const_iterator itr;
  for (itr = m_files.begin(); itr != m_files.end(); ++itr) {

    if (find(deleted_files.begin(), deleted_files.end(), itr->second.first) ==
        deleted_files.end()) {
      deleted_files.push_back(itr->second.first);

#ifndef NDEBUG
      if (m_log.level() <= MSG::DEBUG)
        m_log << MSG::DEBUG << "finalizing stream/file " << itr->first << ":"
              << itr->second.first->GetName()
              << endmsg;
#endif
    } else {
#ifndef NDEBUG
      if (m_log.level() <= MSG::DEBUG)
        m_log << MSG::DEBUG << "already finalized stream " << itr->first << endmsg;
#endif
      continue;
    }


    if (m_print && m_log.level() <= MSG::INFO) {

      m_log << MSG::INFO;
      m_log << "==> File: " << itr->second.first->GetName()
            << "  stream: " << itr->first << endmsg;

      itr->second.first->Print("base");
    }

    string tmpfn=itr->second.first->GetName();

    p_fileMgr->close(itr->second.first, name());

    IIncidentSvc *pi(0);
    if (service("IncidentSvc",pi).isFailure()) {
      m_log << MSG::ERROR << "Unable to get the IncidentSvc" << endmsg;
      return StatusCode::FAILURE;
    }

    if (itr->second.second==SHARE) {

      //Merge File
      void* vf(0);
      int r = p_fileMgr->open(Io::ROOT,name(), m_sharedFiles[itr->first],
			      Io::WRITE|Io::APPEND,vf,"HIST");

      if (r != 0 ) {
	m_log << MSG::ERROR << "unable to open Final Output File: \""
	      << m_sharedFiles[itr->first] << "\" for merging"
	      << endmsg;
        return StatusCode::FAILURE;
      }

      TFile *outputfile = (TFile*) vf;
      pi->fireIncident(FileIncident(name(), IncidentType::WroteToOutputFile,
                                     m_sharedFiles[itr->first]));

      if (m_log.level() <= MSG::DEBUG)
        m_log << MSG::DEBUG << "THistSvc::write()::Merging Rootfile "<<endmsg;

      vf = 0;
      r = p_fileMgr->open(Io::ROOT,name(),tmpfn,Io::READ,vf,"HIST");

      if (r != 0) {
	m_log << MSG::ERROR << "unable to open temporary file: \""
	      << tmpfn << endmsg;
        return StatusCode::FAILURE;
      }

      TFile *inputfile = (TFile*) vf;

      outputfile->SetCompressionLevel( inputfile->GetCompressionLevel() );

      MergeRootFile(outputfile, inputfile);

      outputfile->Write();
      p_fileMgr->close(outputfile,name());
      p_fileMgr->close(inputfile,name());

      if (m_log.level() <= MSG::DEBUG)
        m_log << MSG::DEBUG << "Trying to remove temporary file \"" << tmpfn
              << "\""<<endmsg;

      std::remove(tmpfn.c_str());
    }
    delete itr->second.first;
  }

  m_sharedFiles.clear();
  m_fileStreams.clear();
  m_files.clear();
  m_uids.clear();
  m_ids.clear();
  m_tobjs.clear();

  return Service::finalize();
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

bool
THistSvc::browseTDir(TDirectory *dir) const {

  if (dir == 0) {
    std::cerr << "TDirectory == 0" << std::endl;
    return false;
  }

  GlobalDirectoryRestore restore;

  dir->cd();


  cout << "-> " << dir->GetPath() << "  "
       << dir->GetListOfKeys()->GetSize() << endl;

  //  TIter nextkey(dir->GetListOfKeys());
  TIter nextkey(dir->GetList());
  while (TKey *key = (TKey*)nextkey()) {

    TObject *obj = key->ReadObj();
    if (obj == 0) { cout << key->GetName() << " obj==0"<< endl; continue; }
    //    if (obj->IsA()->InheritsFrom("TDirectory")) {
      cout << "  Key: " << key->GetName() << "   "
           << " tit: " << obj->GetTitle() << "   "
           << " (" << key->GetClassName() << ")" << endl;
      //    }
  }

  nextkey = dir->GetListOfKeys();
  while (TKey *key = (TKey*)nextkey()) {

    TObject *obj = key->ReadObj();
    if (obj == 0) { cout << key->GetName() << " obj==0"<< endl; continue; }
    if (obj->IsA()->InheritsFrom("TDirectory")) {
      TDirectory *tt = dynamic_cast<TDirectory*>(obj);
      browseTDir(tt);
    }
  }

  return true;
}


//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getTHists(TDirectory *td, TList & tl, bool rcs) const {
  GlobalDirectoryRestore restore;

  gErrorIgnoreLevel = kBreak;

  if (!td->cd()) {
    m_log << MSG::ERROR << "getTHists: No such TDirectory \"" << td->GetPath()
          << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  if (m_log.level() <= MSG::DEBUG)
    m_log << MSG::DEBUG << "getTHists: \"" << td->GetPath() << "\": found "
          << td->GetListOfKeys()->GetSize() << " keys" << endmsg;

  TIter nextkey(td->GetListOfKeys());
  while (TKey *key = (TKey*)nextkey()) {
    if (m_log.level() <= MSG::DEBUG)
      m_log << MSG::DEBUG << "  key: " << key->GetName();
    TObject *obj = key->ReadObj();
    if (obj != 0 && obj->IsA()->InheritsFrom("TDirectory")) {
      if (m_log.level() <= MSG::DEBUG)
        m_log << " (" << obj->IsA()->GetName() << ")";
    } else if (obj != 0 && obj->IsA()->InheritsFrom("TH1")) {
      if (m_log.level() <= MSG::DEBUG)
        m_log << " (" << obj->IsA()->GetName() << ")";
      tl.Add(obj);
    } else if (obj != 0) {
      if (m_log.level() <= MSG::DEBUG)
        m_log << " [" << obj->IsA()->GetName() << "]";
    }
    if (m_log.level() <= MSG::DEBUG)
      m_log << endmsg;
  }

  // operate recursively
  if (rcs) {
    nextkey = td->GetListOfKeys();
    while (TKey *key = (TKey*)nextkey()) {
      TObject *obj = key->ReadObj();
      if (obj != 0 && obj->IsA()->InheritsFrom("TDirectory")) {
          TDirectory *tt = dynamic_cast<TDirectory*>(obj);
          getTHists(tt, tl, rcs);
      }
    }
  }

  return StatusCode::SUCCESS;


}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getTHists(const std::string& dir, TList & tl, bool rcs) const {

  GlobalDirectoryRestore restore;

  gErrorIgnoreLevel = kBreak;

  StatusCode sc;

  std::string stream,rem,r2;
  parseString(dir,stream,rem);

  map< string,pair<TFile*,Mode> >::const_iterator itr = m_files.find(stream);
  if (itr != m_files.end()) {
    r2 = itr->second.first->GetName();
    r2 += ":/";
    r2 += rem;

    if (m_log.level() <= MSG::DEBUG)
      m_log << MSG::DEBUG << "getTHists: \"" << dir
            << "\" looks like a stream name."  << " associated TFile: \""
            << itr->second.first->GetName() << "\"" << endmsg;

    if (gDirectory->cd(r2.c_str())) {
      m_curstream = stream;
      sc = getTHists(gDirectory,tl,rcs);
      m_curstream = "";
      return sc;
    } else {
      if (m_log.level() <= MSG::DEBUG)
        m_log << MSG::DEBUG << "getTHists: no such TDirectory \""
              << r2 << "\"" << endmsg;
    }

  } else {
    if (m_log.level() <= MSG::DEBUG)
      m_log << MSG::DEBUG << "getTHists: stream \"" << stream << "\" not found"
            << endmsg;
  }

  if (!gDirectory->cd(dir.c_str())) {
    m_log << MSG::ERROR << "getTHists: No such TDirectory/stream \"" << dir
          << "\"" << endmsg;
    sc = StatusCode::FAILURE;
  } else {
    sc = getTHists(gDirectory,tl,rcs);
  }

  return sc;

}
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getTTrees(TDirectory *td, TList & tl, bool rcs) const {
  GlobalDirectoryRestore restore;

  gErrorIgnoreLevel = kBreak;

  if (!td->cd()) {
    m_log << MSG::ERROR << "getTTrees: No such TDirectory \""
          << td->GetPath() << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  if (m_log.level() <= MSG::DEBUG)
    m_log << MSG::DEBUG << "getTHists: \"" << td->GetPath() << "\": found "
          << td->GetListOfKeys()->GetSize() << " keys" << endmsg;

  TIter nextkey(td->GetListOfKeys());
  while (TKey *key = (TKey*)nextkey()) {
    if (m_log.level() <= MSG::DEBUG)
      m_log << MSG::DEBUG << "  key: " << key->GetName();
    TObject *obj = key->ReadObj();
    if (obj != 0 && obj->IsA()->InheritsFrom("TDirectory")) {
      if (m_log.level() <= MSG::DEBUG)
        m_log << " (" << obj->IsA()->GetName() << ")";
    } else if (obj != 0 && obj->IsA()->InheritsFrom("TTree")) {
      if (m_log.level() <= MSG::DEBUG)
        m_log << " (" << obj->IsA()->GetName() << ")";
      tl.Add(obj);
    } else if (obj != 0) {
      if (m_log.level() <= MSG::DEBUG)
        m_log << " [" << obj->IsA()->GetName() << "]";
    }
    m_log << endmsg;
  }

  // operate recursively
  if (rcs) {
    nextkey = td->GetListOfKeys();
    while (TKey *key = (TKey*)nextkey()) {
      TObject *obj = key->ReadObj();
      if (obj != 0 && obj->IsA()->InheritsFrom("TDirectory")) {
          TDirectory *tt = dynamic_cast<TDirectory*>(obj);
          getTTrees(tt, tl, rcs);
      }
    }
  }

  return StatusCode::SUCCESS;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getTTrees(const std::string& dir, TList & tl, bool rcs) const {
  GlobalDirectoryRestore restore;

  gErrorIgnoreLevel = kBreak;

  StatusCode sc;

  std::string stream,rem,r2;
  parseString(dir,stream,rem);

  map< string,pair<TFile*,Mode> >::const_iterator itr = m_files.find(stream);
  if (itr != m_files.end()) {
    r2 = itr->second.first->GetName();
    r2 += ":/";
    r2 += rem;

    if (m_log.level() <= MSG::DEBUG)
      m_log << MSG::DEBUG << "getTTrees: \"" << dir
            << "\" looks like a stream name."  << " associated TFile: \""
            << itr->second.first->GetName() << "\"" << endmsg;

    if (gDirectory->cd(r2.c_str())) {
      return getTTrees(gDirectory,tl,rcs);
    } else {
      if (m_log.level() <= MSG::DEBUG)
        m_log << MSG::DEBUG << "getTTrees: no such TDirectory \""
              << r2 << "\"" << endmsg;
    }

  } else {
    if (m_log.level() <= MSG::DEBUG)
      m_log << MSG::DEBUG << "getTTrees: stream \"" << stream << "\" not found"
            << endmsg;
  }

  if (!gDirectory->cd(dir.c_str())) {
    m_log << MSG::ERROR << "getTTrees: No such TDirectory/stream \"" << dir
          << "\"" << endmsg;
    sc = StatusCode::FAILURE;
  } else {
    sc = getTTrees(gDirectory,tl,rcs);
  }

  return sc;


}


//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getTHists(TDirectory *td, TList & tl, bool rcs, bool reg) {

  GlobalDirectoryRestore restore;

  gErrorIgnoreLevel = kBreak;

  if (!td->cd()) {
    m_log << MSG::ERROR << "getTHists: No such TDirectory \"" << td->GetPath()
          << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  if (m_log.level() <= MSG::DEBUG)
    m_log << MSG::DEBUG << "getTHists: \"" << td->GetPath() << "\": found "
          << td->GetListOfKeys()->GetSize() << " keys" << endmsg;

  TIter nextkey(td->GetListOfKeys());
  while (TKey *key = (TKey*)nextkey()) {
    if (m_log.level() <= MSG::DEBUG)
      m_log << MSG::DEBUG << "  key: " << key->GetName();
    TObject *obj = key->ReadObj();
    if (obj != 0 && obj->IsA()->InheritsFrom("TDirectory")) {
      if (m_log.level() <= MSG::DEBUG)
        m_log << " (" << obj->IsA()->GetName() << ")";
    } else if (obj != 0 && obj->IsA()->InheritsFrom("TH1")) {
      if (m_log.level() <= MSG::DEBUG)
        m_log << " (" << obj->IsA()->GetName() << ")";
      tl.Add(obj);
      if (reg && m_curstream != "") {
        string dir = td->GetPath();
        string fil = td->GetFile()->GetName();
        dir.erase(0,fil.length()+1);
        string id = "/" + m_curstream;
        if ( dir == "/" ) {
          id = id + "/" + key->GetName();
        } else {
          id = id + dir + "/" + key->GetName();
        }
        if (!exists(id)) {
          if (m_log.level() <= MSG::DEBUG)
            m_log << "  reg as \"" << id << "\"";
          regHist(id).ignore();
        } else {
          if (m_log.level() <= MSG::DEBUG)
            m_log << "  already registered";
        }
      }
    } else if (obj != 0) {
      if (m_log.level() <= MSG::DEBUG)
        m_log << " [" << obj->IsA()->GetName() << "]";
    }
    if (m_log.level() <= MSG::DEBUG)
      m_log << endmsg;
  }

  // operate recursively
  if (rcs) {
    nextkey = td->GetListOfKeys();
    while (TKey *key = (TKey*)nextkey()) {
      TObject *obj = key->ReadObj();
      if (obj != 0 && obj->IsA()->InheritsFrom("TDirectory")) {
          TDirectory *tt = dynamic_cast<TDirectory*>(obj);
          getTHists(tt, tl, rcs, reg);
      }
    }
  }

  return StatusCode::SUCCESS;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getTHists(const std::string& dir, TList & tl, bool rcs, bool reg) {

  GlobalDirectoryRestore restore;

  gErrorIgnoreLevel = kBreak;

  StatusCode sc;

  std::string stream,rem,r2;
  parseString(dir,stream,rem);

  map< string,pair<TFile*,Mode> >::const_iterator itr = m_files.find(stream);
  if (itr != m_files.end()) {
    r2 = itr->second.first->GetName();
    r2 += ":/";
    r2 += rem;

    if (m_log.level() <= MSG::DEBUG)
      m_log << MSG::DEBUG << "getTHists: \"" << dir
            << "\" looks like a stream name."  << " associated TFile: \""
            << itr->second.first->GetName() << "\"" << endmsg;

    if (gDirectory->cd(r2.c_str())) {
      m_curstream = stream;
      sc = getTHists(gDirectory,tl,rcs,reg);
      m_curstream = "";
      return sc;
    } else {
      if (m_log.level() <= MSG::DEBUG)
        m_log << MSG::DEBUG << "getTHists: no such TDirectory \""
              << r2 << "\"" << endmsg;
    }

  } else {
    if (m_log.level() <= MSG::DEBUG)
      m_log << MSG::DEBUG << "getTHists: stream \"" << stream << "\" not found"
            << endmsg;
  }

  if (!gDirectory->cd(dir.c_str())) {
    m_log << MSG::ERROR << "getTHists: No such TDirectory/stream \"" << dir
          << "\"" << endmsg;
    sc = StatusCode::FAILURE;
  } else {
    if (reg) {
      m_log << MSG::WARNING << "Unable to register histograms automatically "
            << "without a valid stream name" << endmsg;
      reg = false;
    }
    sc = getTHists(gDirectory,tl,rcs,reg);
  }

  return sc;

}
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getTTrees(TDirectory *td, TList & tl, bool rcs, bool reg) {

  GlobalDirectoryRestore restore;

  gErrorIgnoreLevel = kBreak;

  if (!td->cd()) {
    m_log << MSG::ERROR << "getTTrees: No such TDirectory \""
          << td->GetPath() << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  if (m_log.level() <= MSG::DEBUG)
    m_log << MSG::DEBUG << "getTHists: \"" << td->GetPath() << "\": found "
          << td->GetListOfKeys()->GetSize() << " keys" << endmsg;

  TIter nextkey(td->GetListOfKeys());
  while (TKey *key = (TKey*)nextkey()) {
    if (m_log.level() <= MSG::DEBUG)
      m_log << MSG::DEBUG << "  key: " << key->GetName();
    TObject *obj = key->ReadObj();
    if (obj != 0 && obj->IsA()->InheritsFrom("TDirectory")) {
      if (m_log.level() <= MSG::DEBUG)
        m_log << " (" << obj->IsA()->GetName() << ")";
    } else if (obj != 0 && obj->IsA()->InheritsFrom("TTree")) {
      if (m_log.level() <= MSG::DEBUG)
        m_log << " (" << obj->IsA()->GetName() << ")";
      tl.Add(obj);
      if (reg && m_curstream != "") {
        string dir = td->GetPath();
        string fil = td->GetFile()->GetName();
        dir.erase(0,fil.length()+1);
        string id = "/" + m_curstream;
        if ( dir == "/" ) {
          id = id + "/" + key->GetName();
        } else {
          id = id + dir + "/" + key->GetName();
        }
        if (!exists(id)) {
          if (m_log.level() <= MSG::DEBUG)
            m_log << "  reg as \"" << id << "\"";
          regHist(id).ignore();
        } else {
          if (m_log.level() <= MSG::DEBUG)
            m_log << "  already registered";
        }
      }
    } else if (obj != 0) {
      if (m_log.level() <= MSG::DEBUG)
        m_log << " [" << obj->IsA()->GetName() << "]";
    }
    if (m_log.level() <= MSG::DEBUG)
      m_log << endmsg;
  }

  // operate recursively
  if (rcs) {
    nextkey = td->GetListOfKeys();
    while (TKey *key = (TKey*)nextkey()) {
      TObject *obj = key->ReadObj();
      if (obj != 0 && obj->IsA()->InheritsFrom("TDirectory")) {
          TDirectory *tt = dynamic_cast<TDirectory*>(obj);
          getTTrees(tt, tl, rcs, reg);
      }
    }
  }

  return StatusCode::SUCCESS;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getTTrees(const std::string& dir, TList & tl, bool rcs, bool reg) {

  GlobalDirectoryRestore restore;

  gErrorIgnoreLevel = kBreak;

  StatusCode sc;

  std::string stream,rem,r2;
  parseString(dir,stream,rem);

  map< string,pair<TFile*,Mode> >::const_iterator itr = m_files.find(stream);
  if (itr != m_files.end()) {
    r2 = itr->second.first->GetName();
    r2 += ":/";
    r2 += rem;

    if (m_log.level() <= MSG::DEBUG)
      m_log << MSG::DEBUG << "getTTrees: \"" << dir
            << "\" looks like a stream name."  << " associated TFile: \""
            << itr->second.first->GetName() << "\"" << endmsg;

    if (gDirectory->cd(r2.c_str())) {
      return getTTrees(gDirectory,tl,rcs,reg);
    } else {
      if (m_log.level() <= MSG::DEBUG)
        m_log << MSG::DEBUG << "getTTrees: no such TDirectory \""
              << r2 << "\"" << endmsg;
    }

  } else {
    if (m_log.level() <= MSG::DEBUG)
      m_log << MSG::DEBUG << "getTTrees: stream \"" << stream << "\" not found"
            << endmsg;
  }

  if (!gDirectory->cd(dir.c_str())) {
    m_log << MSG::ERROR << "getTTrees: No such TDirectory/stream \"" << dir
          << "\"" << endmsg;
    sc = StatusCode::FAILURE;
  } else {
    sc = getTTrees(gDirectory,tl,rcs,reg);
  }

  return sc;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::deReg(TObject* obj) {

  objMap::iterator itr = m_tobjs.find(obj);
  if (itr != m_tobjs.end()) {
    THistID hid = itr->second;

    uidMap::iterator itr2 = m_uids.find(hid.id);
    if (itr2 == m_uids.end()) {
      m_log << MSG::ERROR << "Problems deregistering TObject \""
            << obj->GetName()
            << "\" with id \"" << hid.id << "\"" << endmsg;
      return StatusCode::FAILURE;
    }

    std::string id,root,rem;
    parseString(hid.id, root, rem);

    idMap::iterator itr3;
    bool found(false);

    std::pair<idMap::iterator, idMap::iterator> mitr = m_ids.equal_range(rem);
    if (mitr.first == mitr.second) {
      m_log << MSG::ERROR << "Problems deregistering TObject \""
            << obj->GetName()
            << "\" with id \"" << hid.id << "\"" << endmsg;
      return StatusCode::FAILURE;
    } else {
      for (itr3 = mitr.first; itr3 != mitr.second; ++itr3) {
        if (itr3->second.obj == obj) {
          found = true;
          break;
        }
      }
      if (!found) {
        m_log << MSG::ERROR << "Problems deregistering TObject \""
              << obj->GetName()
              << "\" with id \"" << hid.id << "\"" << endmsg;
      }
    }

    m_tobjs.erase(itr);
    m_uids.erase(itr2);
    m_ids.erase(itr3);

    return StatusCode::SUCCESS;

  } else {
    m_log << MSG::ERROR << "Cannot unregister TObject \"" << obj->GetName()
          << "\": not known to THistSvc" << endmsg;
    return StatusCode::FAILURE;
  }

}


//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::deReg(const std::string& id) {

  uidMap::iterator itr = m_uids.find(id);
  if (itr == m_uids.end()) {
    m_log << MSG::ERROR << "Problems deregistering id \""
          << id << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  TObject* obj = itr->second.obj;

  return deReg(obj);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::regHist(const std::string& id) {

  TH1 *hist(0);

  return regHist_i(hist, id);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::regHist(const std::string& id, TH1* hist) {
  return regHist_i(hist, id);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::regHist(const std::string& id, TH2* hist) {
  return regHist_i(hist, id);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::regHist(const std::string& id, TH3* hist) {
  return regHist_i(hist, id);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::regTree(const std::string& id) {
  TTree *hist(0);
  return regHist_i(hist, id);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::regTree(const std::string& id, TTree* hist) {
  StatusCode sc = regHist_i(hist, id);
  if (hist != 0 && sc.isSuccess()) {
    if (m_autoSave != 0)
      hist->SetAutoSave(m_autoSave);
    hist->SetAutoFlush(m_autoFlush);
  }
  return sc;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::regGraph(const std::string& id) {
  TGraph *hist(0);
  return regHist_i(hist, id);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::regGraph(const std::string& id, TGraph* hist) {
  if ( strcmp(hist->GetName(),"Graph") == 0 ) {

    std::string id2(id);
    string::size_type i = id2.rfind("/");
    if (i != string::npos) {
      id2.erase(0,i+1);
    }

    m_log << MSG::INFO << "setting name of TGraph id: \"" << id << "\" to \""
          << id2 << "\" since it is unset" << endmsg;
    hist->SetName(id2.c_str());
  }

  return regHist_i(hist, id);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getHist(const std::string& id, TH1*& hist) const {
  return getHist_i(id, hist);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getHist(const std::string& id, TH2*& hist) const {
  return getHist_i(id, hist);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getHist(const std::string& id, TH3*& hist) const {
  return getHist_i(id, hist);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

std::vector<std::string>
THistSvc::getHists() const {

  std::vector<std::string> names;
  names.reserve(m_uids.size());

  uidMap::const_iterator itr;
  for (itr = m_uids.begin(); itr != m_uids.end(); ++itr) {
    THistID tid = itr->second;

    if (tid.obj->IsA()->InheritsFrom("TH1")) {
      names.push_back(itr->first);
    }

  }

  return names;

}
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getTree(const std::string& id, TTree*& hist) const {
  return getHist_i(id, hist);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

std::vector<std::string>
THistSvc::getTrees() const {

  std::vector<std::string> names;
  names.reserve(m_uids.size());

  uidMap::const_iterator itr;
  for (itr = m_uids.begin(); itr != m_uids.end(); ++itr) {
    THistID tid = itr->second;

    if (tid.obj->IsA()->InheritsFrom("TTree")) {
      names.push_back(itr->first);
    }

  }

  return names;

}
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getGraph(const std::string& id, TGraph*& hist) const {
  return getHist_i(id, hist);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

std::vector<std::string>
THistSvc::getGraphs() const {

  std::vector<std::string> names;
  names.reserve(m_uids.size());

  uidMap::const_iterator itr;
  for (itr = m_uids.begin(); itr != m_uids.end(); ++itr) {
    THistID tid = itr->second;

    if (tid.obj->IsA()->InheritsFrom("TGraph")) {
      names.push_back(itr->first);
    }

  }

  return names;

}
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::readHist(const std::string& id, TH1*& hist) const {
  return readHist_i(id, hist);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::readHist(const std::string& id, TH2*& hist) const {
  return readHist_i(id, hist);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::readHist(const std::string& id, TH3*& hist) const {
  return readHist_i(id, hist);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::readTree(const std::string& id, TTree*& hist) const {
  return readHist_i(id, hist);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

bool
THistSvc::findStream(const string& id, string& stream, string& rem,
                   TFile*& file) const {

  string::size_type pos = id.find("/");

  if (pos == string::npos) {
    stream = "temp";
    rem = id;
  } else if (pos != 0) {
    stream = "temp";
    rem = id;
  } else {

    string::size_type pos2 = id.find("/",pos+1);

    if (pos2 == string::npos) {
      m_log << MSG::ERROR << "badly formed Hist/Tree id: \"" << id << "\""
            << endmsg;
      return false;
    }

    parseString(id,stream,rem);

  }

  if (stream == "temp") {
    file = 0;
    return true;
  }

  map< string,pair<TFile*,Mode> >::const_iterator itr = m_files.find(stream);
  if (itr != m_files.end()) {
    file = itr->second.first;
  } else {
    file = 0;
    m_log << MSG::WARNING << "no stream \"" << stream
          << "\" associated with id: \"" << id << "\""
          << endmsg;
  }

  return true;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
THistSvc::parseString(const string& id, string& root, string& rem) const {
  string::size_type pos = id.find("/");

  if (pos == string::npos) {
    root = "";
    rem = id;
    return;
  }

  if (pos == 0) {
    parseString(id.substr(1,id.length()),root,rem);
  } else {
    root = id.substr(0,pos);
    rem = id.substr(pos+1,id.length());
  }

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
THistSvc::setupCompressionLevel( Property& /* cl */ )
{

  m_log << MSG::WARNING << "\"CompressionLevel\" Property has been deprecated. "
        << "Set it via the \"CL=\" parameter in the \"Output\" Property"
        << endmsg;

}


//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
THistSvc::setupInputFile( Property& /*m_inputfile*/ )
{

  if (FSMState() < Gaudi::StateMachine::CONFIGURED || !m_okToConnect ) {

    m_log <<MSG::DEBUG << "Delaying connection of Input Files until Initialize"
	  << ". now in " << FSMState()
	  << endmsg;

    m_delayConnect = true;
  } else {

    m_log <<MSG::DEBUG << "Now connecting of Input Files"
	  << endmsg;

  StatusCode sc = StatusCode::SUCCESS;

  typedef std::vector<std::string> Strings_t;
  for ( Strings_t::const_iterator
          itr  = m_inputfile.value().begin(),
          iEnd = m_inputfile.value().end();
        itr != iEnd;
        ++itr ) {
    if ( m_alreadyConnectedInFiles.end() ==
         m_alreadyConnectedInFiles.find( *itr ) ) {
      if ( connect(*itr).isFailure() ) {
        sc = StatusCode::FAILURE;
      } else {
        m_alreadyConnectedInFiles.insert( *itr );
      }
    }
  }

  if ( !sc.isSuccess() ) {
    throw GaudiException( "Problem connecting inputfile !!", name(),
                          StatusCode::FAILURE );
  }

  }

  return;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
THistSvc::setupOutputFile( Property& /*m_outputfile*/ )
{
  if (FSMState() < Gaudi::StateMachine::CONFIGURED || !m_okToConnect) {
    m_log <<MSG::DEBUG << "Delaying connection of Input Files until Initialize"
	  << ". now in " << FSMState()
	  << endmsg;
    m_delayConnect = true;
  } else {

  StatusCode sc = StatusCode::SUCCESS;

  typedef std::vector<std::string> Strings_t;
  for ( Strings_t::const_iterator
          itr  = m_outputfile.value().begin(),
          iEnd = m_outputfile.value().end();
        itr != iEnd;
        ++itr ) {
    if ( m_alreadyConnectedOutFiles.end() ==
         m_alreadyConnectedOutFiles.find( *itr ) ) {
      if ( connect(*itr).isFailure() ) {
        sc = StatusCode::FAILURE;
      } else {
        m_alreadyConnectedOutFiles.insert( *itr );
      }
    }
  }

  if ( !sc.isSuccess() ) {
    throw GaudiException( "Problem connecting outputfile !!", name(),
                          StatusCode::FAILURE );
  }
  return;
  }
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
THistSvc::updateFiles() {

  // If TTrees grow beyond TTree::fgMaxTreeSize, a new file is
  // automatically created by root, and the old one closed. We
  // need to migrate all the UIDs over to show the correct file
  // pointer. This is ugly.

  if (m_log.level() <= MSG::DEBUG)
    m_log << MSG::DEBUG << "updateFiles()" << endmsg;


  uidMap::iterator uitr, uitr2;
  for (uitr=m_uids.begin(); uitr != m_uids.end(); ++uitr) {
#ifndef NDEBUG
    if (m_log.level() <= MSG::VERBOSE)
      m_log << MSG::VERBOSE << " update: " << uitr->first << " "
	    << uitr->second.id << " " << uitr->second.mode << endmsg;
#endif
    TObject* to = uitr->second.obj;
    TFile* oldFile = uitr->second.file;
    if (!to) {
      m_log << MSG::WARNING << uitr->first << ": TObject == 0" << endmsg;
    } else if ( uitr->second.temp || uitr->second.mode == READ ) {
      // do nothing - no need to check how big the file is since we
      // are just reading it.
#ifndef NDEBUG
    if (m_log.level() <= MSG::VERBOSE)
      m_log << MSG::VERBOSE << "     skipping" << endmsg;
#endif

    } else if (to->IsA()->InheritsFrom("TTree")) {
      TTree* tr = dynamic_cast<TTree*>(to);
      TFile* newFile = tr->GetCurrentFile();

      if (oldFile != newFile) {
        std::string newFileName = newFile->GetName();
        std::string oldFileName(""), streamName, rem;
        TFile* dummy;
        findStream(uitr->second.id, streamName, rem, dummy);

        map<string, pair<TFile*,Mode> >::iterator itr;
        for (itr=m_files.begin(); itr!= m_files.end(); ++itr) {
          if (itr->second.first == oldFile) {
            itr->second.first = newFile;

          }
        }

        uitr2 = uitr;
        for (; uitr2 != m_uids.end(); ++uitr2) {
          if (uitr2->second.file == oldFile) {
            uitr2->second.file = newFile;
          }
        }

        streamMap::iterator sitr;
        for (sitr = m_fileStreams.begin(); sitr!=m_fileStreams.end(); ++sitr) {
          if (sitr->second == streamName) {
            oldFileName = sitr->first;
            break;
          }
        }


#ifndef NDEBUG
      if (m_log.level() <= MSG::DEBUG)
        m_log << MSG::DEBUG << "migrating uid: " << uitr->second.id
              << "   stream: " << streamName
              << "   oldFile: " << oldFileName
              << "   newFile: " << newFileName
              << endmsg;
#endif


        if (oldFileName != "") {
          while ( (sitr=m_fileStreams.find(oldFileName)) != m_fileStreams.end() ) {

#ifndef NDEBUG
            if (m_log.level() <= MSG::DEBUG)
              m_log << MSG::DEBUG << "changing filename \"" << oldFileName
                    << "\" to \"" << newFileName << "\" for stream \""
                    << sitr->second << "\"" << endmsg;
#endif
            m_fileStreams.erase(sitr);
	    m_fileStreams.insert( make_pair(newFileName,streamName) );
          }


        } else {
          m_log << MSG::ERROR
                << "Problems updating fileStreams with new file name" << endmsg;
        }

      }

    }
  }

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::write() {

  updateFiles();

  map<string, pair<TFile*,Mode> >::const_iterator itr;
  for (itr=m_files.begin(); itr!= m_files.end(); ++itr) {
    if (itr->second.second == WRITE || itr->second.second == UPDATE
        ||itr->second.second==SHARE) {
      itr->second.first->Write("",TObject::kOverwrite);
    } else if (itr->second.second == APPEND) {
      itr->second.first->Write("");
    }
  }


  if (m_log.level() <= MSG::DEBUG)
    m_log << MSG::DEBUG << "THistSvc::write()::List of Files connected in ROOT "
          << endmsg;

  TSeqCollection *filelist=gROOT->GetListOfFiles();
  for (int ii=0; ii<filelist->GetEntries(); ii++) {
    if (m_log.level() <= MSG::DEBUG)
      m_log << MSG::DEBUG
            << "THistSvc::write()::List of Files connected in ROOT: \""
            << filelist->At(ii)->GetName()<<"\""<<endmsg;
  }

  return StatusCode::SUCCESS;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::connect(const std::string& ident) {

  Tokenizer tok(true);

  string::size_type loc = ident.find(" ");
  string stream = ident.substr(0,loc);
  char typ(0);
  typedef std::pair<std::string,std::string>      Prop;
  std::vector<Prop> props;
  string val,VAL,TAG,filename,db_typ("ROOT");
  int cl(1);

  tok.analyse(ident.substr(loc+1,ident.length()), " ", "", "", "=", "'", "'");

  for ( Tokenizer::Items::iterator i = tok.items().begin(); i != tok.items().end(); i++)    {
    const std::string& tag = (*i).tag();
    TAG = tag;
    toupper(TAG);

    val = (*i).value();
    VAL = val;
    toupper(VAL);

    if (TAG == "FILE" || TAG == "DATAFILE") {
      filename = val;
      removeDoubleSlash( filename );
    } else if ( TAG == "OPT" ) {
      if ( VAL == "APPEND" || VAL == "UPDATE" ) {
        typ = 'A';
      } else if ( VAL == "CREATE" || VAL == "NEW" || VAL == "WRITE" ) {
        typ = 'N';
      } else if ( VAL == "RECREATE" ) {
        typ = 'R';
      } else if (VAL == "SHARE") {
        typ = 'S';
      } else if ( VAL == "OLD" || VAL == "READ" ) {
        typ = 'O';
      } else {
        m_log << MSG::ERROR << "Unknown OPT: \"" << (*i).value() << "\""
            << endmsg;
        typ = 0;
      }
    } else if (TAG == "TYP") {
      db_typ = (*i).value();
    } else if (TAG == "CL") {
      cl = atoi(val.c_str());
    } else {
      props.push_back( Prop((*i).tag(), (*i).value()));
    }

  }

  if (stream == "temp") {
    m_log << MSG::ERROR << "in JobOption \"" << ident
          << "\": stream name \"temp\" reserved."
          << endmsg;
    return StatusCode::FAILURE;
  }

  if (db_typ != "ROOT") {
    m_log << MSG::ERROR << "in JobOption \"" << ident
          << "\": technology type \"" << db_typ << "\" not supported."
          << endmsg;
    return StatusCode::FAILURE;
  }


  if (m_files.find(stream) != m_files.end()) {
    m_log << MSG::ERROR << "in JobOption \"" << ident
          << "\":\n stream \"" << stream << "\" already connected to file: \""
          << m_files[stream].first->GetName() << "\""
          << endmsg;
    return StatusCode::FAILURE;
  }

  Mode newMode;
  if (typ == 'O') {
    newMode = THistSvc::READ;
  } else if (typ == 'N') {
    newMode = THistSvc::WRITE;
  } else if (typ == 'A') {
    newMode = THistSvc::APPEND;
  } else if (typ == 'R') {
    newMode = THistSvc::UPDATE;
  } else if (typ == 'S') {
    newMode = THistSvc::SHARE;
  } else {
    // something else?
    m_log << MSG::ERROR << "No OPT= specified or unknown access mode in: "
          << ident << endmsg;
    return StatusCode::FAILURE;
  }

  // Is this file already connected to another stream?
  if (m_fileStreams.find(filename) != m_fileStreams.end()) {
    std::pair<streamMap::iterator, streamMap::iterator> fitr =
      m_fileStreams.equal_range(filename);

    std::string oldstream = (fitr.first)->second;

    std::pair<TFile*,Mode> f_info = m_files[oldstream];

    if (newMode != f_info.second) {
      m_log << MSG::ERROR << "in JobOption \"" << ident
            << "\":\n file \"" << filename << "\" already opened by stream: \""
            << oldstream << "\" with different access mode."
            << endmsg;
      return StatusCode::FAILURE;
    } else {
      TFile *f2 = f_info.first;
      m_files[stream] = make_pair(f2,newMode);
      if (m_log.level() <= MSG::DEBUG)
        m_log << MSG::DEBUG << "Connecting stream: \"" << stream
              << "\" to previously opened TFile: \"" << filename << "\""
              << endmsg;
      return StatusCode::SUCCESS;
    }
  }


  IIncidentSvc *pi(0);
  if (service("IncidentSvc",pi).isFailure()) {
    m_log << MSG::ERROR << "Unable to get the IncidentSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  void* vf(0);
  TFile *f(0);

  if (newMode == THistSvc::READ) {
    // old file

    int r = p_fileMgr->open(Io::ROOT,name(), filename,Io::READ,vf,"HIST");

    if (r != 0) {
      m_log << "Unable to open ROOT file " << filename << " for reading"
	    << endmsg;
      return StatusCode::FAILURE;
    }


    f = (TFile*) vf;

    // FIX ME!
    pi->fireIncident(FileIncident(name(), "BeginHistFile",
                                   filename));


  } else if (newMode == THistSvc::WRITE) {
    // new file. error if file exists

    int r = p_fileMgr->open(Io::ROOT,name(),filename, (Io::WRITE|Io::CREATE|Io::EXCL),
			    vf,"HIST");

    if (r != 0) {
      m_log << "Unable to open ROOT file " << filename << " for writing"
	    << endmsg;
      return StatusCode::FAILURE;
    }

    f = (TFile*)vf;

  } else if (newMode == THistSvc::APPEND) {
    // update file

    int r = p_fileMgr->open(Io::ROOT,name(),filename, (Io::WRITE | Io::APPEND),
			    vf,"HIST");
    if (r != 0) {
      m_log << MSG::ERROR << "unable to open file \"" << filename
            << "\" for appending" << endmsg;
      return StatusCode::FAILURE;
    }

    f = (TFile*) vf;


  } else if (newMode == THistSvc::SHARE) {
    // SHARE file type
    //For SHARE files, all data will be stored in a temp file and will be merged into the target file
    //in write() when finalize(), this help to solve some confliction. e.g. with storegate

    static int ishared = 0;
    stringstream out;
    string realfilename=filename;
    out << ishared++;
    filename = string("tmp_THistSvc_")+out.str()+string(".root");

    if (m_log.level() <= MSG::DEBUG)
      m_log << MSG::DEBUG << "Creating temp file \"" << filename
            << "\" and realfilename="<<realfilename << endmsg;
    m_sharedFiles[stream]=realfilename;


    int r = p_fileMgr->open(Io::ROOT,name(), filename, (Io::WRITE|Io::CREATE|Io::EXCL),
			    vf,"HIST");

    if (r != 0) {
      m_log << "Unable to open ROOT file " << filename << " for writing"
	    << endmsg;
      return StatusCode::FAILURE;
    }

    f = (TFile*)vf;

  } else if (newMode == THistSvc::UPDATE) {
    // update file

    int r = p_fileMgr->open(Io::ROOT,name(), filename, (Io::WRITE|Io::CREATE),
			    vf, "HIST");

    if (r != 0) {
      m_log << "Unable to open ROOT file " << filename << " for appending"
	    << endmsg;
      return StatusCode::FAILURE;
    }

    f = (TFile*)vf;

  }

  m_files[stream] = make_pair(f,newMode);
  m_fileStreams.insert(make_pair(filename,stream));

  if (m_log.level() <= MSG::DEBUG)
    m_log << MSG::DEBUG << "Opening TFile \"" << filename << "\"  stream: \""
          << stream << "\"  mode: \"" << typ << "\"" << " comp level: " << cl
          << endmsg;

  return StatusCode::SUCCESS;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

TDirectory*
THistSvc::changeDir(const THistSvc::THistID& hid) const {

  string uid = hid.id;
  TFile* file = hid.file;
  string stream, fdir, bdir, dir, id;

  if (file != 0) {
    file->cd("/");
  } else {
    gROOT->cd();
  }

  fdir = uid;
  bdir = dirname(fdir);

  while ( (dir = dirname(fdir)) != "") {
    if (! gDirectory->GetKey(dir.c_str())) {
      gDirectory->mkdir(dir.c_str());
    }
    gDirectory->cd(dir.c_str());
  }

  return gDirectory;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

std::string
THistSvc::dirname(std::string& dir) const {


  string::size_type i = dir.find("/");

  if (i == string::npos) {
    return "";
  }

  if ( i == 0 ) {
    dir.erase(0,1);
    return dirname(dir);
  }

  string root = dir.substr(0,i);
  dir.erase(0,i);

  return root;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

THistSvc::GlobalDirectoryRestore::GlobalDirectoryRestore() {
  m_gd = gDirectory;
  m_gf = gFile;
  m_ge = gErrorIgnoreLevel;
}

THistSvc::GlobalDirectoryRestore::~GlobalDirectoryRestore() {
  gDirectory = m_gd;
  gFile = m_gf;
  gErrorIgnoreLevel = m_ge;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
THistSvc::removeDoubleSlash(std::string& id) const {

  while (id.find("//") != std::string::npos) {
    id.replace(id.find("//"),2,"/");
  }

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void THistSvc::MergeRootFile(TDirectory *target, TDirectory *source) {

  if (m_log.level() <= MSG::DEBUG)
    m_log <<MSG::DEBUG << "Target path: " << target->GetPath() << endmsg;
  TString path( (char*)strstr(target->GetPath(), ":") );
  path.Remove( 0, 2);

  source->cd(path);
  TDirectory *current_sourcedir = gDirectory;

  // loop over all keys in this directory
  TList *lkeys=current_sourcedir->GetListOfKeys();
  int nkeys=lkeys->GetEntries();
  TKey *key;
  for (int jj=0; jj<nkeys; jj++) {
    key=(TKey*) lkeys->At(jj);
    string pathnameinsource=current_sourcedir->GetPath()+string("/")+key->GetName();
    if (m_log.level() <= MSG::DEBUG)
      m_log <<MSG::DEBUG << "Reading Key:" << pathnameinsource << endmsg;
    //key->Dump();
    //TObject *obj=key->ReadObj();
    TObject *obj=source->Get(pathnameinsource.c_str());

    if (obj) {
    if (obj->IsA()->InheritsFrom("TDirectory") ) {
      // it's a subdirectory

      if (m_log.level() <= MSG::DEBUG)
        m_log <<MSG::DEBUG << "Found subdirectory " << obj->GetName()
              << endmsg;

      // create a new subdir of same name and title in the target file
      target->cd();
      TDirectory *newtargetdir =
        target->mkdir(obj->GetName(), obj->GetTitle() );

      MergeRootFile(newtargetdir, source);

    } else if (obj->IsA()->InheritsFrom("TTree")) {
      if (m_log.level() <= MSG::DEBUG)
        m_log <<MSG::DEBUG << "Found TTree " << obj->GetName() << endmsg;
      TTree *mytree=dynamic_cast<TTree*>(obj);
      int nentries=(int) mytree->GetEntries();
      mytree->SetBranchStatus("*",1);

      if (m_log.level() <= MSG::DEBUG)
        m_log <<MSG::DEBUG << "Dumping TTree " << nentries <<" entries"
              << endmsg;
      //mytree->Print();
      //for (int ij=0; ij<nentries; ij++) {
      //m_log <<MSG::DEBUG << "Dumping TTree Show( " << ij <<" )"
      //<< endmsg;
      //mytree->Show(ij);
      //}
      target->cd();
      mytree->CloneTree();

      //m_log <<MSG::DEBUG << "Writing TTree to target file: ( "
      //<< mycopiedtree->Write(key->GetName()) <<" ) bytes written"
      //<< endmsg;

    } else {
      target->cd();
      obj->Write(key->GetName() );
    }
    }

  } // while ( ( TKey *key = (TKey*)nextkey() ) )

  // save modifications to target file

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

bool
THistSvc::exists( const std::string& name ) const {

  TH1* h;

  return getHist_i(name,h,true).isSuccess();


}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
THistSvc::handle( const Incident& /* inc */ ) {

  if (signaledStop) return ;

  if (m_maxFileSize.value() == -1) {
    return;
  }

  // convert to bytes.
  Long64_t mfs = (Long64_t)m_maxFileSize.value() * (Long64_t)1048576;
  Long64_t mfs_warn = mfs * 95 / 100;

  updateFiles();

  map<string, pair<TFile*,Mode> >::const_iterator itr;
  for (itr=m_files.begin(); itr!= m_files.end(); ++itr) {
    TFile* tf = itr->second.first;

#ifndef NDEBUG
    if (m_log.level() <= MSG::DEBUG)
      m_log << MSG::DEBUG << "stream: " << itr->first << "  name: "
            << tf->GetName() << "  size: " << tf->GetSize()
            << endmsg;
#endif

    // Signal job to terminate if output file is too large
    if (tf->GetSize() > mfs) {

      signaledStop = true;

      m_log << MSG::FATAL << "file \"" << tf->GetName()
            << "\" associated with stream \"" << itr->first
            << "\" has exceeded the max file size of "
            << m_maxFileSize.value() << "MB. Terminating Job."
            << endmsg;

      IEventProcessor* evt(0);
      if (service("ApplicationMgr", evt, true).isSuccess()) {
        evt->stopRun();
        evt->release();
      } else {
        abort();
      }
    } else if (tf->GetSize() > mfs_warn) {
      m_log << MSG::WARNING << "file \"" << tf->GetName()
            << "\" associated with stream \"" << itr->first
            << "\" is at 95% of its maximum allowable file size of "
            << m_maxFileSize.value() << "MB"
            << endmsg;
    }
  }
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

/** helper function to recursively copy the layout of a TFile into a new TFile
 */
void
THistSvc::copyFileLayout (TDirectory *dst, TDirectory *src) {

  if (m_log.level() <= MSG::DEBUG)
    m_log << MSG::DEBUG
	  << "copyFileLayout() to dst path: " << dst->GetPath () << endmsg;

    // strip out URLs
    TString path ((char*)strstr (dst->GetPath(), ":"));
    path.Remove (0, 2);

    src->cd (path);
    TDirectory *cur_src_dir = gDirectory;

    // loop over all keys in this directory
    TList *key_list = cur_src_dir->GetListOfKeys ();
    int n = key_list->GetEntries ();
    for ( int j = 0; j < n; ++j ) {
      TKey *k = (TKey*)key_list->At (j);
      const std::string src_pathname = cur_src_dir->GetPath()
                                     + std::string("/")
                                     + k->GetName();
      TObject *o=src->Get (src_pathname.c_str());

    if (o != NULL && o->IsA()->InheritsFrom ("TDirectory")) {
      if (m_log.level() <= MSG::VERBOSE)
	m_log << MSG::VERBOSE << " subdir [" << o->GetName() << "]..."
	      << endmsg;
        dst->cd ();
        TDirectory * dst_dir = dst->mkdir (o->GetName(), o->GetTitle());
      copyFileLayout (dst_dir, src);
      }
    } // loop over keys
    return;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//
/** @brief callback method to reinitialize the internal state of
 *         the component for I/O purposes (e.g. upon @c fork(2))
 */
StatusCode
THistSvc::io_reinit ()
{
  bool all_good = true;
  if (m_log.level() <= MSG::DEBUG)
    m_log << MSG::DEBUG << "reinitializing I/O..." << endmsg;

  // retrieve the I/O component manager...

  IIoComponentMgr* iomgr(0);

  if (service("IoComponentMgr", iomgr, true).isFailure()) {
    m_log << MSG::ERROR << "could not retrieve I/O component manager !"
          << endmsg;
    return StatusCode::FAILURE;
  }

  GlobalDirectoryRestore restore;
  // to hide the expected errors upon closing the files whose
  // file descriptors have been swept under the rug...
  gErrorIgnoreLevel = kFatal;

  typedef std::map<std::string, std::pair<TFile*,Mode> > FileReg_t;

  for (FileReg_t::iterator ifile = m_files.begin(), iend=m_files.end();
       ifile != iend; ++ifile) {
    TFile *f = ifile->second.first;
    std::string fname = f->GetName();
    if (m_log.level() <= MSG::DEBUG)
      m_log << MSG::DEBUG << "file [" << fname << "] mode: ["
          << f->GetOption() << "] r:"
          << f->GetFileBytesRead()
          << " w:" << f->GetFileBytesWritten()
          << " cnt:" << f->GetFileCounter()
          << endmsg;

    if ( ifile->second.second == READ ) {
      if (m_log.level() <= MSG::DEBUG)
	m_log << MSG::DEBUG
            << "  TFile opened in READ mode: not reassigning names" << endmsg;
      continue;
    }

    if ( !iomgr->io_retrieve (this, fname).isSuccess () ) {
      m_log << MSG::ERROR << "could not retrieve new name for [" << fname
            << "] !!" << endmsg;
      all_good = false;
      continue;
    } else {
      if (m_log.level() <= MSG::DEBUG)
	m_log << MSG::DEBUG << "got a new name [" << fname << "]..." << endmsg;
    }
    // create a new TFile
    // TFile *newfile = TFile::Open (fname.c_str(), f->GetOption());

    void* vf;
    Option_t *opts = f->GetOption();
    int r = p_fileMgr->open(Io::ROOT,name(),fname,Io::WRITE,vf,"HIST");
    if (r != 0) {
      m_log << MSG::ERROR << "unable to open file \"" << fname
	    << "\" for writing" << endmsg;
      return StatusCode::FAILURE;
    }
    TFile *newfile = (TFile*) vf;
    newfile->SetOption(opts);


    if (ifile->second.second != THistSvc::READ) {
      copyFileLayout (newfile, f);
      ifile->second.first = newfile;
    }

    // loop over all uids and migrate them to the new file
    // XXX FIXME: this double loop sucks...
    for ( uidMap::iterator uid = m_uids.begin(), uid_end = m_uids.end();
          uid != uid_end;
          ++uid ) {
      THistID& hid = uid->second;
      if ( hid.file != f ) {
        continue;
      }
      TDirectory *olddir = this->changeDir (hid);
      hid.file = newfile;
      // side-effect: create needed directories...
      TDirectory *newdir = this->changeDir (hid);
      TClass *cl = hid.obj->IsA();

      // migrate the objects to the new file.
      // thanks to the object model of ROOT, it is super easy.
      if (cl->InheritsFrom ("TTree")) {
        dynamic_cast<TTree*> (hid.obj)->SetDirectory (newdir);
        dynamic_cast<TTree*> (hid.obj)->Reset();
      }
      else if (cl->InheritsFrom ("TH1")) {
        dynamic_cast<TH1*> (hid.obj)->SetDirectory (newdir);
        dynamic_cast<TH1*> (hid.obj)->Reset();
      }
      else if (cl->InheritsFrom ("TGraph")) {
        olddir->Remove (hid.obj);
        newdir->Append (hid.obj);
      } else {
        m_log << MSG::ERROR
              << "id: \"" << hid.id << "\" is not a inheriting from a class "
              << "we know how to handle (received [" << cl->GetName()
              << "], " << "expected [TTree, TH1 or TGraph]) !"
              << endmsg
              << "attaching to current dir [" << newdir->GetPath() << "] "
              << "nonetheless..." << endmsg;
        olddir->Remove (hid.obj);
        newdir->Append (hid.obj);
      }
    }
    f->ReOpen ("READ");
    p_fileMgr->close(f,name());
    f = newfile;
  }

  return all_good ? StatusCode::SUCCESS : StatusCode::FAILURE;
}


//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::rootOpenAction( const Io::FileAttr* fa, const std::string& caller) {

  if (fa->tech() != Io::ROOT) {
    // This should never happen
    return StatusCode::SUCCESS;
  }

  if (fa->desc() != "HIST") {
    return StatusCode::SUCCESS;
  }

  p_incSvc->fireIncident(FileIncident(caller, "OpenHistFile", fa->name()));

  if ( fa->flags().isRead() ) {
    p_incSvc->fireIncident(FileIncident(caller, "BeginHistFile", fa->name()));
  } else if ( fa->flags().isWrite() ) {
    p_incSvc->fireIncident(FileIncident(caller, IncidentType::BeginOutputFile,
					fa->name()));
  } else {
    // for Io::RW
    p_incSvc->fireIncident(FileIncident(caller, IncidentType::BeginOutputFile,
					fa->name()));
  }

  return StatusCode::SUCCESS;


}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::rootOpenErrAction( const Io::FileAttr* fa, const std::string& caller) {

  if (fa->tech() != Io::ROOT) {
    // This should never happen
    return StatusCode::SUCCESS;
  }

  if (fa->desc() != "HIST") {
    return StatusCode::SUCCESS;
  }

  if ( fa->flags().isRead() ) {
    p_incSvc->fireIncident(FileIncident(caller, IncidentType::FailInputFile,
					fa->name()));
  } else if ( fa->flags().isWrite() ) {
    p_incSvc->fireIncident(FileIncident(caller, IncidentType::FailOutputFile,
					fa->name()));
  } else {
    // for Io::RW
    p_incSvc->fireIncident(FileIncident(caller, "FailRWFile", fa->name()));
  }


  return StatusCode::SUCCESS;


}
