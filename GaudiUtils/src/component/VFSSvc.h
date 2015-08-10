#ifndef GaudiSvc_VFSSvc_H
#define GaudiSvc_VFSSvc_H 1

// Include files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IFileAccess.h"
#include "GaudiKernel/HashMap.h"

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

class VFSSvc: public extends1<Service, IFileAccess> {
public:
  /// Initialize Service
  StatusCode initialize() override;
  /// Finalize Service
  StatusCode finalize() override;

  /// @see IFileAccess::open
  std::unique_ptr<std::istream> open(const std::string &url) override;

  /// @see IFileAccess::protocols
  const std::vector<std::string> &protocols() const override;

  /// Standard constructor
  VFSSvc(const std::string& name, ISvcLocator* svcloc);

  ~VFSSvc() override = default; ///< Destructor

private:

  /// Names of the handlers to use
  std::vector<std::string> m_urlHandlersNames = { { {"FileReadTool"} } };

  /// Protocols registered
  std::vector<std::string> m_protocols;

  /// Protocol to use in case there is not a specific tool to handle the URL.
  std::string m_fallBackProtocol;

  /// Map of the tools handling the known protocols.
  GaudiUtils::HashMap<std::string,IFileAccess*> m_urlHandlers;

  /// Handle to the tool service.
  SmartIF<IToolSvc> m_toolSvc;

  /// List of acquired tools (needed to release them).
  std::vector<IAlgTool*> m_acquiredTools;

};

#endif
