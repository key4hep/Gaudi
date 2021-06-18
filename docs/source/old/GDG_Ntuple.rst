.. include:: ./global.rst

|newpage|

|Gaudi logo|

.. _chapNtup:

N-tuple and Event Collection facilities
==========================================

Overview
-------------

    In this chapter we describe facilities available in Gaudi to create
    and retrieve N-tuples. We discuss how Event Collections, which can
    be considered an extension of N-tuples, can be used to make
    preselections of event data. Finally, we explore some possible tools
    for the interactive analysis of N-tuples.

N-tuples and the N-tuple Service
-------------------------------------

    User data - so called N-tuples - are very similar to event data. Of
    course, the scope may be different: a row of an N-tuple may
    correspond to a track, an event or complete runs. Nevertheless, user
    data must be accessible by interactive tools such as PAW or ROOT.

    Gaudi N-tuples allow to freely format structures. Later, during the
    running phase of the program, data are accumulated and written to
    disk.

    The transient image of an N-tuple is stored in a Gaudi data store
    which is connected to the N-tuple service. Its purpose is to store
    user created objects that have a lifetime of more than a single
    event.

    As with the other data stores, all access to data is via a service
    interface. In this case it is via the INTupleSvc interface which
    extends the IDataProviderSvc interface. In addition the interface to
    the N-tuple service provides methods for creating N-tuples, saving
    the current row of an N-tuple or retrieving N-tuples from a file.
    The N-tuples are derived from DataObject in order to be storable,
    and are stored in the same type of tree structure as the event data.
    This inheritance allows to load and locate N-tuples on the store
    with the same smart pointer mechanism as is available for event data
    items (c.f. :numref:`chapData`).

.. _ntup-accs:

Access to the N-tuple Service from an Algorithm
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    The Algorithm base class defines a member function which returns a
    pointer to the INTupleSvc interface

    .. code-block:: cpp

        INTupleSvc* ntupleSvc()

    The N-tuple service provides methods for the creation and
    manipulation of N-tuples and the location of N-tuples within the
    persistent store.

    The top level directory of the N-tuple transient data store is
    always called "/NTUPLES". The next directory layer is connected to
    the different output streams: e.g. "/NTUPLES/FILE1", where FILE1 is
    the logical name of the requested output file for a given stream.
    There can be several output streams connected to the service. In
    case of persistency using HBOOK, "FILE1" corresponds to the top
    level RZ directory of the file (...the name given to HROPEN). From
    then on the tree structure is reflected with normal RZ directories
    (caveat: HBOOK only accepts directory names with less than 8
    characters! It is recommended to keep directory names to less than 8
    characters even when using another technology (e.g. ROOT) for
    persistency, to make the code independent of the persistency
    choice.). Note that the top level directory name "/NTUPLES/" can be
    omitted when referring to an N-tuple in the transient data store -
    in the example above the name could start with "FILE1" (without a
    leading "/").

Using the N-tuple Service
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    This section explains the steps to be performed when defining an
    N-tuple:

    | · The N-tuple tags must be defined.
    |  · The N-tuple must be booked and the tags must be declared to the N-tuple.
    |  · The N-tuple entries have to be filled.
    |  · The filled row of the N-tuple must be committed.
    |  · Persistent aspects are steered by the job options.

.. _ntup-dftg:

Defining N-tuple tags
``````````````````````````````

    When creating an N-tuple it is necessary to first define the tags to
    be filled in the N-tuple, as shown for example in :numref:`lstg-defntuptag`:

    .. was 10.1

    .. code-block:: cpp
        :name: lstg-defntuptag
        :caption: Definition of N-tuple tags

        NTuple::Item<long> m_ntrk; // A scalar item (number)
        NTuple::Array<bool> m_flag; // Vector items
        NTuple::Array<long> m_index;
        NTuple::Array<float> m_px, m_py, m_pz;
        NTuple::Matrix<long> m_hits; // Two dimensional tag

    Typically the tags belong to the filling algorithm and hence should
    be provided in the Algorithm's header file. Currently the supported
    data types are: bool, long, float and double. double types (Fortran
    REAL\*8) are not recommened if using HBOOK for persistency: HBOOK
    will complain if the N-tuple structure is not defined in a way that
    aligns double types to 8 byte boundaries. In addition PAW cannot
    understand double types.

