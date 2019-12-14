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
#ifndef GaudiSvc_VFSSvc_H
#define GaudiSvc_VFSSvc_H 1

// Include files
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/IFileAccess.h"
#include "GaudiKernel/Service.h"

#include <list>

// Forward declarations
class IToolSvc;
class IAlgTool;

/** @class VFSSvc VFSSvc.h
 *
 *  Simple service that allows to read files independently from the storage.
 *  The service uses tools to resolve URLs and serve the files as input streams.
 *  The basic implementations read from the filesystem, and simple extensions allow to
 *  read from databases, web...
 *
 *  @author Marco Clemencic
 *  @date   2008-01-18
 */

class VFSSvc : public extends<Service, IFileAccess> {
public:
  /// Inherited constructor
  using extends::extends;
  /// Initialize Service
  StatusCode initialize() override;
  /// Finalize Service
  StatusCode finalize() override;

  /// @see IFileAccess::open
  std::unique_ptr<std::istream> open( const std::string& url ) override;

  /// @see IFileAccess::protocols
  const std::vector<std::string>& protocols() const override;

private:
  Gaudi::Property<std::vector<std::string>> m_urlHandlersNames{
      this, "FileAccessTools", {{"FileReadTool"}}, "List of tools implementing the IFileAccess interface."};
  Gaudi::Property<std::string> m_fallBackProtocol{this, "FallBackProtocol", "file",
                                                  "URL prefix to use if the prefix is not present."};

  /// Protocols registered
  std::vector<std::string> m_protocols;

  /// Map of the tools handling the known protocols.
  GaudiUtils::HashMap<std::string, std::vector<IFileAccess*>> m_urlHandlers;

  /// Handle to the tool service.
  SmartIF<IToolSvc> m_toolSvc;

  /// List of acquired tools (needed to release them).
  std::vector<IAlgTool*> m_acquiredTools;
};

#endif
