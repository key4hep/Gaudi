.. include:: ./global.rst

|newpage|

|Gaudi logo|

.. _chapOver:

Introduction
==============

Purpose of the document
---------------------------

    This document is intended as a combination of user guide and
    tutorial for the use of the Gaudi application framework software. It
    is complemented principally by two other "documents": the
    Architecture Design Document (ADD) [LHCB-98-064]_, and the online
    auto-generated reference documentation [GAUDI-DOXY]_. A third document
    [LHCB-98-065]_ lists the User Requirements
    and Scenarios that were used as input and validation of the
    architecture design. All these documents and other information about
    Gaudi, including this user guide and source code documentation, are
    available via the Gaudi home page: http://cern.ch/proj-gaudi.

    The ADD contains a discussion of the architecture of the framework,
    the major design choices taken in arriving at this architecture and
    some of the reasons for these choices. It should be of interest to
    anyone who wishes to write anything other than private analysis
    code.

    As discussed in the ADD the application framework should be usable
    for implementing the full range of offline computing tasks: the
    generation of events, simulation of the detector, event
    reconstruction, testbeam data analysis, detector alignment,
    visualisation, etc. etc..

    In this document we present the main components of the framework
    which are available in the current release of the software. It is
    intended to increment the functionality of the software at each
    release, so this document will also develop as the framework
    increases in functionality. Having said that, physicist users and
    developers actually see only a small fraction of the framework code
    in the form of a number of key interfaces. These interfaces should
    change very little if at all and the user of the framework cares
    very little about what goes on in the background.

    The document is arranged as follows: :numref:`chapArch` is a short resume of the
    framework architecture, presented from an "Algorithm-centric" point
    of view, and re-iterating only a part of what is presented in the
    ADD.

    :numref:`chapInst` contains a summary of
    the functionality which is present in the current release, and
    details of how to obtain and install the software.

    :numref:`chapStar` discusses in some detail
    an example which comes with the framework library. It covers the
    main program, some of the basic aspects of implementing algorithms,
    the specification of job options and takes a look at how the code is
    actually executed. The subject of coding algorithms is treated in
    more depth in :numref:`chapAlgo`.

    :numref:`chapData` discusses the use of the
    framework data stores and event data. :numref:`chapEven` is a placeholder for describing
    the experiment specific event data models. :numref:`chapDetd`, :numref:`chapHist`, :numref:`chapNtup`
    discuss the other types of data accessible via these stores:
    detector description data (material and geometry), histogram data
    and n-tuples.

    :numref:`chapServ` deals with services
    available in the framework: job options, messages, particle
    properties, auditors, chrono & stat, random numbers, incidents,
    introspection. It also has a section on developing new services.
    Framework tools are discussed in :numref:`chapTool`, the use and implementation of
    converter classes in :numref:`chapConv`.

    :numref:`chapScri` discusses scripting as a
    means of configuring and controlling the application interactively.
    This is followed by a description in :numref:`chapVisu` of how visualisation
    facilities might be implemented inside Gaudi.

    :numref:`chapLibr`  describes the package
    structure of Gaudi and discusses the different types of libraries in
    the distribution.

    :numref:`chapUtil` gives pointers to the
    documentation for class libraries which we are recommending to be
    used within Gaudi.

    :numref:`chapRefe` contains a list of
    references. :numref:`chapOpti` lists the
    options which may be specified for the standard components available
    in the current release. :numref:`chapJobo` gives the details of the syntax
    and possible error messages of the job options compiler. Finally,
    :numref:`chapDesi` is a small guide to
    designing classes that are to be used in conjunction with the
    application framework.

Conventions
---------------