Booking and Declaring Tags to the N-tuple
``````````````````````````````````````````````````

    :numref:`lstg-crecolntup` shows how to book a
    column-wise N-Tuple. The first directory specifier (FILE1 in the
    example) must correspond to an open output stream (see :numref:`ntup-rcol`);
    lower directory levels are created automatically. After booking, the previously
    defined tags must be declared to the N-tuple; if not, they are
    invalid and will cause an access violation at run-time.

    .. was 10.2

    .. code-block:: cpp
        :name: lstg-crecolntup
        :caption: Creation of a column-wise N-tuple in a specified directory and file

        #include "GaudiKernel/NTuple.h"
        // ..
        NTuplePtr nt1(ntupleSvc(), "FILE1/MC/1");
        if ( !nt1 ) { // Check if already booked
            nt1=ntupleSvc()->book("FILE1/MC/1",CLID_ColumnWiseTuple,"Hello World");
            if ( 0 != nt1 ) {
                // Add an index column
                status = nt1->addItem ("Ntrack", m_ntrk, 0, 5000 );
                // Add a variable size column, type float (length=length of index col)
                status = nt1->addIndexedItem ("px", m_ntrk, m_px);
                status = nt1->addIndexedItem ("py", m_ntrk, m_py);
                status = nt1->addIndexedItem ("pz", m_ntrk, m_pz);
                // Another one, but this time of type bool
                status = nt1->addIndexedItem ("flg",m_ntrk, m_flag);
                // Another one, type integer, numbers must be within [0, 5000]
                status = nt1->addIndexedItem ("idx",m_ntrk, m_index, 0, 5000 );
                // Add 2-dim column: [0:m_ntrk][0:2]; numerical numbers within [0, 8]
                status = nt1->addIndexedItem ("hit",m_ntrk, 2, m_hits, 0, 8 );
            }
            else { // did not manage to book the N tuple....
                return StatusCode::FAILURE;
            }
        }

    In previous versions of Gaudi (up to v8), indexed items were added
    with the addItem function, causing confusion for users. For this
    reason the calls to add indexed arrays and matrices were changed,
    these should now be added using the member function addIndexedItem.
    Please consult the doxygen code documentation for further details.
    The old calls still exist, however they are deprecated.

    Row wise N-tuples are booked in the same way, but giving the type
    CLID\_RowWiseTuple. However, only individual items (class
    NTuple::Item) can be filled, no arrays and no matrices. Clearly this
    excludes the usage of indexed items. For row-wise N-tuples to be
    saved in HBOOK format, it is recommended to use only float type, for
    the reasons explained in :numref:`ntup-hbok`.

    When using HBOOK for persistency, the N-tuple identifier ("1" in
    this example) must be a number and must be unique in a given
    directory. This is a limitation imposed by HBOOK RZ directories. It
    is recommended to keep this number unique even when using another
    technology (e.g. ROOT) for persistency, to make the code independent
    of the persistency choice.

