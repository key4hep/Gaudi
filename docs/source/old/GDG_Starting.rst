.. include:: ./global.rst

|newpage|

|Gaudi logo|

.. _chapStar:

Getting started
=================

Overview
------------

    In this chapter we walk through one of the example applications
    (RandomNumber) which are distributed with the framework. We look
    briefly at the different files and go over the steps needed to
    compile and execute the code. We also outline where various subjects
    are covered in more detail in the remainder of the document. Finally
    we cover briefly the other example applications which are
    distributed and say a few words on what each one is intended to
    demonstrate.

Creating a job
------------------

    Traditionally, a "job" is the running of a program on a specified
    set of input data to produce a set of output data, usually in batch.

    For the example applications supplied this is essentially a two step
    process. First the executable must be produced, and secondly the
    necessary environment variables must be set and the required job
    options specified, as illustrated in :numref:`fig-job`.

    .. figure:: images/job.png
        :name: fig-job

        Creating a job from the AlgSequencer example application

    The example applications consist of a number of "source code" files
    which together allow you to generate an executable program. These
    are:

    | · The main program.
    | · Header and implementation files for each of the concrete algorithm classes.
    | · A CMT requirements file.
    | · The set of Gaudi libraries.

    In order for the job to run as desired you must provide the correct
    configuration information for the executable. This is done via
    entries in the job options file.

The main program
--------------------

    The main program is needed to bootstrap the job. It can be
    completely general, and can be reused by all Gaudi applications. An
    example main program, from the package GaudiExamples, is shown in
    :numref:`main`.

    .. was 4.1

    .. code-block:: cpp
        :caption: The example main program
        :name: main

        // Include files
        #include "GaudiKernel/SmartIF.h"
        #include "GaudiKernel/Bootstrap.h"
        #include "GaudiKernel/IAppMgrUI.h"
        #include "GaudiKernel/IProperty.h"
        #include <iostream>

        //--- Example main program
        int main(int argc, char** argv) {

            // Create an instance of an application manager
            IInterface* iface = Gaudi::createApplicationMgr();

            SmartIF<IProperty> propMgr ( IID_IProperty, iface );
            SmartIF<IAppMgrUI> appMgr ( IID_IAppMgrUI, iface );

            if( !appMgr.isValid() || !propMgr.isValid() ) {
                std::cout << "Fatal error creating ApplicationMgr " << std::endl;
                return 1;
            }

            // Get the input configuration file from arguments
            std:: string opts = (argc>1) ? argv[1] : "../options/job.opts";

            propMgr->setProperty( "JobOptionsPath", opts );

            // Run the application manager and process events
            appMgr->run();

            // All done - exit
            return 0;
        }

    It is constructed as follows:

    | Include
    |
    |  These are needed for the creation of the application manager and Smart interface pointers.
    |
    | Application Manager instantiation
    |
    |  Line 12 instantiates an ApplicationMgr object. The application manager is essentially the job controller. It is responsible for creating and correctly initialising all of the services and algorithms required, for looping over the input data events and executing the algorithms specified in the job options file, and for terminating the job cleanly.
    |
    | Retrieval of Interface pointers
    |
    |  The code on lines 14 and 15 retrieves the pointers to the IProperty and IAppMgrUI interfaces of the application manager.
    |
    | Setting the application manager's properties
    |
    |  The only property which needs to be set explicitly in the main program is the name of the job options file which contains all of the other configuration information needed to run the job. In this example, the name is the first argument of the program and defaults to "../options/job.opts" (line 23); it is set on line 25.
    |
    | Program execution
    |
    |  All of the code before line 28 is essentially for setting up the job. Once this is done, a call to appMgr::run() is all that is needed to start the job proper! The steps that occur within this method are discussed briefly in  section :numref:`star-exec`.

