.. include:: ./global.rst

|newpage|

|Gaudi logo|

.. _chapUtil:

Analysis utilities
=====================

Overview
-------------

    In this chapter we give pointers to some of the third party software
    libraries that we use within Gaudi or recommend for use by
    algorithms implemented in Gaudi.

CLHEP
----------

    CLHEP ("Class Library for High Energy Physics") is a set of
    HEP-specific foundation and utility classes such as random
    generators, physics vectors, geometry and linear algebra. It is
    structured in a set of packages independent of any external package.
    The documentation for CLHEP can be found on WWW at
    http://proj-clhep.web.cern.ch/proj-clhep/

    CLHEP is used extensively inside Gaudi, in the GaudiSvc and GaudiDb
    packages.

HTL
--------

    HTL ("Histogram Template Library") is used internally in Gaudi
    (GaudiSvc package) to provide histogramming functionality. It is
    accessed through its abstract AIDA [AIDA]_ compliant interfaces.
    Gaudi uses only the transient part of HTL. Histogram persistency is
    available with ROOT or HBOOK.

    The documentation on HTL is available at
    http:cern.ch/anaphe/documentation.html.

NAG C
----------

    The NAG C library is a commercial mathematical library providing a
    similar functionality to the FORTRAN mathlib (part of CERNLIB). It
    is organised into chapters, each chapter devoted to a branch of
    numerical or statistical computation. A full list of the functions
    is available at
    http://cern.ch/anaphe/documentation/Nag_C/NAGdoc/cl/html/mark6.html

    NAG C is not explicitly used in the Gaudi framework, but developers
    are encouraged to use it for mathematical computations. Instructions
    for linking NAG C with Gaudi can be found at
    http://cern.ch/lhcb-comp/Support/NagC/nagC.html

    Some NAG C functions print error messages to stdout by default,
    without any information about the calling algorithm and without
    filtering on severity level. A facility is provided by Gaudi to
    redirect these messages to the Gaudi MessageSvc. This is documented
    at http://cern.ch/lhcb-comp/Support/NagC/GaudiNagC.html

ROOT
---------

    ROOT is used by Gaudi for I/O and as a persistency solution for
    event data, histograms and n-tuples. In addition, it can be used for
    interactive analysis, as discussed in :numref:`chapNtup`. Information about ROOT can be found
    at http://root.cern.ch/
