#ifndef GAUDISVC_THISTSVC_H
#define GAUDISVC_THISTSVC_H

#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIoComponent.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"

#include "TGraph.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TList.h"
#include "TObject.h"
#include "TTree.h"

#include <map>
#include <set>
#include <string>
#include <vector>

class IIncidentSvc;
class THistSvc : public extends<Service, ITHistSvc, IIncidentListener, IIoComponent>
{

public:
  StatusCode initialize() override;
  StatusCode reinitialize() override;
  StatusCode finalize() override;

  StatusCode regHist( const std::string& name ) override;
  StatusCode regHist( const std::string& name, TH1* ) override;
  StatusCode regHist( const std::string& name, TH2* ) override;
  StatusCode regHist( const std::string& name, TH3* ) override;

  StatusCode getHist(const std::string& name, TH1*&, size_t ind=0) const override;
  StatusCode getHist(const std::string& name, TH2*&, size_t ind=0) const override;
  StatusCode getHist(const std::string& name, TH3*&, size_t ind=0) const override;

  StatusCode regSharedHist(const std::string& name, std::unique_ptr<TH1>&,
                           LockedHandle<TH1>&) override;
  StatusCode regSharedHist(const std::string& name, std::unique_ptr<TH2>&,
                           LockedHandle<TH2>&) override;
  StatusCode regSharedHist(const std::string& name, std::unique_ptr<TH3>&,
                           LockedHandle<TH3>&) override;

  StatusCode getSharedHist(const std::string& name, LockedHandle<TH1>&) const override;
  StatusCode getSharedHist(const std::string& name, LockedHandle<TH2>&) const override;
  StatusCode getSharedHist(const std::string& name, LockedHandle<TH3>&) const override;


  StatusCode regTree( const std::string& name ) override;
  StatusCode regTree( const std::string& name, TTree* ) override;
  StatusCode getTree( const std::string& name, TTree*& ) const override;

  StatusCode regGraph( const std::string& name ) override;
  StatusCode regGraph( const std::string& name, TGraph* ) override;
  StatusCode getGraph( const std::string& name, TGraph*& ) const override;


  StatusCode regSharedGraph(const std::string& name, std::unique_ptr<TGraph>&,
                            LockedHandle<TGraph>&) override;
  StatusCode getSharedGraph(const std::string& name, 
                            LockedHandle<TGraph>&) const override;


  StatusCode deReg(TObject* obj) override;
  StatusCode deReg(const std::string& name) override;

  std::vector<std::string> getHists() const override;
  std::vector<std::string> getTrees() const override;
  std::vector<std::string> getGraphs() const override;

  StatusCode getTHists( TDirectory* td, TList&, bool recurse = false ) const override;
  StatusCode getTHists( const std::string& name, TList&, bool recurse = false ) const override;

  StatusCode getTHists( TDirectory* td, TList& tl, bool recurse = false, bool reg = false ) override;
  StatusCode getTHists( const std::string& name, TList& tl, bool recurse = false, bool reg = false ) override;

  StatusCode getTTrees( TDirectory* td, TList&, bool recurse = false ) const override;
  StatusCode getTTrees( const std::string& name, TList&, bool recurse = false ) const override;

  StatusCode getTTrees( TDirectory* td, TList& tl, bool recurse = false, bool reg = false ) override;
  StatusCode getTTrees( const std::string& name, TList& tl, bool recurse = false, bool reg = false ) override;

  bool exists( const std::string& name ) const override;

  THistSvc( const std::string& name, ISvcLocator* svc );

  void handle( const Incident& ) override;

  // From IIoComponent
  StatusCode io_reinit () override;

  StatusCode merge(const std::string& id) override;
  StatusCode merge(TObject*) override;

protected:
  ~THistSvc() override = default;

private:

  typedef std::recursive_mutex THistSvcMutex_t;

  class GlobalDirectoryRestore {
  public:
    GlobalDirectoryRestore(THistSvcMutex_t& mut);
    ~GlobalDirectoryRestore();

  private:
    TDirectory* m_gd;
    TFile* m_gf;
    int m_ge;
    std::lock_guard<THistSvcMutex_t> m_lock;
  };

  enum Mode { READ, WRITE, UPDATE, APPEND, SHARE, INVALID };

  typedef std::mutex histMut_t;

  struct THistID {
    std::string id     {""};
    bool        temp   {true};
    TObject*    obj    {nullptr};
    TFile*      file   {nullptr};
    Mode        mode   {INVALID};
    histMut_t*  mutex  {nullptr};
    bool        shared {false};

    THistID() = default;
    THistID(const THistID& rhs) = default;
    THistID(std::string& i, bool& t, TObject* o, TFile* f)
      : id(i), temp(t), obj(o), file(f) {
    }
    THistID(std::string& i, bool& t, TObject* o, TFile* f, Mode m)
      : id(i), temp(t), obj(o), file(f), mode(m) {
    }

    void reset() { id = ""; temp=true; obj=nullptr; file=nullptr; mode=INVALID;
      mutex = nullptr; shared=false; }

    bool operator < (THistID const &rhs) const {
      return (obj < rhs.obj);
    }

