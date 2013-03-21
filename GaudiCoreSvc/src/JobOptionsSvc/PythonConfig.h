#ifndef GAUDISVC_PYTHONCONFIG_H
#define GAUDISVC_PYTHONCONFIG_H
#include <string>
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/SmartIF.h"

// Class to translate between Python and C++ worlds
// It will be exposed to Python via Boost (see .cpp file)
class PythonAdaptor {
public:
  PythonAdaptor(IJobOptionsSvc* jos) : 
    m_IJobOptionsSvc(jos){}
  void addPropertyToJobOptions(const std::string &client, const std::string &name, const std::string &value){
      m_IJobOptionsSvc->addPropertyToCatalogue(client, StringProperty(name,value) );      
  }
private:
    SmartIF<IJobOptionsSvc> m_IJobOptionsSvc;
};

// Helper class to be invoked by JobOptionsSvc 
class PythonConfig {
public:
  PythonConfig(IJobOptionsSvc* jos) : 
    m_IJobOptionsSvc(jos){};
  StatusCode readConfig(const std::string& filename);
private:
  SmartIF<IJobOptionsSvc> m_IJobOptionsSvc;
};

#endif
