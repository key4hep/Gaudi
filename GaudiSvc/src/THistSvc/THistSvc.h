/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/IFileMgr.h>
#include <GaudiKernel/IIncidentListener.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/IIoComponent.h>
#include <GaudiKernel/ITHistSvc.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/System.h>
#include <TEfficiency.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TList.h>
#include <TObject.h>
#include <TTree.h>
#include <map>
#include <string>

class THistSvc : public extends<Service, ITHistSvc, IIncidentListener, IIoComponent> {
public:
  THistSvc( const std::string& name, ISvcLocator* svcloc );

  StatusCode initialize() override;
  StatusCode reinitialize() override;
  StatusCode finalize() override;

public:
  // Methods from ITHistSvc
  /// @name Functions to manage ROOT histograms of any kind
  /// @{

  /// Register a new ROOT histogram TH*X with a name
  StatusCode regHist( const std::string& name ) override;
  /// Register an existing ROOT histogram TH*X with name and moved unique_ptr
  /// @param [in] name      defines the histogram id/name under which it is recorded
  /// @param [in] hist      transfers ownership of the histogram to the THistSvc
  StatusCode regHist( const std::string& name, std::unique_ptr<TH1> hist ) override;
  /// @deprecated {Just for compatibility purposes. Ownership should be clearly managed.}
  /// Register an existing ROOT histogram TH*X with name and pointer
  StatusCode regHist( const std::string& name, TH1* ) override;

  /// Return histogram with given name as TH1*, THistSvcMT still owns object.
  StatusCode getHist( const std::string& name, TH1*&, size_t index = 0 ) const override;
  /// Return histogram with given name as TH2*, THistSvcMT still owns object.
  StatusCode getHist( const std::string& name, TH2*&, size_t index = 0 ) const override;
  /// Return histogram with given name as TH3*, THistSvcMT still owns object.
  StatusCode getHist( const std::string& name, TH3*&, size_t index = 0 ) const override;

  /// @}

  /// @name Functions to manage TTrees
  /// @{

  /// Register a new TTree with a given name
  StatusCode regTree( const std::string& name ) override;
  /// Register an existing TTree with a given name and moved unique_ptr
  StatusCode regTree( const std::string& name, std::unique_ptr<TTree> ) override;
  /// @deprecated {Just kept for compatibiltiy to current ATLAS code. Pleas use std::unique_ptrs instead!}
  /// Register a new TTree with a given name and a raw pointer
  StatusCode regTree( const std::string& name, TTree* ) override;
  /// Return TTree with given name
  StatusCode getTree( const std::string& name, TTree*& ) const override;

  /// @}

  /// @name Functions to manage TGraphs
  /// @{

  /// Register a new TGraph with a given name
  StatusCode regGraph( const std::string& name ) override;
  /// Register an existing TGraph with a given name and moved unique_ptr
  StatusCode regGraph( const std::string& name, std::unique_ptr<TGraph> ) override;
  /// @deprecated {Just kept for compatibiltiy to current ATLAS code. Pleas use std::unique_ptrs instead!}
  /// Register a new TGraph with a given name and a raw pointer
  virtual StatusCode regGraph( const std::string& name, TGraph* ) override;
  /// Return TGraph with given name
  StatusCode getGraph( const std::string& name, TGraph*& ) const override;

  /// Register a new TEfficiency with a given name
  StatusCode regEfficiency( const std::string& name ) override;
  /// Register an existing TEfficiency with a given name and moved unique_ptr
  StatusCode regEfficiency( const std::string& name, std::unique_ptr<TEfficiency> ) override;
  /// @deprecated {Just kept for compatibiltiy to current ATLAS code. Pleas use std::unique_ptrs instead!}
  /// Register a new TEfficiency with a given name and a raw pointer
  virtual StatusCode regEfficiency( const std::string& name, TEfficiency* ) override;
  /// Return TEfficiency with given name
  StatusCode getEfficiency( const std::string& name, TEfficiency*& ) const override;

  /// @}

  /// @name Functions managing shared objects
  /// @{