Configuring the job
-----------------------

    The application framework makes use of a job options file for job
    configuration. Part of the job options file of an example
    application is shown in :numref:`optionsio`.

    .. was 4.2

    .. code-block:: cpp
        :name: optionsio
        :caption: Part of the job options file for the RootIORead example application

        // Include standard option files
        #include "$STDOPTS/Common.opts"

        // Private Application Configuration options
        ApplicationMgr.DLLs += { "GaudiDb", "GaudiRootDb" };
        ApplicationMgr.ExtSvc += { "DbEventCnvSvc/RootEvtCnvSvc" };
        ApplicationMgr.TopAlg = { "ReadAlg" };

        // Set output level threshold (2=DEBUG,3=INFO,4=WARNING,5=ERROR,6=FATAL)
        MessageSvc.OutputLevel = 4;
        EventSelector.OutputLevel = 2;

        // Input File
        EventSelector.Input = {"DATAFILE='RootDst.root' TYP='ROOT' OPT='READ'"};
        EventSelector.FirstEvent = 1;
        ApplicationMgr.EvtMax = 5;

        // Persistency service setup:
        EventPersistencySvc.CnvServices += { "RootEvtCnvSvc" };

        // Setup for ROOT I/O system
        RootEvtCnvSvc.DbType = "ROOT";

    The format of an options file is discussed fully in `Chapter
    11 <GDG_Services.html#1010951>`__. Options may be set both for
    algorithms and services and the list of available options for
    standard components is given in :numref:`chapOpti`. Here we look briefly at
    a few of the more commonly used options.

Defining the algorithms to be executed
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    The option ApplicationMgr.TopAlg (line 7) is a list of algorithms that will
    be created and controlled directly by the application manager, the
    so-called top-level algorithms. The syntax is a list of the form:

    .. code-block:: python

        ApplicationMgr.TopAlg = { "Type1/Name1", "Type2/Name2" };

    The line above instructs the application manager to create two top
    level algorithms. One of type Type1 called "Name1" and one of type
    Type2 called "Name2".

    In the case where the name of the algorithm is the same as the
    algorithm's type (i.e. class), only the class name is necessary. In
    the example, an instance of the class "ReadAlg" will be created with
    name "ReadAlg".

Defining the job input
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Event data input is controlled by an EventSelector. The
    EventSelector uses a storage technology dependent data persistency
    service to load the data into the transient event data store, with
    the help of converters which are able to convert the data from the
    technology dependent persistent representation, to the technology
    independent representation in the transient data store.

    In order to set up this mechanism, one needs a number of job
    options:

    | - Line 14 defines the input data file, and the persistency technology (ROOT I/O in this example).
    | - Line 6 tells the application manager to create a new event conversion service, to be called RootEvtCnvSvc. Note that this is just a name for our convenience, the service is of type DbEventCnvSvc and does not (yet) know that it will deal with ROOT technology. The configuration of RootEvtCnvSvc to use the ROOT I/O technology is done in line 22.
    | - Line 19 tells the event persistency service (EventPersistencySvc created by the application manager by default) to use the RootEvtCnvSvc to do the conversion between persistent and transient data representations.
    | - Line 5 tells the application manager which additional libraries to load in order to find the required conversion service. In this example, the GaudiDb library contains the DbEventCnvSvc class, the GaudiRootDb library contains the ROOT specific database drivers.
    | - Finally, the options on lines 15 and 16 tell the EventSelector to start reading sequentially from the first event in the file, for five events.

    In the special case where no event input is required (e.g. for event
    generation), one can replace the above options by the two options:

    .. code-block:: cpp

        ApplicationMgr.EvtMax = 20; // events to be processed (default is 10)
        ApplicationMgr.EvtSel = "NONE"; // do not use any event input

    A discussion of event I/O can be found in :numref:`chapNtup`. Converters and the conversion
    process are described in :numref:`chapConv`.

.. _star-jobi:

Defining job output
~~~~~~~~~~~~~~~~~~~~~~~~~

    One can consider three types of job output: event data (including
    event collections and n-tuples), statistical data (histograms) and
    printout. Here we discuss only the simplest (printout); histograms
    are discussed in :numref:`chapHist`, event
    data in :numref:`data-save`, event collections in :numref:`serv-prop`.

    Printout in Gaudi is handled by the message service (described in
    :numref:`chapServ`), which allows to control
    the amount of printout according to severity level. The global
    threshold for printout is set by the option on line 10 - in this example only messages
    of severity level WARNING or above will be printed. This can be
    over-ridden for individual algorithms or services, as in
    line 11, where the threshold for EventSelector is set to DEBUG.

Algorithms
--------------

    The subject of specialising the Algorithm base class to do something
    useful will be covered in detail in :numref:`chapAlgo`. Here we will limit ourselves to
    looking at an example HelloWorld class.

