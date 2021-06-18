.. include:: ./global.rst

|newpage|

|Gaudi logo|

.. _chapInst:

Release notes and software installation
=========================================

Release History
-------------------

    The Gaudi architecture and framework, which was initially developed
    by the LHCb collaboration, became a joint development project
    between several experiments, starting from release v6. At this time
    the package structure was modified, to split the experiment specific
    packages from the common packages. The following table reflects the
    version history since the re-packaging. For the history of earlier
    releases, please refer to previous versions of the (LHCb) Gaudi
    Users Guide.

    +--------------------------+--------------------------+----------------------------+
    | Version                  | Date                     | Package List          |    |
    +==========================+==========================+============================+
    | v7                       | 23/03/2001               | | GaudiPolicy[v4],         |
    |                          |                          | | GaudiExamples[v7],       |
    |                          |                          | | GaudiKernel[v9],         |
    |                          |                          | | GaudiSvc[v5],            |
    |                          |                          | | GaudiAud[v3],            |
    |                          |                          | | GaudiAlg[v3],            |
    |                          |                          | | GaudiTools[v3],          |
    |                          |                          | | GaudiNagC[v5r3p1],       |
    |                          |                          | | GaudiDb[v3],             |
    |                          |                          | | HbookCnv[v9],            |
    |                          |                          | | RootHistCnv[v3],         |
    |                          |                          | | SIPython[v1r1]           |
    +--------------------------+--------------------------+----------------------------+
    | v8r1                     | July 2001                | | GaudiPolicy[v5],         |
    |                          |                          | | GaudiExamples[v8],       |
    |                          |                          | | GaudiKernel[v10],        |
    |                          |                          | | GaudiSvc[v6],            |
    |                          |                          | | GaudiAud[v4],            |
    |                          |                          | | GaudiAlg[v4],            |
    |                          |                          | | GaudiTools[v4],          |
    |                          |                          | | GaudiNagC[v5r3p2],       |
    |                          |                          | | GaudiDb[v4],             |
    |                          |                          | | GauiRootDb[v4],          |
    |                          |                          | | GaudiODBCDb[v3],         |
    |                          |                          | | HbookCnv[v10r1],         |
    |                          |                          | | RootHistCnv[v4],         |
    |                          |                          | | SIPython[v2]             |
    +--------------------------+--------------------------+----------------------------+
    | v9                       | Dec 2001                 | | GaudiPolicy[v5],         |
    |                          |                          | | GaudiExamples[v9],       |
    |                          |                          | | GaudiKernel[v11],        |
    |                          |                          | | GaudiSvc[v7],            |
    |                          |                          | | GaudiAud[v5],            |
    |                          |                          | | GaudiAlg[v5],            |
    |                          |                          | | GaudiTools[v5],          |
    |                          |                          | | GaudiNagC[v6],           |
    |                          |                          | | GaudiDb[v5],             |
    |                          |                          | | GauiRootDb[v5],          |
    |                          |                          | | GaudiODBCDb[v5],         |
    |                          |                          | | HbookCnv[v11],           |
    |                          |                          | | RootHistCnv[v5],         |
    |                          |                          | | GaudiPython[v2],         |
    |                          |                          | | GaudiObjDesc[v2],        |
    |                          |                          | | GaudiIntrospection[v2]   |
    +--------------------------+--------------------------+----------------------------+