Units
~~~~~~~~~~~

    We have decided to adopt the same system of units as CLHEP, as used
    also by GEANT4. This system is fully documented in the CLHEP web
    pages, at the URL: http://proj-clhep.web.cern.ch/proj-clhep/doc/CLHEP_1_7/UserGuide/Units/units.html

    The list of basic units is reproduced in :numref:`tab-units`. Note that this differs from the
    convention used in GEANT 3, where the basic units of length, time
    and energy are, respectively, centimetre, GeV, second..

    .. table:: CLHEP system of units
        :name: tab-units

        +--------------------------------------+--------------------------------------+
        | Quantity                             | Unit                                 |
        +======================================+======================================+
        | Length                               | millimeter                           |
        +--------------------------------------+--------------------------------------+
        | Time                                 | nanosecond                           |
        +--------------------------------------+--------------------------------------+
        | Energy                               | MeV                                  |
        +--------------------------------------+--------------------------------------+
        | Electric charge                      | positron charge                      |
        +--------------------------------------+--------------------------------------+
        | Temperature                          | Kelvin                               |
        +--------------------------------------+--------------------------------------+
        | Amount of substance                  | mole                                 |
        +--------------------------------------+--------------------------------------+
        | Plane angle                          | radian                               |
        +--------------------------------------+--------------------------------------+

    Users should not actually need to know what units are used in the
    internal representation of the data, as long as they are consistent
    throughout the Gaudi data stores. What they care about is that they
    can define and plot quantities with the correct units. In some
    specialised algorithms they may also wish to renormalise the data to
    a different set of units, if the default set would lead to numerical
    precision problems.

    We therefore propose the following rules, which are discussed more
    fully in reference [LHCB-UNITS]_.

    | 1. All dimensioned quantities in the Gaudi data stores shall conform to the CLHEP system of units.
    | 2. All definitions of dimensioned quantities shall be dimensioned by multiplying by the units defined in the CLHEP/Units/SystemOfUnits.h header file. For example:

    .. code-block:: cpp

        const double my_height = 170*cm;
        const double my_weight = 75*kg;

    |  Note that the user should not care about the numerical value of the numbers my\_height and my\_weight. Internally these numbers are represented as 1700. and 4.68e+26. respectively, but the user does not need to know.
    | 3. All output of dimensioned quantities should be converted to the required units by dividing by the units defined in the CLHEP/Units/SystemOfUnits.h header file. For example:

    .. code-block:: cpp

        my_hist = histoSvc()->book( "/stat/diet","corpulence (kg / m ** 2)", 30, 10., 40.);
        double my_corpulence = my_weight / ( my_height * my_height );
        my_hist->fill( my_corpulence / ( kg / m2 ), 1. );

    | which, for a healthy person, should plot a number between 19. and 25....
    | 4. Physical constants should not be defined in user code. They should be taken directly from the CLHEP/Units/PhysicalConstants.h header file. For example:

    .. code-block:: cpp

        float my_rest_energy = my_weight* c_squared;

    | 5. Users may wish to use a different set of units for specific purposes (e.g. when the default units may lead to precision problems). In this case algorithms can renormalise their private copy of the data (as shown in the last line of the rule 3 example) for internal use, but making sure that any data subsequently published to the public data stores is converted back to the CLHEP units.

Coding Conventions
~~~~~~~~~~~~~~~~~~~~~~~~

    The Gaudi software follows (or should follow!) the LHCb C++ coding
    conventions described in reference [LHCB-2001-054]_.

File extensions
```````````````````````

    One consequence of following the LHCb coding conventions is that the
    specification of the C++ classes is done in two parts: the header or
    ".h" file and the implementation or ".cpp" file.

    We also define file extensions for Gaudi specific files. The
    recommended file extension for Job Options files is ".opts" (see
    :numref:`serv_jobo`. For
    Python scripts, the extension ".py" is mandatory (see :numref:`chapScri`).

.. _over-name:

Naming Conventions
~~~~~~~~~~~~~~~~~~~~~~~~

    |  Histograms
    |  In order to avoid clashes in histogram identifiers, we suggest that histograms are placed in named subdirectories of the transient histogram store. The top level subdirectory should be the name of a sub-detector group (e.g. VELO). Below this, users are free to define their own structure. One possibility is to group all histograms produced by a given algorithm into a directory named after the algorithm.

Conventions of this document
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    |  Angle brackets are used in two contexts. To avoid confusion we outline the difference with an example.

    The definition of a templated class uses angle brackets. These are
    required by C++ syntax, so in the instantiation of a templated class
    the angle brackets are retained:

    .. code-block:: cpp

        AlgFactory<UserDefinedAlgorithm> s_factory;

    This is to be contrasted with the use of angle brackets to denote
    "replacement" such as in the specification of the string:

    .. code-block:: xml

        "<concreteAlgorithmType>/<algorithmName>"

    which implies that the string should look like:

    .. code-block:: cpp

        "EmptyAlgorithm/Empty"

    Hopefully what is intended will be clear from the context.

Reporting problems
----------------------

    Users of the Gaudi software are encouraged to report problems and
    requests for new features via https://gitlab.cern.ch/gaudi/Gaudi .

    To report a new problem, go to the Gaudi gitlab home page
    https://gitlab.cern.ch/gaudi/Gaudi, click on Issue, create a "New Issue"
    and fill in the form. This will add the report to the system
    and notify the developers by E-mail. You will receive E-mail
    notification of any changes in the status of your report.

    To view the list of current problems, and their status click the "List"
    button under the Issues tab at https://gitlab.cern.ch/gaudi/Gaudi/issues

    Active developers of the Gaudi software are encouraged to use the
    gaudi-developers mailing list for discussion of Gaudi features and
    future developments. This list is not, primarily, intended for bug
    reports. In order to send mail to gaudi-developers@cern.ch,
    you must first subscribe to the list, using the form at
    https://e-groups.cern.ch/e-groups/EgroupsSubscription.do?egroupName=gaudi-developers.
    You need a CERN mailserver account to be able to use this form ...

    The archive of the mailing list is publically accessible on the Web,
    at https://groups.cern.ch/group/gaudi-developers/default.aspx.

Editor's note
-----------------

    This document is a snapshot of the Gaudi software at the time of the
    release of the current version. We have made every effort to ensure that the
    information it contains is correct, but in the event of any
    discrepancies between this document and information published on the
    Web, the latter should be regarded as correct, since it is
    maintained between releases and, in the case of code documentation,
    it is automatically generated from the code.

    We encourage our readers to provide feedback about the structure,
    contents and correctness of this document and of other Gaudi
    documentation. Please send your comments to the editor,
    Marco.Cattaneo@cern.ch
