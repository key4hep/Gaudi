// $Id: ToolSvc.h,v 1.12 2008/06/02 14:21:35 marcocle Exp $
#ifndef GAUDISVC_TOOLSVC_H
#define GAUDISVC_TOOLSVC_H

// Include Files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IToolSvc.h"
#include <vector>

/** @class ToolSvc ToolSvc.h
 *  This service manages tools.
 *  Tools can be common, in which case a single instance
 *  can be shared by different algorithms, or private
 *  in which case it is necessary to specify the parent
 *  when requesting it.
 *  The parent of a tool can be an algortihm or a Service
 *  The environment of a tool is set by using that of the
 *  parent. A common tool is considered to belong to the
 *  ToolSvc itself.
 *
 *  @author G. Corti, P. Mato
 */
class ToolSvc : public extends1<Service, IToolSvc> {

public:

  // Typedefs
  typedef std::list<IAlgTool*>     ListTools;

  /// Initialize the service.
  virtual StatusCode initialize();

  /// Finalize the service.
  virtual StatusCode finalize();

  // Start transition for tools
  virtual StatusCode start();
  
  // Stop transition for tools
  virtual StatusCode stop();

  /// Retrieve tool, create it by default as common tool if it does not already exist
  virtual StatusCode retrieve(const std::string& type, const InterfaceID& iid,
                              IAlgTool*& tool, const IInterface* parent,
                              bool createIf);

  /// Retrieve tool, create it by default as common tool if it does not already exist
  virtual StatusCode retrieve(const std::string& tooltype, const std::string& toolname,
                              const InterfaceID& iid, IAlgTool*& tool,
                              const IInterface* parent, bool createIf);

  /// Get names of all tool instances of a given type
  virtual std::vector<std::string> getInstances( const std::string& toolType );

  /// Release tool
  virtual StatusCode releaseTool( IAlgTool* tool );

  /// Create Tool standard way with automatically assigned name
  StatusCode create(const std::string& type, const IInterface* parent,
                            IAlgTool*& tool);

  /// Create Tool standard way with specified name
  StatusCode create(const std::string& type, const std::string& name,
                            const IInterface* parent, IAlgTool*& tool);

  /// Check if the tool instance exists
  bool existsTool( const std::string& toolname) const;

  /// Get Tool full name by combining nameByUser and "parent" part
  std::string nameTool(const std::string& nameByUser, const IInterface* parent);

  /// Get current refcount for tool
  unsigned long refCountTool( IAlgTool* tool ) const { return tool->refCount(); }

  /** Standard Constructor.
   *  @param  name   String with service name
   *  @param  svc    Pointer to service locator interface
   */
  ToolSvc( const std::string& name, ISvcLocator* svc );

  /// Destructor.
  virtual ~ToolSvc();

  virtual void registerObserver(IToolSvc::Observer *obs) ;
  virtual void unRegisterObserver(IToolSvc::Observer *obs) ;


private: // methods

  // helper functions
  //
  /** The total number of refCounts on all tools in the instancesTools list */
  unsigned long totalToolRefCount() const;
  /** The total number of refCounts on all tools in the list */
  unsigned long totalToolRefCount( const ListTools& ) const;
  /** The minimum number of refCounts of all tools */
  unsigned long minimumToolRefCount() const;

  /// Finalize the given tool, with exception handling
  StatusCode finalizeTool( IAlgTool* itool ) const;

private: // data

  /// Common Tools
  ListTools    m_instancesTools;        // List of all instances of tools

  /// Pointer to HistorySvc
  IHistorySvc* m_pHistorySvc;

  std::vector<IToolSvc::Observer*> m_observers;
};

#endif


