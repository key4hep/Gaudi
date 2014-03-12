#ifndef GAUDISVC_DATAONDEMANDSVC_H
#define GAUDISVC_DATAONDEMANDSVC_H
// ============================================================================
// Include Files
// ============================================================================
// STD & STL
// ============================================================================
#include <map>
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/ChronoEntity.h"
#include "GaudiKernel/StatEntity.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/IDODAlgMapper.h"
#include "GaudiKernel/IDODNodeMapper.h"
// ============================================================================
// ROOT TClass
// ============================================================================
#include "TClass.h"
// ============================================================================
// Forward declarations
// ============================================================================
class IAlgTool;
class IAlgorithm;
class IAlgManager;
class IIncidentSvc;
class IDataProviderSvc;
class IToolSvc;
// ============================================================================
/** @class DataOnDemandSvc DataOnDemandSvc.h
 *
 * The DataOnDemandSvc listens to incidents typically
 * triggered by the data service of the configurable name
 * "IncidentName".
 * In the job options handlers can be declared, which allow
 * to configure this service. Such handlers are either:
 *
 * - Node handlers, if objects other than the default object
 *   type have to be instantiated.
 *   DataOnDemandSvc.Nodes = {
 *     "DATA='/Event/Rec'      TYPE='DataObject'",
 *     "DATA='/Event/Rec/Muon' TYPE='DataObject'"
 *   };
 *
 * - Leaf handlers (Algorithms), which get instantiated
 *   and executed on demand.
 *   DataOnDemandSvc.Algorithms = {
 *     "DATA='/Event/Rec/Muon/Digits' TYPE='MuonDigitAlg/MyMuonDigits'"
 *   };
 *   If the algorithm name is omitted the class name will be the
 *   instance name.
 *
 * The handlers only get called if the exact path matches.
 * In the event already the partial path to any handler is
 * missing a leaf handler may be triggered, which includes
 * the partial paths ( DataOnDemandSvc.UsePreceedingPath = true )
 *
 *
 *  2006-10-15: New options  (using map-like semantics:)
 *
 *  @code
 *
 *    DataOnDemandSvc.AlgMap  +=
 *   { "Phys/StdLoosePions/Particles" : "PreLoadParticles/StdLoosePions" ,
 *     "Phys/StdLoosePions/Vertioces" : "PreLoadParticles/StdLoosePions" } ;
 *
 *    DataOnDemandSvc.NodeMap +=
 *   { "Phys" : "DataObject" ,
 *     "MC"   : "DataObject" } ;
 *
 *  @endcode
 *
 *    New treatment of preceding paths. for each registered leaf or node
 *    the all parent nodes are added into the node-map with default
 *    directory type 'DataObject'
 *
 *    The major properties are equipped with handlers
 *    (more or less mandatory for interactive work in python)
 *
 *    From now the default prefix ( "/Event/" ) could be omitted from
 *    any data-item. It will be added automatically.
 *
 * @author  M.Frank
 * @version 1.0
 */