The HelloWorld.h header file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    The HelloWorld class definition is shown in :numref:`lstg-helloheader`

    .. was 4.3

    .. code-block:: cpp
        :name: lstg-helloheader
        :caption: The header file of the class: HelloWorld

        // Include files
        #include "GaudiKernel/Algorithm.h"
        // Required for inheritance
        #include "GaudiKernel/Property.h"
        #include "GaudiKernel/MsgStream.h"

        class HelloWorld : public Algorithm {

        public:

          /// Constructor of this form must be provided
          HelloWorld(const std::string& name, ISvcLocator* pSvcLocator);

          /// Three mandatory member functions of any algorithm
          StatusCode initialize();
          StatusCode execute();
          StatusCode finalize();

        private:

          /// These data members are used in the execution of this algorithm
          /// and are set in the initialisation phase by the job options service
          int m_int;
          double m_double;
          std::string m_string;
        };


    Note the following:

    | · The class is derived from the Algorithm base class as must be all specialised algorithm classes. This implies that the Algorithm.h file must be included (line 6).
    | · All derived algorithm classes must provide a constructor with the parameters shown in line 9 . The first parameter is the name of the algorithm and is used amongst other things to locate any options that may have been specified in the job options file.
    | · The HistoAlgorithm class has three (private) data members, defined in lines 18 to 20. These are properties that can be set via the job options file.
    | · The three methods on lines 12 to 14 must be implemented, since they are pure virtual in the base class.

The HelloWorld implementation file
----------------------------------------

    The implementation file contains the actual code for the constructor
    and for the methods: initialize(), execute() and finalize(). It also
    contains two lines of code for the HelloWorld factory, which we will
    discuss in section :numref:`algo-crea`

    | The constructor
    |
    |  must call the base class constructor, passing on its two arguments. As usual, member variables should be initialised. Here we declare and initialise the member variables that we wish to be set by the job options service. This is done by calling the declareProperty() method.

    .. code-block:: cpp

        HelloWorld::HelloWorld(const std::string& name, ISvcLocator* ploc) : Algorithm(name, ploc) {
            //-----------------------------------------------------------------------
            // Declare the algorithm's properties
            declareProperty( "Int", m_int = 100 );
            declareProperty( "Double", m_double = 100.);
            declareProperty( "String", m_string = std::string("one hundred"));
        }

    | Initialisation
    |
    |  The application manager invokes the sysInitialize() method of the algorithm base class which, in turn, invokes the initialize() method of the base class, the setProperties() method, and finally the initialize() method of the concrete algorithm class. As a consequence all of an algorithm's properties will have been set before its initialize() method is invoked, and all of the standard services such as the message service are available. This is discussed in more detail in :numref:`chapAlgo`.

    Looking at the code in the example (:numref:`lstg-helloinit`) we see that we are now able to
    print out the values of the algorithm's properties, using the
    message service and the MsgStream utility class. A local MsgStream
    object is created (line 3 ), which
    uses the Algorithm's standard message service via the msgSvc()
    accessor, and the algorithm's name via the name() accessor. The use
    of these is discussed in more detail in :numref:`chapServ`.

    Note that the job will stop if the initialize() method of any
    algorithm does not return StatusCode::SUCCESS. This is to avoid
    processing with a badly configured application.:

    .. was 4.4

    .. code-block:: cpp
        :name: lstg-helloinit
        :caption: Example of initialize() method


        StatusCode HelloWorld::initialize() {
        //----------------------------------------------------------------------
            MsgStream log(msgSvc(), name());
            log << MSG::INFO << "initializing...." << endreq;
            log << MSG::INFO << "Property Int = " << m_int << endreq;
            log << MSG::INFO << "Property Double = " << m_double << endreq;
            log << MSG::INFO << "Property String = " << m_string << endreq;

            m_initialized = true;
            return StatusCode::SUCCESS;
        }

    | execution
    |
    |  The execute() method is called by the application manager once for every event. This is where most of the real action should take place. The trivial HelloWorld class just prints out a message... Note that the method must return StatusCode::SUCCESS on successful completion. If a particular algorithm returns a FAILURE status code more than a (configurable) maximum number of times, the application manager will decide that this algorithm is badly configured and jump to the finalisation stage before all events have been processed.

    .. code-block:: cpp

        StatusCode HelloWorld::execute() {
        //----------------------------------------------------------------------
            MsgStream log( msgSvc(), name() );
            log << MSG::INFO << "executing...." << endreq;

            return StatusCode::SUCCESS;
        }


    |  Finalisation
    |
    |   The finalize() method is called at the end of the job. In this trivial example a message is printed.

    .. code-block:: cpp

        StatusCode HelloWorld::finalize() {
        //----------------------------------------------------------------------
            MsgStream log(msgSvc(), name());
            log << MSG::INFO << "finalizing...." << endreq;

            return StatusCode::SUCCESS;
        }

