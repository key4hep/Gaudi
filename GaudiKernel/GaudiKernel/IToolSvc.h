#ifndef GAUDIKERNEL_ITOOLSVC_H
#define GAUDIKERNEL_ITOOLSVC_H

#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/System.h"
#include <string>

// Forward declaration
class IAlgTool;

/** @class IToolSvc IToolSvc.h GaudiKernel/IToolSvc.h
 *
 * The interface implemented by the IToolSvc base class.
 *
 * @author G.Corti
 */
class GAUDI_API IToolSvc: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IToolSvc,2,0);

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
  virtual StatusCode retrieve ( const std::string& type            ,
                                const InterfaceID& iid             ,
                                IAlgTool*&         tool            ,
                                const IInterface*  parent   = 0    ,
                                bool               createIf = true ) = 0;

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
  virtual StatusCode retrieve ( const std::string& type            ,
                                const std::string& name            ,
                                const InterfaceID& iid             ,
                                IAlgTool*&         tool            ,
                                const IInterface*  parent   = 0    ,
                                bool               createIf = true ) = 0 ;

  /** Get all instance of tool by type
   *  @param toolType type of tool
   */
  virtual std::vector<std::string> getInstances( const std::string& toolType ) = 0;

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
  StatusCode retrieveTool ( const std::string& type            ,
                            T*&                tool            ,
                            const IInterface*  parent   = 0    ,
                            bool               createIf = true )
  {
    return retrieve( type,
                     T::interfaceID(),
                     (IAlgTool*&)tool,
                     parent,
                     createIf );
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
  StatusCode retrieveTool ( const std::string& type            ,
                            const std::string& name            ,
                            T*&                tool            ,
                            const IInterface*  parent   = 0    ,
                            bool               createIf = true )
  {
    return retrieve ( type,
                      name,
                      T::interfaceID(),
                      (IAlgTool*&)tool,
                      parent,
                      createIf );
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
    virtual ~Observer() {}
    virtual void onCreate(const IAlgTool*) {}
    virtual void onRetrieve(const IAlgTool*) {}
  };

  virtual void registerObserver(Observer *obs) = 0;
  virtual void unRegisterObserver(Observer *obs) = 0;


};


#endif // GAUDIKERNEL_ITOOLSVC_H