Filling the N-tuple
````````````````````````````

    Tags are usable just like normal data items, where

    | · Items<TYPE> are the equivalent of numbers: bool, long, float.
    | · Array<TYPE> are equivalent to 1 dimensional arrays: bool[size], long[size], float[size]
    | · Matrix<TYPE> are equivalent to an array of arrays or matrix: bool[dim1][dim2].

    Implicit bounds checking is not possible without a rather big
    overhead at run-time. Hence it is up to the user to ensure the
    arrays do not overflow.

    When all entries are filled, the row must be committed, i.e. the
    record of the 7N-tuple must be written.

    .. was 10.3

    .. code-block:: cpp
        :name: lstg-fillntuple
        :caption: Filling an N-tuple

        m_ntrk = 0;
        for( MyTrackVector::iterator i = mytracks->begin(); i != mytracks->end(); i++ ) {
            const HepLorentzVector& mom4 = (*i)->fourMomentum();
            m_px[m_ntrk] = mom4.px();
            m_py[m_ntrk] = mom4.py();
            m_pz[m_ntrk] = mom4.pz();
            m_index[m_ntrk] = cnt;
            m_flag[m_ntrk] = (m_ntrk%2 == 0) ? true : false;
            m_hits[m_ntrk][0] = 0;
            m_hits[m_ntrk][1] = 1;
            m_ntrk++;
            // Make sure the array(s) do not overflow.
            if ( m_ntrk > m_ntrk->range().distance() ) break;
        }
        // Commit N tuple row. See :numref:`lstg-crecolntup` for initialisation of m_ntuple
        status = m_ntuple->write();
        if ( !status.isSuccess() ) {
            log << MSG::ERROR << "Cannot fill id 1" << endreq;
        }


Reading N-tuples
`````````````````````````

    Although N-tuples intended for interactive analysis, they can also
    be read by a regular program. An example of reading back such an
    N-tuple is given in :numref:`lstg-readntuple`.

    .. code-block:: cpp
        :name: lstg-readntuple
        :caption: Reading an N-tuple

        NTuplePtr nt(ntupleSvc(), "FILE1/ROW\_WISE/2");
        if ( nt ) {
            long count = 0;
            NTuple::Item<float> px, py, pz;
            status = nt->item("px", px);
            status = nt->item("py", py);
            status = nt->item("pz", pz);
            // Access the N tuple row by row and print the first 10 tracks
            while ( nt->read().isSuccess() ) {
                log << MSG::INFO << " Entry [" << count++ << "]:";
                log << " Px=" << px << " Py=" << py << " Pz=" << pz << endreq;
            }
        }


N-tuple Persistency
~~~~~~~~~~~~~~~~~~~~~~~~~~

Choice of persistency technology
`````````````````````````````````````````

    N-tuples are of special interest to the end-user, because they can
    be accessed using commonly known tools such as PAW, ROOT or Java
    Analysis Studio (JAS). In the past it was not a particular strength
    of the software used in HEP to plug into many possible persistent
    data representations. Except for JAS, only proprietary data formats
    are understood. For this reason the choice of the output format of
    the data depends on the preferred analysis tool/viewer.

    | HBOOK
    |
    |  This data format is used by PAW. PAW can understand this and only this data format. Files of this type can be converted to the ROOT format using the h2root data conversion program. The use of PAW in the long term is deprecated.
    | ROOT
    |
    |  This data format is used by the interactive ROOT program.

    In the current implementation, N-tuples must use the same
    persistency technology as histograms. The choice of technology is
    therefore made in the same way as for histograms, as described in
    :numref:`hist-stor`. Obviously the options
    have to be given only once and are valid for both histograms and
    N-tuples. The only difference is that histograms are saved to a
    different output file (defined by the job option
    HistogramPersistencySvc.OuputFile), a different output file (or set
    of output files) must be defined for the N-tuples.

.. _ntup-spec:

