.. include:: ./global.rst

|newpage|

|Gaudi logo|

.. _chapConv:

Converters
=============

Overview
-------------

    Consider a small piece of detector; a silicon wafer for example.
    This "object" will appear in many contexts: it may be drawn in an
    event display, it may be traversed by particles in a Geant4
    simulation, its position and orientation may be stored in a
    database, the layout of its strips may be queried in an analysis
    program, etc. All of these uses or views of the silicon wafer will
    require code.

    One of the key issues in the design of the framework was how to
    encompass the need for these different views within Gaudi. In this
    chapter we outline the design adopted for the framework and look at
    how the conversion process works. This is followed by sections which
    deal with the technicalities of writing converters for reading from
    and writing to ROOT files.

Persistency converters
---------------------------

    Gaudi gives the possibility to read event data from, and to write
    data back to, ROOT files. The use of ODBC compliant databases is
    also possible, though this is not yet part of the Gaudi release.
    Other persistency technologies have been implemented for LHCb, in
    particular the reading of data from LHCb DSTs based on ZEBRA.

    :numref:`fig-conv` is a schematic
    illustrating how converters fit into the transient-persistent
    translation of event data. We will not discuss in detail how the
    transient data store (e.g. the event data service) or the
    persistency service work, but simply look at the flow of data in
    order to understand how converters are used.

    .. figure:: images/EventConverters.png
        :name: fig-conv

        Persistency conversion services in Gaudi

    An introduction to the persistency mechanism of Gaudi can be found
    in reference [LHCB-TALK-2000-003]_.

    One of the issues considered when designing the Gaudi framework was
    the capability for users to "create their own data types and save
    objects of those types along with references to already existing
    objects". A related issue was the possibility of having links
    between objects which reside in different stores (i.e. files and
    databases) and even between objects in different types of store.

    :numref:`fig-conv` shows that data may be
    read from an ODBC database and/or ROOT files into the transient
    event data store and that data may be written out again to the same
    media. It is the job of the persistency service to orchestrate this
    transfer of data between memory and disk.

    The figure shows two "slave" services: the ODBC conversion service
    and the ROOT I/O service. These services are responsible for
    managing the conversion of objects between their transient and
    persistent representations. Each one has a number of converter
    objects which are actually responsible for the conversion itself. As
    illustrated by the figure a particular converter object converts
    between the transient representation and one other form, here either
    MS Access or ROOT.

