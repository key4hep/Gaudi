#ifndef GAUDISVC_STATUSCODESVC_H
#define GAUDISVC_STATUSCODESVC_H

#include "GaudiKernel/IStatusCodeSvc.h"
#include "GaudiKernel/Service.h"

#include "tbb/concurrent_unordered_map.h"

#include <set>
#include <string>

class StatusCodeSvc : public extends<Service, IStatusCodeSvc> {
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
    // the atomic forces us to define these otherwise trivial constructors:
    StatCodeDat( const std::string f, const std::string l ) : fnc( f ), lib( l ) {}
    StatCodeDat( StatCodeDat&& o ) : fnc( o.fnc ), lib( o.lib ), count( o.count.load() ) {}
    StatCodeDat( const StatCodeDat& o ) : fnc( o.fnc ), lib( o.lib ), count( o.count.load() ) {}
    std::string               fnc;
    std::string               lib;
    std::atomic<unsigned int> count{1};
  };

  void parseFilter( const std::string& str, std::string& fnc, std::string& lib ) const;
  bool failsFilter( const std::string& fnc, const std::string& lib ) const;

  Gaudi::Property<std::vector<std::string>> m_pFilter{this, "Filter"};
  Gaudi::Property<bool>                     m_abort{this, "AbortOnError", false};
  Gaudi::Property<bool>                     m_suppress{this, "SuppressCheck", false};
  Gaudi::Property<bool>                     m_dict{this, "IgnoreDicts", true};

  tbb::concurrent_unordered_map<std::string, StatCodeDat> m_dat;
  std::set<std::string>                                   m_filterfnc, m_filterlib;
};

#endif