  /// Register shared object of type TH1 and return LockedHandle for that object
  StatusCode regShared( const std::string& name, std::unique_ptr<TH1>, LockedHandle<TH1>& ) override;
  /// Register shared object of type TH2 and return LockedHandle for that object
  StatusCode regShared( const std::string& name, std::unique_ptr<TH2>, LockedHandle<TH2>& ) override;
  /// Register shared object of type TH3 and return LockedHandle for that object
  StatusCode regShared( const std::string& name, std::unique_ptr<TH3>, LockedHandle<TH3>& ) override;
  /// Register shared object of type TGraph and return LockedHandle for that object
  StatusCode regShared( const std::string& name, std::unique_ptr<TGraph>, LockedHandle<TGraph>& ) override;
  /// Register shared object of type TEfficiency and return LockedHandle for that object
  StatusCode regShared( const std::string& name, std::unique_ptr<TEfficiency>, LockedHandle<TEfficiency>& ) override;
  /// Retrieve shared object with given name as TH1 through LockedHandle
  StatusCode getShared( const std::string& name, LockedHandle<TH1>& ) const override;
  /// Retrieve shared object with given name as TH2 through LockedHandle
  StatusCode getShared( const std::string& name, LockedHandle<TH2>& ) const override;
  /// Retrieve shared object with given name as TH3 through LockedHandle
  StatusCode getShared( const std::string& name, LockedHandle<TH3>& ) const override;
  /// Retrieve shared object with given name as TGraph through LockedHandle
  StatusCode getShared( const std::string& name, LockedHandle<TGraph>& ) const override;
  /// Retrieve shared object with given name as TEfficiency through LockedHandle
  StatusCode getShared( const std::string& name, LockedHandle<TEfficiency>& ) const override;

  /// @}

  /// @name Functions that work on any TObject in the THistSvcMT
  /// @{

  /// Deregister object with given name and give up ownership (without deletion!)
  StatusCode deReg( const std::string& name ) override;
  /// Deregister obejct identified by TObject* and give up ownership (without deletion!)
  StatusCode deReg( TObject* obj ) override;

  /// Merge all clones for object with a given id
  StatusCode merge( const std::string& id ) override;
  /// Merge all clones for given TObject*
  StatusCode merge( TObject* ) override;

  /// Check if object with given name is managed by THistSvcMT
  /// exists calls existsHist and only works for TH1-descendants
  bool exists( const std::string& name ) const override;
  /// Check if histogram with given name is managed by THistSvcMT
  bool existsHist( const std::string& name ) const override;
  /// Check if tree with given name is managed by THistSvcMT
  bool existsTree( const std::string& name ) const override;
  /// Check if graph with given name is managed by THistSvcMT
  bool existsGraph( const std::string& name ) const override;
  /// Check if TEfficiency with given name is managed by THistSvcMT
  bool existsEfficiency( const std::string& name ) const override;

  /// @}

  /// @name Functions returning lists of all histograms, trees and graphs
  /// @{

  std::vector<std::string> getHists() const override;
  std::vector<std::string> getTrees() const override;
  std::vector<std::string> getGraphs() const override;
  std::vector<std::string> getEfficiencies() const override;

  StatusCode getTHists( TDirectory* td, TList&, bool recurse = false ) const override;
  StatusCode getTHists( const std::string& name, TList&, bool recurse = false ) const override;
  StatusCode getTHists( TDirectory* td, TList& tl, bool recurse = false, bool reg = false ) override;
  StatusCode getTHists( const std::string& name, TList& tl, bool recurse = false, bool reg = false ) override;

  StatusCode getTTrees( TDirectory* td, TList&, bool recurse = false ) const override;
  StatusCode getTTrees( const std::string& name, TList&, bool recurse = false ) const override;
  StatusCode getTTrees( TDirectory* td, TList& tl, bool recurse = false, bool reg = false ) override;
  StatusCode getTTrees( const std::string& name, TList& tl, bool recurse = false, bool reg = false ) override;