.. _star-exec:

Job execution
-----------------

    From the main program and the CMT requirements file we can make an
    executable, as explained in section :numref:`inst-usin`. This executable together
    with the file of job options form a job which may be submitted for
    batch or run interactively. :numref:`fig-extrace` shows a trace of an example
    program execution. The diagram is not intended to be complete,
    merely to illustrate a few of the points mentioned earlier in the
    chapter.

    .. figure:: images/Exampletrace.png
        :name: fig-extrace

        A sequence diagram showing a part of the execution of an example program

    | 1. The application manager instantiates the required services and initialises them. The message service is done first to allow the other services to use it, and the job options service is second so that the other services may be configured at run time.
    | 2. The algorithms which have been declared to the application manager within the job options (via the TopAlg option) are created. We denote these algorithms "top-level" as they are the only ones controlled directly by the application manager. For illustration purposes we instantiate an EmptyAlgorithm and a HistoAlgorithm.
    | 3. The top-level algorithms are initialised. Their properties (if they have any) are set and they may make use of the message service. If any algorithm fails to initialise, the job is stopped.
    | 4. The application manager now starts to loop over events. After each event is read, it executes each of the top level algorithms in order. The order of execution of the algorithms is the order in which they appear in the TopAlg option. This will continue until the required number of events has been processed, unless one or more of the algorithms return a FAILURE status code more than the maximum number of times, in which case the application manager will jump to the finalisation stage before all events have been processed.
    | 5. After the required data sample has been read the application manager finalises each top level algorithm.
    | 6. Services are finalised.
    | 7. All objects are deleted and resources freed. The program terminates.

Examples distributed with Gaudi
-----------------------------------

    A number of examples is included in the current release of the
    framework, in the GaudiExamples package. The package has some
    sub-directories in addition to the standard ones shown in :numref:`fig-packlay`. The options sub-directory
    contains files of standard job options common to many examples.
    These files are included in the job options of the specific examples
    when necessary. The specific job options files can be found in the
    home sub-directory.

    The code of the examples is in sub-directories of the src directory,
    one sub-directory per example. The intention is that each example
    demonstrates how to make use of some part of the functionality of
    the framework. The list of available examples is shown in :numref:`tab-startexamples`.

    .. table:: List of examples available in Gaudi release v9
        :name: tab-startexamples

        +--------------------------------------+--------------------------------------+
        | Example Name                         | Target Functionality                 |
        +======================================+======================================+
        | AlgSequencer                         | Illustraing the use of the sequencer |
        |                                      | algorithm provided with teh GaudiAlg |
        |                                      | package                              |
        +--------------------------------------+--------------------------------------+
        | AlgTool                              | Example implementation and use of a  |
        |                                      | Gaudi Tool                           |
        +--------------------------------------+--------------------------------------+
        | Common                               | Actually not a complete example:     |
        |                                      | contains the main program used in    |
        |                                      | the examples                         |
        +--------------------------------------+--------------------------------------+
        | GPython                              | Exercise the Python scripting        |
        |                                      | packages                             |
        +--------------------------------------+--------------------------------------+
        | Properties                           | Trivial algorithm showing how to set |
        |                                      | and retrieve Properties              |
        +--------------------------------------+--------------------------------------+
        | RandomNumber                         | Example of use of the Random Number  |
        |                                      | service                              |
        +--------------------------------------+--------------------------------------+
        | RootIO                               | Two examples, reading and writing    |
        |                                      | persistend data with ROOT I/O        |
        +--------------------------------------+--------------------------------------+
