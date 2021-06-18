.. include:: ./global.rst

|newpage|

|Gaudi logo|

.. _chapTool:

Tools and ToolSvc
====================

Overview
-------------

    Tools are light weight objects whose purpose is to help other
    components perform their work. A framework service, the ToolSvc, is
    responsible for creating and managing Tools. An Algorithm requests
    the tools it needs to the ToolSvc, specifying if requesting a
    private instance by declaring itself as the parent. Since Tools are
    managed by the ToolSvc, any component [#]_ can
    request a tool. Algorithms, Services and other Tools can declare
    themselves as Tools parents.

    In this chapter we first describe these objects and the difference
    between "private" and "shared" tools. We then look at the AlgTool
    base class and how to write concrete Tools.

    In section :numref:`tool-serv` we describe the ToolSvc
    and show how a component can retrieve Tools via the service.

    Finally we describe Associators, common utility GaudiTools for which
    we provide the interface and base class.

Tools and Services
-----------------------

    As mentioned elsewhere Algorithms make use of framework services to
    perform their work. In general the same instance of a service is
    used by many algorithms and Services are setup and initialized once
    at the beginning of the job by the framework. Algorithms also
    delegate some of their work to sub-algorithms. Creation and
    execution of sub-algorithms are the responsibilities of the parent
    algorithm whereas the initialize() and finalize() methods are
    invoked automatically by the framework while initializing the parent
    algorithm. The properties of a sub-algorithm are automatically set
    by the framework but the parent algorithm can change them during
    execution. Sharing of data between nested algorithms is done via the
    Transient Event Store.

    Both Services and Algorithms are created during the initialization
    stage of a job and live until the jobs ends.

    Sometimes an encapsulated piece of code needs to be executed only
    for specific events, in which case it is desirable to create it only
    when necessary. On other occasions the same piece of code needs to
    be executed many times per event. Moreover it can be necessary to
    execute a sub-algorithm on specific contained objects that are
    selected by the parent algorithm or have the sub-algorithm produce
    new contained objects that may or may not be put in the Transient
    Store. Finally different algorithms may wish to configure the same
    piece of code slightly differently or share it as-is with other
    algorithms.

    To provide this kind of functionality we have introduced a category
    of processing objects that encapsulate these "light" algorithms. We
    have called this category Tools.

    Some examples of possible tools are single track fitters,
    association to Monte Carlo truth information, vertexing between
    particles, smearing of Monte Carlo quantities.

"Private" and "Shared" Tools
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Algorithms can share instances of Tools with other Algorithms if the
    configuration of the tool is suitable. In some cases however an
    Algorithm will need to customize a tool in a specific way in order
    to use it. This is possible by requesting the ToolSvc to provide a
    "private" instance of a tool.

    If an Algorithm passes a pointer to itself when it asks the ToolSvc
    to provide it with a tool, it is declaring itself as the parent and
    a "private" instance is supplied. Private instances can be
    configured according to the needs of each particular Algorithm.

    As mentioned above many Algorithms can use a tool as-is, in which
    case only one instance of a Tool is created, configured and passed
    by the ToolSvc to the different algorithms. This is called a
    "shared" instance. The parent of "shared" tools is the ToolSvc.

The Tool classes
~~~~~~~~~~~~~~~~~~~~~~~

.. _tool-atbc:

The AlgTool base class
```````````````````````````````

    The main responsibilities of the AlgTool base class (see `Listing
    12.1 <GDG_Tools.html#1166507>`__) are the identification of the
    tools instances, the initialisation of certain internal pointers
    when the tool is created and the management of the tools properties.
    The AlgTool base class also offers some facilities to help in the
    implementation of derived tools and management of the additional
    tools interfaces..

    .. was 12.1

    .. code-block:: cpp
        :name: defalgbaseclass
        :caption: The definition of the AlgTool Base class. Highlighted in bold are methods relevant for the implementation of concrete tools

        class AlgTool : public virtual IAlgTool,
                        public virtual IProperty {

        public:

            // Standard Constructor.
            AlgTool( const std::string& type,
                     const std::string& name,
                    const IInterface* parent);

            ISvcLocator* serviceLocator() const;
            IMessageSvc* msgSvc() const;

            virtual StatusCode setProperty( const Property& p );
            virtual StatusCode setProperty( std::istream& s );
            virtual StatusCode setProperty( const std::string& n, const std::string& v );
            virtual StatusCode getProperty(Property* p) const;
            virtual const Property& getProperty( const std::string& name ) const;
            virtual StatusCode getProperty( const std::string& n,std::string& v ) const;
            virtual const std::vector<Property*>& getProperties( ) const;

            StatusCode setProperties();

            template <class T>
            StatusCode declareProperty(const std::string& name, T& property) const

            virtual const std::string& name() const;
            virtual const std::string& type() const;
            virtual const IInterface* parent() const;

            virtual StatusCode initialize();
            virtual StatusCode finalize();

            virtual StatusCode queryInterface(const IID& riid, void** ppvUnknown);
            void declInterface( const IID&, void*);

            template <class I> class declareInterface {

            public:

                template <class T> declareInterface(T* tool)
            }

        protected:

            // Standard destructor.
            virtual ~AlgTool();
        };

    | Constructor
    |
    |  The base class has a single constructor which takes three arguments. The first is the type (i.e. the class) of the Tool object being instantiated, the second is the full name of the object and the third is a pointer to the IInterface of the parent component. The name is used for the identification of the tool instance as described below.The parent interface is used by the tool to access for example the outputLevel of the parent.
    |
    | Access to Services
    |
    |  A serviceLocator() method is provided to enable the derived tools to locate the services necessary to perform their jobs. Since concrete Tools are instantiated by the ToolSvc upon request, all Services created by the framework prior to the creation of a tool are available. In addition access to the message service is provided via the msgSvc() method. Both pointers are retrieved from the parent of the tool.
    |
    | Properties
    |
    |  A template method for declaring properties similarly to Algorithms is provided. This allows tuning of data members used by the Tools via JobOptions files. The ToolSvc takes care of calling the setProperties() method of the AlgTool base class after having instantiated a tool. Properties need to be declared in the constructor of a Tool. The property outputLevel is declared in the base class and is identically set to that of the parent component, unless specified otherwise in the JobOptions. For details on Properties see section :numref:`serv-prop`.
    |
    | IAlgTool Interface
    |
    |  It consists of three accessor methods for the identification and managment of the tools: type(), name() and parent(). These methods are all implemented by the base class and should not be overridden. Two additional methods, initialize() and finalize(), allow concrete tools to be configured after creation and orderly terminated before deletion. An empty implementation is provided by the AlgTool base class and concrete tools need to implement these methods only when relevant for their purpose. The ToolSvc is responsible for calling these methods at the appropriate time.
    |
    | Tools Interfaces
    |
    |  Concrete tools must implement additional interfaces that will inherit from IAlgTool. When a component implements more that one interface it is necessary to "recognize" the various interfaces. This is taken care of by the AlgTool base class once the additional interface is declared by a concrete tool (or tools' base class). The declaration of the additional interface must be done in the constructor of a concrete tool and is done via the template method declareInterface.

Tools identification
`````````````````````````````

    A tool instance is identified by its full name. The name consist of
    the concatenation of the parent name, a dot, and a tool dependent
    part. The tool dependent part can be specified by the user, when not
    specified the tool type (i.e. the class) is automatically taken as
    the tool dependent part of the name. Examples of tool names are
    RecPrimaryVertex.VertexSmearer (a private tool) and
    ToolSvc.AddFourMom (a shared tool). The full name of the tool has to
    be used in the jobOptions file to set its properties.

Concrete tools classes
```````````````````````````````

    Operational functionalities of tools must be provided in the derived
    tool classes. A concrete tool class must inherit directly or
    indirectly from the AlgTool base class to ensure that it has the
    predefined behaviour needed for management by the ToolSvc.

    Concrete tools must implement additional interfaces, specific to the
    task a tool is designed to perform. Specialised tools intended to
    perform similar tasks can be derived from a common base class that
    will provide the common functionality and implement the common
    interface. Consider as example the vertexing of particles, where
    separate tools can implement different algorithms but the arguments
    passed are the same. The ToolSvc interacts with specialized tools
    only through the additional tools interface, therefore the interface
    itself must inherit from the IAlgTool interface in order for the
    tool to be correctly managed by the ToolSvc.

    The inheritance structure of derived tools is shown in :numref:`fig-tools`. ConcreteTool1 implements one
    additional abstract interface while ConcreteTool2 and ConcreteTool3
    derive from a base class SubTool that provides them with additional
    common functionality.

    .. figure:: images/ToolsHierarchy.png
        :name: fig-tools

        Tools class hierarchy

Implementation of concrete tools
`````````````````````````````````````````

    An example minimal implementation of a concrete tool is shown in
    :numref:`exconctooladdint`, :numref:`exconctooladdintheader` and
    :numref:`exconctooladdintcpp`, taken from the LHCb ToolsAnalysis
    example application

    .. was 12.2

    .. code-block:: cpp
        :name: exconctooladdint
        :caption: Example of a concrete tool additional interface

        static const InterfaceID IID_IVertexSmearer("IVertexSmearer", 1 , 0);

        class IVertexSmearer : virtual public IAlgTool {
        public:

            /// Retrieve interface ID
            static const InterfaceID& interfaceID() { return IID_IVertexSmearer; }
            // Actual operator function
            virtual StatusCode smear( MyAxVertex* ) = 0;
        };

    .. was 12.3

    .. code-block:: cpp
        :name: exconctooladdintheader
        :caption: Example of a concrete tool minimal implementation header file

        #include "GaudiKernel/AlgTool.h"

        class VertexSmearer : public AlgTool, virtual public IVertexSmearer {
        public:

            // Constructor
            VertexSmearer( const std::string& type,
                           const std::string& name,
                           const IInterface* parent);
            // Standard Destructor
            virtual ~VertexSmearer() { }
            // specific method of this tool
            StatusCode smear( MyAxVertex* pvertex );
        };

    .. was 12.4

    .. code-block:: cpp
        :name: exconctooladdintcpp
        :caption: Example of a concrete tool minimal implementation file

        #include "GaudiKernel/ToolFactory.h"
        // Static factory for instantiation of algtool objects
        static ToolFactory<VertexSmearer> s_factory;
        const IToolFactory& VertexSmearerFactory = s_factory;

        // Standard Constructor
        VertexSmearer::VertexSmearer( const std::string& type,
                                      const std::string& name,
                                      const IInterface* parent) : AlgTool( type, name, parent ) {

            // Locate service needed by the specific tool
            m_randSvc = 0;

            if( serviceLocator() ) {

                StatusCode sc=StatusCode::FAILURE;
                sc = serviceLocator()->service( "RndmGenSvc", m_randSvc, true );
            }

            // Declare additional interface
            declareInterface<IVertexSmearer>(this);

            // Declare properties of the specific tool
            declareProperty("dxVtx", m_dxVtx = 9 * micrometer);
            declareProperty("dyVtx", m_dyVtx = 9 * micrometer);
            declareProperty("dzVtx", m_dzVtx = 38 * micrometer);
        }

        // Implement the specific method ....
        StatusCode VertexSmearer::smear( MyAxVertex* pvertex ) {...}


    The creation of concrete tools is similar to that of Algorithms,
    making use of a Factory Method. As for Algorithms, Tool factories
    enable their creator to instantiate new tools without having to
    include any of the concrete tools header files. A template factory
    is provided and a tool developer will only need to add the concrete
    factory in the implementation file as shown in lines 1 4 of
    :numref:`exconctooladdintcpp`

    In addition a concrete tool class must specify a single constructor
    with the same parameter signatures as the constructor of the AlgTool
    base class as shown in line 5 of :numref:`exconctooladdintheader`.

    Below is the minimal checklist of the code necessary when developing
    a Tool:

    1. Define the specific interface (inheriting from the IAlgTool interface).
    2. Derive the tool class from the AlgTool base class
    3. Provide the constructor
    4. Declare the additional interface in the constructor.
    5. Implement the factory adding the lines of code shown in :numref:`exconctooladdintcpp`
    6. Implement the specific interface methods.

    In addition if a tool requires special initialization and
    termination you can implement the initialize and finalize methods.

.. _tool-serv:

The ToolSvc
----------------

    The ToolSvc manages Tools. It is its responsibility to create tools,
    configure them, make them available to Algorithms or Services and
    terminate them in an orderly fashion before deleting them.

    The ToolSvc verifies if a tool type is available and creates the
    necessary instance after having verified if it doesn't already
    exist. If a tool instance exists the ToolSvc will not create a new
    identical one but pass to the algorithm the existing instance. Tools
    are created on a "first request" basis: the first Algorithm
    requesting a tool will prompt its creation. The relationship between
    an algorithm, the ToolSvc and Tools is shown in :numref:`fig-toolsdia`.

    .. figure:: images/ToolsDiagram.png
        :name: fig-toolsdia

        ToolSvc design diagram

    Immediately after having created a tool, the ToolSvc will configure
    it by setting its properties and calling the tool initialize()
    method.

    The ToolSvc will "hold" a tool until it is no longer used by any
    component or until the finalize() method of the tool service is
    called. Algorithms can inform the ToolSvc they are not going to use
    a tool previously requested via the releaseTool method of the
    IToolSvc interface. Before deleting the tools the ToolSvc will
    cleanly terminate them by calling their finalize() method.

    The ToolSvc is created by default by the ApplicationMgr and
    algorithms wishing to use the service can do so via the algorithm
    toolSvc() accessor method. Services and AlgTools need to retrieve it
    using the serviceLocator() method of their respective base classes.

Retrieval of tools via the IToolSvc interface
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    The IToolSvc interface is the ToolSvc specific interface providing
    methods to retrieve tools. The interface has two retrieve methods
    that differ in their parameters signature, as shown in :numref:`itoolsvcmeths`

    .. was 12.5

    .. code-block:: cpp
        :name: itoolsvcmeths
        :caption: The IToolSvc interface methods

        virtual StatusCode retrieve( const std::string& type,
                                     const IID&,
                                     IAlgTool*& tool,
                                     const IInterface* parent=0,
                                     bool createIf=true ) = 0;
        virtual StatusCode retrieve( const std::string& type,
                                     const IID&,
                                     const std::string& name,
                                     IAlgTool*& tool,
                                     const IInterface* parent=0,
                                     bool createIf=true ) = 0;


    The arguments of the method shown in :numref:`itoolsvcmeths` line 1,
    are the tool type (i.e. the class),
    the tool additional interface ID and the IAlgTool interface of the
    returned tool. In addition there are two arguments with default
    values: one is the IInterface of the component requesting the tool,
    the other a boolean creation flag. If the component requesting a
    tool passes a pointer to itself as the third argument, it declares
    to the ToolSvc that it is asking for a "private" instance of the
    tool. By default a "shared" instance is provided. In general if the
    requested instance of a Tool does not exist the ToolSvc will create
    it. This behaviour can be changed by setting to false the last
    argument of the method.

    The method shown in :numref:`itoolsvcmeths`, line 2
    differs from the one shown in line 1 by an extra argument, a string
    specifying the tool dependent part of the full tool name. This
    enables a component to request two separately configurable instances
    of the same tool.

    When retriving concrete tools, it is recommended to use the two
    templated functions provided in the IToolSvc interface file which
    are shown in :numref:`itoolsvctemplmeths`.

    .. was 12.6

    .. code-block:: cpp
        :name: itoolsvctemplmeths
        :caption: The IToolSvc template methods

        template <class T>
        StatusCode retrieveTool( const std::string& type,
                                 T*& tool,
                                 const IInterface* parent=0,
                                 bool createIf=true ) {...}
        template <class T>
        StatusCode retrieveTool( const std::string& type,
                                 const std::string& name,
                                 T*& tool,
                                 const IInterface* parent=0,
                                 bool createIf=true ) {...}

    The two template methods correspond to the IToolSvc retrieve methods
    but have the tool returned as a template parameter. Using these
    methods the component retrieving a tool avoids explicit
    dynamic-casting to specific additional interfaces or to derived
    classes.

    :numref:`exretrshrdtool` shows an example of retrieval of a shared and of a common tool.

    .. was 12.7

    .. code-block:: cpp
        :name: exretrshrdtool
        :caption: Example of retrieval by an algorithm of a shared tool in line 4 and of a private tool in line 10

        // Example of tool belonging to the ToolSvc and shared between algorithms
        StatusCode sc;
        sc = toolsvc()->retrieveTool("AddFourMom", m_sum4p );
        if( sc.isFailure() ) {
            log << MSG::FATAL << " Unable to create AddFourMom tool" << endreq;
            return sc;
        }
        // Example of private tool
        sc = toolsvc()->retrieveTool("ImpactPar", m_ip, this );
        if( sc.isFailure() ) {
             log << MSG::FATAL << " Unable to create ImpactPar tool" << endreq;
            return sc;
        }

GaudiTools
---------------

    In general concrete tools are specific to applications or detectors'
    code but there are some tools of common utility for which interfaces
    and base classes can be provided. The Associators described below
    and contained in the GaudiTools package are one of such tools.

Associators
~~~~~~~~~~~~~~~~~~

    When working with Monte Carlo data it is often necessary to compare
    the results of reconstruction or physics analysis with the original
    corresponding Monte Carlo quantities on an event-by-event basis as
    well as on a statistical level.

    Various approaches are possible to implement navigation from
    reconstructed simulated data back to the Monte Carlo truth
    information. Each of the approaches has its advantages and could be
    more suited for a given type of event data or data-sets. In addition
    the reconstruction and physics analysis code should treat simulated
    data in an identical way to real data.

    In order to shield the code from the details of the navigation
    procedure, and to provide a uniform interface to the user code, a
    set of Gaudi Tools, called Associators, has been introduced. The
    user can navigate between any two arbitrary classes in the Event
    Model using the same interface as long as a corresponding associator
    has been implemented. Since an Associator retrieves existing
    navigational information, its actual implementation depends on the
    Event Model and how the navigational information is stored. For some
    specific Associators, in addition, it can depend on some algorithmic
    choices: consider as an example a physics analysis particle and a
    possible originating Monte Carlo particle where the associating
    discriminant could be the fractional number of hits used in the
    reconstruction of the tracks. An advantage of this approach is that
    the implementation of the navigation can be modified without
    affecting the reconstruction and analysis algorithms because it
    would affect only the associators. In addition short-cuts or
    complete navigational information can be provided to the user in a
    transparent way. By limiting the use of such associators to
    dedicated monitoring algorithms where the comparison between
    raw/reconstructed data and MC truth is done, one could ensure that
    the reconstruction and analysis code treat simulated and real data
    in an identical way.

    Associators must implement a common interface called IAssociator. An
    Associator base class providing at the same time common
    functionality and some facilities to help in the implementation of
    concrete Associators is provided. A prototype version of these
    classes is provided in the current release of Gaudi.

The IAssociator Interface
``````````````````````````````````

    As already mentioned Associators must implement the IAssociator
    interface.

    In order for Associators to be retrieved from the ToolSvc only via
    the IAssociator interface, the interface itself inherits from the
    IAlgTool interface. While the implementation of the IAlgTool
    interface is done in the AlgTool base class, the implementation of
    the IAssociator interface is the full responsibility of concrete
    associators.

    The four methods of the IAssociator interface that a concrete
    Associator must implement are show in :numref:`iassocmeths`

    .. was 12.8
    .. :name: iassocmeths
    .. :caption: Methods of the IAssociator Interface that must be implemented by concrete associators

    .. code-block:: cpp
        :name: iassocmeths
        :caption: Methods of the IAssociator Interface that must be implemented by concrete associators

        virtual StatusCode i_retrieveDirect( ContainedObject* objFrom,
                                             std::vector<ContainedObject*>& vObjTo,
                                             const CLID idFrom,
                                             const CLID idTo ) = 0;

        virtual StatusCode i_retrieveInverse( ContainedObject* objFrom,
                                              ContainedObject*& objTo,
                                              const CLID idFrom,
                                              const CLID idTo) = 0;

        virtual StatusCode i_retrieveInverse( ContainedObject* objFrom,
                                              std::vector<ContainedObject*>& vObjTo,
                                              const CLID idFrom,
                                              const CLID idTo) = 0;

    Two i\_retrieveDirect methods must be implemented for retrieving
    associated classes following the same direction as the links in the
    data: for example from reconstructed particles to Monte Carlo
    particles. The first parameter is a pointer to the object for which
    the associated Monte Carlo quantity(ies) is requested. The second
    parameter, the discriminating signature between the two methods, is
    one or a vector of pointers to the associated Monte Carlo objects of
    the type requested. Some reconstructed quantities will have only one
    possible Monte Carlo associated object of a certain type, some will
    have many, others will have many out of which a "best" associated
    object can be extracted. If one of the two methods is not valid for
    a concrete associator, such method must return a failure. The third
    and fourth parameters are the class IDs of the objects for which the
    association is requested. This allows to verify at run time if the
    objects' types are those the concrete associator has been
    implemented for.

    The two i\_retrieveInverse methods are complementary and are for
    retrieving the association between the same two classes but in the
    opposite direction to that of the links in the data: for example
    from Monte Carlo particles to reconstructed particles. The different
    name is intended to alert the user that navigation in this direction
    may be a costly operation



    Four corresponding template methods are implemented in IAssociator
    to facilitate the use of Associators by Algorithms
    (see :numref:`iassoctemplmeth`). Using these methods the component
    retrieving a tool avoids some explicit dynamic-casting as well as
    the setting of class IDs. An example of how to use such methods is
    described in section :numref:`tool-iaex`

    .. was 12.9

    .. code-block:: cpp
        :name: iassoctemplmeth
        :caption: Template methods of the IAssociator interface

        template <class T1, class T2>
        StatusCode retrieveDirect( T1* from, T2*& to ) { // ...
                                                       }

        template <class T1>
        StatusCode retrieveDirect( T1* from,
                                   std::vector<ContainedObject*>& objVTo,
                                   const CLID idTo ) { // ...
                                                     }

        template <class T1, class T2>
        StatusCode retrieveInverse( T1* from, T2*& to ) { // ...
                                                        }

        template <class T1>
        StatusCode retrieveInverse( T1* from,
                                    std::vector<ContainedObject*>& objVTo,
                                    const CLID idTo ) { // ...
                                                      }


The Associator base class
``````````````````````````````````

    An associator is a type of AlgTool,so the Associator base class
    inherits from the AlgTool base class. Thus, Associators can be
    created and managed as AlgTools by the ToolSvc. Since all the
    methods of the AlgTool base class (as described in section
    :numref:`tool-atbc`) are available in the
    Associator base class, only the additional functionality is
    described here.

    | Access to Event Data Service
    |
    |  An eventSvc() method is provided to access the Event Data Service since most concrete associators will need to access data, in particular if accessing navigational short-cuts.
    |
    | Associator Properties
    |
    |  Two properties are declared in the constructor and can be set in the jobOptions: "FollowLinks" and "DataLocation". They are respectively a bool with initial value true and a std::string with initial value set to " ". The first is foreseen to be used by an associator when it is possible to either follow links between classes or retrieve navigational short cuts from the data. A user can choose to set either behaviour at run time. The second property contains the location in the data where the stored navigational information is located. Currently it must be set via the jobOptions when necessary, as shown in :numref:`exsetpropassjo` for a particular implementation provided in the Associator example. Two corresponding methods are provided for using the information from these properties: followLinks() and whichTable().
    |
    | Inverse Association
    |
    |  Retrieving information in the direction opposite to that of the links in the data is in general a time consuming operation, that implies checking all the direct associations to access the inverse relation for a specified object. For this reason Associators should keep a local copy of the inverse associations after receiving the first request for an event. A few methods are provided to facilitate the work of Associators in this case. The methods inverseExist() and setInverseFlag(bool) help in keeping track of the status of the locally kept inverse information.The method buildInverse() has to be overridden by concrete associators since they choose in which form to keep the information and should be called by the associator when receiving the first request during the processing of an event.
    |
    | Locally kept information
    |
    |  When a new event is processed, the associator needs to reset its status to the same conditions as those after having been created. In order to be notified of such an incident happening the Associator base class implements the IListener interface and, in the constructor, registers itself with the Incident Service (see section :numref:`serv-inci` for details of the Incident Service). The associator's flushCache() method is called in the implementation of the IListener interface in the Associator base class. This method must be overridden by concrete associators wanting to do a meaningful reset of their initial status.

.. _tool-iaex:

A concrete example
```````````````````````````

    In this section we look at an example implementation of a specific
    associator. The code is taken from the LHCb Associator example, but
    the points illustrated should be clear even without a knowledge of
    the LHCb data model.

    The AxPart2MCParticleAsct provides association between physics
    analysis particles (AxPartCandidate) and the corresponding Monte
    Carlo particles (MCParticle). The direct navigational information is
    stored in the persistent data as short-cuts, and is retrieved in the
    form of a SmartRefTable in the Transient Event Store. This choice is
    specific to AxPart2MCParticleAsct, any associator can use internally
    a different navigational mechanism. The location in the Event Store
    where the navigational information can be found is set in the job
    options via the "DataLocation" property, as shown in `Listing
    12.10 <GDG_Tools.html#1110534>`__.

    .. was 12.10

    .. code-block:: cpp
        :name: exsetpropassjo
        :caption: Example of setting properties for an associator via jobOptions

        ToolSvc.AxPart2MCParticleAsct.DataLocation = "/Event/Anal/AxPart2MCParticle";

    In the current LHCb data model only a single MCParticle can be
    associated to one AxPartCandidate and vice-versa only one or no
    AxPartCandidate can be associated to one MCParticle. For this reason
    only the i\_retrieveDirect and i\_retrieveInverse methods providing
    one-to-one association are meaningful. Both methods verify that the
    objects passed are of the correct type before attempting to retrieve
    the information, as shown in :numref:`checkobjsassoctyp`. When no association is found, a
    StatusCode::FAILURE is returned.

    .. was 12.11

    .. code-block:: cpp
        :name: checkobjsassoctyp
        :caption: Checking if objects to be associated are of the correct type

        if ( idFrom != AxPartCandidate::classID() ) {
            objTo = 0;
            return StatusCode::FAILURE;
        }
        if ( idTo != MCParticle::classID() ) {
            objTo = 0;
            return StatusCode::FAILURE;
        }

    The i\_retrieveInverse method providing the one-to-many association
    returns a failure, while a fake implementation of the one-to-many
    i\_retrieveDirect method is implemented in the example, to show how
    an Algorithm can use such a method. In the AxPart2MCParticleAsct
    example the inverse table is kept locally and both the
    buildInverse() and flushCache() methods are overridden. In the
    example the choice has been made to implement an additional method
    buildDirect() to retrieve the direct navigational information on a
    first request per event basis.

    :numref:`codeasctexalg` shows how a monitoring
    Algorithm can get an associator from the ToolSvc and use it to
    retrieve associated objects through the template interfaces.

    .. was 12.12

    .. code-block:: cpp
        :name: codeasctexalg
        :caption: Extracted code from the AsctExampleAlgorithm

        #include "GaudiTools/IAssociator.h"

        // Example of retrieving an associator IAssociator
        StatusCode sc = toolsvc()->retrieveTool("AxPart2MCParticleAsct", m_pAsct);
        if( sc.isFailure() ) {
            log << MSG::FATAL << "Unable to create Associator tool" << endreq;
            return sc;
        }

        // Example of retrieving inverse one-to-one information from an associator
        SmartDataPtr<MCParticleVector> vmcparts (evt,"/MC/MCParticles");
        for( MCParticleVector::iterator itm = vmcparts->begin(); vmcparts->end() != itm; itm++) {
            AxPartCandidate* mptry = 0;
            StatusCode sc = m_pAsct->retrieveInverse( *itm, mptry );
            if( sc.isSuccess() ) {...}
            else {...}
        }

        // Example of retrieving direct one-to-many information from an associator
        SmartDataPtr<AxPartCandidateVector> candidates(evt, "/Anal/AxPartCandidates");
        std::vector<ContainedObject*> pptry;
        AxPartCandidate* itP = *(candidates->begin());
        StatusCode sa =   m_pAsct->retrieveDirect(itP, pptry, MCParticle::classID());
        if( sa.isFailure() ) {...}
        else {
            for (std::vector<ContainedObject*>::iterator it = pptry.begin(); pptry.end() != it; it++ ) {
                MCParticle* imc = dynamic_cast<MCParticle*>( *it );
            }
        }

.. [#] In this chapter we will use an Algorithm as example component requesting tools.