  StatusCode getTEfficiencies( TDirectory* td, TList&, bool recurse = false ) const override;
  StatusCode getTEfficiencies( const std::string& name, TList&, bool recurse = false ) const override;
  StatusCode getTEfficiencies( TDirectory* td, TList& tl, bool recurse = false, bool reg = false ) override;
  StatusCode getTEfficiencies( const std::string& name, TList& tl, bool recurse = false, bool reg = false ) override;

  /// @}

public:
  // Methods from other interfaces
  // From IIncidentListener
  void handle( const Incident& ) override;

  // From IIoComponent
  StatusCode io_reinit() override;

private:
  typedef std::recursive_mutex THistSvcMutex_t;
  typedef std::mutex           histMut_t;

  /// Helper class that manages ROOts global directory and file
  class GlobalDirectoryRestore {
  public:
    GlobalDirectoryRestore( THistSvcMutex_t& mut );
    ~GlobalDirectoryRestore();

  private:
    TDirectory*                      m_gDirectory;
    TFile*                           m_gFile;
    int                              m_gErrorIgnoreLevel;
    std::lock_guard<THistSvcMutex_t> m_lock;
  };

  /// Enumerating all possible file access modes
  enum Mode { READ, WRITE, UPDATE, APPEND, SHARE, INVALID };

  /// Possible TObject types
  enum class ObjectType { UNKNOWN, TH1, TTREE, TGRAPH, TEFFICIENCY };

  /// Convert a char to a Mode enum
  static Mode charToMode( const char typ ) {
    switch ( typ ) {
    case 'O':
      return READ;
    case 'A':
      return APPEND;
    case 'R':
      return UPDATE;
    case 'S':
      return SHARE;
    default:
      return INVALID;
    }
  }

  /// Helper struct that bundles the histogram ID with a mutex, TFile and TObject*
  struct THistID {
    std::string id{};
    TObject*    obj{ nullptr };
    TFile*      file{ nullptr };
    histMut_t*  mutex{ nullptr };
    Mode        mode{ INVALID };
    ObjectType  type{ ObjectType::UNKNOWN };
    bool        temp{ true };
    bool        shared{ false };

    THistID()                                = default;
    THistID( const THistID& rhs )            = default;
    THistID& operator=( const THistID& rhs ) = default;
    THistID( std::string& i, bool& t, TObject* o, TFile* f ) : id( i ), obj( o ), file( f ), temp( t ) {}
    THistID( std::string& i, bool& t, TObject* o, TFile* f, Mode m )
        : id( i ), obj( o ), file( f ), mode( m ), temp( t ) {}

    bool operator<( THistID const& rhs ) const { return ( obj < rhs.obj ); }

    friend std::ostream& operator<<( std::ostream& ost, const THistID& hid ) {
      ost << "id: " << hid.id << " t: " << hid.temp << " s: " << hid.shared << " M: " << hid.mode << " m: " << hid.mutex
          << " o: " << hid.obj << " T: " << static_cast<int>( hid.type ) << " " << hid.obj->IsA()->GetName();
      return ost;
    }
  };

  /// @name Container definitions
  /// @{

  std::vector<std::string> m_Rstream, m_Wstream;

  /// list of already connected files. This is to keep track of files
  /// registered by the setupInputFile callback method
  std::set<std::string> m_alreadyConnectedInFiles;

  /// list of already connected files. This is to keep track of files
  /// registered by the setupOutputFile callback method
  std::set<std::string> m_alreadyConnectedOutFiles;

  // containers for fast lookups
  // same uid for all elements in vec
  typedef std::vector<THistID> vhid_t;
  // all THistIDs
  typedef std::list<vhid_t*> hlist_t;
  // uid: /stream/name -> vhid
  typedef std::unordered_map<std::string, vhid_t*> uidMap_t;
  // name -> vhid
  typedef std::unordered_multimap<std::string, vhid_t*>            idMap_t;
  typedef std::unordered_map<TObject*, std::pair<vhid_t*, size_t>> objMap_t;

  hlist_t  m_hlist;
  uidMap_t m_uids;
  idMap_t  m_ids;

