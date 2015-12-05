#ifndef GAUDISVC_THISTSVC_H
#define GAUDISVC_THISTSVC_H

#include "GaudiKernel/Service.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIoComponent.h"
#include "GaudiKernel/MsgStream.h"

#include "TObject.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TTree.h"
#include "TGraph.h"
#include "TList.h"


#include <vector>
#include <string>
#include <set>
#include <map>

class IIncidentSvc;
class THistSvc: public extends3<Service, ITHistSvc, IIncidentListener,
				IIoComponent> {

public:

  StatusCode initialize() override;
  StatusCode reinitialize() override;
  StatusCode finalize() override;

  StatusCode regHist(const std::string& name) override;
  StatusCode regHist(const std::string& name, TH1*) override;
  StatusCode regHist(const std::string& name, TH2*) override;
  StatusCode regHist(const std::string& name, TH3*) override;

  StatusCode getHist(const std::string& name, TH1*&) const override;
  StatusCode getHist(const std::string& name, TH2*&) const override;
  StatusCode getHist(const std::string& name, TH3*&) const override;

  StatusCode regTree(const std::string& name) override;
  StatusCode regTree(const std::string& name, TTree*) override;
  StatusCode getTree(const std::string& name, TTree*&) const override;

  StatusCode regGraph(const std::string& name) override;
  StatusCode regGraph(const std::string& name, TGraph*) override;
  StatusCode getGraph(const std::string& name, TGraph*&) const override;

  StatusCode deReg(TObject* obj) override;
  StatusCode deReg(const std::string& name) override;

  std::vector<std::string> getHists() const override;
  std::vector<std::string> getTrees() const override;
  std::vector<std::string> getGraphs() const override;

  StatusCode getTHists(TDirectory *td, TList &,
           bool recurse=false) const override;
  StatusCode getTHists(const std::string& name, TList &,
           bool recurse=false) const override;

  StatusCode getTHists(TDirectory *td, TList &tl,
           bool recurse=false, bool reg=false) override;
  StatusCode getTHists(const std::string& name, TList &tl,
		  bool recurse=false, bool reg=false) override;

  StatusCode getTTrees(TDirectory *td, TList &,
           bool recurse=false) const override;
  StatusCode getTTrees(const std::string& name, TList &,
           bool recurse=false) const override;

  StatusCode getTTrees(TDirectory *td, TList & tl,
           bool recurse=false, bool reg=false) override;
  StatusCode getTTrees(const std::string& name, TList & tl,
           bool recurse=false, bool reg=false) override;

  bool exists(const std::string& name) const override;

  THistSvc(const std::string& name, ISvcLocator *svc );

  void handle(const Incident&) override;

  // From IIoComponent
  StatusCode io_reinit () override;


protected:

  ~THistSvc() override = default;

private:

  class GlobalDirectoryRestore {
  public:
    GlobalDirectoryRestore();
    ~GlobalDirectoryRestore();
  private:
    TDirectory* m_gd;
    TFile* m_gf;
    int m_ge;
  };

  enum Mode {
    READ,
    WRITE,
    UPDATE,
    APPEND,
    SHARE,
    INVALID
  };

  struct THistID {
    std::string id;
    bool        temp;
    TObject*    obj;
    TFile*      file;
    Mode        mode;

    THistID():id(""),temp(true),obj(0),file(0),mode(INVALID) {}
    THistID(const THistID& rhs):id(rhs.id), temp(rhs.temp),
                                obj(rhs.obj), file(rhs.file), mode(rhs.mode) {}
    THistID(std::string& i, bool& t, TObject* o, TFile* f)
      : id(i), temp(t), obj(o), file(f), mode(INVALID){
    }
    THistID(std::string& i, bool& t, TObject* o, TFile* f, Mode m)
      : id(i), temp(t), obj(o), file(f), mode(m){
    }

    bool operator < (THistID const &rhs) const {
      return (obj < rhs.obj);
    }
  };


  template <typename T>
  StatusCode regHist_i(T* hist, const std::string& name);
  template <typename T>
  StatusCode getHist_i(const std::string& name, T*& hist, bool quiet=false) const;
  template <typename T>
  StatusCode readHist_i(const std::string& name, T*& hist) const;


  StatusCode readHist(const std::string& name, TH1*&) const;
  StatusCode readHist(const std::string& name, TH2*&) const;
  StatusCode readHist(const std::string& name, TH3*&) const;
  StatusCode readTree(const std::string& name, TTree*&) const;

  void updateFiles();
  StatusCode write();
  StatusCode connect(const std::string&);
  TDirectory* changeDir(const THistSvc::THistID& hid) const;
  std::string dirname(std::string& dir) const;
  void removeDoubleSlash(std::string&) const;

  bool browseTDir(TDirectory* dir) const;

  bool findStream(const std::string& name, std::string& root,
                  std::string& rem, TFile*& file) const;
  void parseString(const std::string& id, std::string& root, std::string& rem)
    const;

  /// call-back method to handle input stream property
  void setupInputFile( Property& inputfile );

  /// call-back method to handle output stream property
  void setupOutputFile( Property& outputfile );

  void setupCompressionLevel( Property& cmp );

  void copyFileLayout(TDirectory*, TDirectory*);

  void MergeRootFile( TDirectory *target, TDirectory *source); 

  ////////

  mutable MsgStream m_log;

  StringArrayProperty m_inputfile, m_outputfile;
  std::vector<std::string> m_Rstream, m_Wstream;
  IntegerProperty m_autoSave, m_autoFlush, m_compressionLevel, m_maxFileSize;
  BooleanProperty m_print;

  /// list of already connected files. This is to keep track of files
  /// registered by the setupInputFile callback method
  std::set<std::string> m_alreadyConnectedInFiles;

  /// list of already connected files. This is to keep track of files
  /// registered by the setupOutputFile callback method
  std::set<std::string> m_alreadyConnectedOutFiles;


  typedef std::map<std::string, THistID> uidMap;
  typedef std::multimap<std::string, THistID> idMap;
  typedef std::map<TObject*, THistID> objMap;
  typedef std::multimap<std::string, std::string> streamMap;

  uidMap m_uids;
  idMap  m_ids;
  objMap m_tobjs;

  std::map<std::string, std::pair<TFile*,Mode> > m_files; // stream->file
  streamMap m_fileStreams;                                // fileName->streams

  std::map<std::string, std::string > m_sharedFiles; // stream->filename of shared files

  bool signaledStop = false;
  bool m_delayConnect = false, m_okToConnect = false;

  mutable std::string m_curstream;

  IIncidentSvc* p_incSvc = nullptr;
  IFileMgr* p_fileMgr = nullptr;

  StatusCode rootOpenAction( FILEMGR_CALLBACK_ARGS );
  StatusCode rootOpenErrAction( FILEMGR_CALLBACK_ARGS );

};

#ifndef GAUDISVC_THISTSVC_ICC
 #include "THistSvc.icc"
#endif


#endif