    friend std::ostream& operator<< (std::ostream& ost, const THistID& hid) {
      ost << "id: " << hid.id << " t: " << hid.temp << " s: " << hid.shared
          << " M: " << hid.mode << " m: " << hid.mutex << " o: " << hid.obj
          << " " << hid.obj->IsA()->GetName();
       return ost;
    }
  };

  // class THistIDHash {
  //   std::size_t operator()( const THistID& id) const {
  //     return ( id.__hash__() );
  //   }
  // }
    


  template <typename T>
  StatusCode regHist_i(T* hist, const std::string& name, bool shared, THistID*& hid);
  template <typename T>
  StatusCode getHist_i(const std::string& name, T*& hist, const size_t& ind=0, 
                       bool quiet=false) const;
  template <typename T>
  StatusCode readHist_i( const std::string& name, T*& hist ) const;

  template <typename T>
  StatusCode regSharedObj_i(const std::string& id, std::unique_ptr<T>& hist,
                             LockedHandle<T>& lh);

  template <typename T>
  StatusCode getSharedObj_i(const std::string& name, LockedHandle<T>& lh) const;


  StatusCode readHist(const std::string& name, TH1*&) const;
  StatusCode readHist(const std::string& name, TH2*&) const;
  StatusCode readHist(const std::string& name, TH3*&) const;
  StatusCode readTree(const std::string& name, TTree*&) const;

  void updateFiles();
  StatusCode write();
  StatusCode connect( const std::string& );
  TDirectory* changeDir( const THistSvc::THistID& hid ) const;
  std::string dirname( std::string& dir ) const;
  void removeDoubleSlash( std::string& ) const;

  bool browseTDir( TDirectory* dir ) const;

  bool findStream( const std::string& name, std::string& root, std::string& rem, TFile*& file ) const;
  void parseString( const std::string& id, std::string& root, std::string& rem ) const;

  /// call-back method to handle input stream property
  void setupInputFile( Gaudi::Details::PropertyBase& inputfile );

  /// call-back method to handle output stream property
  void setupOutputFile( Gaudi::Details::PropertyBase& outputfile );

  void setupCompressionLevel( Gaudi::Details::PropertyBase& cmp );

  void copyFileLayout( TDirectory*, TDirectory* );


  size_t findHistID(const std::string& id, const THistID*& hid, 
                    const size_t& index=0) const;

  void dump() const;

  ////////

  Gaudi::Property<int> m_autoSave{this, "AutoSave", 0};
  Gaudi::Property<int> m_autoFlush{this, "AutoFlush", 0};
  Gaudi::Property<bool> m_print{this, "PrintAll", false};
  Gaudi::Property<int> m_maxFileSize{this, "MaxFileSize", 10240,
                                     "maximum file size in MB. if exceeded, will cause an abort. -1 to never check."};
  Gaudi::Property<int> m_compressionLevel{this, "CompressionLevel", 1};
  Gaudi::Property<std::vector<std::string>> m_outputfile{this, "Output", {}};
  Gaudi::Property<std::vector<std::string>> m_inputfile{this, "Input", {}};

  std::vector<std::string> m_Rstream, m_Wstream;

  /// list of already connected files. This is to keep track of files
  /// registered by the setupInputFile callback method
  std::set<std::string> m_alreadyConnectedInFiles;

  /// list of already connected files. This is to keep track of files
  /// registered by the setupOutputFile callback method
  std::set<std::string> m_alreadyConnectedOutFiles;


  typedef std::map<std::string, THistID> uidXMap;
  typedef std::multimap<std::string, THistID> idXMap;
  typedef std::multimap<std::string, std::string> streamMap;

  uidXMap m_uidsX;
  idXMap  m_idsX;

  // containers for fast lookups
  typedef std::vector<THistID> vhid_t;          // same uid for all elements in vec
  typedef std::list< vhid_t* > hlist_t;                          // all THistIDs
  typedef std::unordered_map<std::string, vhid_t*> uidMap_t;     // uid: /stream/name -> vhid
  typedef std::unordered_multimap<std::string, vhid_t*> idMap_t; // name -> vhid
  typedef std::unordered_map<TObject*, std::pair<vhid_t*,size_t> > objMap_t;

  hlist_t  m_hlist;
  uidMap_t m_uids;
  idMap_t  m_ids;

  objMap_t  m_tobjs;

  std::map<std::string, std::pair<TFile*, Mode>> m_files; // stream->file
  streamMap m_fileStreams;                                // fileName->streams

  std::map<std::string, std::string> m_sharedFiles; // stream->filename of shared files

  bool signaledStop   = false;
  bool m_delayConnect = false, m_okToConnect = false;

  mutable std::string m_curstream;

  mutable THistSvcMutex_t m_svcMut;

  IIncidentSvc* p_incSvc = nullptr;
  IFileMgr* p_fileMgr    = nullptr;

  StatusCode rootOpenAction( FILEMGR_CALLBACK_ARGS );
  StatusCode rootOpenErrAction( FILEMGR_CALLBACK_ARGS );

  void dumpVHID(const vhid_t*) const;

  StatusCode merge(const THistID&);
  StatusCode merge(vhid_t*);


};

#ifndef GAUDISVC_THISTSVC_ICC
#include "THistSvc.icc"
#endif

#endif