  // Container holding all TObjects and vhid*s
  objMap_t m_tobjs;

  std::map<std::string, std::pair<TFile*, Mode>>  m_files; // stream->file
  typedef std::multimap<std::string, std::string> streamMap;
  streamMap                                       m_fileStreams; // fileName->streams

  // stream->filename of shared files
  std::map<std::string, std::string> m_sharedFiles;

  /// @}

  /// @name Templated helper functions to register and retrieve Histograms and TObjects
  /// @{

  template <typename T>
  StatusCode regHist_i( std::unique_ptr<T> hist, const std::string& name, bool shared );
  template <typename T>
  StatusCode regHist_i( std::unique_ptr<T> hist, const std::string& name, bool shared, THistID*& hid );
  template <typename T>
  T* getHist_i( const std::string& name, const size_t& ind = 0, bool quiet = false ) const;
  template <typename T>
  T* readHist_i( const std::string& name ) const;

  template <typename T>
  LockedHandle<T> regShared_i( const std::string& id, std::unique_ptr<T> hist );
  template <typename T>
  LockedHandle<T> getShared_i( const std::string& name ) const;

  /// @}

  /// @name Collection of private helper methods
  /// @{

  template <typename T>
  T*     readHist( const std::string& name ) const;
  TTree* readTree( const std::string& name ) const;

  /// Handle case where TTree grows beyond TTree::fgMaxTreeSize
  void        updateFiles();
  StatusCode  writeObjectsToFile();
  StatusCode  connect( const std::string& );
  TDirectory* changeDir( const THistSvc::THistID& hid ) const;
  std::string stripDirectoryName( std::string& dir ) const;
  void        removeDoubleSlash( std::string& ) const;

  void MergeRootFile( TDirectory*, TDirectory* );

  bool findStream( const std::string& name, std::string& root, std::string& rem, TFile*& file ) const;
  void parseString( const std::string& id, std::string& root, std::string& rem ) const;

  /// call-back method to handle input stream property
  void setupInputFile();

  /// call-back method to handle output stream property
  void setupOutputFile();

  /// helper function to recursively copy the layout of a TFile into a new TFile
  void copyFileLayout( TDirectory*, TDirectory* );

  size_t findHistID( const std::string& id, const THistID*& hid, const size_t& index = 0 ) const;

  void dump() const;

  /// Helper method to merge THistID objects
  StatusCode merge( const THistID& );
  /// Helper method to merge vectors of THistID
  StatusCode merge( vhid_t* );

  StatusCode rootOpenAction( FILEMGR_CALLBACK_ARGS );
  StatusCode rootOpenErrAction( FILEMGR_CALLBACK_ARGS );

  /// @}

  /// @name Gaudi properties
  /// @{

  Gaudi::Property<int>                      m_autoSave{ this, "AutoSave", 0 };
  Gaudi::Property<int>                      m_autoFlush{ this, "AutoFlush", 0 };
  Gaudi::Property<bool>                     m_print{ this, "PrintAll", false };
  Gaudi::Property<int>                      m_maxFileSize{ this, "MaxFileSize", 10240,
                                      "maximum file size in MB. if exceeded,"
                                                           " will cause an abort. -1 to never check." };
  Gaudi::Property<int>                      m_compressionLevel{ this, "CompressionLevel", 1, [this]( auto& ) {
                                            this->warning()
                                                << "\"CompressionLevel\" Property has been deprecated. "
                                                << "Set it via the \"CL=\" parameter in the \"Output\" Property"
                                                << endmsg;
                                          } };
  Gaudi::Property<std::vector<std::string>> m_outputfile{
      this, "Output", {}, &THistSvc::setupOutputFile, "", "OrderedSet<std::string>" };
  Gaudi::Property<std::vector<std::string>> m_inputfile{
      this, "Input", {}, &THistSvc::setupInputFile, "", "OrderedSet<std::string>" };

  /// @}

  ServiceHandle<IIncidentSvc> p_incSvc;
  ServiceHandle<IFileMgr>     p_fileMgr;

