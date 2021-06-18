.. include:: ./global.rst

|newpage|

|Gaudi logo|

.. _chapScri:

Scripting and Interactivity
==============================

Overview
-------------

    A scripting capability has been added to the Gaudi framework. The
    current functionality is likely to change rapidly, so users should
    check with the latest release notes for changes or new functionality
    that might not be documented here.

    In keeping with the design philosophy of the Gaudi architecture,
    scripting is defined by an abstract scripting service interface,
    with the possibility of there being several different
    implementations. The first implementation available is based on
    Python, a public-domain programming language. Python is ideal both
    as a scripting interface for modern systems, and as a standalone
    rapid-development language. Its object-oriented nature mixes well
    with frameworks written in C++.

    The Python scripting language will not be described in detail here.
    There are many Python books available, among them we recommend:

    | · Learning Python, by M. Lutz & D. Ascher, O'Reilly, 1999
    | · Programming Python (2nd ed.), by M. Lutz, O'Reilly, 2001

How to enable Python scripting
-----------------------------------

    Three different mechanisms are available for enabling Python
    scripting.

    | 1. Replace the job options text file by a Python script that is specified on the command line.
    | 2. Use a job options text file which hands control over to the Python shell once the initial configuration has been established.
    | 3. Load and start a Gaudi application from a Python shell.

Using a Python script for configuration and control
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    One can avoid using a job options text file for configuration by
    specifying a Python script as a command line argument, as shown in
    :numref:`lstg-pyjobo`.

    .. was 14.1

    .. code-block:: python
        :name: lstg-pyjobo
        :caption: Using a Python script for job configuration

        myjob MyPythonScript.py [1]

    Note: The file extension .py is used to identify the job options file as a Python script. All other extensions are assumed to be job options text files.

    This approach may be used in two modes. The first uses such a script
    to establish the configuration, but results in the job being left at
    the Python shell prompt. This supports interactive sessions. The
    second specifies a complete configuration and control sequence and
    thus supports a batch style of processing. The particular mode is
    controlled by the presence or absence of Gaudi-specific Python
    commands described in Section :numref:`scri-ctje`.

Using a text JobOptions file and giving control to the Python interactive shell
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Python scripting is enabled when using a job options text file for
    job configuration by adding the lines shown in :numref:`lsgt-joboptspy` to the job options file.

    .. was 14.2

    .. code-block:: python
        :name: lsgt-joboptspy
        :caption: Job Options text file entries to enable Python scripting

        ApplicationMgr.DLLs += { "GaudiPython" };  # [1]
        ApplicationMgr.Runable = "PythonScriptingSvc"; # [2]
        PythonScriptingSvc.StartupScript = "../options/AnalysisTest.py"; # [3]

    Notes:

    | 1. This entry specifies the component library that implements Python scripting.
    | 2. This entry specifies that the Python scripting should take the control (runable) of the application.
    | 3. Optional startup python script.

    Once the initial configuration has been established by the job
    options text file, control will be handed over to the Python shell
    when the startup script, if specified, will be executed. The user
    can then issue interactive commands.

Starting a Gaudi application from the Python shell
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    It is also possible to bootstrap a Gaudi application directly from a
    Python shell. The user needs to import the Python extension module
    called gaudimodule, which allows the interaction with Gaudi from
    Python. :numref:`lstg-expyexegaudi` shows a small
    Python program that instantiates a Gaudi application, configure it
    and runs for a number of events. This program would work from the
    Python shell a long as the environment (LD\_LIBRARY\_PATH/PATH) is
    properly set up.

    .. was 14.3

    .. code-block:: python
        :name: lstg-expyexegaudi
        :caption: Example of a Python program that executes a Gaudi program

        from gaudimodule import *
        theApp = AppMgr()
        theApp.JobOptionsType = 'NONE'
        theApp.EvtSel = 'NONE'
        theApp.config()
        theApp.Dlls = ['GaudiAlg']
        myseq = theApp.algorithm('Sequencer/MySeq')
        myseq.Members = ['EventCounter/Count1', 'EventCounter/Count2']
        theApp.topAlg = ['MySeq']
        theApp.initialize()
        theApp.run(10)
        theApp.exit()


Current functionality
--------------------------

    The current functionality is limited to the following capabilities:

    | 1. The ability to set and get basic properties for all framework components (Algorithms, Services, Auditors etc.) and the main ApplicationMgr that controls the application. Arrays of simple properties are mapped into Python Lists.
    | 2. The ability to interact with the transient data stores. Browsing store contents, registering, unregistering and retrieving objects, getting and setting object data members (with the help of the IntrospectionSvc) and limited method invocation.
    | 3. The ability to interact with the Histograms (1D and 2D) in the transient store. This includes booking, filling, dumping contents, etc.
    | 4. The ability to add new services and component libraries and access their capabilities.
    | 5. The ability to control the execution of the application by adding Algorithms into the list of top level Algorithms, executing single events or a set of events, executing single Algorithms, etc.
    | 6. The ability to define Python Algorithms that will be managed and scheduled as normal Gaudi Algorithms.