Current Functionality
-------------------------

    We use an incremental and iterative approach for producing the Gaudi
    software. We plan to expand its capabilities release by release. The
    functionality list that follows is organized by categories.

    | Interfaces
    |  A set of interfaces that facilitates the interaction between the different components of the framework. Mainly these are interfaces to services.
    |  Basic framework services
    |  This set of services offer the minimal functionality needed for constructing applications. They are described in detail in :numref:`chapServ`.
    | The message service is used to send and format messages generated in the code, with an associated severity that is used for filtering and dispatching them.
    | The job options service allows the configuration of the application by end users assigning values to properties defined within the code; properties can be basic types (float, bool, int, string), or extended with bounds checking, hierarchical lists, and immediate callback from string "commands".
    | The Random Numbers service makes available several random number distributions via a standard interface, and ensures that applications use a unique random number engine in a reproducible fashion.
    | The Chrono service offers the functionality for measuring elapsed time and job execution statistics.
    | Auditors and AuditorSvc provide monitoring of various characteristics of the execution of Algorithms. Auditors are called before and after invocation of any Algorithm method.
    | The Incident service provides a synchronization between objects within the Application by using named incidents that are communicated to listener clients.
    | The Tools service, which provides management of Tools, is discussed in :numref:`chapTool`. Tools are lightweight objects which can be requested and used many times by other components to perform well defined tasks. A base class for associator tools has been added in this release.
    | Data services provide the access to the transient data objects (event, detector and statistical data). The data services are described in :numref:`chapData`, :numref:`chapEven`, :numref:`chapDetd`, :numref:`chapHist` and :numref:`chapNtup` . The basic building blocks for the implementation of the experiment specific data models are also described in :numref:`chapData`.
    |  Event data persistent storage
    |  The current version provides a set of generic classes for implementing event data persistency (GaudiDb package) and a set of classes supporting persistent I/O to ROOT files (GaudiRootDb package). Details can be found in :numref:`chapConv`.
    |  Histograms & N-tuples
    |  The framework provides facilities for creating histograms (1 and 2 dimensional) and n-tuples (row and column wise) from user algorithms. The histogram interface is the AIDA [AIDA]_ common interface. Saving histograms and n-tuples is currently implemented using the HBOOK and ROOT format. The interface to histograms and n-tuples from the user code should not be affected if the persistency representation is changed later. Details of the histogram and n-tuple facilities can be found in :numref:`chapHist` and :numref:`chapNtup` respectively.
    |  Event tag collections
    |  The framework provides facilities for creating and using collections of event tags (based on an n-tuples implementation) for fast direct access to physics events. The user can specify an event tag collection as input data to an application and perform sophisticated selections using the facilities existing in the data storage technology. This is explained in :numref:`chapNtup`.
    |  Detector description and geometry
    |  The framework provides facilities for accessing detector description and geometry data. This is described in :numref:`chapDetd`. A concrete implementation exists in LHCb, but is not distributed with Gaudi.
    |  Analysis services
    |  A number of facilities and services are included in the current release to facilitate writing physics analysis code. The GaudiAlg package is a collection of general purpose algorithms, including a sequencer which uses the filtering capability of algorithms to manage the execution of algorithm sequences in a filtering application (see :numref:`algo-filt`). The Particle Properties service (:numref:`serv-prse`) provides the properties of all the elementary particles. Numerical utilities are available via the CLHEP and NAG C libraries ( :numref:`chapUtil`).
    |  Visualization services
    |  The framework provides a mechanism for the visualisation of event and detector data. A prototyp implementation exists in LHCb, but is not distributed with Gaudi. This is briefly described in :numref:`chapVisu`.
    |  Object Description and Object Introspection
    |  The framework provides object modelling and description using XML files. Two code generation back-ends are currently available: to generate the data object header files and to generate the object dictionaries for the object introspection. Refer to
    |  Scripting services
    |  The framework provides a service for interfacing Python with a Gaudi application. The user can interact with a Gaudi application from the Python prompt. The current functionality allows the user to set and get properties from Algorithms and Services, interact with the data stores (event, detector and histogram) using the object introspection capability, and to schedule the execution of the application's algorithms. Refer to .
    |  Dynamic loading of libraries
    |  The framework can be used to implement different data processing applications for different environments. It is important that the services and their concrete implementations are extendable and configurable dynamically at run time and not statically. The latter would imply linking with all the available libraries producing huge executables. And in certain applications and environments some of the libraries will surely never be used. The framework provides support for dynamic libraries for the Windows and Linux platforms.