  bool m_delayConnect = false;
  bool m_okToConnect  = false;
  bool m_hasTTrees    = false;

  mutable std::string m_curstream;

  mutable THistSvcMutex_t m_svcMut;
};

template <typename T>
StatusCode THistSvc::regHist_i( std::unique_ptr<T> hist, const std::string& id, bool shared ) {
  THistID* hid = nullptr;
  return regHist_i( std::move( hist ), id, shared, hid );
}

template <typename T>
StatusCode THistSvc::regHist_i( std::unique_ptr<T> hist_unique, const std::string& id, bool shared, THistID*& phid ) {
  GlobalDirectoryRestore restore( m_svcMut );
  phid = nullptr;

  // It is sad that we lose propper memory management here
  T* hist = nullptr;
  if ( hist_unique.get() != nullptr ) { hist = hist_unique.release(); }
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "regHist_i obj: " << hist << "  id: " << id << "  s: " << shared << endmsg;
  }

  std::string idr( id );
  removeDoubleSlash( idr );

  if ( idr.find( "/" ) == idr.length() ) {
    error() << "Badly formed identifier \"" << idr << "\": "
            << "Must not end with a /" << endmsg;
    delete hist;
    return StatusCode::FAILURE;
  }

  TFile*      f = nullptr;
  std::string stream, name;
  if ( !findStream( idr, stream, name, f ) ) {
    error() << "Could not register id: \"" << idr << "\"" << endmsg;
    delete hist;
    return StatusCode::FAILURE;
  }

  std::string uid = "/" + stream + "/" + name;

  uidMap_t::iterator uitr = m_uids.find( uid );
  bool               exists( false );
  if ( uitr != m_uids.end() ) {
    exists      = true;
    TObject* t1 = uitr->second->at( 0 ).obj;
    if ( hist->Compare( t1 ) != 0 ) {
      error() << "previously registered object with identifier \"" << uid << "\" does not compare to this one"
              << endmsg;
      delete hist;
      return StatusCode::FAILURE;
    } else {
      if ( msgLevel( MSG::DEBUG ) ) {
        debug() << "previously registered id \"" << uid << "\": num " << uitr->second->size() << endmsg;
      }
    }
  }

  bool temp = false;
  if ( !f ) {
    temp = true;
    if ( msgLevel( MSG::DEBUG ) ) { debug() << "Historgram with id \"" << idr << "\" is temporary" << endmsg; }
  }

  TObject* to = nullptr;
  THistID  hid;
  // check to see if this hist is to be read in;
  if ( !temp && m_files.find( stream )->second.second == READ ) {
    if ( hist != 0 ) { warning() << "Registering id: \"" << idr << "\" with non zero pointer!" << endmsg; }

    hist = readHist_i<T>( idr );
    if ( hist == nullptr ) {
      error() << "Unable to read in hist" << endmsg;
      delete hist;
      return StatusCode::FAILURE;
    }
    to  = dynamic_cast<TObject*>( hist );
    hid = THistID( uid, temp, to, f, m_files.find( stream )->second.second );
  } else if ( !hist ) {
    error() << "Unable to read in hist with id: \"" << idr << "\"" << endmsg;
    delete hist;
    return StatusCode::FAILURE;
  } else {
    to = dynamic_cast<TObject*>( hist );
    if ( to == nullptr ) {
      error() << "Could not dcast to TObject. id: \"" << idr << "\"" << endmsg;
      delete hist;
      return StatusCode::FAILURE;
    }

    auto oitr = m_tobjs.find( to );
    if ( oitr != m_tobjs.end() ) {
      error() << "already registered id: \"" << idr << "\" with identifier \""
              << oitr->second.first->at( oitr->second.second ).id << "\"" << endmsg;
      delete hist;
      return StatusCode::FAILURE;
    }
  }

  const auto findF = m_files.find( stream );
  hid = ( findF != m_files.end() ? THistID( uid, temp, to, f, findF->second.second ) : THistID( uid, temp, to, f ) );

  hid.shared      = shared;
  TDirectory* dir = changeDir( hid );

  if ( TTree* tree = dynamic_cast<TTree*>( hist ) ) {
    tree->SetDirectory( dir );
    hid.type    = ObjectType::TTREE;
    m_hasTTrees = true; // at least one TTree is registered
  } else if ( TH1* th1 = dynamic_cast<TH1*>( hist ) ) {
    th1->SetDirectory( dir );
    hid.type = ObjectType::TH1;
  } else if ( TEfficiency* teff = dynamic_cast<TEfficiency*>( hist ) ) {
    teff->SetDirectory( dir );
    hid.type = ObjectType::TEFFICIENCY;
  } else if ( dynamic_cast<TGraph*>( hist ) ) {
    dir->Append( hist );
    hid.type = ObjectType::TGRAPH;
  } else {
    error() << "id: \"" << idr << "\" is not a TH, TTree, TGraph, or TEfficiency. Attaching it to current dir."
            << endmsg;
    dir->Append( hist );
    hid.type = ObjectType::UNKNOWN;
  }

  std::string fname;
  if ( !f ) {
    fname = "none";
  } else {
    fname = f->GetName();
  }

  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "Registering" << ( shared ? " shared " : " " ) << System::typeinfoName( typeid( *hist ) ) << " title: \""
            << hist->GetTitle() << "\"  id: \"" << uid
            << "\"  dir: "
            //          << hist->GetDirectory()->GetPath() << "  "
            << changeDir( hid )->GetPath() << "  file: " << fname << endmsg;
  }

  // create a mutex for all shared histograms
  if ( shared ) { hid.mutex = new histMut_t; }

  if ( exists ) {
    vhid_t* vi = uitr->second;
    vi->push_back( hid );
    phid = &( vi->back() );

    m_tobjs.emplace( to, std::pair<vhid_t*, size_t>( vi, vi->size() - 1 ) );
  } else {
    vhid_t* vi = new vhid_t{ hid };
    m_hlist.emplace( m_hlist.end(), vi );

    phid = &( vi->back() );
    m_uids.emplace( uid, vi );
    m_ids.emplace( name, vi );

    m_tobjs.emplace( to, std::pair<vhid_t*, size_t>( vi, 0 ) );
  }

  if ( msgLevel( MSG::DEBUG ) ) { debug() << "regHist_i  THistID: " << hid << endmsg; }

  return StatusCode::SUCCESS;
}