Input and Output File Specification
````````````````````````````````````````````

    Conversion services exist to convert N-tuple objects into a form
    suitable for persistent storage in a number of storage technologies.
    In order to use this facility it is necessary to add the following
    line in the job options file:

    .. code-block:: cpp

        NTupleSvc.Output = {"FILE1 DATAFILE='tuples.hbook' OPT='NEW'",
        "FILE2 ...",
        // ...
        "FILEN ..."};

    where <tuples.hbook> should be replaced by the name of the file to
    which you wish to write the N-tuple. FILE1 is the logical name of
    the output file, which must be the same as the data store directory
    name described in :numref:`ntup-accs`.
    Several files are possible, corresponding to different data store
    directories whose name can be chosen at will.

    The detailed syntax of the options is as follows. In each case only
    the three leading characters are significant: DATAFILE=<...>,
    DATABASE=<...> or simply DATA=<...> would lead to the same result.

    | · DATAFILE='<file-specs>'
    | Specifies the datafile (file name) of the output stream.
    | · OPT='<opt-spec>'
    | · NEW, CREATE, WRITE: Create a new data file. Not all implementations allow to over-write existing files.
    | · OLD, READ: Access an existing file for read purposes
    | · UPDATE: Open an existing file and add records. It is not possible to update already existing records.

    A similar option NTupleSvc.Input exists for N-tuple input.

.. _ntup-hbok:

Saving row wise N-tuples in HBOOK
``````````````````````````````````````````

    Since the persistent representation of row wise N-tuples in HBOOK is
    done by floats only, a convention is needed to access the proper
    data type. By default the float type is assumed, i.e. all data
    members are of float type. This is the recommended format.

    It is possible to define row wise N-tuples in Gaudi containing data
    types other than float. This was the default in Gaudi versions
    previous to v8, where the first row of the N-tuple contained the
    type information. This possibility can be switched on by using the
    option

    .. code-block:: python

        HistogramPersistencySvc.RowWiseNTuplePolicy = "USE_DATA_TYPES";

    which also provides backwards compatibility for reading back old
    N-tuples produced with old Gaudi versions. Remember however that
    when using PAW to read N-tuples produced using this option, you must
    skip the first row and start with the second event.

Event Collections
----------------------

    Event collections or, to be more precise, event tag collections, are
    used to minimize data access by performing preselections based on
    small amounts of data. Event tag data contain flexible event
    classification information according to the physics needs. This
    information could either be stored as flags indicating that the
    particular event has passed some preselection criteria, or as a
    small set of parameters which describe basic attributes of the
    event. Fast access is required for this type of event data.

    Event tag collections can exist in several versions:

    | · Collections recorded during event processing stages from the online, reconstruction, reprocessing etc.
    | · Event collections defined by analysis groups with pre-computed items of special interest to a given group.
    | · Private user defined event collections.

    Starting from this definition an event tag collection can be
    interpreted as an N-tuple which allows to access the data used to
    create the N-tuple. Using this approach any N-tuple which allows
    access to the data is an event collection.

    Event collections allow pre-selections of event data. These
    pre-selections depend on the underlying storage technology.

    First stage pre-selections based on scalar components of the event
    collection. First stage preselection is not necessarily executed on
    your computer but on a database server e.g. when using ORACLE. Only
    the accessed columns are read from the event collection. If the
    criteria are fulfilled, the N-tuple data are returned to the user
    process. Preselection criteria are set through a job options, as
    described in section :numref:`ntup-rcol`.

    The second stage pre-selection is triggered for all items which
    passed the first stage pre-selection criteria. For this
    pre-selection, which is performed on the client computer, all data
    in the N-tuple can be used. The further preselection is implemented
    in a user defined function object (functor) as described in section
    :numref:`ntup-rcol`. Gaudi algorithms are called
    only when this pre-selector also accepts the event, and normal event
    processing can start.

Writing Event Collections
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Event collections are written to the data file using a Gaudi
    sequencer. A sequencer calls a series of algorithms, as discussed in
    section :numref:`algo-base`. The execution of
    these algorithms may terminate at any point of the series (and the
    event not selected for the collection) if one of the algorithms in
    the sequence fails to pass a filter.