Changes between releases
----------------------------

Changes between current release (v9) and previous release (v8)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    |  · Object description and object introspection. Two new packages has been added that provide the object description based on XML files and run-time object introspection capability. Refer to :numref:`data-defi` and :numref:`serv-refl` for more details.
    |  · Python service. The scripting service based on Python has been re-implemented using the Boost library [#]_. Its functionality has been extended. Refer to :numref:`chapScri` for more details.
    |  · Algorithms. Added toolSvc() accessor to Algorithm base class.
    |  · Algorithm Tools. Added initialize() and finalize() methods in IAlgTool interface. The base class AlgTool implements them as dummy but allows an implementation of them on specific Algorithm Tools. Removed the need to implement a queryInterface() in specific Algorithm tools. Instead use the expression declareInterface<Ixxxx>(this) in the constructor.
    |  · A number of small internal framework improvements:
    |  · ApplicationMgr. Re-organization to relocate the management of services to ServiceManager class. The interfaces ISvcManager and ISvcLocator have changed.
    |  · Introduced a new constructor for InterfaceID that uses a name (class name) instead of an interface number.
    |  · JobOptions. Introduced new options #pragma print on, #pragma print off to switch the printing of job options on and off.
    |  · Histograms. New job option HistogramPersistencySvc.PrintHistos to steer printing to standard output. Allow RZ directory names up to 16 characters rather than 8.

Incompatible changes
````````````````````````````

    In this section we will list changes that users need to make to
    their code in order to upgrade to the current version of Gaudi from
    the previous version.

    | 1. In the area of Data Stores many low level base classes (DataObject, DataSvc, Converters, Registry, GenericAddress, etc.) have changed together with some basic interfaces (IConverter, IDataManagerSvc, IDataProviderSvc, etc.). This implies that some packages, typically converters packages, will need deep changes in the code. Instructions on how to upgrade them can be found in http://cern.ch/lhcb-comp/Frameworks/Gaudi/Gaudi_v9/Changes_cookbook.pdf.
    | End user algorithm packages should not be too affected by these changes.
    | 2. Removed the list of default interfaces in ApplicationMgr. Services are late created if needed. This may cause problems if the order of creation played a role. The Algorithms and Tools that were accessing service using the call serviceLocator()->service("name", interface) may require to force the creation of a previously default service by adding a third argument with true to force such creation if not existing.
    | 3. The constant CLID\_Event has been removed from ClassID.h. It needs to be defined now in the Event.h header file.
    | 4. Algorithm tools are required to implement an interface (pure abstract base class) using the facility provided for declaring it as mentioned above in the list of changes.

Changes between release v8 and release v7
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Incompatible changes
````````````````````````````

    In this section we will list changes that users need to make to
    their code in order to upgrade to version v8 of Gaudi from version
    v7.

    | 1. Location of Histogram Interfaces. Gaudi version v8 uses the standard AIDA interfaces for histograms. These interfaces are located in the AIDA project area. The changes to the end-user code is that the include file should be prefixed with AIDA/ instead of the current GaudiKernel/.
    | 2. Persistent representation of N-tuples. N-tuples saved in HBOOK format no longer have type information in the first row. See the discussion in :numref:`ntup-spec` for more details.
    | 3. The output of N-tuples to ODBC () is no longer supported. N-Tuple preselections based on SQL or interpreted C++ are no longer available. If you rely on these features, please contact the Gaudi development team.
    | 4. When saving data objects in a data store, all the sub-directory nodes in the path must already exist or should be explicitly created. In fact this is not a new feature, but a bug fix! Implicit creation of sub-directory nodes will be implemented in a future version.

Changes between release v7 and release v6
-----------------------------------------------

    |  · The control of the "physics event" loop has been separated from the ApplicationMgr and has become a new component, the event loop manager. A number of subsequent specializations have been provided: MininalEventLoopMgr, EventLoopMgr, and GaudiEventLoopMgr. These changes have been made to allow the possibility to have other types of event loop processing. These changes are backward compatible.
    |  · The first version of a scripting service based on Python has been released.
    |  · A number of small internal framework improvements:
    |  · Elimination of the up to now required static libraries.
    |  · Added version number (major and minor) to the Interface ID to check for interface compatibility at run-time.
    |  · Re-shuffling of the System class and conversion to a namespace.
    |  · Handling empty vectors in JobOptions.

Incompatible changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    In this section we will list changes that users need to make to
    their code in order to upgrade to version v7 of Gaudi from version
    v6.

    | 1. Histogram persistency. In previous versions, the HBOOK histogram persistency service was created by default. From this version there is no default histogram persistency: the ROOT or HBOOK persistency services have to be explicitly declared. See :numref:`hist-stor` for details.
    | 2. EvtMax. In previous versions it was possible to declare the number of events to be processed through either of the properties ApplicationMgr.EvtMax or EventSelector.EvtMax. In this release, only ApplicationMgr.EvtMax is supported, the default being all the events in the input file
    | 3. The property EventSelector.JobInput has been removed. Use EventSelector.Input instead (note the change in format of the value string).

Deprecated features
~~~~~~~~~~~~~~~~~~~~~~~~~

    We list here features of the framework which have become obsolete,
    because they have been superseded by more recent features. Users are
    discouraged from using the deprecated features, which may be removed
    in a future release.

    | Adding indexed items to N-tuples
    | Use the function addIndexedItem instead of addItem.
    | Accessors names in Algorithm
    | Use the service accessors with short names (e.g. msgSvc() ) instead of the long equivalent ones (e.g. messageService() )
    | Access to extra services from Algorithms.
    | Use the templated method service() instead of using the serviceLocator() together with the queryInterface() to get a reference to a service interface.
    | User Parameters in detector elements.
    | The XML tag for user parameters in the detector description (detector elements, etc.) is now <param/> instead of <userparameter/>. The old name will be maintained for a while. The methods in DetectorElement and Condition classes will accordonly be changed to use the word param instead of userParameter.

Availability
----------------

    The application framework is supported on the following platforms:

    | · Windows NT4 and Windows 2000, using the Developer Studio 6.0 SP2 Visual C++ environment
    | · RedHat Linux 6.1 (certified CERN Linux distribution with SUE and AFS) with egcs-2.91.66 and gcc-2.95.2.

    The code, documentation and installation instructions are available
    from the Gaudi web site at: http://cern.ch/proj-gaudi/

    Framework sources and binaries are also available in the CERN AFS cell, at /afs/cern.ch/sw/Gaudi .

.. _inst-usin:

Using the framework
-----------------------

CVS repository
~~~~~~~~~~~~~~~~~~~~

    The framework sources are stored in CVS and can be accessed using
    the CVS server. You have to specify the following option in your CVS
    command:

    | -d :pserver:cerncvs@lhcbcvs.cern.ch:/local/gaudicvs

    You have to login to the CVS server first:

    | cvs -d :pserver:cerncvs@lhcbcvs.cern.ch:/local/gaudicvs login

    The server will ask for a password, reply CERNuser. You can now send
    all the CVS commands that don't require write access. If you use a
    command like commit, you will get an error message. When you have
    finished, you can logout of the server.

CMT
~~~~~~~~~

    The framework libraries have been built using the Configuration
    Management Tool (CMT) [CMT]_.
    Therefore, using the CMT tool is the recommended way to modify
    existing packages or re-build the examples included in the release.
    If CMT is not available in your system, please follow the
    installation instructions in [CMT]_. The following simple
    examples are for Unix, but similar commands exist also for Windows.
    They assume that the CMTPATH environment variable is set to
    $HOME/mycmt:$GAUDIHOME.

    | Getting a copy of a package:
    |  Suppose you want to build the latest released version of the GaudiExamples package:

    .. code-block:: bash

        cd mycmt
        cmt checkout GaudiExamples -r v7


    |  Building and running an example:
    |  Now that you have the code, suppose you want to modify the AlgSequencer example, then build it and run it:

    .. code-block:: bash

        cd GaudiExamples/v7/src/AlgSequencer
        emacs HelloWorld.cpp
        ----- Make your modification, then
        cd ../../cmt
        source setup.csh
        emacs requirements
        ----- Uncomment AlgSequencer and comment all the others
        gmake
        cd ../home
        emacs AlgSequencer.txt
        ----- Make any modification if needed
        ../$CMTCONFIG/AlgSequencer.exe


    |  Modifying a library and rerunning the example:
    |  Suppose now you want to modify one of the Gaudi libraries, build it, then rerun the AlgSequencer example with it:

    .. code-block:: bash

        cd $HOME/mycmt
        cmt checkout GaudiAlg v3
        cd GaudiAlg/v3/src
        emacs ...
        ---- Make your modification...
        cd ../cmt
        source setup.csh
        gmake
        cd $HOME/mycmt/GaudiExamples/v7/cmt
        cmt show uses
        ---- Verify the you are now using the GaudiAlg version from $HOME/mycmt
        ---- There is no need to relink, since GaudiAlg is a component library
        cd ../home
        ../$CMTCONFIG/AlgSequencer.exe


Using the framework on Windows with Developer Studio or Nmake
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    The libraries for Windows are available for download from the web,
    and in AFS in the Win32Debug subdirectory of each package.

    .. Instructions for installing the Gaudi environment on Windows and for customising MS Visual Studio will be made available here in due course. For now, please refer to the LHCb specific instructions at: http://cern.ch/lhcb-comp/Support/html/DevStudio/default.htm

    The requirements files and CMT commands expect the following
    environment variables:

    | · HOME Needs to be set to the user\'s home directory. Typically this is in a network server and will have the form "\server\username" or can also be a local directory like "C:\\home". This environment variable is used to locate the .cmtrc file that contains the default CMTPATH.
    | · PATH Should be set up correctly to locate the Developer Studio executables (this is typically the case after installation).
    | · TEMP Location for temporary files. This is set correctly after the operating system installation.
    | · SITEROOT This is the root where software is installed. Typically it will point to some share in some server (\server\siteroot) or to the locally mounted AFS drive (F:\\cern.ch).
    | · CMTPATH The first location where CMT is looking for packages. This is typically the local directory C:/mycmt
    | · CMTSITE This is your site name. At CERN site and for the Windows platform we use CERN\_WIN32.

Using the framework in Unix
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    |  The libraries for Linux are available for download from the web, and in AFS in the i386\_linux22 an Linuxdbx subdirectories of each package (for the optimised and debug versions respectively).
    |  Instructions for installing the Gaudi environment on Linux will be made available here in due course. For now, please refer to the LHCb specific instructions at: http://cern.ch/lhcb-comp/Support/html/start_gaudi.htm

Working with development releases
-------------------------------------

    This User Guide corresponds to release v9 of the Gaudi software.

    For Gaudi packages, before they are publicly released and frozen,
    the development versions are periodically rebuilt from the head
    revision of the CVS repository in the development release area.
    These versions are not guaranteed to work and may change without
    notice; they are intended for integration tests. They should be used
    with care, mainly if you wish to use new features of the software
    which have not yet been incorporated in a public release.

Installation of the framework outside CERN
----------------------------------------------

Package installation
~~~~~~~~~~~~~~~~~~~~~~~~~~

    To use the Gaudi framework you also need to have access to
    installations of some external packages, listed below:

    CMT, CLHEP, NAG C, HTL, Python, Xerces, qqlhcb, ROOT, BOOST and
    CERNLIB.

    Up to date instructions for installation of these packages and
    setting of the environment (variables, path,..) needed to use the
    framework can be found on the Web at
    http://cern.ch/lhcb-comp/Support/html/Install.htm.


.. [#] http://www.boost.org/libs/libraries.htm