template <typename T>
T* THistSvc::getHist_i( const std::string& id, const size_t& ind, bool quiet ) const {
  // id starts with "/": unique

  GlobalDirectoryRestore restore( m_svcMut );

  T*             hist = nullptr;
  const THistID* hid  = nullptr;
  size_t         num  = findHistID( id, hid, ind );
  if ( num == 0 ) {
    // no matches found
    if ( !quiet ) { error() << "could not locate Hist with id \"" << id << "\"" << endmsg; }
    return nullptr;
  } else if ( num > 1 ) {
    if ( !quiet ) {
      // return failure if trying to GET a single hist
      error() << "Multiple matches with id \"" << id << "\"."
              << " Further specifications required." << endmsg;
      return nullptr;
    } else {
      info() << "Found multiple matches with id \"" << id << "\"" << endmsg;
      // return first match if just INQUIRING (i.e. != nullptr)
      hist = dynamic_cast<T*>( hid->obj );
      if ( hist == nullptr ) {
        error() << "dcast failed, Hist id: \"" << id << "\"" << endmsg;
        return nullptr;
      }
    }
  } else {
    hist = dynamic_cast<T*>( hid->obj );
    if ( hist == nullptr ) {
      error() << "dcast failed, Hist id: \"" << id << "\"" << endmsg;
      return nullptr;
    }
    if ( msgLevel( MSG::VERBOSE ) ) {
      verbose() << "found unique Hist title: \"" << hist->GetTitle() << "\"  id: \"" << id << "\"" << endmsg;
    }
  }

  return hist;
}