Defining the Address Tag
`````````````````````````````````

    The event data is accessed using a special N-tuple tag of the type

    .. code-block:: cpp

        NTuple::Item<IOpaqueAddress*> m_evtAddress

    It is defined in the algorithm's header file in addition to any
    other ordinary N-tuple tags, as described in section
    :numref:`ntup-dftg`. When booking the N-tuple,
    the address tag must be declared like any other tag, as shown in
    :numref:`lstg-conncolladdr`. It is recommended to use
    the name "Address" for this tag.

    .. was 10.5

    .. code-block:: cpp
        :name: lstg-conncolladdr
        :caption: Connecting an address tag to an event collection N-tuple

        StatusCode status = service("EvtTupleSvc", m_evtTupleSvc);
        if ( status.isSuccess() ) {
            NTuplePtr nt(m_evtTupleSvc, "/NTUPLES/EvtColl/Collection");
            // ... Book N-tuple
            // Add an event address column
        status = nt->addItem ("Address", m_evtAddress);

    The usage of this tag is identical to any other tag except that it
    only accepts variables of type IOpaqueAddress - the information
    necessary to retrieve the event data.

    Please note that the event tuple service necessary for writing event
    collections is not instantiated by default and hence must be
    specified in the job options file:

    .. was 10.6

    .. code-block:: cpp
        :name: lstg-addcolljobopts
        :caption: Adding the event tag collection service to the job options

        ApplicationMgr.ExtSvc += { "TagCollectionSvc/EvtTupleSvc" };

    It is up to the user to locally remember within the algorithm
    writing the event collection tuple the reference to the
    corresponding service. Although the TagCollectionSvc looks like an
    N-tuple service, the implementation is different.

Filling the Event Collection
`````````````````````````````````````

    At fill time the address of the event must be supplied to the
    Address item. Otherwise the N-tuple may be written, but the
    information to retrieve the corresponding event data later will be
    lost. :numref:`lstg-filladdntupexec` also demonstrates
    the setting of a filter to steer whether the event is written out to
    the event collection.

    .. was 10.7

    .. code-block:: cpp
        :name: lstg-filladdntupexec
        :caption: Fill the address tag of an N-tuple at execution time

        SmartDataPtr<Event> evt(eventSvc(),"/Event");
        if ( evt ) {
        // ... Some data analysis deciding whether to keep the event or not
        // keep_event=true if event should be written to event collection
            setFilterPassed( keep_event );
            m_evtAddrColl = evt->address();
        }

.. _ntup-ecpe:

Event Collection Persistency
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Output File Specification
``````````````````````````````````

    Conversion services exist to convert event collection objects into a
    form suitable for persistent storage in a number of storage
    technologies. In order to use this facility it is necessary to add
    the following line in the job options file:

    .. code-block:: cpp

        EvtTupleSvc.Output = {"FILE1 DATAFILE='coll.root' TYP='ROOT' OPT='NEW'",
        "FILE2 ...",
        ...
        "FILEN ..."};

    where <coll.root> should be replaced by the name of the file to
    which you wish to write the event collection. FILE1 is the logical
    name of the output file - it could be any other string.

    The options are the same as for N-tuples (see
    :numref:`ntup-spec`) with the following
    additions:

    | · TYP='<typ-spec>'
    |
    |  Specifies the type of the output stream. Currently supported types are:
    |
    |   · ROOT: Write as a ROOT tree.
    |   · MS Access: Write as a Microsoft Access database.
    |
    |   · There is also weak support for the following database types1:
    |    · SQL Server
    |    · MySQL
    |    · Oracle ODBC
    |   These database technologies are supported through their ODBC interface. They were tested privately on local installations. However all these types need special setup to grant access to the database.
    |
    |  You need to specify the use of the technology specific persistency package (i.e. GaudiRootDb) in your CMT requirements file and to load explicitly in the job options the DLLs containing the generic (GaudiDb) and technology specific (e.g. GaudiRootDb) implementations of the database access drivers:
    |  ApplicationMgr.DLLs += { "GaudiDb", "GaudiRootDb" };
    |
    | · SVC='<service-spec>' (optional)
    |
    |  Connect this file directly to an existing conversion service. This option however needs special care. It should only be used to replace default services.
    |
    | · AUTHENTICATION='<authentication-specs>' (optional)
    |
    |  For protected datafiles (e.g. Microsoft Access) it can happen that the file is password protected. In this case the authentication string allows to connect to these databases. The connection string in this case is the string that must be passed to ODBC, for example: AUTH='SERVER=server\_host;UID=user\_name;PWD=my\_password;'
    |
    | · All other options are passed without any interpretation directly to the conversion service responsible to handle the specified output file.

    For all options at most three leading characters are significant:
    DATAFILE=<...>, DATABASE=<...> or simply DATA=<...> would lead to
    the same result. Additional options are availible when accessing
    events using an event tag collection.

