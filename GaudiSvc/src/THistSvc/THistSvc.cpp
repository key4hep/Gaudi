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
#include "GaudiKernel/IEventProcessor.h"

#include "TROOT.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TKey.h"
#include "TError.h"
#include "TGraph.h"

#include <sstream>
#include <streambuf>
#include <cstdio>
#include <algorithm>

using namespace std;


DECLARE_SERVICE_FACTORY(THistSvc)

inline void toupper(std::string &s)
{
  std::transform(s.begin(), s.end(), s.begin(), 
                 (int(*)(int)) toupper);
}


//*************************************************************************//

THistSvc::THistSvc( const std::string& name, ISvcLocator* svc )
  : base_class(name, svc), m_log(msgSvc(), name ), signaledStop(false) {

  declareProperty ("Output", m_outputfile );
  declareProperty ("Input", m_inputfile );
  declareProperty ("AutoSave", m_autoSave=0 );
  declareProperty ("PrintAll", m_print=false);
  declareProperty ("CompressionLevel", m_compressionLevel=1 );
  declareProperty ("MaxFileSize", m_maxFileSize=5120, 
		   "maximum file size in MB. if exceeded, will cause an abort");

  m_inputfile.declareUpdateHandler ( &THistSvc::setupInputFile,  this );
  m_outputfile.declareUpdateHandler( &THistSvc::setupOutputFile, this );
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

THistSvc::~THistSvc() {

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::initialize() {
  GlobalDirectoryRestore restore;

  m_alreadyConnectedOutFiles.clear();
  m_alreadyConnectedInFiles.clear();

  StatusCode status = Service::initialize();
  m_log.setLevel( m_outputLevel.value() );

  if (status.isFailure()) {
    MsgStream log ( msgSvc(), name() );
    log << MSG::ERROR << "initializing service" << endmsg;
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
    m_log << MSG::VERBOSE << "ROOT already initialized, debug = "
	  << gDebug<< endmsg;
  }

  IIncidentSvc* p_incSvc(0);
  
  if (service("IncidentSvc", p_incSvc, true).isFailure()) {
    m_log << MSG::ERROR << "unable to get the IncidentSvc" << endmsg;
    st = StatusCode::FAILURE;
  } else {
    p_incSvc->addListener( this, "EndEvent", 100, true);
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

  m_log << MSG::DEBUG << "THistSvc::finalize" << endmsg;

  uidMap::const_iterator uitr;
  for (uitr=m_uids.begin(); uitr != m_uids.end(); ++uitr) {

    TObject* to = uitr->second.obj;

    string dirname("none");
    if (to->IsA()->InheritsFrom("TTree")) {
      TTree* tr = dynamic_cast<TTree*>(to);
      if (tr->GetDirectory() != 0) {
        dirname = tr->GetDirectory()->GetPath();
      }
    } else if (to->IsA()->InheritsFrom("TGraph")) {
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
    } else if (to->IsA()->InheritsFrom("TH1")) {
      TH1* th = dynamic_cast<TH1*>(to);
      if (th == 0) {
        m_log << MSG::ERROR << "Couldn't dcast: " << uitr->first << endmsg;
      } else {
        if (th->GetDirectory() != 0) {
          dirname = th->GetDirectory()->GetPath();
        }
      }
    }

    m_log << MSG::DEBUG << "uid: \"" << uitr->first << "\"  temp: "
        << uitr->second.temp << "  dir: " << dirname
        << endmsg;


//     if (uitr->second.temp == true) {
//       log << MSG::INFO << "Deleting \"" << uitr->first << "\"" << endmsg;
//       delete uitr->second.obj;
//     }
  }

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

      m_log << MSG::DEBUG << "finalizing stream/file " << itr->first << ":"
	    << itr->second.first->GetName()
	    << endmsg;
    } else {
      m_log << MSG::DEBUG << "already finalized stream " << itr->first << endmsg;
      continue;
    }


    if (m_print && m_log.level() <= MSG::INFO) {

      m_log << MSG::INFO;
      m_log << "==> File: " << itr->second.first->GetName()
	    << "  stream: " << itr->first << endmsg;

      itr->second.first->Print("base");
    }

    string tmpfn=itr->second.first->GetName();

    itr->second.first->Close();

    if (itr->second.second==SHARE) {
      TFile *outputfile;
      //Merge File
      try {
	m_log << MSG::DEBUG << "Opening Final Output File: " <<m_sharedFiles[itr->first].c_str()<<endmsg;
	outputfile = new TFile(m_sharedFiles[itr->first].c_str(), "UPDATE");
      } catch (const std::exception& Exception) {
      m_log << MSG::ERROR << "exception caught while trying to open root"
	    << " file for appending: " << Exception.what() << std::endl
	    << "  -> file probably corrupt." << endmsg;
      return StatusCode::FAILURE;
      } catch (...) {
	m_log << MSG::ERROR << "Problems opening output file  \"" << m_sharedFiles[itr->first]
	    << "\" for append: probably corrupt" << endmsg;
	return StatusCode::FAILURE;
      }

      m_log << MSG::DEBUG << "THistSvc::write()::Merging Rootfile "<<endmsg;
      TFile *inputfile;
      try {
	m_log << MSG::DEBUG << "Openning again Temporary File: " <<tmpfn.c_str()<<endmsg;
	inputfile=new TFile(tmpfn.c_str(),"READ");
      } catch (const std::exception& Exception) {
	m_log << MSG::ERROR << "exception caught while trying to open root"
	      << " file for appending: " << Exception.what() << std::endl
	      << "  -> file probably corrupt." << endmsg;
	return StatusCode::FAILURE;
      } catch (...) {
	m_log << MSG::ERROR << "Problems opening output file  \"" 
	      << tmpfn.c_str()
	      << "\" for append: probably corrupt" << endmsg;
	return StatusCode::FAILURE;
      }

      MergeRootFile(outputfile, inputfile);

      outputfile->SetCompressionLevel( m_compressionLevel );

      outputfile->Write();
      outputfile->Close();
      inputfile->Close();

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
THistSvc::getTHists(TDirectory *td, TList & tl) const {

  GlobalDirectoryRestore restore;

  gErrorIgnoreLevel = kBreak;

  if (!td->cd()) {
    m_log << MSG::ERROR << "No such TDirectory \"" << td->GetPath() << "\""
	  << endmsg;
    return StatusCode::FAILURE;
  }

  TIter nextkey(td->GetList());
  while (TObject *key = (TObject*)nextkey()) {
    if (key != 0) {
      if (key->IsA()->InheritsFrom("TH1")) { tl.Add(key); }
    }
  }


  return StatusCode::SUCCESS;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getTHists(const std::string& dir, TList & tl) const {

  GlobalDirectoryRestore restore;

  gErrorIgnoreLevel = kBreak;

  StatusCode sc;

  if (!gDirectory->cd(dir.c_str())) {
    m_log << MSG::ERROR << "No such TDirectory \"" << dir << "\""
	  << endmsg;
    sc = StatusCode::FAILURE;
  } else {
    sc = getTHists(gDirectory,tl);
  }

  return sc;

}
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getTTrees(TDirectory *td, TList & tl) const {

  GlobalDirectoryRestore restore;

  gErrorIgnoreLevel = kBreak;

  if (!td->cd()) {
    m_log << MSG::ERROR << "No such TDirectory \"" << td->GetPath() << "\""
	  << endmsg;
    return StatusCode::FAILURE;
  }

  TIter nextkey(td->GetList());
  while (TObject *key = (TObject*)nextkey()) {
    if (key != 0) {
      if (key->IsA()->InheritsFrom("TTree")) { tl.Add(key); }
    }
  }

  return StatusCode::SUCCESS;

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getTTrees(const std::string& dir, TList & tl) const {

  GlobalDirectoryRestore restore;

  gErrorIgnoreLevel = kBreak;

  StatusCode sc;

  if (!gDirectory->cd(dir.c_str())) {
    m_log << MSG::ERROR << "No such TDirectory \"" << dir << "\""
	  << endmsg;
    sc = StatusCode::FAILURE;
  } else {
    sc = getTTrees(gDirectory,tl);
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
  if (hist != 0 && m_autoSave != 0 && sc.isSuccess()) {
    hist->SetAutoSave(m_autoSave);
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
THistSvc::setupInputFile( Property& /*m_inputfile*/ )
{
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
  return;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
THistSvc::setupOutputFile( Property& /*m_outputfile*/ )
{
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

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
THistSvc::updateFiles() {

  // If TTrees grow beyond TTree::fgMaxFileSize, a new file is
  // automatically created by root, and the old one closed. We
  // need to migrate all the UIDs over to show the correct file
  // pointer. This is ugly.

  uidMap::iterator uitr, uitr2;
  for (uitr=m_uids.begin(); uitr != m_uids.end(); ++uitr) {
    TObject* to = uitr->second.obj;
    TFile* oldFile = uitr->second.file;
    if (to->IsA()->InheritsFrom("TTree")) {
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
	m_log << MSG::DEBUG << "migrating uid: " << uitr->second.id
	      << "   stream: " << streamName 
	      << "   oldFile: " << oldFileName
	      << "   newFile: " << newFileName
	      << endmsg;
#endif


	if (oldFileName != "") {
	  while ( (sitr=m_fileStreams.find(oldFileName)) != m_fileStreams.end() ) {

#ifndef NDEBUG
	    m_log << MSG::DEBUG << "changing filename \"" << oldFileName
		  << "\" to \"" << newFileName << "\" for stream \""
		  << sitr->second << "\"" << endmsg;
#endif
	    m_fileStreams.erase(sitr);
	    m_fileStreams.insert( make_pair<std::string,std::string>(newFileName,streamName) );
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


  m_log << MSG::DEBUG << "THistSvc::write()::List of Files connected in ROOT "
	<< endmsg;

  TSeqCollection *filelist=gROOT->GetListOfFiles();
  for (int ii=0; ii<filelist->GetEntries(); ii++) {
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
      m_files[stream] = make_pair<TFile*,Mode>(f2,newMode);
      m_log << MSG::DEBUG << "Connecting stream: \"" << stream
	    << "\" to previously opened TFile: \"" << filename << "\""
	    << endmsg;
      return StatusCode::SUCCESS;
    }
  }


  TFile *f(0) ;
  if (newMode == THistSvc::READ) {
    // old file

    try {
      f = TFile::Open(filename.c_str(),"READ");
    } catch (const std::exception& Exception) {
      m_log << MSG::ERROR << "exception caught while trying to open root"
	    << " file for reading: " << Exception.what() << std::endl
	    << "  -> file probably corrupt." << endmsg;
      return StatusCode::FAILURE;
    } catch (...) {
      m_log << MSG::ERROR << "Problems opening input file  \"" << filename
	    << "\": probably corrupt" << endmsg;
      return StatusCode::FAILURE;
    }

    if (!f->IsOpen()) {
      m_log << MSG::ERROR << "Unable to open input file \"" << filename
	    << "\": file does not exist" << endmsg;
      return StatusCode::FAILURE;
    }


  } else if (newMode == THistSvc::WRITE) {
    // new file

    f = TFile::Open(filename.c_str(),"NEW");
    if (!f->IsOpen()) {
      m_log << MSG::ERROR << "Unable to create new output file \"" << filename
	    << "\" for writing: file already exists" << endmsg;
      return StatusCode::FAILURE;
    }
    f->SetCompressionLevel( m_compressionLevel );

  } else if (newMode == THistSvc::APPEND) {
    // update file

    try {
      f =  TFile::Open(filename.c_str(),"UPDATE");
    } catch (const std::exception& Exception) {
      m_log << MSG::ERROR << "exception caught while trying to open root"
	    << " file for appending: " << Exception.what() << std::endl
	    << "  -> file probably corrupt." << endmsg;
      return StatusCode::FAILURE;
    } catch (...) {
      m_log << MSG::ERROR << "Problems opening output file  \"" << filename
	    << "\" for append: probably corrupt" << endmsg;
      return StatusCode::FAILURE;
    }

    if (!f->IsOpen()) {
      m_log << MSG::ERROR << "Unable to open output file \"" << filename
	    << "\" for appending" << endmsg;
      return StatusCode::FAILURE;
    }
    f->SetCompressionLevel( m_compressionLevel );

  } else if (newMode == THistSvc::SHARE) {
    // SHARE file type
    //For SHARE files, all data will be stored in a temp file and will be merged into the target file
    //in write() when finalize(), this help to solve some confliction. e.g. with storegate
    
    static int ishared = 0;
    stringstream out;
    string realfilename=filename;
    out << ishared++;
    filename = string("tmp_THistSvc_")+out.str()+string(".root");
    
    m_log << MSG::DEBUG << "Creating temp file \"" << filename
	  << "\" and realfilename="<<realfilename << endmsg;
    m_sharedFiles[stream]=realfilename;
    
    try {
      f = TFile::Open(filename.c_str(),"NEW");
    } catch (const std::exception& Exception) {
      m_log << MSG::ERROR << "exception caught while trying to open root"
	    << " file for appending: " << Exception.what() << std::endl
	    << "  -> file probably corrupt." << endmsg;
      return StatusCode::FAILURE;
    } catch (...) {
      m_log << MSG::ERROR << "Problems opening output file  \"" << filename
	    << "\" for append: probably corrupt" << endmsg;
      return StatusCode::FAILURE;
    }
    
    if (!f->IsOpen()) {
      m_log << MSG::ERROR << "Unable to open output file \"" << filename
	  << "\" for appending" << endmsg;
      return StatusCode::FAILURE;
    }
    f->SetCompressionLevel( m_compressionLevel );

  } else if (newMode == THistSvc::UPDATE) {
    // update file

    try {
      f =  TFile::Open(filename.c_str(),"RECREATE");
    } catch (const std::exception& Exception) {
      m_log << MSG::ERROR << "exception caught while trying to open root"
	    << " file for updating: " << Exception.what() << std::endl
	    << "  -> file probably corrupt." << endmsg;
      return StatusCode::FAILURE;
    } catch (...) {
      m_log << MSG::ERROR << "Problems opening output file  \"" << filename
	    << "\" for update: probably corrupt" << endmsg;
      return StatusCode::FAILURE;
    }

    if (!f->IsOpen()) {
      m_log << MSG::ERROR << "Unable to open output file \"" << filename
	    << "\" for updating" << endmsg;
      return StatusCode::FAILURE;
    }
    f->SetCompressionLevel( m_compressionLevel );

  }

  m_files[stream] = make_pair<TFile*,Mode>(f,newMode);
  m_fileStreams.insert(make_pair<std::string,std::string>(filename,stream));

  m_log << MSG::DEBUG << "Opening TFile \"" << filename << "\"  stream: \""
	<< stream << "\"  mode: \"" << typ << "\""
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
    m_log <<MSG::DEBUG << "Reading Key:" << pathnameinsource << endmsg;
    //key->Dump();
    //TObject *obj=key->ReadObj();
    TObject *obj=source->Get(pathnameinsource.c_str());

    if (obj->IsA()->InheritsFrom("TDirectory") ) {
      // it's a subdirectory

      m_log <<MSG::DEBUG << "Found subdirectory " << obj->GetName()
	    << endmsg;

      // create a new subdir of same name and title in the target file
      target->cd();
      TDirectory *newtargetdir =
	target->mkdir(obj->GetName(), obj->GetTitle() );

      MergeRootFile(newtargetdir, source);

    } else if (obj->IsA()->InheritsFrom("TTree")) {
      m_log <<MSG::DEBUG << "Found TTree " << obj->GetName() << endmsg;
      TTree *mytree=dynamic_cast<TTree*>(obj);
      int nentries=(int) mytree->GetEntries();
      mytree->SetBranchStatus("*",1);

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

    } else if (obj) {
      target->cd();
      obj->Write(key->GetName() );
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

  // convert to bytes.
  Long64_t mfs = (Long64_t)m_maxFileSize.value() * (Long64_t)1048576;

  updateFiles();

  map<string, pair<TFile*,Mode> >::const_iterator itr;
  for (itr=m_files.begin(); itr!= m_files.end(); ++itr) {
    TFile* tf = itr->second.first;
  
#ifndef NDEBUG
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
    }
  }
}
