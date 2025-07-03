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
#include <Gaudi/Interfaces/IOptionsSvc.h>

#include <GaudiKernel/StatusCode.h>

#include <string>

// Class to translate between Python and C++ worlds
// It will be exposed to Python via Boost (see .cpp file)
class PythonAdaptor {
public:
  PythonAdaptor( Gaudi::Interfaces::IOptionsSvc* optsSvc ) : m_optsSvc( optsSvc ) {}
  void addPropertyToJobOptions( const std::string& client, const std::string& name, const std::string& value ) {
    m_optsSvc->set( client + '.' + name, value );
  }

private:
  Gaudi::Interfaces::IOptionsSvc* m_optsSvc;
};

// Helper class to be invoked by JobOptionsSvc
class PythonConfig {
public:
  PythonConfig( Gaudi::Interfaces::IOptionsSvc* optsSvc ) : m_optsSvc( optsSvc ) {}
  StatusCode evaluateConfig( const std::string& filename, const std::string& preAction, const std::string& postAction );

private:
  Gaudi::Interfaces::IOptionsSvc* m_optsSvc;
};
