.. include:: ./global.rst

|newpage|

|Gaudi logo|

.. _chapHist:

Histogram facilities
======================

Overview
------------

    The histogram data store is one of the data stores discussed in
    :numref:`chapArch`. Its purpose is to
    store statistics based data and user created objects that have a
    lifetime of more than a single event (e.g. histograms).

    As with the other data stores, all access to data is via a service
    interface. In this case it is via the IHistogramSvc interface, which
    is derived from the IDataProviderSvc interface discussed in `Chapter
    6 <GDG_DataAccess.html#1010951>`__. The user asks the Histogram
    Service to book a histogram and register it in the histogram data
    store. The service returns a pointer to the histogram, which can
    then be used to fill and manipulate the histogram, using the methods
    defined in the IHistogram1D and IHistogram2D interfaces and
    documented on the AIDA (Abstract Interfaces for Data Analysis)
    project web pages: http://wwwinfo.cern.ch/asd/lhc++/AIDA/.

    Internally, Gaudi uses the transient part of HTL (Histogram Template
    Library, http://wwwinfo.cern.ch/asd/lhc++/HTL/) to implement
    histograms.

The Histogram service
-------------------------

    An instance of the histogram data service is created by the
    application manager. After the service has been initialised, the
    histogram data store will contain a root directory, always called
    "/stat", in which users may book histograms and/or create
    sub-directories (for example, in the code fragment below, the
    histogram is stored in the subdirectory "/stat/simple"). A suggested
    naming convention for the sub-directories is given in
    :numref:`over-name`. Note that the string
    "/stat/" can be omitted when referring to a histogram in the data
    store: "/stat/simple" is equivalent to "simple", without a leading
    "/".

    As discussed in :numref:`algo-base`, the
    Algorithm base class defines a member function which returns a
    pointer to the IHistogramSvc interface of the standard histogram
    data service

    .. code-block:: cpp

        IHistogramSvc* histoSvc()


    . Access to any other non-standard histogram data service (if one
    exists) must be sought via the ISvcLocator interface of the
    application manager as discussed in section :numref:`serv-requ`.

Using histograms and the histogram service
----------------------------------------------

    The code fragment below shows how to book a 1D histogram and place
    it in a directory within the histogram data store, followed by a
    simple statement which fills the histogram.

    .. code-block:: cpp

        #include "AIDA/IHistogram1d.h"
        // ...

        // Book 1D histogram in the histogram data store
        IHistogram1d* m_hTrackCount= histoSvc()->
            book( "simple", 1, "TrackCount", 100, 0., 3000. );
        SmartDataPtr<MyTrackVector> particles( eventSvc(), "/Event/MyTracks" )
        if ( 0 != particles ) {
            m_hTrackCount->fill(particles->size(), 1.); // Filling the track count histogram
        }

    The parameters of the book function are the directory in which to
    store the histogram in the data store, the histogram identifier, the
    histogram title, the number of bins and the lower and upper limits
    of the X axis. 1D histograms with fixed and variable binning are
    available. In the case of 2D histograms, the book method requires in
    addition the number of bins and lower and upper limits of the Y
    axis.

    If using HBOOK for persistency, the histogram identifier should be a
    valid HBOOK histogram identifier (number) and must be unique within
    the RZ directory the histogram is assigned to. The name of the RZ
    directory is given by the directory and parent directories in the
    transient histogram store. Please note that HBOOK accepts only
    directory names, which are shorter than 16 characters and that HBOOK
    internally converts any directory name into upper case. Even if
    using another persistency solution (e.g. ROOT) it is recommended to
    comply with the HBOOK constraints in order to make the code
    independent of the persistency choice.

    The call to histoSvc()->book(...) returns a pointer to an object of
    type IHistogram1D (or IHistogram2D in the case of a 2D histogram).
    All the methods of this interface can be used to further manipulate
    the histogram, and in particular to fill it, as shown in the
    example. Note that this pointer is guaranteed to be non-null, the
    algorithm would have failed the initialisation step if the histogram
    data service could not be found. On the contrary the user variable
    particles may be null (in case of absence of tracks in the transient
    data store and in the persistent storage), and the fill statement
    would fail - so the value of particles must be checked before using
    it.

    Algorithms that create histograms will in general keep pointers to
    those histograms, which they may use for filling operations. However
    it may be that you wish to share histograms between different
    algorithms. Maybe one algorithm is responsible for filling the
    histogram and another algorithm is responsible for fitting it at the
    end of the job. In this case it may be necessary to look for
    histograms within the store. The mechanism for doing this is
    identical to the method for locating event data objects within the
    event data store, namely via the use of smart pointers, as discussed
    in section :numref:`data-smrt`.

    .. code-block:: cpp

        SmartDataPtr<IHistogram1D> hist1D( histoSvc(), "simple/1" );
        if( 0 != hist1D ) {
            histoSvc()->print( hist1D ); // Print the found histogram
        }

.. _hist-stor:

Persistent storage of histograms
------------------------------------

    By default, Gaudi does not produce a persistent histogram output.
    The options exist to write out histograms either in HBOOK or in ROOT
    format. The choice is made by giving the job option
    ApplicationMgr.HistogramPersistency, which can take the values
    "NONE" (no histograms saved, default), "HBOOK" or "ROOT". Depending
    on the choice, additional job options are needed, as described
    below.

HBOOK persistency
~~~~~~~~~~~~~~~~~~~~~~~

    The HBOOK conversion service converts objects of types IHistogram1D
    and IHistogram2D into a form suitable for storage in a standard
    HBOOK file. In order to use it you first need to tell Gaudi where to
    find the HbookCnv shared library. If you are using CMT, this is done
    by adding the following line to the CMT requirements file:

    .. code-block:: bash

        use HbookCnv v*

    | You then have to tell the application manager to load this shared library and to create the HBOOK conversion service, by adding the following lines to your job options file:

    .. code-block:: cpp

        ApplicationMgr.DLLs += {"HbookCnv"};
        ApplicationMgr.HistogramPersistency = "HBOOK";

    Finally, you have to tell the histogram persistency service the name
    of the output file:

    .. code-block:: cpp

        HistogramPersistencySvc.OuputFile = "histo.hbook";

    Note that it is also possible to print the histograms to the
    standard output destination (HISTDO) by setting the following job
    option (default is false).

    .. code-block:: cpp

        HistogramPersistencySvc.PrintHistos = true;

ROOT persistency
~~~~~~~~~~~~~~~~~~~~~~

    The ROOT conversion service converts objects of types IHistogram1D
    and IHistogram2D into a form suitable for storage in a standard ROOT
    file. In order to use it you first need to tell Gaudi where to find
    the RootHistCnv shared library. If you are using CMT, this is done
    by adding the following line to the CMT requirements file:

    .. code-block:: bash

        use RootHistCnv v*

    | You then have to tell the application manager to load this shared library and to create the ROOT histograms conversion service, by adding the following lines to your job options file:

    .. code-block:: cpp

        ApplicationMgr.DLLs += {"RootHistCnv"};
        ApplicationMgr.HistogramPersistency = "ROOT";

    Finally, you have to tell the histogram persistency service the name
    of the output file:

    .. code-block:: cpp

        HistogramPersistencySvc.OuputFile = "histo.rt";
