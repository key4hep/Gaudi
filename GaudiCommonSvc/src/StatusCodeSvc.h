
#ifndef GAUDISVC_STATUSCODESVC_H
#define GAUDISVC_STATUSCODESVC_H

#include "GaudiKernel/Service.h"
#include "GaudiKernel/IStatusCodeSvc.h"

#include <string>
#include <map>
#include <set>

class StatusCodeSvc: public extends1<Service, IStatusCodeSvc> {

public:

  virtual StatusCode initialize();
  virtual StatusCode reinitialize();
  virtual StatusCode finalize();

  virtual void regFnc(const std::string &func, const std::string &lib);
  virtual void list() const;
  virtual bool suppressCheck() const { return m_suppress.value() ; }

  StatusCodeSvc( const std::string& name, ISvcLocator* svc );

  // Destructor.
  virtual ~StatusCodeSvc();

private:

  struct StatCodeDat {
    std::string fnc;
    std::string lib;
    int count;
  };

  void parseFilter(const std::string& str, std::string& fnc, std::string& lib);
  void filterFnc(const std::string&);
  void filterLib(const std::string&);

  StringArrayProperty m_pFilter;
  BooleanProperty m_abort, m_suppress, m_dict;

  std::map<std::string,StatCodeDat> m_dat;
  std::set<std::string> m_filterfnc, m_filterlib;

};

#endif
