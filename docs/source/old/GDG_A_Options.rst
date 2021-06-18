.. include:: ./global.rst

|newpage|

|Gaudi logo|

.. _chapOpti:

Appendix B Options for standard components
==========================================

    The following is a list of options that may be set for the standard components: e.g. data files for input, print-out level for the message service, etc. The options are listed in tabular form for each component along with the default value and a short explanation. The component name is given in the table caption thus: [ComponentName].

    .. table:: Standard Options for the Application manager [ApplicationMgr]
        :name: tab-b1

        ==================== ================ =====================================================================================================================================================
        Option name          Default value    Meaning
        ==================== ================ =====================================================================================================================================================
        EvtSel               ""               If "NONE", no event input [#]_
        EvtMax               -1               Maximum number of events to process. The default is -1 (infinite) unless EvtSel = "NONE"; in which case it is 10.
        TopAlg               {}               List of top level algorithms. Format:
                                              {<Type>/<Name>[, <Type2>/<Name2>,...]};
        ExtSvc               {}               List of external services to be explicitly created by the ApplicationMgr (see section :numref:`serv-requ`). Format:
                                              {<Type>/<Name>[, <Type2>/<Name2>,...]};
        OutStream            {}               Declares an output stream object for writing data to a persistent store, e.g. {"DstWriter"}; See also :numref:`tab-b10`
        DLLs                 {}               Search list of libraries for dynamic loading. Format: {<dll1>[,<dll2>,...]};
        HistogramPersistency "NONE"           Histogram and N-tuple persistency mechanism. Available options are "HBOOK", "ROOT", "NONE"
        Runable              "AppMgrRunable"  Type of runable object to be created by Application manager
        EventLoop            "EventLoopMgr"   Type of event loop:
                                                "EventLoopMgr" is standard event loop
                                                "MinimalEventLoop" executes algorithms but does not read events
        OutputLevel          MSG::INFO        Same as MessageSvc.OutputLevel. See :numref:`tab-b2` for possible values
        JobOptionsType       "FILE"           Type of file (FILE implies ascii)
        JobOptionsPath       "jobOptions.txt" Path for job options source
        ==================== ================ =====================================================================================================================================================

    | NB: The last two options define the source of the job options file and so they cannot be defined in the job options file itself. There are two possibilities to set these options, the first one is using a environment variable called JOBOPTPATH or setting the option to the application manager directly from the main program [#]_. The coded option takes precedence.

.. [#] A basic DataObject object is created as event root ("/Event")
.. [#] The setting of properties from the main program is discussed in :numref:`chapStar`.

    .. container::

        .. table:: Standard Options for the message service [MessageSvc]
            :name: tab-b2

            =========== ======================== =====================================
            Option name Default value            Meaning
            =========== ======================== =====================================
            OutputLevel 0                        Verboseness threshold level:
                                                 0=NIL,1=VERBOSE, 2=DEBUG, 3=INFO,
                                                 4=WARNING, 5=ERROR, 6=FATAL, 7=ALWAYS
            Format      "% F%18W%S%7W%R%T %0W%M" Format string.
            =========== ======================== =====================================

        .. table:: Standard Options for all algorithms [<myAlgorithm>]. Any algorithm derived from the Algorithm base class can override the global Algorithm options thus:
            name: tab-b3

            =============== ============= ===================================================================================================================
            Option name     Default value Meaning
            =============== ============= ===================================================================================================================
            OutputLevel     0             Message Service Verboseness threshold level. See :numref:`tab-b2` for possible values
            Enable          true          If false, application manager skips execution of this algorithm
            ErrorMax        1             Job stops when this number of errors is reached
            ErrorCount      0             Current error count
            AuditInitialize false         Enable/Disable auditing of Algorithm initialisation
            AuditExecute    true          Enable/Disable auditing of Algorithm execution
            AuditFinalize   false         Enable/Disable auditing of Algorithm finalisation
            =============== ============= ===================================================================================================================

        .. table:: Standard Options for all services [<myService>]. Any service derived from the Service base class can override the global
            :name: tab-b4

            =========== ============= ===================================================================================================================
            Option name Default value Meaning
            =========== ============= ===================================================================================================================
            OutputLevel 0             Message Service Verboseness threshold level. See :numref:`tab-b2` for possible values
            =========== ============= ===================================================================================================================

    .. container::

        .. table:: Standard Options for all Tools [<myTool>]. Any tool derived from the AlgTool base class can override the global
            :name: tab-b5

            =========== ============= ===================================================================================================================
            Option name Default value Meaning
            =========== ============= ===================================================================================================================
            OutputLevel 0             Message Service Verboseness threshold level. See :numref:`tab-b2` for possible values
            =========== ============= ===================================================================================================================

        .. table:: Standard Options for all Associators [<myAssociator>]
            :name: tab-b6

            ============ ============= ===============================================================================
            Option name  Default value Meaning
            ============ ============= ===============================================================================
            FollowLinks  true          Instruct the associator to follow the links instead of using cached information
            DataLocation ""            Location where to get association information in the data store
            ============ ============= ===============================================================================

        .. table:: Standard Options for Auditor service [AuditorSvc]
            :name: tab-b7

            =========== ============= ==============================================================================
            Option name Default value Meaning
            =========== ============= ==============================================================================
            Auditors    {};           List of Auditors to be loaded and to be used.
                                      See section :numref:`serv-audi` for list of possible auditors
            =========== ============= ==============================================================================

        .. table:: Standard Options for all Auditors [<myAuditor>]. Any Auditor derived from the Auditor base class can override the global Auditor options thus:
            :name: tab-b8

            =========== ============= ===================================================================================================================
            Option name Default value Meaning
            =========== ============= ===================================================================================================================
            OutputLevel 0             Message Service Verboseness threshold level. See :numref:`tab-b2` for possible values
            Enable      true          If false, application manager skips execution of the auditor
            =========== ============= ===================================================================================================================

        .. table:: Options of Algorithms in GaudiAlg package(see :numref:`algo-filt`)
            :name: tab-b9

            ============== ============= ============= ========================================================
            Algorithm name Option Name   Default value Meaning
            ============== ============= ============= ========================================================
            EventCounter   Frequency     1;            Frequency with which number of events should be reported
            Prescaler      PercentPass   100.0;        Percentage of events that should be passed
            Sequencer      Members                     Names of algorithms in the sequence
            Sequencer      BranchMembers               Names of algorithms on the branch
            Sequencer      StopOverride  false;        If true, do not stop sequence if a filter fails
            ============== ============= ============= ========================================================

    .. container::

        .. table:: Options available for output streams (e.g. DstWriter). Output stream objects are used for writing user created data into data files or databases. They are created and named by setting the option ApplicationMgr.OutStream. For each output stream the following options are available
            :name: tab-b10

            ================ ===================== ================================================================================
            Option name      Default value         Meaning
            ================ ===================== ================================================================================
            ItemList         {}                    The list of data objects to be written to this stream, e.g.
                                                   {"/Event#1","Event/MyTracks/#1"};
            Preload          true;                 Preload items in ItemList
            Output           ""                    Output data stream specification. Format:
                                                   {"DATAFILE='mydst.root' TYP='ROOT'"};
            OutputFile       ""                    Output file specification - same as DATAFILE in previous option
            EvtDataSvc       "EventDataSvc"        The service from which to retrieve objects.
            EvtConversionSvc "EventPersistencySvc" The persistency service to be used
            AcceptAlgs       {}                    If any of these algorithms sets filterflag=true; the event is accepted
            RequireAlgs      {}                    If any of these algorthms is not executed, the event is rejected
            VetoAlgs         {}                    If any of these algorithms does not set filterflag = true; the event is rejected
            ================ ===================== ================================================================================

        .. table:: Standard Options for persistency services (e.g. EventPersistencySvc)
            :name: tab-b11

            =========== ============= =====================================================================================================
            Option name Default value Meaning
            =========== ============= =====================================================================================================
            CnvServices {}            Conversion services to be used by the service to load or store persistent data (e.g. "RootEvtCnvSvc")
            =========== ============= =====================================================================================================

        .. table:: Standard Options for conversion services (e.g. RootEvtCnvSvc)
            :name: tab-b12

            =========== ============= ====================================
            Option name Default value Meaning
            =========== ============= ====================================
            DbType      ""            Persistency technology (e.g. "ROOT")
            =========== ============= ====================================

        .. table:: Standard Options for the histogram service [HistogramPersistencySvc]
            :name: tab-b13

            =================== ============= ============================================================================================================================================================
            Option name         Default value Meaning
            =================== ============= ============================================================================================================================================================
            OutputFile          ""            Output file for histograms. Histograms not saved if not given.
            RowWiseNTuplePolicy "FLOAT_ONLY"  Persistent representation of NTuple data types. Other possible value is "USE_DATA_TYPES". See :numref:`ntup-spec` for details
            PrintHistos         false         Print the histograms also to standard output (HBOOK only)
            =================== ============= ============================================================================================================================================================

        .. table:: Standard Options for the N-tuple service [NTupleSvc] (see :numref:`ntup-spec`)
            :name: tab-b14

            =========== ============= ====================================================
            Option name Default value Meaning
            =========== ============= ====================================================
            Input       {}            Input file(s) for n-tuples. Format:
                                      {"FILE1 DATAFILE='tuple1.typ' OPT='OLD' ",
                                      ÃƒÂ‚ ["FILE2 DATAFILE='tuple2.typ' OPT='OLD' ",...]}
            Output      {}            Output file(s) for n-tuples. Format:
                                      {"FILE1 DATAFILE='tuple1.typ' OPT='NEW'",
                                      ÃƒÂ‚ ["FILE2 DATAFILE='tuple2.typ' OPT='NEW'",...]}
            StoreName   "/NTUPLES"    Name of top level entry
            =========== ============= ====================================================

        .. table:: Standard Options for the Event Collection service [TagCollectionSvc] (see :numref:`ntup-ecpe`)
            :name: tab-b15

            =========== ============= ===========================================================================================
            Option name Default value Meaning
            =========== ============= ===========================================================================================
            Output      {}            Output file specification. See :numref:`ntup-ecpe` for details
            StoreName   "/NTUPLES"    Name of top level entry
            =========== ============= ===========================================================================================

        .. table:: Standard Options for the standard event selector [EventSelector]
            :name: tab-b16

            =========== ============= ===========================================================================
            Option name Default value Meaning
            =========== ============= ===========================================================================
            Input       {}            Input data stream specification.
                                      Format: "<tagname> = '<tagvalue>' <opt>"
                                      Possible tags are different depending on input data type.
                                      For Event data, see :numref:`data-redp`
                                      For Event Collections, see :numref:`ntup-ecpe`
            FirstEvent  1             First event to process (allows skipping of preceding events)
            PrintFreq   10            Frequency with which event number is reported
            =========== ============= ===========================================================================

        .. table:: Event Tag Collection Selector [EventCollectionSelector]. The following options are used internally by the EventCollectionSelector. They should not normally be used directly by users, who should set them via the "tags" of the EventSelector.Input option
            :name: tab-b17

            ============== ======================================== ================== =============================
            Option name    Corresponding tag of EventSelector.Input Default value      Meaning
            ============== ======================================== ================== =============================
            CnvService     SVC                                      "EvtTupleSvc"      Conversion service to be used
            Authentication AUTH                                     ""                 Authentication to be used
            Container                                               "B2PiPi"           Container name
            Item                                                    "Address"          Item name
            Criteria       SEL                                      ""                 Selection criteria
            DB             DATAFILE                                 ""                 Database name
            DbType         TYP                                      ""                 Database type
            Function       FUN                                      "NTuple::Selector" Selection function
            ============== ======================================== ================== =============================

        .. table:: Standard Options for Random Numbers Generator Service [RndmGenSvc]
            :name: tab-b18

            =========== =============================== ==================================
            Option name Default value                   Meaning
            =========== =============================== ==================================
            Engine      "HepRndm::Engine<RanluxEngine>" Random number generator engine
            Seeds                                       Table of generator seeds
            Column      0                               Number of columns in seed table -1
            Row         1                               Number of rows in seed table -1
            Luxury      3                               Luxury value for the generator
            UseTable    false                           Switch to use seeds table
            =========== =============================== ==================================

        .. table:: Standard Options for Particle Property Service [ParticlePropertySvc]
            :name: tab-b19

            ====================== =============================== =====================================
            Option name            Default value                   Meaning
            ====================== =============================== =====================================
            ParticlePropertiesFile "($LHCBDBASE)/cdf/particle.cdf" Particle properties database location
            ====================== =============================== =====================================

    .. container::

        .. table:: Standard Options for Chrono and Stat Service [ChronoStatSvc]
            :name: tab-b20

            ====================== ============= ========================================================
            Option name            Default value Meaning
            ====================== ============= ========================================================
            ChronoPrintOutTable    true          Global switch for profiling printout
            PrintUserTime          true          Switch to print User Time
            PrintSystemTime        false         Switch to print System Time
            PrintEllapsedTime      false         Switch to print Elapsed time (Note typo in option name!)
            ChronoDestinationCout  false         If true, printout goes to cout rather than MessageSvc
            ChronoPrintLevel       3             Print level for profiling (values as for MessageSvc)
            ChronoTableToBeOrdered true          Switch to order printed table
            StatPrintOutTable      true          Global switch for statistics printout
            StatDestinationCout    false         If true, printout goes to cout rather than MessageSvc
            StatPrintLevel         3             Print level for profiling (values as for MessageSvc)
            StatTableToBeOrdered   true          Switch to order printed table
            ====================== ============= ========================================================

B.1  Obsolete options
---------------------

    The following options are obsolete and should not be used. They are documented here for completeness and may be removed in a future release.

    .. table:: Obsolete Options
        :name: tab-b21

        ==================== ======================================================================
        Obsolete Option      Replacement
        ==================== ======================================================================
        EventSelector.EvtMax ApplicationMgr.EvtMax (:numref:`tab-b1`)
        ==================== ======================================================================