Writing out the Event Collection
`````````````````````````````````````````

    The event collection is written out by an EvtCollectionStream, which
    is the last member of the event collection Sequencer. :numref:`lstg-joboptwritevtcoll` (which is taken from the job
    options of EvtCollection example), shows how to set up such a
    sequence consisting of a user written Selector algorithm (which
    could for example contain the code in :numref:`lstg-filladdntupexec`), and of the EvtCollectionStream.

    .. was 10.8

    .. code-block:: cpp
        :name: lstg-joboptwritevtcoll
        :caption: Job options for writing out an event collection

        ApplicationMgr.OutStream = { "Sequencer/EvtCollection" };
        ApplicationMgr.ExtSvc += { "TagCollectionSvc/EvtTupleSvc" };
        EvtCollection.Members = { "EvtCollectionWrite/Selector", "EvtCollectionStream/Writer"};
        Writer.ItemList = { "/NTUPLES/EvtColl/Collection" };
        Writer.EvtDataSvc = "EvtTupleSvc";
        EvtTupleSvc.Output = { "EvtColl DATAFILE='MyEvtCollection.root' OPT='NEW' TYP='ROOT'" };

.. _ntup-rcol:

Reading Events using Event Collections
```````````````````````````````````````````````

    Reading event collections as the input for further event processing
    in Gaudi is transparent. The main change is the specification of the
    input data to the event selector:

    .. was 10.9

    .. code-block:: cpp
        :name: lstg-connaddtup
        :caption: Connecting an address tag to an N-tuple

        EventSelector.Input = {                                                                                                                       "COLLECTION='Collection' ADDRESS='Address' DATAFILE='MyEvtCollection.root' TYP='ROOT' SEL='(Ntrack>80)' FUN='EvtCollectionSelector'"
        };

    The tags that were not already introduced earlier are:

    | · COLLECTION
    |
    |  Specifies the sub-path of the N-tuple used to write the collection. If the N-tuple which was written was called e.g. "/NTUPLES/FILE1/Collection", the value of this tag must be "Collection".
    |
    | · ADDRESS (optional)
    |
    |  Specifies the name of the N-tuple tag which was used to store the opaque address to be used to retrieve the event data later. This is an optional tag, the default value is "Address". Please use this default value when writing, conventions are useful!
    |
    | · SELECTION (optional):
    |
    |  Specifies the selection string used for the first stage pre-selection. The syntax depends on the database implementation; it can be:
    |
    | · SQL like, if the event collection was written using ODBC. Example: (NTrack>200 AND Energy>200)
    |
    | · C++ like, if the event collection was written using ROOT. Example: (NTrack>200 && Energy>200).
    |
    | Note that event collections written with ROOT also accept the SQL operators 'AND' instead of '&&' as well as 'OR' instead of '\|\|'. Other SQL operators are not supported.
    |
    | · FUNCTION (optional)
    |
    |  Specifies the name of a function object used for the second-stage preselection. An example of a such a function object is shown in :numref:`lstg-exfunobpresel`. Note that the factory declaration on line 16 is mandatory in order to allow Gaudi to instantiate the function object.
    |
    | · The DATAFILE and TYP tags, as well as additional optional tags, have the same meaning and syntax as for N-tuples, as described in section :numref:`ntup-spec`.

    .. was 10.10

    .. code-block:: cpp
        :name: lstg-exfunobpresel
        :caption: Example of a function object for second stage pre-selections

        class EvtCollectionSelector : public NTuple::Selector {

            NTuple::Item<long> m_ntrack;

        public:

            EvtCollectionSelector(IInterface* svc) : NTuple::Selector(svc) { }
            virtual ~EvtCollectionSelector() { }
            /// Initialization
            virtual StatusCode initialize(NTuple::Tuple* nt) {
                return nt->item("Ntrack", m_ntrack);
            }

            /// Specialized callback for NTuples
            virtual bool operator()(NTuple::Tuple* nt) {
                return m_ntrack>cut;
            }
        };

        ObjectFactory<EvtCollectionSelector> EvtCollectionSelectorFactory