Property manipulation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    An example of the use of the scripting language to display and set
    component properties is shown in :numref:`lstg-propmanipsh`:

    .. was 14.4

    .. code-block:: python
        :name: lstg-propmanipsh
        :caption: Property manipulation from the Python interactive shell

        >>> theApp [1][2]
        <AppMgr object at 00AD22E8>
        >>> theApp.ExtSvc [3]
        ['IntrospectionSvc', 'ParticlePropertySvc']
        >>> theApp.ExtSvc = theApp.ExtSvc + ['AnotherSvc'] [4]
        >>> theApp.ExtSvc
        ['IntrospectionSvc', 'ParticlePropertySvc', 'AnotherSvc']
        >>> theApp.EvtMax = 100
        >>> theApp.properties() [5]
        {'EvtMax': 100, 'JobOptionsType': 'NONE', 'TopAlg': ['seq1', 'PhysAnalAlg'], 'Go': 0, 'Exit': 0, 'Dlls': ['GaudiAlg', 'GaudiIntrospection'], 'JobOptionsPath': '..\home\test.py', 'OutStream': [], 'OutputLevel': 3, 'EventLoop': 'EventLoopMgr', 'HistogramPersistency': 'NONE', 'EvtSel': 'NONE', 'ExtSvc': ['IntrospectionSvc', 'ParticlePropertySvc', 'AnotherSvc'], 'Runable': 'PythonScriptingSvc'}
        >>> theApp.algorithms() [6]
        ['seq1', 'WriteAlg', 'PhysAnalAlg']
        >>> alg = Algorithm('WriteAlg') [7]
        >>> alg.properties()
        {'ErrorCount': 0, 'OutputLevel': 0, 'AuditExecute': 1, 'AuditInitialize': 0, 'Enable': 1, 'AuditFinalize': 0, 'ErrorMax': 1}

    Notes:

    | 1. The ">>>" is the Python shell prompt. Typing the name of a variable, Python prints its value in textual form.
    | 2. The variable theApp is always defined and it represents the ApplicationMgr. An alias g has also been defined.
    | 3. The name of the property is used as a data member in Python. It returns the correct type directly.
    | 4. You can use the properties in normal Python expressions
    | 5. The list of all properties (as a Python Dictionary) of a component can be obtained with the method properties().
    | 6. The list of algorithms can be obtained with the method algorithms().
    | 7. To access an Algorithm by name (creating it if it does not exist) the constructor Algorithm() is used. Similarly for services with Service().

Creating Algorithms and Services
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    It is possible to create new Algorithms or Services as a result of a
    scripting command. Examples of this are shown in :numref:`lstg-expycralg`:

    .. was 14.5

    .. code-block:: python
        :name: lstg-expycralg
        :caption: Examples of Python commands that create new Algorithms or Services

        >>> myseq = theApp.algorithm('Sequencer/MySeq')
        >>> myseq.members = ['HelloWorld', 'WriteAlg']
        MySeq INFO HelloWorld doesn't exist - created and appended to member list-table
        MySeq INFO WriteAlg already exists - appended to member list
        >>> theApp.topAlg = ['MySeq']
        >>> g.run(1)
        HelloWorld INFO initializing....
        HelloWorld INFO executing....
        WriteAlg INFO Generated event 5
        StatusCode::SUCCESS

    If the specified Algorithm or Service does not exist, it is created.
    Its properties can immediately be accessed for read and write. They
    will be initialized when the application will start processing
    events.

