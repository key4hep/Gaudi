/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDISVC_PYTHONCONFIG_H
#define GAUDISVC_PYTHONCONFIG_H
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/SmartIF.h"
#include <string>

// Class to translate between Python and C++ worlds
// It will be exposed to Python via Boost (see .cpp file)
class PythonAdaptor {
public:
  PythonAdaptor( IJobOptionsSvc* jos ) : m_IJobOptionsSvc( jos ) {}
  void addPropertyToJobOptions( const std::string& client, const std::string& name, const std::string& value ) {
    m_IJobOptionsSvc->addPropertyToCatalogue( client, Gaudi::Property<std::string>( name, value ) )
        .ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
  }

private:
  SmartIF<IJobOptionsSvc> m_IJobOptionsSvc;
};

// Helper class to be invoked by JobOptionsSvc
class PythonConfig {
public:
  PythonConfig( IJobOptionsSvc* jos ) : m_IJobOptionsSvc( jos ){};
  StatusCode evaluateConfig( const std::string& filename, const std::string& preAction, const std::string& postAction );

private:
  SmartIF<IJobOptionsSvc> m_IJobOptionsSvc;
};

#endif
