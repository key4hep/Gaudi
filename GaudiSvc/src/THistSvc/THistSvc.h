#ifndef GAUDISVC_THISTSVC_H
#define GAUDISVC_THISTSVC_H

#include "GaudiKernel/Service.h"
#include "GaudiKernel/ITHistSvc.h"
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

// Forward declarations
template <class TYPE> class SvcFactory;

class THistSvc: public extends3<Service, ITHistSvc, IIncidentListener, 
				IIoComponent> {

public:

  virtual StatusCode initialize();
  virtual StatusCode reinitialize();
  virtual StatusCode finalize();

  virtual StatusCode regHist(const std::string& name);
  virtual StatusCode regHist(const std::string& name, TH1*);
  virtual StatusCode regHist(const std::string& name, TH2*);
  virtual StatusCode regHist(const std::string& name, TH3*);

  virtual StatusCode getHist(const std::string& name, TH1*&) const;
  virtual StatusCode getHist(const std::string& name, TH2*&) const;
  virtual StatusCode getHist(const std::string& name, TH3*&) const;

  virtual StatusCode regTree(const std::string& name);
  virtual StatusCode regTree(const std::string& name, TTree*);
  virtual StatusCode getTree(const std::string& name, TTree*&) const;

  virtual StatusCode regGraph(const std::string& name);
  virtual StatusCode regGraph(const std::string& name, TGraph*);
  virtual StatusCode getGraph(const std::string& name, TGraph*&) const;

  virtual StatusCode deReg(TObject* obj);
  virtual StatusCode deReg(const std::string& name);

  virtual std::vector<std::string> getHists() const;
  virtual std::vector<std::string> getTrees() const;
  virtual std::vector<std::string> getGraphs() const;

  virtual StatusCode getTHists(TDirectory *td, TList &, 
			       bool recurse=false) const;
  virtual StatusCode getTHists(const std::string& name, TList &, 
			       bool recurse=false) const;

  virtual StatusCode getTHists(TDirectory *td, TList &tl,
			       bool recurse=false, bool reg=false);
  virtual StatusCode getTHists(const std::string& name, TList &tl,
			       bool recurse=false, bool reg=false);

  virtual StatusCode getTTrees(TDirectory *td, TList &, 
			       bool recurse=false) const;
  virtual StatusCode getTTrees(const std::string& name, TList &, 
			       bool recurse=false) const;

  virtual StatusCode getTTrees(TDirectory *td, TList & tl,
			       bool recurse=false, bool reg=false);
  virtual StatusCode getTTrees(const std::string& name, TList & tl,
			       bool recurse=false, bool reg=false);

  virtual bool exists(const std::string& name) const;

  THistSvc(const std::string& name, ISvcLocator *svc );

  void handle(const Incident&);

  // From IIoComponent
  virtual StatusCode io_reinit ();


protected:

  virtual ~THistSvc();

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


  struct THistID {
    std::string id;
    bool        temp;
    TObject*    obj;
    TFile*      file;

    THistID():id(""),temp(true),obj(0),file(0) {}
    THistID(const THistID& rhs):id(rhs.id), temp(rhs.temp),
                                obj(rhs.obj), file(rhs.file) {}
    THistID(std::string& i, bool& t, TObject* o, TFile* f)
      : id(i), temp(t), obj(o), file(f){}

    bool operator < (THistID const &rhs) const {
      return (obj < rhs.obj);
    }
  };

  enum Mode {
    READ,
    WRITE,
    UPDATE,
    APPEND,
    SHARE
  };

  mutable MsgStream m_log;

  typedef std::map<std::string, THistID> uidMap;
  typedef std::multimap<std::string, THistID> idMap;
  typedef std::map<TObject*, THistID> objMap;
  typedef std::multimap<std::string, std::string> streamMap;

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

  //  std::map<TObject*, TFile*> m_tobjs;

  uidMap m_uids;
  idMap  m_ids;
  objMap m_tobjs;

  std::map<std::string, std::pair<TFile*,Mode> > m_files; // stream->file
  streamMap m_fileStreams;                                // fileName->streams

  std::map<std::string, std::string > m_sharedFiles; // stream->filename of shared files
  void MergeRootFile( TDirectory *target, TDirectory *source); 

  bool signaledStop;

  mutable std::string m_curstream;

};

#ifndef GAUDISVC_THISTSVC_ICC
 #include "THistSvc.icc"
#endif


#endif