Collaborators in the conversion process
--------------------------------------------

    In general the conversion process occurs between the transient
    representation of an object and some other representation. In this
    chapter we will be using persistent forms, but it should be borne in
    mind that this could be any other "transient" form such as those
    required for visualisation or those which serve as input into other
    packages (e.g. Geant4).

    :numref:`fig-convclassdiag` shows the interfaces
    (classes whose name begins with "I") which must be implemented in
    order for the conversion process to function

    .. figure:: images/ConversionClassDiag.png
        :name: fig-convclassdiag

        The classes (and interfaces) collaborating in the conversion process

    The conversion process is essentially a collaboration between the
    following types:

    | · IConversionSvc
    | · IConverter
    | · IOpaqueAddress

    For each persistent technology, or "non-transient" representation, a
    specific conversion service is required. This is illustrated in the
    figure by the class AConversionSvc which implements the
    IConversionSvc interface.

    A given conversion service will have at its disposal a set of
    converters. These converters are both type and technology specific.
    In other words a converter knows how to convert a single transient
    type (e.g. MuonHit) into a single persistent type (e.g. RootMuonHit)
    and vice versa. Specific converters implement the IConverter
    interface, possibly by extending an existing converter base class.

    A third collaborator in this process are the opaque address objects.
    A concrete opaque address class must implement the IOpaqueAddress
    interface. This interface allows the address to be passed around
    between the transient data service, the persistency service, and the
    conversion services without any of them being able to actually
    decode the address. Opaque address objects are also technology
    specific. The internals of an OdbcAddress object are different from
    those of a RootAddress object.

    Only the converters themselves know how to decode an opaque address.
    In other words only converters are permitted to invoke those methods
    of an opaque address object which do not form a part of the
    IOpaqueAddress interface.

    Converter objects must be "registered" with the conversion service
    in order to be usable. For the "standard" converters this will be
    done automatically. For user defined converters (for user defined
    types) this registration must be done at initialisation time (see
    :numref:`data-pers`.

The conversion process
---------------------------

    As an example (see :numref:`fig-convtrace`) we
    consider a request from the event data service to the persistency
    service for an object to be loaded from a data file.

    .. figure:: images/ConversionTrace1.png
        :name: fig-convtrace

        A trace of the creation of a new transient object

    As we saw previously, the persistency service has one conversion
    service slave for each persistent technology in use. The persistency
    service receives the request in the form of an opaque address
    object. The svcType() method of the IOpaqueAddress interface is
    invoked to decide which conversion service the request should be
    passed onto. This returns a "technology identifier" which allows the
    persistency service to choose a conversion service.

    The request to load an object (or objects) is then passed onto a
    specific conversion service. This service then invokes another
    method of the IOpaqueAddress interface, clID(), in order to decide
    which converter will actually perform the conversion. The opaque
    address is then passed onto the concrete converter who knows how to
    decode it and create the appropriate transient object.

    The converter is specific to a specific type, thus it may
    immediately create an object of that type with the new operator. The
    converter must now "unpack" the opaque address, i.e. make use of
    accessor methods specific to the address type in order to get the
    necessary information from the persistent store.

    For example, a ZEBRA converter might get the name of a bank from the
    address and use that to locate the required information in the ZEBRA
    common block. On the other hand a ROOT converter may extract a file
    name, the names of a ROOT TTree and an index from the address and
    use these to load an object from a ROOT file. The converter would
    then use the accessor methods of this "persistent" object in order
    to extract the information necessary to build the transient object.

    We can see that the detailed steps performed within a converter
    depend very much on the nature of the non-transient data and (to a
    lesser extent) on the type of the object being built.

    If all transient objects were independent, i.e. if there were no
    references between objects then the job would be finished. However
    in general objects in the transient store do contain references to
    other objects.

    These references can be of two kinds:

    | i. "Macroscopic" references appear as separate "leaves" in the data store. They have to be registered with a separate opaque address structure in the data directory of the object being converted. This must be done after the object was registered in the data store in the method fillObjRefs().
    |
    | ii. Internal references must be handled differently. There are two possibilities for resolving internal references:
    |
    |  1. Load on demand. If the object the reference points to should only be loaded when accessed, the pointer must no longer be a raw C++ pointer, but rather a smart pointer object containing itself the information for later resolution of the reference. This is the preferred solution for references to objects within the same data store (e.g. references from Monte-Carlo tracks to Monte-Carlo vertices) and is generated by th Object Description Tools when a relation tag is found in the XML class description (see :numref:`data-sref`).
    |
    |  2. Filling of raw C++ pointers. This is only necessary if the object points to an object in another store, e.g. the detector data store, and should be avoided in classes foreseen to be made persistent. To resolve the reference a converter has to retrieve the other object and set the raw pointer. These references should be set in the fillObjRefs() method. This of course is more complicated, because it must be ensured that both objects are present at the time the reference is accessed (i.e. when the pointer is actually used).

Converter implementation - general considerations
------------------------------------------------------

    After covering the ground work in the preceding sections, let us
    look exactly what needs to be implemented in a specific converter
    class. The starting point is the Converter base class from which a
    user converter should be derived.

    .. was 13.1

    .. code-block:: cpp
        :name: lstg-exconfclass
        :caption: An example converter class

        // Converter for class UDO.
        extern const CLID& CLID_UDO;
        extern unsigned char OBJY_StorageType;
        static CnvFactory<UDOCnv> s_factory;
        const ICnvFactory& UDOCnvFactory = s_factory;
        class UDOCnv : public Converter {

        public:

            UDOCnv(ISvcLocator* svcLoc) : Converter( Objectivity_StorageType,
                                                     CLID_UDO,
                                                     svcLoc) { }

            createRep(DataObject* pO, IOpaqueAddress*& a); // transient->persistent
            createObj(IOpaqueAddress* pa, DataObject*& pO); // persistent->transient
            fillObjRefs( ... ); // transient->persistent
            fillRepRefs( ... ); // persistent->transient
        }

    The converter shown in :numref:`lstg-exconfclass` is responsible for the
    conversion of UDO type objects into objects that may be stored into
    an Objectivity database and vice-versa. The UDOCnv constructor calls
    the Converter base class constructor with arguments which contain
    this information. These are the values CLID\_UDO, defined in the UDO
    class, and Objectivity\_StorageType which is also defined elsewhere.
    The first two extern statements simply state that these two
    identifiers are defined elsewhere.

    All of the "book-keeping" can now be done by the Converter base
    class. It only remains to fill in the guts of the converter. If
    objects of type UDO have no links to other objects, then it suffices
    to implement the methods createRep() for conversion from the
    transient form (to Objectivity in this case) and createObj() for the
    conversion to the transient form.

    If the object contains links to other objects then it is also
    necessary to implement the methods fillRepRefs() and fillObjRefs().

Storing Data using the ROOT I/O Engine
-------------------------------------------

    One possibility for storing data is to use the ROOT I/O engine to
    write ROOT files. Although ROOT by itself is not an object oriented
    database, with modest effort a structure can be built on top to
    allow the Converters to emulate this behaviour. In particular, the
    issue of object linking had to be solved in order to resolve
    pointers in the transient world.

    The concept of ROOT supporting paged tuples called trees and
    branches is adequate for storing bulk event data. Trees split into
    one or several branches containing individual leaves with data.

    The data structure within the Gaudi data store is also tree like. In
    the transient world Gaudi objects are sub-class instances of the
    "DataObject". The DataObject offers some basic functionality like
    the implicit data directory which allows e.g. to browse a data
    store. This tree structure will be mapped to a flat structure in the
    ROOT file resulting in a separate tree representing each leaf of the
    data store. Each data tree contains a single branch containing
    objects of the same type. The Gaudi tree is split up into individual
    ROOT trees in order to give easy access to individual items
    represented in the transient model without the need of loading
    complete events from the root file i.e. to allow for selective data
    retrieval. The feature of ROOT supporting selective data reading
    using split trees did not seem too attractive since, generally,
    complete nodes in the transient store should be made available in
    one go.

    However, ROOT expects "ROOT" objects, they must inherit from
    TObject. Therefore the objects from the transient store have to be
    converted to objects understandable by ROOT.

    The following sections are an introduction to the machinery provided
    by the Gaudi framework to achieve the migration of transient objects
    to persistent objects. The ROOT specific aspects are not discussed
    here; the ROOT I/O engine is documetned on the ROOT web site
    http://root.cern.ch). Note that Gaudi only uses the I/O engine, not
    all ROOT classes are available. Within Gaudi the ROOT I/O engine is
    implemented in the GaudiRootDb package.

The Conversion from Transient Objects to ROOT Objects
----------------------------------------------------------

    As for any conversion of data from one representation to another
    within the Gaudi framework, conversion to/from ROOT objects is based
    on Converters. The support of a "generic" Converter accesses
    pre-defined entry points in each object. The transient object
    converts itself to an abstract byte stream. However, for specialized
    objects specific converters must be built.

    Whenever objects must change their representation within Gaudi, data
    converters are involved. For the ROOT case, the converters must have
    some knowledge of ROOT internals and of the service finally used to
    migrate ROOT objects (->TObject) to a file. They must be able to
    translate the functionality of the DataObject component to/from the
    ROOT storage. Within ROOT itself the object is stored as a Binary
    Large Object (BLOB).

    The generic data conversion mechanism relies on two functionalities,
    which must be present:

    | . When writing or reading objects, the object's data must be "serializable". The corresponding persistent type is of a generic type, the data are stored as a machine independent byte stream. This method is implemented automatically if the class is described using the Gaudi Object Description tools (described in :numref:`data-defi`).
    | . When reading objects, an empty object must be created before any de-serialization can take place. The constructor must be called. This functionality does not imply any knowledge of the conversion mechanism itself and hence can be encapsulated into an object factory simply returning a DataObject. These data object factories are distinguished within Gaudi through the persistent data type information, the class ID. For this reason the class ID of objects, which are written must only depend on the object type, i.e. every class needs it's own class ID. The instantiation of the appropriate factory is done by a macro. Please see the RootIO example for details how to instantiate the factory.

Storing Data using other I/O Engines
-----------------------------------------

    Once objects are stored as BLOBs, it is possible to adopt any
    storage technology supporting this datatype. This is the case not
    only for ROOT, but also for

    | · Objectivity/DB
    | · most relational databases, which support an ODBC interface like
    |  · Microsoft Access,
    |  · Microsoft SQL Server,
    |  · MySQL,
    |  · ORACLE and others.

    Note that although storing objects using these technologies is
    possible, there is currently no implementation available in the
    Gaudi release. If you desperately want to use Objectivity or one of
    the ODBC databases, please contact Markus Frank
    (Markus.Frank@cern.ch).