Interaction with Transient Data Stores
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    It is possible to get the list of data stores and to interact with
    them. The following commands are available:

    | theApp.datastores()
    |  Returns the list of all available data stores (all services that implement the IDataProviderSvc interface).
    |
    | theApp.datastore(name)
    |  Returns a generic data store.
    |
    | theApp.histoSvc()
    |  Returns the standard histogram data service ("HistogramDataSvc")
    |
    | theApp.evtSvc()
    |  Returns the standard event data service ("EventDataSvc")
    |
    | theApp.detSvc()
    |  Returns the standard detector data service ("DetectorDataSvc")
    |
    | theApp.ntupleSvc()
    |  Returns the standard ntuple data service ("NTupleSvc")
    |
    | datastore.dump()
    |  Prints the contents of the transient data store (names and types)
    |
    | datastore.clear()
    |  Clears the contents of the transient data store
    |
    | datastore.object(name), datastore[name], datastore.retrieve(name)
    |  Retrieves the named object from the transient data store. If the IntrospectionSvc [#]_ is loaded and the dictionaries are available for the requested object then it creates an object that can be introspected.
    |
    | datastore.register(name, obj), datastore.unregister(name)
    |  Registers and unregisters an object to/from the data store
    |
    | datastore.clear(),datastore.clear(name)
    |  Clears the whole store or a sub-tree.

    A complete example of the capabilities on the interaction with the
    event transient store is shown in

    .. code-block:: python
        :name: lstg-exinttes
        :caption: Example of interaction with the event transient store taken from LHCb

        evt = theApp.evtSvc()
        evt.dump()
        parts = evt['/Event/MC/MCParticles']
        for p in parts :
            print p.particleID.id

Interaction with Histograms
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    The data store commands described in the previous section are also
    available for the histogram data store. The following histogram
    specific commands are available in addition:

    | histosvc.histo(name), histosvc[name]
    |  Retrieves the histogram from the histogram data store. It returns either an 1D or 2D histogram.
    |
    | histosvc.book(id, title, xbin, xmin, xmax [,ybin, ymin, ymax])
    |  Books 1D or 2D histogram and registers it in the histogram store.
    |
    | histo.title(), dim(), mean(), rms(), maxbin(), minbin()
    |  Returns the title, dimensions, mean, rms, maximum bin contents, minimum bin contents of the histogram
    |
    | histo.fill(x [,y,w]), histo.reset()
    |  Fills 1D or 2D histogram, resets the contents
    |
    | histo.heights(), entries(), errors(), edges()
    |  Returns in a Python list the heights, entries, errors and edges of the 1D histogram
    |
    | histo.projections()
    |  Returns X and Y projections (tuple) of a 2D histogram

Interaction with Data Objects
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    If the dictionaries of the classes have been loaded by the object
    introspection service, then it is possible to browse and interact
    with the data objects in the transient store. Loading the
    dictionaries is done by configuring the application as it is shown
    in :numref:`lstg-confrefljobo`

    .. was 14.7

    .. code-block:: python
        :name: lstg-confrefljobo
        :caption: Configuring the Gaudi Introspection Service using the JobOptions text file

        ApplicationMgr.DLLs += { "GaudiIntrospection"};
        pplicationMgr.ExtSvc += { "IntrospectionSvc" };
        IntrospectionSvc.Dictionaries = { "PhysEventDict" };


    The following commands are currently available:

    | obj.<attribute>
    |  Returns the value of the attribute for the object. If the attribute is of a complex type it returns a reference to it, such that it can be browsed recursively.
    |
    | obj.<attribute> = value
    |  Sets the value of attribute for the object if this is a simple type.
    |
    | obj.<method>()
    |  Invokes the class method. This is currently only available for methods without arguments.
    |
    | obj.values()
    |  Returns a Python dictionary with all the attributes and their values.
    |
    | obj.names()
    |  Returns a Python list with all the available attribute names.
    |
    | obj.desc()
    |  Prints the description of the class of the object.
    |
    | obj.type()
    |  Returns the object type (C++ class)

.. _scri-ctje:

Controlling job execution
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    There exist a few commands to control the job execution
    interactively:

    | theApp.run(number)
    |  The control is returned from the Python shell to the Gaudi environment with this command. The argument is the number (-1 for infinite) of events to be processed, after which control will be returned to the Python shell.
    |
    | theApp.exit()
    |  Typing Ctrl-D (or Ctrl-Z in Windows) at the Python shell prompt will cause an orderly termination of the job. Alternatively, this command will also cause an orderly application termination.

Physics Analysis Environment
---------------------------------

    It is possible to declare an Algorithm in Python that can be
    declared to the list of top level algorithms to be executed for each
    event by the ApplicationMgr. This can be useful for constructing an
    interactive physics analysis environment. An example is shown in
    :numref:`lstg-exintana`

    .. code-block:: python
        :name: lstg-exintana
        :caption: Example Analysis

        # -- User analysis algortihm class
        class PhysAnalAlg(PyAlgorithm): [1]

            def initialize(self):
                global h1
                h1 = his.book('h1','Histogram Test', 10, 0., 10.)
                print '....User Analysis Initialized'
                return 1

            def finalize(self):
                print 'Finalizing User Analysis...'
                return 1

            def execute(self)
                cands = evt['Anal/AxPartCandidates']
                print 'Found '+ \`len(cands)\` + ' candidates'
                for c in cands :
                    h1.fill(c.momentum)
                return 1

        # -- Initialization and Configuration
        his = theApp.histoSvc() [2]
        evt = theApp.evtSvc()
        det = theApp.detSvc()
        pdt = PartSvc()
        physalg = PhysAnalAlg(theApp,'PhysAnalAlg') [3]
        theApp.topAlg = theApp.topAlg + ['PhysAnalAlg']

    Notes:

    | 1. The analysis algorithm must inherit from the class PyAlgorithm
    | 2. Useful variables to avoid long typing
    | 3. An instance of the new class needs to be instantiated and declared in the list of top level algorithms.

.. [#] See :numref:`serv-refl`
