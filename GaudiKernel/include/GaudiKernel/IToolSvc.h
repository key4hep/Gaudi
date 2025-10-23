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

#include <GaudiKernel/IInterface.h>
#include <GaudiKernel/System.h>
#include <functional>
#include <list>
#include <string>

// Forward declaration
class IAlgTool;

/** @class IToolSvc IToolSvc.h GaudiKernel/IToolSvc.h
 *
 * The interface implemented by the IToolSvc base class.
 *
 * @author G.Corti
 */
class GAUDI_API IToolSvc : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IToolSvc, 2, 1 );

  // Typedefs
  typedef std::list<IAlgTool*> ListTools;

  /** Retrieve tool with tool dependent part of the name automatically
   *  assigned. By default a tool will be created if it does not exist,
   *  unless otherwise specified. By default it will be a common tool
   *  unless a parent is specified. The parent of a common tool is
   *  automatically taken as the ToolSvc itself.
   *  @param type AlgTool type name
   *  @param iid  the unique interface identifier
   *  @param tool returned tool
   *  @param parent constant reference to the parent (def=none)
   *  @param createIf creation flag (def=create if not existing)
   */
  virtual StatusCode retrieve( std::string_view type, const InterfaceID& iid, IAlgTool*& tool,
                               const IInterface* parent = 0, bool createIf = true ) = 0;

  /** Retrieve tool with tool dependent part of the name specified
   *  by the requester. By default a tool will be created if it does
   *  not exist, unless otherwise specified. By default it will be
   *  a common tool unless a parent is specified. The parent of a
   *  common tool is automatically taken as the ToolSvc itself.
   *  @param type AlgTool type name
   *  @param name name to be assigned to tool dependent part of the name
   *  @param iid  the unique interface identifier
   *  @param tool returned tool
   *  @param parent constant reference to parent (def=none)
   * @param createIf creation flag (def=create if not existing)
   */
  virtual StatusCode retrieve( std::string_view type, std::string_view name, const InterfaceID& iid, IAlgTool*& tool,
                               const IInterface* parent = 0, bool createIf = true ) = 0;

  /** Get the names of all instances of tools of a given type.
   *  @param toolType type of tool
   */
  virtual std::vector<std::string> getInstances( std::string_view toolType ) = 0;

  /** Get the names all tool instances.
   */
  virtual std::vector<std::string> getInstances() const = 0;

  /** Get pointers to all tool instances.
   */
  virtual std::vector<IAlgTool*> getTools() const = 0;

  /** Release the tool
   *  @param tool to be released
   */
  virtual StatusCode releaseTool( IAlgTool* tool ) = 0;

  /** Retrieve specified tool sub-type with tool dependent part of the name
   *  automatically assigned. Internally it uses the corresponding
   *  IToolSvc::retrieve and does the dynamic casting.
   *
   *  @code
   *
   *  IToolSvc* svc  = ... ;
   *  IMyTool*  tool = 0   ;
   *  StatusCode sc = svc->retrieveTool ( "MyToolType" , tool ) ;
   *
   *  @endcode
   *  For this example public tool of type <c>'MyToolType'</c>
   *  will be retrieved from Tool Service (created on demand).
   *  The full name of the tool instance is set to be
   *  <c>ToolSvc.MyToolType</c>
   *
   *  @code
   *
   *  IToolSvc*   svc = ... ;
   *  IAlgorithm* alg = ... ;
   *  IMyTool*  tool  = 0   ;
   *  StatusCode sc   = svc->retrieveTool ( "MyToolType" , tool , alg ) ;
   *
   *  @endcode
   *  For this example the private tool of type <c>'MyToolType'</c>
   *  will be retrieved from Tool Service (created on demand).
   *  The full name of the tool instance is set to be
   *  <c><AlgName>.MyToolType </c>, where
   *  <c><AlgName></c> is a name of the algorithm.
   *
   *
   *  @code
   *
   *  IToolSvc* svc  = ... ;
   *  IMyTool*  tool = 0   ;
   *  StatusCode sc = svc->retrieveTool ( "MyToolType/MyToolName" , tool ) ;
   *
   *  @endcode
   *  For this example public tool of type <c>'MyToolType'</c>
   *  will be retrieved from Tool Service (created on demand).
   *  The full name of the tool instance is set to be
   *  <c>ToolSvc.MyToolName</c>
   *
   *  @code
   *
   *  IToolSvc*   svc = ... ;
   *  IAlgorithm* alg = ... ;
   *  IMyTool*  tool  = 0   ;
   *  StatusCode sc   = svc ->
   *                retrieveTool ( "MyToolType/MyToolName" , tool , alg ) ;
   *
   *  @endcode
   *  For this example the private tool of type <c>'MyToolType'</c>
   *  will be retrieved from Tool Service (created on demand).
   *  The full name of the tool instance is set to be
   *  <c>&lt;AlgName&gt;.MyToolName </c>, where
   *  <c>&lt;AlgName&gt;</c> is a name of the algorithm.
   *
   *  @param tool returned tool
   *  @param parent constant reference to parent (def=none)
   *  @param createIf creation flag (def=create if not existing)
   *
   */
  template <class T>
  StatusCode retrieveTool( std::string_view type, T*& tool, const IInterface* parent = nullptr, bool createIf = true ) {
    IAlgTool*  ialgtool{ nullptr };
    StatusCode sc = retrieve( type, T::interfaceID(), ialgtool, parent, createIf );
    tool          = Gaudi::Cast<T>( ialgtool );
    return sc;
  }

  /** Retrieve specified tool sub-type with tool dependent part of the name
   *  tool dependent part of the name specified by the requester.
   *  Internally it uses the corresponding IToolSvc::retrieve and does the
   *  dynamic casting.
   *
   *  @code
   *
   *  IToolSvc* svc  = ... ;
   *  IMyTool*  tool = 0   ;
   *  StatusCode sc = svc->retrieveTool ( "MyToolType" ,
   *                                      "MyToolName" , tool ) ;
   *
   *  @endcode
   *  For this example public tool of type <c>'MyToolType'</c>
   *  will be retrieved from Tool Service (created on demand).
   *  The full name of the tool instance is set to be
   *  <c>ToolSvc.MyToolName</c>
   *
   *  @code
   *
   *  IToolSvc*   svc = ... ;
   *  IAlgorithm* alg = ... ;
   *  IMyTool*  tool  = 0   ;
   *  StatusCode sc   = svc->retrieveTool ( "MyToolType" ,
   *                                        "MyToolName" , tool , alg ) ;
   *
   *  @endcode
   *  For this example the private tool of type <c>'MyToolType'</c>
   *  will be retrieved from Tool Service (created on demand).
   *  The full name of the tool instance is set to be
   *  <c>&lt;AlgName&gt;.MyToolName </c>, where
   *  <c>&lt;AlgName&gt;</c> is a name of the algorithm.
   *
   *  If <c>name</c> is empty (<c>""</c>) it is assumed to be equal to the
   *  <c>type</c>
   *
   *  @param type AlgTool type name
   *  @param name name to be assigned to tool dependent part of the name
   *  @param tool returned tool
   *  @param parent constant reference to parent (def=none)
   *  @param createIf creation flag (def=create if not existing)
   *
   */
  template <class T>
  StatusCode retrieveTool( std::string_view type, std::string_view name, T*& tool, const IInterface* parent = nullptr,
                           bool createIf = true ) {
    IAlgTool*  ialgtool{ nullptr };
    StatusCode sc = retrieve( type, name, T::interfaceID(), ialgtool, parent, createIf );
    tool          = Gaudi::Cast<T>( ialgtool );
    return sc;
  }

  /**  allow call-backs when a tool is a created
   *   or retrieved
   *
   *   @code
   *
   *   class myObserver : public ITooSvc::Observer {
   *        void onCreate(IAlgTool& tool) {
   *            cout << "tool " << tool.name() << " created " << endl;
   *        }
   *        void onRetrieve(IAlgTool& tool) {
   *            cout << "someone requested " << tool.name() <<endl;
   *        }
   *
   *   }
   *
   *   myObserver *observer = new myObserver;
   *   IToolSvc* svc = ...
   *   svc->registerObserver(observer);
   *
   *   @endcode
   *
   *   IToolSvc implementations will call 'Observer::onCreate' when
   *   a tool is created, and 'Observer::onRetrieve' if a tool is
   *   retrieved.
   *
   *   The user is responsible of the life time of the object and must
   *   un-register it before it is deleted (e.g. during the finalization).
   *
   */
  class Observer {
  public:
    virtual ~Observer() {
      if ( m_unregister ) m_unregister();
    }
    void setUnregister( std::function<void()> unregister ) { m_unregister = std::move( unregister ); }

    virtual void onCreate( const IAlgTool* ) {}
    virtual void onRetrieve( const IAlgTool* ) {}

  private:
    std::function<void()> m_unregister;
  };

  virtual void registerObserver( Observer* obs ) = 0;
};
