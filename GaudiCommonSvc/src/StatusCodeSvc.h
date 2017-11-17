
#ifndef GAUDISVC_STATUSCODESVC_H
#define GAUDISVC_STATUSCODESVC_H

#include "GaudiKernel/IStatusCodeSvc.h"
#include "GaudiKernel/Service.h"

#include <map>
#include <set>
#include <string>

class StatusCodeSvc : public extends<Service, IStatusCodeSvc>
{
public:
  using extends::extends;
  StatusCode initialize() override;
  StatusCode reinitialize() override;
  StatusCode finalize() override;

  void regFnc( const std::string& func, const std::string& lib ) override;
  void list() const override;
  bool suppressCheck() const override { return m_suppress.value(); }

private:
  struct StatCodeDat final {
    std::string fnc;
    std::string lib;
    int count;
  };

  void parseFilter( const std::string& str, std::string& fnc, std::string& lib );
  void filterFnc( const std::string& );
  void filterLib( const std::string& );

  Gaudi::Property<std::vector<std::string>> m_pFilter{this, "Filter"};
  Gaudi::Property<bool> m_abort{this, "AbortOnError", false};
  Gaudi::Property<bool> m_suppress{this, "SuppressCheck", false};
  Gaudi::Property<bool> m_dict{this, "IgnoreDicts", true};

  std::map<std::string, StatCodeDat> m_dat;
  std::set<std::string> m_filterfnc, m_filterlib;
};

#endif
