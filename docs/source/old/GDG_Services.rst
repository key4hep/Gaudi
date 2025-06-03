.. include:: ./global.rst

|newpage|

|Gaudi logo|

.. _chapServ:

Framework services
=====================

Overview
-------------

    Services are generally sizeable components that are setup and
    initialized once at the beginning of the job by the framework and
    used by many algorithms as often as they are needed. It is not
    desirable in general to require more than one instance of each
    service. Services cannot have a "state" because there are many
    potential users of them so it would not be possible to guarantee
    that the state is preserved in between calls.

    In this chapter we describe how services are created and accessed,
    and then give an overview of the various services, other than the
    data access services, which are available for use within the Gaudi
    framework. The Job Options service, the Message service, the
    Particle Properties service, the Chrono & Stat service, the Auditor
    service, the Random Numbers service, the Incident service and the
    Introspection service are available in this release. The Tools
    service is described in :numref:`chapTool`.

    We also describe how to implement new services for use within the
    Gaudi environment. We look at how to code a service, what facilities
    the Service base class provides and how a service is managed by the
    application manager.

.. _serv-requ:

Requesting and accessing services
--------------------------------------

    The Application manager only creates by default the JobOptionsSvc
    and MessageSvc. Other services are created on demand the first time
    they are accessed, provided the corresponding DLL has been loaded.
    The services in the GaudiSvc package are accessible in this way by
    default - these are the default data store services (EventDataSvc,
    DetectorDataSvc, HistogramDataSvc, NTupleSvc) and many of the
    framework services described in this chapter and in :numref:`chapTool` (ToolSvc, ParticlePropertySvc,
    ChronoStatSvc, AuditorSvc, RndmGenSvc, IncidentSvc).

    Additional services can be made accessible by loading the
    appropriate DLL, using the property ApplicationMgr.DLLs in the job
    options file, as shown for example in :numref:`lstg-optsoutsto`.

    Sometimes it may be necessary to force the Application Manager to
    create a service at initialisation (for example if the order of
    creation is important). This can be done using the property
    ApplicationMgr.ExtSvc. In the example below this option is used to
    create a specific type of persistency service.:

    .. was 11.1

    .. code-block:: cpp
        :name: joboptaddserv
        :caption: Job Option to create additional services

        ApplicationMgr.ExtSvc += { "DbEventCnvSvc/RootEvtCnvSvc" };

    Once created, services must be accessed via their interface. The
    Algorithm base class provides a number of accessor methods for the
    standard framework services, listed on lines 25 to 36
    of :numref:`lstg-algobase`. Other services can be
    located using the templated service function. In the example below
    we use this function to return the IParticlePropertySvc interface of
    the Particle Properties Service:

    .. was 11.2

    .. code-block:: cpp
        :name: ipartpropsvcint
        :caption: Code to access the IParticlePropertySvc interface from an Algorithm

        #include "GaudiKernel/IParticlePropertySvc.h"
        // ...
        IParticlePropertySvc* m_ppSvc;
        StatusCode sc = service( "ParticlePropertySvc", m_ppSvc, true );
        if ( sc.isFailure) {
        // ...

    The third argument is optional: when set to true, the service will
    be created if it does not already exist; if it is missing, or set to
    false, the service will not be created if it is not found, and an
    error is returned.

    In components other than Algorithms and Services (e.g. Tools,
    Converters), which do not provide the service function, you can
    locate a service using the serviceLocator function:

    .. code-block:: cpp

        #include "GaudiKernel/IParticlePropertySvc.h"
        // ...
        IParticlePropertySvc* m_ppSvc;
        IService* theSvc;
        StatusCode sc=serviceLocator()->getService("ParticlePropertySvc",theSvc,true);
        if ( sc.isSuccess() ) {
            sc = theSvc->queryInterface(IID_IParticlePropertySvc, (void**)&m_ppSvc);
        }
        if ( sc.isFailure) {
        // ...

.. _serv-jopt:

The Job Options Service
----------------------------

    The Job Options Service is a mechanism which allows to configure an
    application at run time, without the need to recompile or relink.
    The options, or properties, are set via a job options file, which is
    read in when the Job Options Service is initialised by the
    Application Manager. In what follows we describe the format of the
    job options file, including some examples.

.. _serv-prop:

Algorithm, Tool and Service Properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    In general a concrete Algorithm, Service or Tool will have several
    data members which are used to control execution. These data members
    (properties) can be of a basic data type (int, float, etc.) or class
    (Property) encapsulating some common behaviour and higher level of
    functionality. Each concrete Algorithm, Service, Tool declares its
    properties to the framework using the declareProperty templated
    method as shown for example on line 12 of :numref:`evtcounter`
    (see also :numref:`algo-decl`). The method
    setProperties() is called by the framework in the initialization
    phase; this causes the job options service to make repeated calls to
    the setProperty() method of the Algorithm, Service or Tool (once for
    each property in the job options file), which actually assigns
    values to the data members.

SimpleProperties
`````````````````````````

    Simple properties are a set of classes that act as properties
    directly in their associated Algorithm, Tool or Service, replacing
    the corresponding basic data type instance. The primary motivation
    for this is to allow optional bounds checking to be applied, and to
    ensure that the Algorithm, Tool or Service itself doesn't violate
    those bounds. Available SimpleProperties are:

    | · int ==> IntegerProperty or SimpleProperty<int>
    | · double ==> DoubleProperty or SimpleProperty<double>
    | · bool ==> BooleanProperty or SimpleProperty<bool>)
    | · std::string ==> StringProperty or SimpleProperty<std::string>

    and the equivalent vector classes

    | · std::vector<int> ==> IntegerArrayProperty or SimpleProperty<std::vector<int>>
    | · etc.

    The use of these classes is illustrated by the EventCounter class
    (Listings :numref:`evtcounterheader` and :numref:`evtcounter`).

    .. was 11.3

    .. code-block:: cpp
        :name: evtcounterheader
        :caption: EventCounter.h

        #include "GaudiKernel/Algorithm.h"
        #include "GaudiKernel/Property.h"

        class EventCounter : public Algorithm {

        public:

            EventCounter( const std::string& name, ISvcLocator* pSvcLocator );
            ~EventCounter( );
            StatusCode initialize();
            StatusCode execute();
            StatusCode finalize();

        private:

            IntegerProperty m_frequency;
            int m_skip;
            int m_total;
        };

    .. was 11.4

    .. code-block:: cpp
        :name: evtcounter
        :caption: EventCounter.cpp

        #include "GaudiAlg/EventCounter.h"
        #include "GaudiKernel/MsgStream.h"
        #include "GaudiKernel/AlgFactory.h"

        static const AlgFactory<EventCounter> Factory;
        const IAlgFactory& EventCounterFactory = Factory;

        EventCounter::EventCounter(const std::string& name, ISvcLocator* pSvcLocator) :
            Algorithm(name, pSvcLocator), m_skip ( 0 ), m_total( 0 ) {
                declareProperty( "Frequency", m_frequency=1 ); // [1]
                m_frequency.setBounds( 0, 1000 ); // [2]
        }

        StatusCode EventCounter::initialize() {
            MsgStream log(msgSvc(), name());
            log << MSG::INFO << "Frequency: " << m_frequency << endreq; // [3]
            return StatusCode::SUCCESS;
        }

    In the Algorithm constructor, when calling declareProperty, you can
    optionally set the bounds using any of:

    | setBounds( const T& lower, const T& upper );
    |  setLower ( const T& lower );
    |  setUpper ( const T& upper );

    There are similar selectors and modifiers to determine whether a
    bound has been set etc., or to clear a bound.

    | bool hasLower( )
    |  bool hasUpper( )
    |  T lower( )
    |  T upper( )
    |  void clearBounds( )
    |  void clearLower( )
    |  void clearUpper( )

    You can set the value using the "=" operator or the set functions

    | bool set( const T& value )
    |  bool setValue( const T& value )

    The function value indicates whether the new value was within any
    bounds and was therefore successfully updated. In order to access
    the value of the property, use:

    | m\_property.value( );

    In addition there's a cast operator, so you can also use m\_property
    directly instead of m\_property.value().

CommandProperty
````````````````````````

    CommandProperty is a subclass of StringProperty that has a handler
    that is called whenever the value of the property is changed.
    Currently that can happen only during the job initialization so it
    is not terribly useful. Alternatively, an Algorithm could set the
    property of one of its sub-algorithms. However, it is envisaged that
    Gaudi will be extended with a scripting language such that
    properties can be modified during the course of execution.

    The relevant portion of the interface to CommandProperty is:

    .. code-block:: cpp

        class CommandProperty : public StringProperty {
        public:

            // [...]
            virtual void handler( const std::string& value ) = 0;
            [...]
        };

    Thus subclasses should override the handler() member function, which
    will be called whenever the property value changes. A future
    development is expected to be a ParsableProperty (or something
    similar) that would offer support for parsing the string.

.. _serv-modi:

Accessing and modifiying properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Properties are private data which are initialised by the framework
    using the default values given when they are declared in
    constructors, or the values read from the job options file. On
    occasions it may be necessary for components to access (or even
    modify) the values of properties of other components. This can be
    done by using the getProperty() and setProperty() methods of the
    IProperty interface. In the example below,

    .. code-block:: cpp

        Algorithm* myAlg;
        // ...
        std:string dfltCut;
        StatusCode sc = myAlg->getProperty( "TheCut", dfltCut );
        if ( sc.isSuccess() ) {
            msgAlg->setProperty( "TheCut", "0.8" );
            StatusCode sc1 = myAlg->execute();
            // ...
        }
        if( scl.isSuccess() ) msgProp->setProperty( "The Cut", dfltCut );

    ,an algorithm stores the default value of a cut of its
    sub-algorithm, then executes the sub-algorithm with a different cut,
    before resetting the cut back to its default value. Note that in the
    example we begin with a pointer to an Algorithm object, not an
    IAlgorithm interface. This means that we have access to the methods
    of both the IAlgorithm and IProperty interfaces and can therefore
    call the methods of the IProperty interface. In the general one may
    need to navigate to the IProperty interface first, as explaned in
    :numref:`libr-quin`.

.. _serv_jobo:

Job options file format
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    An example of a job options file was shown in :numref:`optionsio`.
    The job options file has a
    well-defined syntax (similar to a simplified C++-Syntax) without
    data types. The data types are recognised by the "Job Options
    Compiler", which interprets the job options file according to the
    syntax (described in :numref:`chapJobo`
    together with possible compiler error codes).

    The job options file is an ASCII-File, composed logically of a
    series of statements. The end of a statement is signalled by a
    semicolon ";" - as in C++.

    Comments are the same as in C++, with '//' until the end of the
    line, or between '/\*' and '\*/'.

    There are four constructs which can be used in a job options file:

    | · Assignment statement
    | · Append statement
    | · Include directive
    | · Platform dependent execution directive

Assignment statement
`````````````````````````````

    An assignment statement assigns a certain value (or a vector of
    values) to a property of an object or identifier. An assignment
    statement has the following structure:

    .. code-block:: cpp

        <Object / Identifier> . < Propertyname > = < value >;

    The first token (Object / Identifier) specifies the name of the
    object whose property is to be set. This must be followed by a dot
    ('.')

    The next token (Propertyname) is the name of the option to be set,
    as declared in the declareProperty() method of the IProperty
    interface. This must be followed by an assign symbol ('=').

    The final token (value) is the value to be assigned to the property.
    It can be a vector of values, in which case the values are enclosed
    in array brackets ('{\`,'}\`), and separated by commas (,). The
    token must be terminated by a semicolon (';').

    The type of the value(s) must match that of the variable whose value
    is to be set, as declared in declareProperty(). The following types
    are recognised:

    | Boolean-type, written as true or false. e.g. true; false;
    |
    | Integer-type, written as an integer value (containing one or more of the digits '0', '1', '2', '3', '4', '5', '6', '7', '8', '9') e.g.: 123; -923; or in scientific notation, e.g.: 12e2;
    |
    | Real-type (similar to double in C++), written as a real value (containing one or more of the digits '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' followed by a dot '.' and optionally one or more of digits again) e.g.: 123.; -123.45; or in scientific notation, e.g. 12.5e7;
    |
    | String type, written within a pair of double quotes (\` " ')  e.g.: "I am a string"; (Note: strings without double quotes are not allowed!)
    |
    | Vector of the types above, within array-brackets ('{', '}'), separated by a comma (',') e.g.: {true, false, true}; {124, -124, 135e2}; {123.53, -23.53, 123., 12.5e2}; {"String 1", "String 2", "String 3"};
    |
    | A single element which should be stored in a vector must be within array-brackets without a comma e.g. {true}; {"String"};
    |
    | A vector which has already been defined earlier in the file (or in included files) can be reset to an empty vector e.g. {};

Append Statement
`````````````````````````

    Because of the possibility of including other job option files (see
    below), it is sometimes necessary to extend a vector of values
    already defined in the other job option file. This functionality is
    provided be the append statement.

    An append statement has the following syntax:

    .. code-block:: cpp

        <Object / Identifier> . < Propertyname > += < value >;

    The only difference from the assignment statement is that the append
    statement requires the '+=' symbol instead of the \`=' symbol to
    separate the Propertyname and value tokens.

    The value must be an array of one or more values

    | e.g.: {true};
    | e.g.: {"String"};
    | e.g.: {true, false, true};
    | e.g.: {124, -124, 135e2};
    | e.g.: {123.53, -23.53, 123., 12.5e2};
    | e.g.: {"String 1", "String 2", "String 3"};

    The job options compiler itself tests if the object or identifier
    already exists (i.e. has already been defined in an included file)
    and the type of the existing property. If the type is compatible and
    the object exists the compiler appends the value to the existing
    property. If the property does not exist then the append operation
    "+=" behaves as assignment operation "=".

Including other Job Option Files
`````````````````````````````````````````

    It is possible to include other job option files in order to use
    pre-defined options for certain objects. This is done using the
    #include directive:

    .. code-block:: cpp

        #include "filename.opts"

    The "filename" can also contain the path where this file is located.
    By convention we use ".opts" as the file extension for job options.
    The include directive can be placed anywhere in the job option file,
    usually at the top (as in C++). Note that the value of a property
    defined earlier in the file may be over-ridden by assigning a new
    value to the same property: the last value assigned is the valid
    value! This makes it possible to over-ride the value of a property
    defined in a previously included file without changing the include
    file.

    It is possible to use environment variables in the #include
    statement, either standalone or as part of a string. Both Unix style
    ("$environmentvariable") and Windows style ("%environmentvariable%")
    are understood (on both platforms!). For example, in line 2 of :numref:`optionsio`
    the logical name $STDOPTS, which
    is defined in the GaudiExamples package, points to a directory
    containing a number of standard job options include files that can
    be used by applications.

    As mentioned above, you can append values to vectors defined in an
    included job option file. The interpreter creates these vectors at
    the moment he interprets the included file, so you can only append
    elements defined in a file included before the append-statement!

    As in C/C++, an included job option file can include other job
    option files. The compiler checks itself whether the include file
    has already been included, so there is no need for #ifndef
    statements as in C or C++ to check for multiple inclusion.

Platform dependent execution
`````````````````````````````````````

    The possibility exists to execute statements only according to the
    used platform. Statements within platform dependent clauses are only
    executed if they are asserted to the current used platform.:

    .. code-block:: cpp

        #ifdef WIN32
        // (Platform-Dependent Statement)
        #else (optional)
        // (Platform-Dependent Statement)
        #endif

    Only the variable WIN32 is defined! An #ifdef WIN32 will check if
    the used platform is a Windows platform. If so, it will execute the
    statements until an #endif or an optional #else. On non-Windows
    platforms it will execute the code within #else and #endif.
    Alternatively one directly can check for a non-Windows platform by
    using the #ifndef WIN32 clause.

Switching on/off printing
``````````````````````````````````

    By default, the Job Options Service prints out the contents of the
    Job Options files to the standard output destination. The
    possibility exists to switch off this printing, and to toggle
    between the two states, as shown below:

    .. code-block:: cpp

        // Switch off printing
        #pragma print off
        // ..(some job options)
        //Switch printing back on
        #pragma print on

    In the example above, all lines between line 2 and line
    5 will not be printed.

.. _serv-mess:

The Standard Message Service
---------------------------------

    One of the components directly visible to an algorithm object is the
    message service. The purpose of this service is to provide
    facilities for the logging of information, warnings, errors etc. The
    advantage of introducing such a component, as opposed to using the
    standard std::cout and std::cerr streams available in C++ is that we
    have more control over what is printed and where it is printed.
    These considerations are particularly important in an online
    environment.

    The Message Service is configurable via the job options file to only
    output messages if their "activation level" is equal to or above a
    given "output level". The output level can be configured with a
    global default for the whole application:

    .. code-block:: cpp

        // Set output level threshold
        //(1=VERBOSE, 2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL, 7=ALWAYS)
        MessageSvc.OutputLevel = 4;

    and/or locally for a given client object (e.g. myAlgorithm):

    .. code-block:: cpp

        myAlgorithm.OutputLevel = 2;

    Any object wishing to print some output should (must) use the
    message service. A pointer to the IMessageSvc interface of the
    message service is available to an algorithm via the accessor method
    msgSvc(), see section :numref:`algo-base`. It is
    of course possible to use this interface directly, but a utility
    class called MsgStream is provided which should be used instead.

The MsgStream utility
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    The MsgStream class is responsible for constructing a Message object
    which it then passes onto the message service. Where the message is
    ultimately sent to is decided by the message service.

    In order to avoid formatting messages which will not be sent because
    the verboseness level is too high, a MsgStream object first checks
    to see that a message will be printed before actually constructing
    it. However the threshold for a MsgStream object is not dynamic,
    i.e. it is set at creation time and remains the same. Thus in order
    to keep synchronized with the message service, which in principle
    could change its printout level at any time, MsgStream objects
    should be made locally on the stack when needed. For example, if you
    look at the listing of the HelloWorld class (see also :numref:`usemsgstrobj`
    below) you will note that
    MsgStream objects are instantiated locally (i.e. not using new) in
    all three of the IAlgorithm methods and thus are destructed when the
    methods return. If this is not done messages may be lost, or too
    many messages may be printed.

    The MsgStream class has been designed to resemble closely a normal
    stream class such as std::cout, and in fact internally uses an
    ostrstream object. All of the MsgStream member functions write
    unformatted data; formatted output is handled by the insertion
    operators.

    An example use of the MsgStream class is shown below.

    .. was 11.5

    .. code-block:: cpp
        :name: usemsgstrobj
        :caption: Use of a MsgStream object

        #include "GaudiKernel/MgsStream.h"

        StatusCode myAlgo::finalize() {
            StatusCode status = Algorithm::finalise();
            MsgStream log(msgSvc(), name());
            if ( status.isFailure() ) {
                // Print a two line message in case of failure.
                log << MSG::ERROR << " Finalize failed" << endl
                    << "Error initializing Base class." << endreq;
            }
            else {
                log << MSG::DEBUG << "Finalize completed successfully" << endreq;
            }
            return status;
        }

    When using the MsgStream class just think of it as a configurable
    output stream whose activation is actually controlled by the first
    word (message level) and which actually prints only when "endreq" is
    supplied. For all other functionality simply refer to the C++
    ostream class.

    The "activation level" of the MsgStream object is controlled by the
    first expression, e.g. MSG::ERROR or MSG::DEBUG in the example
    above. Possible values are given by the enumeration below:

    .. code-block:: cpp

        enum MSG::Level { VERBOSE, DEBUG, INFO, WARNING, ERROR, FATAL, ALWAYS };

    Thus the code in :numref:`usemsgstrobj` will
    produce NO output if the print level of the message service is set
    higher than MSG::ERROR. In addition if the service's print level is
    lower than or equal to MSG::DEBUG the "Finalize completed
    successfully" message will be printed (assuming of course it was
    successful).

    | User interface
    |
    |  What follows is a technical description of the part of the MsgStream user interface most often seen by application developers. Please refer to the header file for the complete interface.
    |
    |  Insertion Operator
    |
    |   The MsgStream class overloads the '<<\` operator as described below.
    |
    |   MsgStream& operator <<(TYPE arg);
    |    Insertion operator for various types. The argument is only formatted by the stream object if the print level is sufficiently high and the stream is active. Otherwise the insertion operators simply return. Through this mechanism extensive debug printout does not cause large run-time overheads. All common base types such as char, unsigned char, int, float, etc. are supported
    |
    |   MsgStream& operator <<(MSG::Level level);
    |    This insertion operator does not format any output, but rather (de)activates the stream's formatting and forwarding engine depending on the value of level.
    |
    |  Accepted Stream Manipulators
    |
    |   The MsgStream specific manipulators are presented below, e.g. endreq: MsgStream& endreq(MsgStream& stream). Besides these, the common ostream and ios manipulators such as std::ends, std::endl,... are also accepted.
    |
    |   endl
    |    Inserts a newline sequence. Opposite to the ostream behaviour this manipulator does not flush the buffer. Full name: MsgStream& endl(MsgStream& s)
    |   ends
    |    Inserts a null character to terminate a string. Full name: MsgStream& ends(MsgStream& s)
    |   flush
    |    Flushes the stream's buffer but does not produce any output! Full name: MsgStream& flush(MsgStream& s)
    |   endreq
    |    Terminates the current message formatting and forwards the message to the message service. If no message service is assigned the output is sent to std::cout. Full name: MsgStream& endreq(MsgStream& s)
    |   endmsg
    |    Same as endreq

.. _serv-prse:

The Particle Properties Service
------------------------------------

    The Particle Property service is a utility to find information about a named particle's Geant3 ID, Jetset/Pythia ID, Geant3 tracking type, charge, mass or lifetime. The database used by the service can be changed, but by default is the same as that used by the LHCb SICB program. Note that the units conform to the CLHEP convention, in particular MeV for masses and ns for lifetimes. Any comment to the contrary in the code is just a leftover which has been overlooked!

Initialising and Accessing the Service
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    This service is created by adding the following line in the Job
    Options file::

    .. code-block:: cpp

        // Create the particle properties service
        ApplicationMgr.ExtSvc += { "ParticlePropertySvc" };

    :numref:`ipartpropsvcint` shows how to access this
    service from within an algorithm.

Service Properties
~~~~~~~~~~~~~~~~~~~~~~~~~

    The Particle Property Service currently only has one property:
    ParticlePropertiesFile. This string property is the name of the
    database file that should be used by the service to build up its
    list of particle properties. The default value of this property, on
    all platforms, is $LHCBDBASE/cdf/particle.cdf [#]_

Service Interface
~~~~~~~~~~~~~~~~~~~~~~~~

    The service implements the IParticlePropertySvc interface. In order
    to use it, clients must include the file
    GaudiKernel/IParticlePropertySvc.h.

    The service itself consists of one STL vector to access all of the
    existing particle properties, and three STL maps, one to map
    particles by name, one to map particles by Geant3 ID and one to map
    particles by stdHep ID.

    Although there are three maps, there is only one copy of each
    particle property and thus each property must have a unique particle
    name and a unique Geant3 ID. Particles that are known to Geant but
    not to stdHep, such as Deuteron, have an artificial stdHep ID using
    unreserved (>7) digits. Although retrieving particles by name should
    be sufficient, the second and third maps are there because most
    often generated data stores a particle's Geant3 ID or stdHep ID, and
    not the particle's name. These maps speed up searches using the IDs.

    The IParticlePropertySvc interface provides the following functions:

    .. was 11.6

    .. code-block:: cpp
        :name: ipartpropsvcint2
        :caption: The IParticlePropertySvc interface

        // IParticlePropertySvc interface:
        // Create a new particle property.
        // Input: particle, String name of the particle.
        // Input: geantId, Geant ID of the particle.
        // Input: jetsetId, Jetset ID of the particle.
        // Input: type, Particle type.
        // Input: charge, Particle charge (/e).
        // Input: mass, Particle mass (MeV).
        // Input: tlife, Particle lifetime (ns).
        // Return: StatusCode - SUCCESS if the particle property was added.
        virtual StatusCode push_back( const std::string& particle, int geantId, int jetsetId, int type, double charge, double mass, double tlife );
        // Create a new particle property.
        // Input: pp, a particle property class.
        // Return: StatusCode - SUCCESS if the particle property was added.
        virtual StatusCode push_back( ParticleProperty* pp );
        // Get a const reference to the begining of the map.
        virtual const_iterator begin() const;
        // Get a const reference to the end of the map.
        virtual const_iterator end() const;
        // Get the number of properties in the map.
        virtual int size() const;
        // Retrieve a property by geant id.
        // Pointer is 0 if no property found.
        virtual ParticleProperty* find( int geantId );
        // Retrieve a property by particle name.
        // Pointer is 0 if no property found.
        virtual ParticleProperty* find( const std::string& name );
        // Retrieve a property by StdHep id
        // Pointer is 0 if no property found.
        virtual ParticleProperty* findByStdHepID( int stdHepId );
        // Erase a property by geant id.
        virtual StatusCode erase( int geantId );
        // Erase a property by particle name.
        virtual StatusCode erase( const std::string& name );
        // Erase a property by StdHep id
        virtual StatusCode eraseByStdHepID( int stdHepId );

    The IParticlePropertySvc interface also provides some typedefs for
    easier coding:

    .. code-block:: cpp

        typedef ParticleProperty* mapped_type;
        typedef std::map< int, mapped_type, std::less<int> > MapID;
        typedef std::map< std::string, mapped_type, std::less<std::string> > MapName;
        typedef std::map< int, mapped_type, std::less<int> > MapStdHepID;
        typedef IParticlePropertySvc::VectPP VectPP;
        typedef IParticlePropertySvc::const_iterator const_iterator;
        typedef IParticlePropertySvc::iterator iterator;

Examples
~~~~~~~~~~~~~~~

    Below are some extracts of code from the LHCb ParticleProperties
    example to show how one might use the service:

    .. was 11.7

    .. code-block:: cpp
        :name: findpartpropbyname
        :caption: Code fragment to find particle properties by particle name

        // Try finding particles by the different methods
        log << MSG::INFO << "Trying to find properties by Geant3 ID..." << endreq;
        ParticleProperty* pp1 = m_ppSvc->find( 1 );
        if ( pp1 ) log << MSG::INFO << *pp1 << endreq;
        log << MSG::INFO << "Trying to find properties by name..." << endreq;
        ParticleProperty* pp2 = m_ppSvc->find( "e+" );
        if ( pp2 ) log << MSG::INFO << *pp2 << endreq;
        log << MSG::INFO << "Trying to find properties by StdHep ID..." << endreq;
        ParticleProperty* pp3 = m_ppSvc->findByStdHepID( 521 );
        if ( pp3 ) log << MSG::INFO << *pp3 << endreq;

    .. was 11.8

    .. code-block:: cpp
        :name: usemapitaccprop
        :caption: Code fragment showing how to use the map iterators to access particle properties

        // List all properties
        log << MSG::DEBUG << "Listing all properties..." << endreq;
        for( IParticlePropertySvc::const_iterator i = m_ppSvc->begin(); i != m_ppSvc->end(); i++ ) {
            if ( *i ) log << *(*i) << endreq;
        }


The Chrono & Stat service
------------------------------

    The Chrono & Stat service provides a facility to do time profiling
    of code (Chrono part) and to do some statistical monitoring of
    simple quantities (Stat part). The service is created by default by
    the Application Manager, with the name "ChronoStatSvc" and service
    ID extern const CLID& IID\_IChronoStatSvc To access the service from
    inside an algorithm, the member function chronoSvc() is provided.
    The job options to configure this service are described in
    :numref:`chapOpti`, :numref:`tab-b20`.

Code profiling
~~~~~~~~~~~~~~~~~~~~~

    Profiling is performed by using the chronoStart() and chronoStop()
    methods inside the codes to be profiled, e.g:

    .. code-block:: cpp

        /// ...
        IChronoStatSvc* svc = chronoSvc();
        /// start
        svc->chronoStart( "Some Tag" );
        /// here some user code are placed:
        // ...
        /// stop
        svc->chronoStop( "SomeTag" );


    The profiling information accumulates under the tag name given as
    argument to these methods. The service measures the time elapsed
    between subsequent calls of chronoStart() and chronoStop() with the
    same tag. The latter is important, since in the sequence of calls
    below, only the elapsed time between lines 3 and 5 lines and between
    lines 7 and 9 lines would be accumulated.:

    .. code-block:: cpp

        svc->chronoStop("Tag");
        svc->chronoStop("Tag");
        svc->chronoStart("Tag");
        svc->chronoStart("Tag");
        svc->chronoStop("Tag");
        svc->chronoStop("Tag");
        svc->chronoStart("Tag");
        svc->chronoStart("Tag");
        svc->chronoStop("Tag");

    The profiling information could be printed either directly using the
    chronoPrint() method of the service, or in the summary table of
    profiling information at the end of the job.

    Note that this method of code profiling should be used only for fine
    grained monitoring inside algorithms. To profile a complete
    algorithm you should use the Auditor service, as described in
    section :numref:`serv-audi`.

Statistical monitoring
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Statistical monitoring is performed by using the stat() method
    inside user code:

    .. code-block:: cpp

        /// ... Flag and Weight to be accumulated:
        svc->stat( " Number of Tracks " , Flag , Weight );

    The statistical information contains the "accumulated" flag, which
    is the sum of all Flags for the given tag, and the "accumulated"
    weight, which is the product of all Weights for the given tag. The
    information is printed in the final table of statistics.

    In some sense the profiling could be considered as statistical
    monitoring, where the variable Flag equals the elapsed time of the
    process.

Chrono and Stat helper classes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    To simplify the usage of the Chrono & Stat Service, two helper
    classes were developed: class Chrono and class Stat. Using these
    utilities, one hides the communications with Chrono & Stat Service
    and provides a more friendly environment.

Chrono
```````````````

    Chrono is a small helper class which invokes the chronoStart()
    method in the constructor and the chronoStop() method in the
    destructor. It must be used as an automatic local object.

    It performs the profiling of the code between its own creation and
    the end of the current scope, e.g:

    .. code-block:: cpp

        #include GaudiKernel/Chrono.h
        /// ...
        { // begin of the scope
            Chrono chrono( chronoSvc() , "ChronoTag" ) ;
            /// some codes:
            // ...
            ///
        } // end of the scope
        /// ...

    If the Chrono & Stat Service is not accessible, the Chrono object
    does nothing

Stat
`````````````

    Stat is a small helper class, which invokes the stat() method in the
    constructor.

    .. code-block:: cpp

        GaudiKernel/Stat.h
        /// ...
        Stat stat( chronoSvc() , "StatTag" , Flag , Weight ) ;
        /// ...

    If the Chrono & Stat Service is not accessible, the Stat object does
    nothing.

Performance considerations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    The implementation of the Chrono & Stat Service uses two std::map
    containers and could generate a performance penalty for very
    frequent calls. Usually the penalty is small relative to the elapsed
    time of algorithms, but it is worth avoiding both the direct usage
    of the Chrono & Stat Service as well as the usage of it through the
    Chrono or Stat utilities inside internal loops:

    .. code-block:: cpp

        /// ...
        { /// begin of the scope
            Chrono chrono( chronoSvc() , "Good Chrono"); /// OK
            long double a = 0 ;
            for( long i = 0 ; i < 1000000 ; ++i )
            {
                Chrono chrono( svc , "Bad Chrono"); /// not OK
                /// some codes :
                a += sin( cos( sin( cos( (long double) i ) ) ) );
                /// end of codes
                Stat stat ( svc , "Bad Stat", a ); /// not OK
            }
            Stat stat ( svc , "Good Stat", a); /// OK
        } /// end of the scope!
        /// ...

.. _serv-audi:

The Auditor Service
------------------------

    The Auditor Service provides a set of auditors that can be used to
    provide monitoring of various characteristics of the execution of
    Algorithms. Each auditor is called immediately before and after each
    call to each Algorithm instance, and can track some resource usage
    of the Algorithm. Calls that are thus monitored are initialize(),
    execute() and finalize(), although monitoring can be disabled for
    any of these for particular Algorithm instances. Only the execute()
    function monitoring is enabled by default.

    Several examples of auditors are provided. These are:

    | · NameAuditor. This just emits the name of the Algorithm to the Standard Message Service immediately before and after each call. It therefore acts as a diagnostic tool to trace program execution.
    |  · ChronoAuditor. This monitors the cpu usage of each algorithm and reports both the total and per event average at the end of job.
    |  · MemoryAuditor. This monitors the state of memory usage during execution of each Algorithm, and will warn when memory is allocated within a call without being released on exit. Unfortunately this will in fact be the general case for Algorithms that are creating new data and registering them with the various transient stores. Such Algorithms will therefore cause warning messages to be emitted. However, for Algorithms that are just reading data from the transient stores, these warnings will provide an indication of a possible memory leak. Note that currently the MemoryAuditor is only available for Linux.
    |  · MemStatAuditor. The same as MemoryAuditor, but prints a table of memory usage statistics at the end.

Enabling the Auditor Service and specifying the enabled Auditors
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    The Auditor Service is enabled by the following line in the Job
    Options file:

    .. code-block:: cpp

        // Enable the Auditor Service
        ApplicationMgr.DLLs += { "GaudiAud" };

    Specifying which auditors are enabled is illustrated by the
    following example:

    .. code-block:: cpp

        // Enable the NameAuditor and ChronoAuditor
        AuditorSvc.Auditors = { "NameAuditor", "ChronoAuditor" };

Overriding the default Algorithm monitoring
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    By default, only monitoring of the Algorithm execute() function is
    enabled by default. This default can be overridden for individual
    Algorithms by use of the following Algorithm properties:

    .. code-block:: cpp

        // Enable initialize and finalize auditing & disable execute auditing
        // for the myAlgorithm Algorithm
        myAlgorithm.AuditInitialize = true;
        myAlgorithm.AuditExecute = false;
        myAlgorithm.AuditFinalize = true;

Implementing new Auditors
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    The relevant portion of the IAuditor abstract interface is shown
    below:

    .. code-block:: cpp

        virtual StatusCode beforeInitialize( IAlgorithm* theAlg ) = 0;
        virtual StatusCode afterInitialize ( IAlgorithm* theAlg ) = 0;
        virtual StatusCode beforeExecute ( IAlgorithm* theAlg ) = 0;
        virtual StatusCode afterExecute ( IAlgorithm* theAlg ) = 0;
        virtual StatusCode beforeFinalize ( IAlgorithm* theAlg ) = 0;
        virtual StatusCode afterFinalize ( IAlgorithm* theAlg ) = 0;

    A new Auditor should inherit from the Auditor base class and
    override the appropriate functions from the IAuditor abstract
    interface. The following code fragment is taken from the
    ChronoAuditor:

    .. code-block:: cpp

        #include "Gaudi/Auditor.h"
        class ChronoAuditor : virtual public Auditor {

        public:

            ChronoAuditor(const std::string& name, ISvcLocator* pSvcLocator);
            virtual ~ChronoAuditor();
            virtual StatusCode beforeInitialize(IAlgorithm* alg);
            virtual StatusCode afterInitialize(IAlgorithm* alg);
            virtual StatusCode beforeExecute(IAlgorithm* alg);
            virtual StatusCode afterExecute(IAlgorithm* alg);
            virtual StatusCode beforeFinalize(IAlgorithm* alg);
            virtual StatusCode afterFinalize(IAlgorithm* alg);
         };

The Random Numbers Service
-------------------------------

    When generating random numbers two issues must be considered:

    | · reproducibility and
    | · randomness of the generated numbers.

    In order to ensure both, Gaudi implements a single service ensuring
    that these criteria are met. The encapsulation of the actual random
    generator into a service has several advantages:

    | · Random seeds are set by the framework. When debugging the detector simulation, the program could start at any event independent of the events simulated before. Unlike the random number generators that were known from CERNLIB, the state of modern generators is no longer defined by one or two numbers, but rather by a fairly large set of numbers. To ensure reproducibility the random number generator must be initialized for every event.
    | · The distribution of the random numbers generated is independent of the random number engine behind. Any distribution can be generated starting from a flat distribution.
    | · The actual number generator can easily be replaced if at some time in the future better generators become available, without affecting any user code.

    The implementation of both generators and random number engines are
    taken from CLHEP. The default random number engine used by Gaudi is
    the RanLux engine of CLHEP with a luxury level of 3, which is also
    the default for Geant4, so as to use the same mechanism to generate
    random numbers as the detector simulation.

    :numref:`fig-services` shows the general
    architecture of the Gaudi random number service. The client
    interacts with the service in the following way:

    | · The client requests a generator from the service, which is able to produce a generator according to a requested distribution. The client then retrieves the requested generator.
    | · Behind the scenes, the generator service creates the requested generator and initializes the object according to the parameters. The service also supplies the shared random number engine to the generator.
    | · After the client has finished using the generator, the object must be released in order to inhibit resource leaks

    .. figure:: images/GDG_Services2.png
        :name: fig-services

    The architecture of the random number service. The client requests from the service a random number generator satisfying certain criteria

    There are many different distributions available. The shape of the
    distribution must be supplied as a parameter when the generator is
    requested by the user.

    Currently implemented distributions include the following. See also
    the header file GaudiKernel/RndmGenerators.h for a description of
    the parameters to be supplied.

    | · Generate random bit patterns with parameters Rndm::Bit()
    | · Generate a flat distribution with boundaries [min, max] with parameters:
    | Rndm::Flat(double min, double max)
    | · Generate a gaussian distribution with parameters: Rndm::Gauss(double mean, double sigma)
    | · Generate a poissonian distribution with parameters: Rndm::Poisson(double mean)
    | · Generate a binomial distribution according to n tests with a probability p with parameters: Rndm::Binomial(long n, double p)
    | · Generate an exponential distribution with parameters: Rndm::Exponential(double mean)
    | · Generate a Chi\*\*2 distribution with n\_dof degrees of freedom with parameters: Rndm::Chi2(long n\_dof)
    | · Generate a Breit-Wigner distribution with parameters: Rndm::BreitWigner(double mean, double gamma)
    | · Generate a Breit-Wigner distribution with a cut-off with parameters: Rndm::BreitWignerCutOff (mean, gamma, cut-off)
    | · Generate a Landau distribution with parameters: Rndm::Landau(double mean, double sigma)
    | · Generate a user defined distribution. The probability density function is given by a set of descrete points passed as a vector of doubles: Rndm::DefinedPdf(const std::vector<double>& pdf, long intpol)

    Clearly the supplied list of possible parameters is not exhaustive,
    but probably represents most needs. The list only represents the
    present content of generators available in CLHEP and can be updated
    in case other distributions will be implemented.

    Since there is a danger that the interfaces are not released, a
    wrapper is provided that automatically releases all resources once
    the object goes out of scope. This wrapper allows the use of the
    random number service in a simple way. Typically there are two
    different usages of this wrapper:

    | · Within the user code a series of numbers is required only once, i.e. not every event. In this case the object is used locally and resources are released immediately after use. This example is shown in :numref:`rndnumgenhist`

    .. was 11.9

    .. code-block:: cpp
        :name: rndnumgenhist
        :caption: Example of the use of the random number generator to fill a histogram with a Gaussian distribution within a standard Gaudi algorithm

        Rndm::Numbers gauss(randSvc(), Rndm::Gauss(0.5,0.2));
        if ( gauss ) {
            IHistogram1D* his = histoSvc()->book("/stat/2","Gaussian",40,0.,3.);
            for ( long i = 0; i < 5000; i++ )
            his->fill(gauss(), 1.0);
        }


    | · One or several random numbers are required for the processing of every event. An example is shown in :numref:`rndnumbgenuseevt`

    .. was 11.10

    .. code-block:: cpp
        :name: rndnumbgenuseevt
        :caption: Example of the use of the random number generator within a standard Gaudi algorithm, for use at every event. The wrapper to the generator is part of the Algorithm itself and must be initialized before being used. Afterwards the usage is identical to the example described in :numref:`rndnumgenhist`

        #include "GaudiKernel/RndmGenerators.h"

        // Constructor
        class myAlgorithm : public Algorithm {
            Rndm::Numbers m_gaussDist;
            // ...
        };

        // Initialisation
        StatusCode myAlgorithm::initialize() {
            ...
            StatusCode sc=m_gaussDist.initialize( randSvc(), Rndm::Gauss(0.5,0.2));
            if ( !status.isSuccess() ) {
                // put error handling code here...
            }
            ...
        }

    There are a few points to be mentioned in order to ensure the
    reproducibility:

    | · Do not keep numbers across events. If you need a random number ask for it. Usually caching does more harm than good. If there is a performance penalty, it is better to find a more generic solution.
    | · Do not access the RndmEngine directly.
    | · Do not manipulate the engine. The random seeds should only be set by the framework on an event by event basis.

.. _serv-inci:

The Incident Service
-------------------------

    The Incident service provides synchronization facilities to
    components in a Gaudi application. Incidents are named software
    events that are generated by software components and that are
    delivered to other components that have requested to be informed
    when that incident happens. The Gaudi components that want to use
    this service need to implement the IIncidentListener interface,
    which has only one method: handle(Incident&), and they need to add
    themselves as Listeners to the IncidentSvc. The following code
    fragment works inside Algorithms.

    .. code-block:: cpp

        #include "GaudiKernel/IIncidentListener.h"
        #include "GaudiKernel/IIncidentSvc.h"
        class MyAlgorithm : public Algorithm, virtual public IIncidentListener {
            ...
        };

        MyAlgorithm::Initialize() {
            IIncidentSvc* incsvc;
            StatusCode sc = service("IncidentSvc", incsvc);
            int priority = 100;
            if ( sc.isSuccess() ) {
                incsvc->addListener( this, "BeginEvent", priority);
                incsvc->addListener( this, "EndEvent");
            }
        }
        MyAlgorithm::handle(Incident& inc) {
            log << "Got informed of incident: " << inc.type()
                << " generated by: " << inc.source() << endreq;
        }

    The third argument in method addListener() is for specifying the
    priority by which the component will be informed of the incident in
    case several components are listeners of the same named incident.
    This parameter is used by the IncidentSvc to sort the listeners in
    order of priority.

Known Incidents
~~~~~~~~~~~~~~~~~~~~~~

    .. was 11.1

    .. table:: Table of known named incidents

        +--------------------------+--------------------------+--------------------------+
        | Incident Type            | Source                   | Description              |
        +==========================+==========================+==========================+
        | BeginEvent               | ApplicationMgr           | The ApplicationMgr is    |
        |                          |                          | starting processing of a |
        |                          |                          | new physics event. This  |
        |                          |                          | incident can be use to   |
        |                          |                          | clear caches of the      |
        |                          |                          | previous event in        |
        |                          |                          | Services and Tools.      |
        +--------------------------+--------------------------+--------------------------+
        | EndEvent                 | ApplicationMgr           | The ApplicationMgr has   |
        |                          |                          | finished processing the  |
        |                          |                          | physics event. The Event |
        |                          |                          | data store is not yet    |
        |                          |                          | purged at this moment.   |
        +--------------------------+--------------------------+--------------------------+

.. _serv-refl:

The Gaudi Introspection Service
-------------------------------------

    Introspection is the ability of a programming language to interact
    with objects from a meta-level. The Gaudi Introspection package
    defines a meta-model which gives the layout of this
    meta-information.

    The data to fill this meta-information (i.e. the dictionary) can be
    generated by the Gaudi Object Description package (described in :numref:`data-defi`) by adding
    a few lines to the CMT requirements file, as shown for example in
    :numref:`cmtreqgendic`.

    .. was 11.1

    .. code-block:: cpp
        :name: cmtreqgendic
        :caption: CMT requirements for generation of data dictionary of the LHCbEvent package

        #---- dictionary
        document obj2dict LHCbEventObj2Dict -group=dict ../xml/LHCbEvent.xml
        library LHCbEventDict -group=dict ../dict/*.cpp
        macro LHCbEventDict_shlibflags "$(use_linkopts) $(libraryshr_linkopts)"


    The C++-code generated in this way is compiled into a dll and loaded
    into the Gaudi Introspection Model at runtime.

    To get a reference to information about a real object, clients have
    to use the Gaudi Introspection Service (IntrospectionSvc). The
    service can also be used to load the meta-information into the
    model. The Gaudi Introspection Service is already used in several
    places in the framework (e.g. Interface to Python, Data Store
    Browser).

    Further information about this service is available at
    http://cern.ch/lhcb-comp/Frameworks/DataDictionary/default.htm.

Developing new services
-----------------------------

The Service base class
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Within Gaudi we use the term "Service" to refer to a class whose job
    is to provide a set of facilities or utilities to be used by other
    components. In fact we mean more than this because a concrete
    service must derive from the Service base class and thus has a
    certain amount of predefined behaviour; for example it has
    initialize() and finalize() methods which are invoked by the
    application manager at well defined times.

    :numref:`fig-services2` shows the inheritance
    structure for an example service called SpecificService. The key
    idea is that a service should derive from the Service base class and
    additionally implement one or more pure abstract classes
    (interfaces) such as IConcreteSvcType1 and IConcreteSvcType2 in the
    figure.

    .. figure:: images/GDG_Servicesa.png
        :name: fig-services2

    Implementation of a concrete service class. Though not shown in the figure, both of the IConcreteSvcType interfaces are derived from IInterface

    As discussed above, it is necessary to derive from the Service base
    class so that the concrete service may be made accessible to other
    Gaudi components. The actual facilities provided by the service are
    available via the interfaces that it provides. For example the
    ParticleProperties service implements an interface which provides
    methods for retrieving, for example, the mass of a given particle.
    In :numref:`fig-services2` the service
    implements two interfaces each of two methods.

    A component which wishes to make use of a service makes a request to
    the application manager. Services are requested by a combination of
    name, and interface type, i.e. an algorithm would request
    specifically either IConcreteSvcType1 or IConcreteSvcType2.

    The identification of what interface types are implemented by a
    particular class is done via the queryInterface method of the
    IInterface interface. This method must be implemented in the
    concrete service class. In addition the initialize() and finalize()
    methods should be implemented. After initialization the service
    should be in a state where it may be used by other components.

    The service base class offers a number of facilities itself which
    may be used by derived concrete service classes:

    | · Properties are provided for services just as for algorithms. Thus concrete services may be fine tuned by setting options in the job options file.
    | · A serviceLocator method is provided which allows a component to request the use of other services which it may need.
    | · A message service.

Implementation details
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    The following is essentially a checklist of the minimal code
    required for a service.

    | 1. Define the interfaces
    | 2. Derive the concrete service class from the Service base class.
    | 3. Implement the queryInterface() method.
    | 4. Implement the initialize() method. Within this method you should make a call to Service::initialize() as the first statement in the method and also make an explicit call to setProperties() in order to read the service's properties from the job options (note that this is different from Algorithms, where the call to setProperties() is done in the base class).

    .. was 11.12

    .. code-block:: cpp
        :name: intclass
        :caption: An interface class

        #include "GaudiKernel/IInterface.h"

        class IConcreteSvcType1 : virtual public IInterface {

        public:

            void method1() = 0;
            int method2() = 0;
        }

        #include "IConcreteSvcType1.h"
        const IID& IID_IConcreteSvcType1 = 143; // UNIQUE within LHCb !!

    .. was 11.13

    .. code-block:: cpp
        :name: minservimpl
        :caption: A minimal service implementation

        #include "GaudiKernel/Service.h"
        #include "IConcreteSvcType1.h"
        #include "IConcreteSvcType2.h"

        class SpecificService : public Service,
            virtual public IConcreteSvcType1,
            virtual public IConcreteSvcType2 {

        public:

            // Constructor of this form required:
            SpecificService(const std::string& name, ISvcLocator* sl);
            queryInterface(constIID& riid, void** ppvIF);
        };

        // Factory for instantiation of service objects
        static SvcFactory<SpecificService> s_factory;
        const ISvcFactory& SpecificServiceFactory = s_factory;

        // UNIQUE Interface identifiers defined elsewhere
        extern const IID& IID_IConcreteSvcType1;
        extern const IID& IID_IConcreteSvcType2;

        // queryInterface
        StatusCode SpecificService::queryInterface(const IID& riid, void** ppvIF) {
            if(IID_IConcreteSvcType1 == riid) {
                *ppvIF = dynamic_cast<IConcreteSvcType1*> (this);
                return StatusCode::SUCCESS;
            } else if(IID_IConcreteSvcType2 == riid) {
                *ppvIF = dynamic_cast<IConcreteSvcType2*> (this);
                return StatusCode::SUCCESS;
            } else {
                return Service::queryInterface(riid, ppvIF);
            }
        }

        StatusCode SpecificService::initialize() { ... }
        StatusCode SpecificService::finalize() { ... }

        // Implement the specifics ...
        SpecificService::method1() {...}
        SpecificService::method2() {...}
        SpecificService::method3() {...}
        SpecificService::method4() {...}


.. [#] This is an LHCb specific file. A generic implementation will be available in a future release of Gaudi
