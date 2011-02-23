#ifndef GAUDIKERNEL_ITHISTSVC_H
#define GAUDIKERNEL_ITHISTSVC_H

#ifndef GAUDIKERNEL_ISERVICE_H
 #include "GaudiKernel/IService.h"
#endif
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

class GAUDI_API ITHistSvc: virtual public IService {

public:
  /// InterfaceID
  DeclareInterfaceID(ITHistSvc,2,0);

  virtual StatusCode regHist(const std::string& name) = 0;
  virtual StatusCode regHist(const std::string& name, TH1*) = 0;
  virtual StatusCode regHist(const std::string& name, TH2*) = 0;
  virtual StatusCode regHist(const std::string& name, TH3*) = 0;

  virtual StatusCode getHist(const std::string& name, TH1*&) const = 0;
  virtual StatusCode getHist(const std::string& name, TH2*&) const = 0;
  virtual StatusCode getHist(const std::string& name, TH3*&) const = 0;

  virtual StatusCode regTree(const std::string& name) = 0;
  virtual StatusCode regTree(const std::string& name, TTree*) = 0;
  virtual StatusCode getTree(const std::string& name, TTree*&) const = 0;

  virtual StatusCode deReg(const std::string& name) = 0;
  virtual StatusCode deReg(TObject* obj) = 0;

  virtual std::vector<std::string> getHists() const = 0;
  virtual std::vector<std::string> getTrees() const = 0;
  virtual std::vector<std::string> getGraphs() const = 0;

  //  virtual StatusCode getTHists(TList&) const = 0;
  virtual StatusCode getTHists(TDirectory *td, TList &, 
			       bool recurse=false) const = 0;
  virtual StatusCode getTHists(const std::string& name, TList &, 
			       bool recurse=false) const = 0;

  virtual StatusCode getTHists(TDirectory *td, TList &, 
			       bool recurse=false, bool reg=false) = 0;
  virtual StatusCode getTHists(const std::string& name, TList &, 
			       bool recurse=false, bool reg=false) = 0;

  //  virtual StatusCode getTTrees(TList&) const = 0;
  virtual StatusCode getTTrees(TDirectory *td, TList &, 
			       bool recurse=false) const = 0;
  virtual StatusCode getTTrees(const std::string& name, TList &, 
			       bool recurse=false) const = 0;
  virtual StatusCode getTTrees(TDirectory *td, TList &, 
			       bool recurse=false, bool reg=false) = 0;
  virtual StatusCode getTTrees(const std::string& name, TList &, 
			       bool recurse=false, bool reg=false) = 0;

  virtual StatusCode regGraph(const std::string& name) = 0;
  virtual StatusCode regGraph(const std::string& name, TGraph*) = 0;
  virtual StatusCode getGraph(const std::string& name, TGraph*&) const = 0;

  virtual bool exists(const std::string& name) const = 0;

};

#endif
