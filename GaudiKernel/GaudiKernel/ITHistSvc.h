#ifndef GAUDIKERNEL_ITHISTSVC_H
#define GAUDIKERNEL_ITHISTSVC_H

#ifndef GAUDIKERNEL_ISERVICE_H
#include "GaudiKernel/IService.h"
#endif

#include "GaudiKernel/LockedHandle.h"

#include <memory>
#include <string>
#include <vector>

class TObject;
class TH1;
class TH2;
class TH3;
class TTree;
class TList;
class TDirectory;
class TGraph;

class GAUDI_API ITHistSvc : virtual public IService
{
public:
  /// InterfaceID
  DeclareInterfaceID( ITHistSvc, 2, 0 );

  virtual StatusCode regHist( const std::string& name ) = 0;
  virtual StatusCode regHist( const std::string& name, std::unique_ptr<TH1> ) = 0;

  virtual TH1* getHistTH1( const std::string& name, size_t index = 0 ) const = 0;
  virtual TH2* getHistTH2( const std::string& name, size_t index = 0 ) const = 0;
  virtual TH3* getHistTH3( const std::string& name, size_t index = 0 ) const = 0;

  virtual LockedHandle<TH1> regSharedHist( const std::string& name, std::unique_ptr<TH1> ) = 0;
  virtual LockedHandle<TH2> regSharedHist( const std::string& name, std::unique_ptr<TH2> ) = 0;
  virtual LockedHandle<TH3> regSharedHist( const std::string& name, std::unique_ptr<TH3> ) = 0;

  virtual LockedHandle<TH1> getSharedHistTH1( const std::string& name ) const = 0;
  virtual LockedHandle<TH2> getSharedHistTH2( const std::string& name ) const = 0;
  virtual LockedHandle<TH3> getSharedHistTH3( const std::string& name ) const = 0;

  virtual StatusCode regTree( const std::string& name ) = 0;
  virtual StatusCode regTree( const std::string& name, std::unique_ptr<TTree> ) = 0;
  virtual TTree* getTree( const std::string& name ) const = 0;

  virtual StatusCode deReg( const std::string& name ) = 0;
  virtual StatusCode deReg( TObject* obj ) = 0;

  virtual std::vector<std::string> getHists() const = 0;
  virtual std::vector<std::string> getTrees() const = 0;
  virtual std::vector<std::string> getGraphs() const = 0;

  virtual StatusCode getTHists( TDirectory* td, TList&, bool recurse = false ) const = 0;
  virtual StatusCode getTHists( const std::string& name, TList&, bool recurse = false ) const = 0;

  virtual StatusCode getTHists( TDirectory* td, TList&, bool recurse = false, bool reg = false ) = 0;
  virtual StatusCode getTHists( const std::string& name, TList&, bool recurse = false, bool reg = false ) = 0;

  virtual StatusCode getTTrees( TDirectory* td, TList&, bool recurse = false ) const = 0;
  virtual StatusCode getTTrees( const std::string& name, TList&, bool recurse = false ) const = 0;
  virtual StatusCode getTTrees( TDirectory* td, TList&, bool recurse = false, bool reg = false ) = 0;
  virtual StatusCode getTTrees( const std::string& name, TList&, bool recurse = false, bool reg = false ) = 0;

  virtual StatusCode regGraph( const std::string& name ) = 0;
  virtual StatusCode regGraph( const std::string& name, std::unique_ptr<TGraph> ) = 0;
  virtual TGraph* getGraph( const std::string& name ) const = 0;

  virtual LockedHandle<TGraph> regSharedGraph( const std::string& name, std::unique_ptr<TGraph> ) = 0;
  virtual LockedHandle<TGraph> getSharedGraph( const std::string& name ) const = 0;

  virtual StatusCode merge( const std::string& id ) = 0;
  virtual StatusCode merge( TObject* ) = 0;

  virtual bool exists( const std::string& name ) const = 0;

  /// virtual destructor
  virtual ~ITHistSvc() = default;
};

#endif