template <typename T>
T* THistSvc::readHist_i( const std::string& id ) const {
  GlobalDirectoryRestore restore( m_svcMut );

  std::string idr( id );
  removeDoubleSlash( idr );

  std::string stream, rem, dir, fdir, bdir, fdir2;
  TFile*      file = nullptr;

  if ( !findStream( idr, stream, rem, file ) ) { return nullptr; }

  if ( !file ) {
    error() << "no associated file found" << endmsg;
    return nullptr;
  }

  file->cd( "/" );

  fdir  = idr;
  bdir  = stripDirectoryName( fdir );
  fdir2 = fdir;
  while ( ( dir = stripDirectoryName( fdir ) ) != "" ) {
    if ( !gDirectory->GetKey( dir.c_str() ) ) {
      error() << "Directory \"" << fdir2 << "\" doesnt exist in " << file->GetName() << endmsg;
      return nullptr;
    }
    gDirectory->cd( dir.c_str() );
  }

  TObject* to = nullptr;
  gDirectory->GetObject( fdir.c_str(), to );

  if ( !to ) {
    error() << "Could not get obj \"" << fdir << "\" in " << gDirectory->GetPath() << endmsg;
    return nullptr;
  }

  T* hist = dynamic_cast<T*>( to );
  if ( hist == nullptr ) {
    error() << "Could not convert \"" << idr << "\" to a " << System::typeinfoName( typeid( *hist ) ) << " as is a "
            << to->IsA()->GetName() << endmsg;
    return nullptr;
  }

  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "Read in " << hist->IsA()->GetName() << "  \"" << hist->GetName() << "\" from file " << file->GetName()
            << endmsg;
    hist->Print();
  }

  return hist;
}

template <typename T>
LockedHandle<T> THistSvc::regShared_i( const std::string& id, std::unique_ptr<T> hist ) {
  LockedHandle<T> lh( nullptr, nullptr );
  const THistID*  hid = nullptr;
  if ( findHistID( id, hid ) == 0 ) {
    T*       phist = hist.get();
    THistID* phid  = nullptr;
    if ( regHist_i( std::move( hist ), id, true, phid ).isSuccess() ) {
      lh.set( phist, phid->mutex );

    } else {
      error() << "regSharedHist: unable to register shared hist with id \"" << id << "\"" << endmsg;
    }
  } else {
    if ( !hid->shared ) {
      error() << "regSharedHist: previously register Hist with id \"" << id << "\" was not marked shared" << endmsg;
    }

    if ( hist->Compare( hid->obj ) != 0 ) {
      error() << "regSharedHist: Histogram " << id << " does not compare with " << hid << endmsg;
    } else {
      T* phist = dynamic_cast<T*>( hid->obj );
      if ( phist == 0 ) {
        error() << "regSharedHist: unable to dcast retrieved shared hist \"" << id << "\" of type "
                << hid->obj->IsA()->GetName() << " to requested type " << System::typeinfoName( typeid( T ) ) << endmsg;
      } else {
        lh.set( phist, hid->mutex );
        delete hist.release();
      }
    }
  }
  return lh;
}

template <typename T>
LockedHandle<T> THistSvc::getShared_i( const std::string& name ) const {
  GlobalDirectoryRestore restore( m_svcMut );

  const THistID* hid = nullptr;
  size_t         i   = findHistID( name, hid );

  LockedHandle<T> hist( nullptr, nullptr );

  if ( i == 1 ) {
    if ( !hid->shared ) {
      error() << "getSharedHist: found Hist with id \"" << name << "\", but it's not marked as shared" << endmsg;
      return hist;
    }
    T* h1 = dynamic_cast<T*>( hid->obj );
    hist  = LockedHandle<T>( h1, hid->mutex );

    if ( msgLevel( MSG::DEBUG ) ) { debug() << "getSharedHist: found THistID: " << *hid << endmsg; }
  } else if ( i == 0 ) {
    error() << "no histograms matching id \"" << name << "\" found" << endmsg;
  } else {
    info() << "multiple matches for id \"" << name << "\" found [" << i << "], probably from different streams"
           << endmsg;
  }
  return hist;
}