Interactive Analysis using Event Tag Collections
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Event tag collections are very similar to N-tuples and hence allow
    within limits some interactive analysis.

ROOT
`````````````

    This data format is used by the interactive ROOT program. Using the
    helper library TBlob located in the package GaudiRootDb it is
    possible to interactively analyse the N-tuples written in ROOT
    format. However, access is only possible to scalar items (int,
    float, ...) not to arrays.

    Analysis is possible through directly plotting variables:

    .. code-block:: cpp

        root [1] gSystem->Load("D:/mycmt/GaudiRootDb/v3/Win32Debug/TBlob");
        root [2] TFile* f = new TFile("tuple.root");
        root [3] TTree* t = (TTree*)f->Get("<local>_MC_ROW_WISE_2");
        root [4] t->Draw("pz");

    or using a ROOT macro interpreted by ROOT's C/C++ interpreter (see
    for example the code fragment interactive.C shown in `Listing
    10.11 <GDG_Ntuple.html#1080947>`__):

    .. code-block:: cpp

        root [0] gSystem->Load("D:/mycmt/GaudiRootDb/v3/Win32Debug/TBlob");
        root [1] .L ./v8/NTuples/interactive.C
        root [2] interactive("./v8/NTuples/tuple.root");

    More detailed explanations can be found in the ROOT tutorials
    (http://root.cern.ch).

    .. was 10.11

    .. code-block:: cpp
        :name: lstg-rootntupana
        :caption: Interactive analysis of ROOT N-tuples: interactive.C

        void interactive(const char* fname) {
            TFile *input = new TFile(fname);
            TTree *tree = (TTree*)input->Get("<local>_MC_ROW_WISE_2");
            if ( 0 == tree ) {
                printf("Cannot find the requested tree in the root file!\n");
                return;
            }
            Int_t ID, OBJSIZE, NUMLINK, NUMSYMB;
            TBlob *BUFFER = 0;
            Float_t px, py, pz;
            tree->SetBranchAddress("ID",&ID);
            tree->SetBranchAddress("OBJSIZE",&OBJSIZE);
            tree->SetBranchAddress("NUMLINK",&NUMLINK);
            tree->SetBranchAddress("NUMSYMB",&NUMSYMB);
            tree->SetBranchAddress("BUFFER", &BUFFER);
            tree->SetBranchAddress("px",&px);
            tree->SetBranchAddress("py",&py);
            tree->SetBranchAddress("pz",&pz);
            Int_t nbytes = 0;
            for (Int_t i = 0, nentries = tree->GetEntries(); i<nentries;i++) {
                nbytes += tree->GetEntry(i);
                printf("Trk#=%d PX=%f PY=%f PZ=%f\n",i,px,py,pz);
                }
            printf("I have read a total of %d Bytes.\n", nbytes);
            delete input;
        }

Known Problems
-------------------

    Nothing is perfect and there are always things to be sorted out....

    | · When building the GaudiRootDb package on Linux using CMT you must first set up the ROOT environment, by sourcing the setup.csh file