class DataOnDemandSvc: public extends1<Service, IIncidentListener>
{
public:
  // ==========================================================================
  // Typedefs
  typedef std::vector<std::string> Setup;
  typedef TClass*                  ClassH;
  // ==========================================================================
  /** @struct Protection
   *  Helper class of the DataOnDemandSvc
   *  @author  M.Frank
   */
  struct Protection
  {
    bool& m_bool;
    Protection(bool& b) : m_bool(b) { m_bool = true;  }
    ~Protection()                   { m_bool = false; }
  };
  // ==========================================================================
  /** @struct Node
   *  Helper class of the DataOnDemandSvc
   *  @author  M.Frank
   *  @version 1.0
   */
  struct Node
  {
    // ========================================================================
    /// the actual class
    ClassH        clazz      ;                              // the actual class
    bool          executing  ;
    std::string   name       ;
    unsigned long num        ;
    /// trivial object? DataObject?
    bool          dataObject ;                  // trivial object? DataObject?
    // =======================================================================
    Node()
      : clazz      (       )
      , executing  ( false )
      , name       (       )
      , num        ( 0     )
      , dataObject ( false )
    {}
    // ========================================================================
    Node ( ClassH             c ,
           bool               e ,
           const std::string& n )
      : clazz     ( c )
      , executing ( e )
      , name      ( n )
      , num       ( 0 )
      , dataObject ( "DataObject" == n )
    {}
    //
    Node( const Node& c )
      : clazz      ( c.clazz      )
      , executing  ( c.executing  )
      , name       ( c.name       )
      , num        ( c.num        )
      , dataObject ( c.dataObject )
    {}
    // ========================================================================
  };
  // ==========================================================================
  /// @struct Leaf
  struct Leaf
  {
    IAlgorithm*   algorithm ;
    bool          executing ;
    std::string   name      ;
    std::string   type      ;
    unsigned long num       ;
    Leaf() : algorithm ( 0 ) , executing (false ) , name() , type() , num ( 0 ) {}
    Leaf(const Leaf& l)
      : algorithm(l.algorithm),
        executing(l.executing), name(l.name), type(l.type), num(l.num)  {}
    Leaf(const std::string& t, const std::string& n)
      : algorithm(0), executing(false), name(n), type(t), num(0)  {}
  };
  // ==========================================================================
public:
  // ==========================================================================
  typedef GaudiUtils::HashMap<Gaudi::StringKey, Node>  NodeMap;
  typedef GaudiUtils::HashMap<Gaudi::StringKey, Leaf>  AlgMap;
  /// Inherited Service overrides: Service initialization
  virtual StatusCode initialize();
  /// Inherited Service overrides: Service finalization
  virtual StatusCode finalize();
  /// Inherited Service overrides: Service reinitialization
  virtual StatusCode reinitialize();
  /// IIncidentListener interfaces overrides: incident handling
  virtual void handle(const Incident& incident);
  /** Standard initializing service constructor.
   *  @param   name   [IN]    Service instance name
   *  @param   svc    [IN]    Pointer to service locator
   *  @return Reference to DataOnDemandSvc object.
   */
  DataOnDemandSvc
  ( const std::string& name ,                    //       Service instance name
    ISvcLocator*       svc  ) ;                  //  Pointer to service locator
  /// Standard destructor.
  virtual ~DataOnDemandSvc();                            // Standard destructor
  // ==========================================================================
protected:
  // ==========================================================================
  /** Configure handler for leaf
   *  @param   leaf   [IN]    Reference to leaf handler
   *  @return StatusCode indicating success or failure
   */
   StatusCode configureHandler(Leaf& leaf);
  // ==========================================================================
  /** Execute leaf handler (algorithm)
   *  @param   tag    [IN]    Path to requested leaf
   *  @param   leaf   [IN]    Reference to leaf handler
   *  @return StatusCode indicating success or failure
   */
  StatusCode execHandler(const std::string& tag, Leaf& leaf);
  // ==========================================================================
  /** Execute node handler (simple object creation using seal reflection)
   *  @param   tag    [IN]    Path to requested leaf
   *  @param   node   [IN]    Reference to node handler
   *  @return StatusCode indicating success or failure
   */
  StatusCode execHandler(const std::string& tag, Node& node);
  // ==========================================================================
  /// Initialize node handlers
  StatusCode setupNodeHandlers();
  // ==========================================================================
  /// Initialize leaf handlers
  StatusCode setupAlgHandlers();
  // ==========================================================================
  /// Setup routine (called by (re-) initialize
  StatusCode setup();
  /// Internal method to initialize a node handler.
  void i_setNodeHandler(const std::string &name, const std::string &type);
  /// Internal method to initialize an algorithm handler.
  StatusCode i_setAlgHandler(const std::string &name, const Gaudi::Utils::TypeNameString &alg);
  // ==========================================================================
public:
  // ==========================================================================
  void update_1 ( Property& p ) ;
  void update_2 ( Property& p ) ;
  void update_3 ( Property& p ) ;
  /// update handler for 'Dump' property
  void update_dump ( Property& /* p */ ) ;// update handler for 'Dump' property
  // ==========================================================================
protected:
  // ==========================================================================
  /// update the handlers
  StatusCode update() ;
  /// get the message stream
  inline MsgStream& stream () const
  {
    if ( 0 == m_log ) { m_log = new MsgStream( msgSvc() , name() ) ; }
    return *m_log;
  }
  /** dump the content of DataOnDemand service
   *  @param level the printout level
   *  @param mode   the printout mode
   */
  void dump ( const MSG::Level level , const bool mode = true ) const ;
  // ==========================================================================
private:
  // ==========================================================================
  /// Incident service
  SmartIF<IIncidentSvc>     m_incSvc;
  /// Algorithm manager
  SmartIF<IAlgManager>      m_algMgr;
  /// Data provider reference
  SmartIF<IDataProviderSvc> m_dataSvc;
  /// Data provider reference
  SmartIF<IToolSvc>         m_toolSvc;
  /// Trap name
  std::string       m_trapType;
  /// Data service name
  std::string       m_dataSvcName;
  /// Flag to allow for the creation of partial leaves
  bool              m_partialPath;
  /// flag to force the printout
  bool              m_dump ;
  /// flag to warm up the configuration
  bool              m_init ;
  /// flag to allow DataOnDemand initialization to succeed even if the
  /// (pre)initialization of the algorithms fails (m_init).
  bool              m_allowInitFailure;
  /// Mapping to algorithms
  Setup             m_algMapping;
  /// Mapping to nodes
  Setup             m_nodeMapping;
  /// Map of algorithms to handle incidents
  AlgMap            m_algs;
  /// Map of "empty" objects to be placed as intermediate nodes
  NodeMap           m_nodes;
  //
  typedef std::map<std::string,std::string> Map ;
  /// the major configuration property { 'data' : 'algorithm' }
  Map                m_algMap          ; // { 'data' : 'algorithm' }
  /// the major configuration property  { 'data' : 'type' }
  Map                m_nodeMap         ; // { 'data' : 'type' }
  bool               m_updateRequired  ;
  std::string        m_prefix          ;
  mutable MsgStream* m_log             ;
  // ==========================================================================
  ChronoEntity       m_total           ;
  ulonglong          m_statAlg         ;
  ulonglong          m_statNode        ;
  ulonglong          m_stat            ;
  // ==========================================================================
  ChronoEntity       m_timer_nodes     ;
  ChronoEntity       m_timer_algs      ;
  ChronoEntity       m_timer_all       ;
  bool               m_locked_nodes    ;
  bool               m_locked_algs     ;
  bool               m_locked_all      ;
  // ==========================================================================
  std::vector<std::string> m_nodeMapTools;
  std::list<IDODNodeMapper *> m_nodeMappers;
  std::vector<std::string> m_algMapTools;
  std::list<IDODAlgMapper *> m_algMappers;
};
// ============================================================================

// ============================================================================
// The END
// ============================================================================
#endif // GAUDISVC_DATAONDEMANDSVC_H
// ============================================================================

