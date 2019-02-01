#ifndef GAUDIKERNEL_ITHISTSVC_H
#define GAUDIKERNEL_ITHISTSVC_H

#include <memory>
#include <string>
#include <vector>

#ifndef GAUDIKERNEL_ISERVICE_H
#  include "GaudiKernel/IService.h"
#endif

#include "GaudiKernel/LockedHandle.h"

class TObject;
class TH1;
class TH2;
class TH3;
class TTree;
class TList;
class TDirectory;
class TGraph;

class GAUDI_API ITHistSvc : virtual public IService {
public:
  /// InterfaceID
  DeclareInterfaceID( ITHistSvc, 3, 0 );

  /// @name Functions to manage ROOT histograms of any kind
  /// @{

  /// Register a new ROOT histogram TH*X with a name
  virtual StatusCode regHist( const std::string& name ) = 0;
  /// Register an existing ROOT histogram TH*X with name and moved unique_ptr
  /// @param [in] name      defines the histogram id/name under which it is recorded
  /// @param [in] hist      transfers ownership of the histogram to the THistSvc
  virtual StatusCode regHist( const std::string& name, std::unique_ptr<TH1> hist ) = 0;
  /// Register an existing ROOT histogram TH*X with name and moved unique_ptr
  /// @param [in] name      defines the histogram id/name under which it is recorded
  /// @param [in] hist      transfers ownership of the histogram to the THistSvc
  /// @param [out] hist_ptr for compatibility: return raw pointer to managed object to support common usage in Athena
  virtual StatusCode regHist( const std::string& name, std::unique_ptr<TH1> hist, TH1* hist_ptr ) = 0;
  /// @deprecated {Just for compatibility purposes. Ownership should be clearly managed.}
  /// Register an existing ROOT histogram TH*X with name and pointer
  virtual StatusCode regHist( const std::string& name, TH1* ) = 0;
  /// Return histogram with given name as TH1*, THistSvcMT still owns object.
  virtual StatusCode getHist( const std::string& name, TH1*&, size_t index = 0 ) const = 0;
  /// Return histogram with given name as TH2*, THistSvcMT still owns object.
  virtual StatusCode getHist( const std::string& name, TH2*&, size_t index = 0 ) const = 0;
  /// Return histogram with given name as TH3*, THistSvcMT still owns object.
  virtual StatusCode getHist( const std::string& name, TH3*&, size_t index = 0 ) const = 0;

  /// @}

  /// @name Functions to manage TTrees
  /// @{

  /// Register a new TTree with a given name
  virtual StatusCode regTree( const std::string& name ) = 0;
  /// Register an existing TTree with a given name and moved unique_ptr
  virtual StatusCode regTree( const std::string& name, std::unique_ptr<TTree> ) = 0;
  /// @deprecated {Just kept for compatibiltiy to current ATLAS code. Pleas use std::unique_ptrs instead!}
  /// Register a new TTree with a given name and a raw pointer
  virtual StatusCode regTree( const std::string& name, TTree* ) = 0;
  /// Return TTree with given name
  virtual StatusCode getTree( const std::string& name, TTree*& ) const = 0;

  /// @}

  /// @name Functions to manage TGraphs
  /// @{

  /// Register a new TGraph with a given name
  virtual StatusCode regGraph( const std::string& name ) = 0;
  /// Register an existing TGraph with a given name and moved unique_ptr
  virtual StatusCode regGraph( const std::string& name, std::unique_ptr<TGraph> ) = 0;
  /// @deprecated {Just kept for compatibiltiy to current ATLAS code. Pleas use std::unique_ptrs instead!}
  /// Register a new TGraph with a given name and a raw pointer
  virtual StatusCode regGraph( const std::string& name, TGraph* ) = 0;
  /// Return TGraph with given name
  virtual StatusCode getGraph( const std::string& name, TGraph*& ) const = 0;

  /// @}

  /// @name Functions managing shared objects
  /// @{

  /// Register shared object of type TH1 and return LockedHandle for that object
  virtual StatusCode regShared( const std::string& name, std::unique_ptr<TH1>, LockedHandle<TH1>& ) = 0;
  /// Register shared object of type TH2 and return LockedHandle for that object
  virtual StatusCode regShared( const std::string& name, std::unique_ptr<TH2>, LockedHandle<TH2>& ) = 0;
  /// Register shared object of type TH3 and return LockedHandle for that object
  virtual StatusCode regShared( const std::string& name, std::unique_ptr<TH3>, LockedHandle<TH3>& ) = 0;
  /// Register shared object of type TGraph and return LockedHandle for that object
  virtual StatusCode regShared( const std::string& name, std::unique_ptr<TGraph>, LockedHandle<TGraph>& ) = 0;
  /// Retrieve shared object with given name as TH1 through LockedHandle
  virtual StatusCode getShared( const std::string& name, LockedHandle<TH1>& ) const = 0;
  /// Retrieve shared object with given name as TH2 through LockedHandle
  virtual StatusCode getShared( const std::string& name, LockedHandle<TH2>& ) const = 0;
  /// Retrieve shared object with given name as TH3 through LockedHandle
  virtual StatusCode getShared( const std::string& name, LockedHandle<TH3>& ) const = 0;
  /// Retrieve shared object with given name as TGraph through LockedHandle
  virtual StatusCode getShared( const std::string& name, LockedHandle<TGraph>& ) const = 0;

  /// @}

  /// @name Functions that work on any TObject in the THistSvcMT
  /// @{

  /// Deregister object with given name and give up ownership (without deletion!)
  virtual StatusCode deReg( const std::string& name ) = 0;
  /// Deregister obejct identified by TObject* and give up ownership (without deletion!)
  virtual StatusCode deReg( TObject* obj ) = 0;

  /// Merge all clones for object with a given id
  virtual StatusCode merge( const std::string& id ) = 0;
  /// Merge all clones for given TObject*
  virtual StatusCode merge( TObject* ) = 0;

  /// Check if object with given name is managed by THistSvcMT
  virtual bool exists( const std::string& name ) const = 0;

  /// @}

  /// @name Functions returning lists of all histograms, trees and graphs
  /// @{

  virtual std::vector<std::string> getHists() const  = 0;
  virtual std::vector<std::string> getTrees() const  = 0;
  virtual std::vector<std::string> getGraphs() const = 0;

  virtual StatusCode getTHists( TDirectory* td, TList&, bool recurse = false ) const                      = 0;
  virtual StatusCode getTHists( const std::string& name, TList&, bool recurse = false ) const             = 0;
  virtual StatusCode getTHists( TDirectory* td, TList&, bool recurse = false, bool reg = false )          = 0;
  virtual StatusCode getTHists( const std::string& name, TList&, bool recurse = false, bool reg = false ) = 0;

  virtual StatusCode getTTrees( TDirectory* td, TList&, bool recurse = false ) const                      = 0;
  virtual StatusCode getTTrees( const std::string& name, TList&, bool recurse = false ) const             = 0;
  virtual StatusCode getTTrees( TDirectory* td, TList&, bool recurse = false, bool reg = false )          = 0;
  virtual StatusCode getTTrees( const std::string& name, TList&, bool recurse = false, bool reg = false ) = 0;

  /// @}

  /// virtual destructor
  virtual ~ITHistSvc() = default;
};

#endif // GAUDIKERNEL_ITHISTSVC_H
